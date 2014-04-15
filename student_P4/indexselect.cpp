#include "catalog.h"
#include "query.h"
#include "index.h"
#include <cstring>

Status Operators::IndexSelect(const string& result,       // Name of the output relation
                              const int projCnt,          // Number of attributes in the projection
                              const AttrDesc projNames[], // Projection list (as AttrDesc)
                              const AttrDesc* attrDesc,   // Attribute in the selection predicate
                              const Operator op,          // Predicate operator
                              const void* attrValue,      // Pointer to the literal value in the predicate
                              const int reclen)           // Length of a tuple in the output relation
{
  cout << "Algorithm: Index Select" << endl;
  
  // Initialize index
  string relation = projNames[0].relName;
  Status status;
  Index index(
    relation,
    attrDesc->attrOffset,
    attrDesc->attrLen,
    (Datatype)attrDesc->attrType,
    0,
    status);
  if (status != OK) {
    return status;
  }
  
  // Initizlize relation heap file scan
  HeapFileScan inputHfs(relation, status);
  if (status != OK) {
    return status;
  }
  
  // Initialize results heap file
  HeapFile outputHfs(result, status);
  
  // Scan index for next RIDs
  int numRecs = inputHfs.getRecCnt();
  for (int i = 0; i < numRecs; ++i) {
    // Fetch rid of next qualifying record
    RID rid;
    status = index.scanNext(rid);
    if (status == NOMORERECS) {
      break;
    } else if (status != OK) {
      return status;
    }
    
    // Fetch record from input heap file with rid
    Record inputRecord;
    status = inputHfs.getRecord(rid, inputRecord);
    if(status != OK) {
      return status;
    }
      
    // Project attributes into new record
    char* projData = new char[reclen];
    int offset = 0; 
    for (int j = 0; j < projCnt; ++j) {
      memcpy(
        projData + offset,
        (char*) inputRecord.data + projNames[j].attrOffset,
        projNames[j].attrLen);
      offset += projNames[j].attrLen;
    }
    Record resultsRecord = {projData, reclen};
    
    // Add new record to results heap file
    status = outputHfs.insertRecord(resultsRecord, rid);
    if (status != OK) {
      return status;
    }
  }

  return OK;
}

