#include "catalog.h"
#include "query.h"
#include "index.h"
#include <string>
#include <cassert>
#include <cstring>

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */
Status Operators::Select(const string & result,      // name of the output relation
	                 const int projCnt,          // number of attributes in the projection
		         const attrInfo projNames[], // the list of projection attributes
		         const attrInfo *attr,       // attribute used inthe selection predicate 
		         const Operator op,         // predicate operation
		         const void *attrValue)     // literal value in the predicate
{
  string relation = projNames[0].relName;
  int relAttrCnt;
  AttrDesc* outAttrs;
  Status status = attrCat->getRelInfo(relation, relAttrCnt, outAttrs);
  if(status != OK)
  { return status;
  }

  assert(projCnt);
  int recLen = 0;

  //Order the output attributes in the correct order (according to the selection)
  for(int i = 0; i < projCnt; i++)
  {
    for(int j = 0; j < relAttrCnt; j++)
    {
      if(!strcmp(projNames[i].attrName, (outAttrs+j)->attrName))
      {
        AttrDesc swap = *(outAttrs + i);
        *(outAttrs + i) = *(outAttrs + j);
        *(outAttrs + j) = swap;
        recLen += (outAttrs + i)->attrLen;
      }
    }
  }

  AttrDesc outAttrDesc;
  if(attr)
  { 
    attrCat->getInfo(relation, attr->attrName, outAttrDesc);
    if(op == EQ && outAttrDesc.indexed)
    {
      return IndexSelect(result, projCnt, outAttrs, &outAttrDesc, op, attrValue, recLen);
    }
    else
    {
      return ScanSelect(result, projCnt, outAttrs,&outAttrDesc , op, attrValue, recLen);    
    }
  }
  return ScanSelect(result, projCnt, outAttrs, NULL, op, attrValue, recLen);
  
}

