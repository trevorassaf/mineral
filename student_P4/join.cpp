#include "catalog.h"
#include "query.h"
#include "sort.h"
#include "index.h"
#include <cmath>
#include <cstring>
#include <string>
#include <cassert>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define DOUBLEERROR 1e-07

/*
 * Joins two relations
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

Status Operators::Join(const string& result,           // Name of the output relation 
                       const int projCnt,              // Number of attributes in the projection
    	               const attrInfo projNames[],     // List of projection attributes
    	               const attrInfo* attr1,          // Left attr in the join predicate
    	               const Operator op,              // Predicate operator
    	               const attrInfo* attr2)          // Right attr in the join predicate
{
  // Fetch attribute descriptions
  string relation1 = attr1->relName;
  string relation2 = attr2->relName;
  
  AttrDesc* inputDescs1, * inputDescs2;
  
  int attrCnt1, attrCnt2;
  
  Status status = attrCat->getRelInfo(relation1, attrCnt1, inputDescs1);
  if (status != OK) {
    return status;
  }
  status = attrCat->getRelInfo(relation2, attrCnt2, inputDescs2);
  if (status != OK) {
    return status;
  }
  
  // Construct list of projected attribute descriptions
  AttrDesc* projAttrDescs = new AttrDesc[projCnt];
  int reclen = 0;
  
  //search through the first relation for an attribute match
  for (int i = 0; i < projCnt; ++i) {
    bool found = false;
    int j;
    for (j = 0; j < attrCnt1; ++j) {
      if (!strcmp(projNames[i].attrName, inputDescs1->attrName) && 
          !strcmp(projNames[i].relName, inputDescs1->relName)) {
        *projAttrDescs++ = *inputDescs1;
        reclen += inputDescs1->attrLen;  
        found = true;
        break;
      } 
      inputDescs1++;
    }
    inputDescs1 -= j;
    
    if (found) {
      continue;
    }
    
    //If not found in the first relation, search for the attribute in the second relation
    for (j = 0; j < attrCnt2; ++j) {
      if (!strcmp(projNames[i].attrName, inputDescs2->attrName) && 
          !strcmp(projNames[i].relName, inputDescs2->relName)) {
        *projAttrDescs++ = *inputDescs2;
        reclen += inputDescs2->attrLen;
        found = true;
        break;
      } 
      inputDescs2++;
    }
    inputDescs2 -= j;
    assert(found); 
  }
  projAttrDescs -= projCnt;
  
  // Convert criterion attributes to AttrDesc
  AttrDesc cDesc1, cDesc2;
  status = attrCat->getInfo(relation1, attr1->attrName, cDesc1);
  if (status != OK) {
    return status;
  }
  status = attrCat->getInfo(relation2, attr2->attrName, cDesc2);
  if (status != OK) {
    return status;
  } 
  
  // Delegate sub-join operations
  if (op != EQ) {
    //Perform SNL. Might have to switch relation 1 and relation2 
    //based on which is more efficient as the outer relation
    HeapFileScan hfs1(relation1, status);
    if(status != OK)
    { return status;
    }
    HeapFileScan hfs2(relation2, status);
    if(status != OK)
    { return status;
    }
    
    //Get num tuples for each file
    int numTuples1 = hfs1.getRecCnt();
    int numTuples2 = hfs2.getRecCnt();
    int relSize1 = 0, relSize2 = 0;
    for (int i = 0; i < attrCnt1; ++i) {
     relSize1 += (inputDescs1 + i)->attrLen;
    }
    for (int i = 0; i < attrCnt1; ++i) {
     relSize2 += (inputDescs2 + i)->attrLen;
    }
    
    //Calculate num pages for each file
    int relPages1 = ceil((numTuples1*relSize1)*1.0/PAGESIZE);
    int relPages2 = ceil((numTuples2*relSize2)*1.0/PAGESIZE);
    
    //Use formula to calculate total IOs, to decide in which order to send
    //SNL the two AttrDesc structs (first parameter is outer, second is inner)
    if(relPages1 + numTuples1*relPages2 <= relPages2 + numTuples2*relPages1) {
      return SNL(
        result,
        projCnt,
        projAttrDescs,
        cDesc1,
        op,
        cDesc2,
        reclen);
     } else {
       //Operator needs to be flipped if the order is switched
       Operator op2;
       switch(op)
       {
         case LT: op2 = GT; break;
         case GT: op2 = LT; break;
         case LTE: op2 = GTE; break;
         case GTE: op2 = LTE; break;
         default: break;
       } 
       return SNL(
        result,
        projCnt,
        projAttrDescs,
        cDesc2,
        op2,
        cDesc1,
        reclen);
     }
  }
  
  if (cDesc1.indexed || cDesc2.indexed) {
    return INL(
      result,
      projCnt,
      projAttrDescs,
      cDesc1,
      op,
      cDesc2,
      reclen);  
  }
  
  return SMJ(
      result,
      projCnt,
      projAttrDescs,
      cDesc1,
      op,
      cDesc2,
      reclen);
}

// Function to compare two record based on the predicate. Returns 0 if the two attributes 
// are equal, a negative number if the left (attrDesc1) attribute is less that the right 
// attribute, otherwise this function returns a positive number.
int Operators::matchRec(const Record& outerRec,     // Left record
                        const Record& innerRec,     // Right record
                        const AttrDesc& attrDesc1,  // Left attribute in the predicate
                        const AttrDesc& attrDesc2)  // Right attribute in the predicate
{
    int tmpInt1, tmpInt2;
    double tmpFloat1, tmpFloat2, floatDiff;

    // Compare the attribute values using memcpy to avoid byte alignment issues
    switch(attrDesc1.attrType)
    {
        case INTEGER:
            memcpy(&tmpInt1, (char *) outerRec.data + attrDesc1.attrOffset, sizeof(int));
            memcpy(&tmpInt2, (char *) innerRec.data + attrDesc2.attrOffset, sizeof(int));
            return tmpInt1 - tmpInt2;

        case DOUBLE:
            memcpy(&tmpFloat1, (char *) outerRec.data + attrDesc1.attrOffset, sizeof(double));
            memcpy(&tmpFloat2, (char *) innerRec.data + attrDesc2.attrOffset, sizeof(double));
            floatDiff = tmpFloat1 - tmpFloat2;
            return (fabs(floatDiff) < DOUBLEERROR) ? 0 : (floatDiff < 0 ? floor(floatDiff) : ceil(floatDiff));

        case STRING:
            return strncmp(
                (char *) outerRec.data + attrDesc1.attrOffset, 
                (char *) innerRec.data + attrDesc2.attrOffset, 
                MAX(attrDesc1.attrLen, attrDesc2.attrLen));
    }

    return 0;
}
