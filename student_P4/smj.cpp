#include "catalog.h"
#include "query.h"
#include "sort.h"
#include "index.h"
#include <cstring>

// TODO: remove this function
void printRecord(const Record& r, const AttrDesc& ad) {
  cout << "\n-- RECORD PRINT\n";
  cout << "\t- relname: " << ad.relName << endl;
  cout << "\t- attrName: " << ad.attrName << endl;
  cout << "\t- attrOffset: " << ad.attrOffset << endl;
  cout << "\t- attrLen: " << ad.attrLen << endl;
  cout << "\t- indexed: " << (ad.indexed ? "TRUE" : "FALSE") << endl;
  cout << "\t- attrType: ";
  char* data = (char*)r.data;
  switch (ad.attrType) {
    case 0:
      {
        cout << "INTEGER\n";
        //int value = *(data);
        int value = *(data + ad.attrOffset);
        cout << "\t- value: " << value << endl;
      }
      break;
    case 1:
      {
        cout << "DOUBLE\n";
        double value = *(data + ad.attrOffset);
        cout << "\t- value: " << value << endl;
      }
      break;
    case 2:
      {
      cout << "STRING\n";
      cout << "\t- value: " << data + ad.attrOffset << endl;
      }
      break;
  }
  
}
/* Consider using Operators::matchRec() defined in join.cpp
 * to compare records when joining the relations */
  
Status Operators::SMJ(const string& result,           // Output relation name
                      const int projCnt,              // Number of attributes in the projection
                      const AttrDesc attrDescArray[], // Projection list (as AttrDesc)
                      const AttrDesc& attrDesc1,      // The left attribute in the join predicate
                      const Operator op,              // Predicate operator
                      const AttrDesc& attrDesc2,      // The left attribute in the join predicate
                      const int reclen)               // The length of a tuple in the result relation
{
  cout << "Algorithm: SM Join" << endl;

  // Relation names
  string relation1 = attrDesc1.relName;
  string relation2 = attrDesc2.relName;

  // Determine num-bytes of individual record for each input relation
  // Relation 1
  int attrCnt1, attrCnt2, recLen1, recLen2;
  AttrDesc* attrs1, * attrs2, * end;
  Status status = attrCat->getRelInfo(relation1, attrCnt1, attrs1); 
  if (status != OK) {
    return status;
  } 
  recLen1 = 0;
  end = attrs1 + attrCnt1;
  while (attrs1 != end) {
    recLen1 += attrs1->attrLen;
    ++attrs1;
  }

  // Relation 2
  status = attrCat->getRelInfo(relation2, attrCnt2, attrs2); 
  if (status != OK) {
    return status;
  } 
  recLen2 = 0;
  end = attrs2 + attrCnt2;
  while (attrs2 != end) {
    recLen2 += attrs2->attrLen;
    ++attrs2;
  }
  
  // Calculate max num tuples for each input relation
  int mnr1 = 0.8 * bufMgr->numUnpinnedPages() * PAGESIZE / recLen1;
  int mnr2 = 0.8 * bufMgr->numUnpinnedPages() * PAGESIZE / recLen2;

  // Initialize sorted files
  // Relation 1
  SortedFile sf1(
    relation1,
    attrDesc1.attrOffset,
    attrDesc1.attrLen,
    (Datatype)attrDesc1.attrType,
    mnr1,
    status
  );
  if (status != OK) {
    return status;
  }
  // Relation 2
  SortedFile sf2(
    relation2,
    attrDesc2.attrOffset,
    attrDesc2.attrLen,
    (Datatype)attrDesc2.attrType,
    mnr2,
    status
  );
  if (status != OK) {
    return status;
  }

  // Initialize heap file for output relation
  HeapFile outputHf(result, status);
  if (status != OK) {
    return status;
  }

  Record rec1, rec2;
  
  //Initialize rec2
  status = sf2.next(rec2);
  if(status == FILEEOF)
  { return OK;
  }
  else if (status != OK)
  { return status;
  }
  
  while (1)
  {
    status = sf1.next(rec1);
    if (status == FILEEOF)
    { break;
    }
    else if (status != OK)
    { return status;
    }
    bool fileEnd = false;
    
    //Step through file 1 while rec2's attribute is greater than rec1's
    while (0 > matchRec(rec1, rec2, attrDesc1, attrDesc2))
    {
      status = sf1.next(rec1);
      if(status == FILEEOF)
      { 
        fileEnd = true;
        break;
      }
      else if (status != OK)
      { return status;
      }
    }
    if(fileEnd)
    { break;
    }
    
    //Step through file 2 while rec1's attribute is greater than rec2's
    while (0 < matchRec(rec1, rec2, attrDesc1, attrDesc2))
    {
      status = sf2.next(rec2);
      if(status == FILEEOF)
      { 
        fileEnd = true;
        break;
      }
      else if (status != OK)
      { return status;
      }
    }
    if(fileEnd)
    { break;
    }
    
    //Found a join match, since neither attribute value is greater than the other
    sf2.setMark();
    while ( 0 == matchRec(rec1, rec2, attrDesc1, attrDesc2))
    {
      // Join records 
      // Build join-record data with projected attributes
      char* joinRecData = new char[reclen];
      for (int adIdx = 0; adIdx < projCnt; ++adIdx) {
        // Determine from which to copy
        Record* inputRecord = strcmp(attrDescArray[adIdx].relName, attrDesc1.relName)
          ? &rec1
          : &rec2;
        
        // Copy data from input record to join record
        memcpy(
          joinRecData,
          (char*)inputRecord->data + attrDescArray[adIdx].attrOffset,
          attrDescArray[adIdx].attrLen
        );

        // Advance join record pointer
        joinRecData += attrDescArray[adIdx].attrLen;
      }
      
      // Insert join record into result heap file
      joinRecData -= reclen;
      Record joinRec = {joinRecData, reclen};
      RID outputRid;
      status = outputHf.insertRecord(joinRec, outputRid);
      if (status != OK) {
        return status;
      }
      
      //step through file 2 as long as a match exists
      status = sf2.next(rec2);
      if(status == FILEEOF)
      { break;
      }
      else if (status != OK)
      { return status;
      }
    }
    
    //Go back to the start of the matching record interval in file 2,
    //to check for possible duplicate values in file 1.
    sf2.gotoMark();
  }
  
  return OK;
}

