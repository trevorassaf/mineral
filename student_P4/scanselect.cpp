#include "catalog.h"
#include "query.h"
#include "index.h"
#include <cstring>

/* 
 * A simple scan select using a heap file scan
 */

Status Operators::ScanSelect(const string& result,       // Name of the output relation
                             const int projCnt,          // Number of attributes in the projection
                             const AttrDesc projNames[], // Projection list (as AttrDesc)
                             const AttrDesc* attrDesc,   // Attribute in the selection predicate
                             const Operator op,          // Predicate operator
                             const void* attrValue,      // Pointer to the literal value in the predicate
                             const int reclen)           // Length of a tuple in the result relation
{
  cout << "Algorithm: File Scan" << endl;
  
  // Initialize heap file scan
  string relation = projNames[0].relName;
  Status status;
  HeapFileScan hfs = attrDesc 
    ? HeapFileScan(
      relation,
      attrDesc->attrOffset,
      attrDesc->attrLen,
      (Datatype)attrDesc->attrType,
      (const char*)attrValue,
      op,
      status) 
    : HeapFileScan(relation, status); 
  if (status != OK) {
    return status;
  }
  
  // Obtain results heap file
  HeapFile outHf(result, status);
  if (status != OK) {
    return status;
  }
    
  // Perform record-wise scan
  int numRecCnt = hfs.getRecCnt();
  for (int i = 0; i < numRecCnt; ++i) {
    // Obtain record satisfying selection criterion
    RID rid;
    Record record;
    status = hfs.scanNext(rid, record);
    if (status == FILEEOF) {
      break;
    } else if (status != OK) {
      return status;
    }
    
    // Project attributes into new record
    char* projData = new char[reclen];
    int offset = 0; 
    for (int j = 0; j < projCnt; ++j) {
      memcpy(
        projData + offset,
        (char *) record.data + projNames[j].attrOffset,
        projNames[j].attrLen);
      offset += projNames[j].attrLen;
    }
    Record resultsRecord = {projData, reclen};
    
    // Add new record to results heap file
    status = outHf.insertRecord(resultsRecord, rid); // we don't care about this rid
    if (status != OK) {
      return status;
    }
  }

  return OK;
}
