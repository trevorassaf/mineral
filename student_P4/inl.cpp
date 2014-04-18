#include "catalog.h"
#include "query.h"
#include "sort.h"
#include "index.h"
#include <cstring>
/* 
 * Indexed nested loop evaluates joins with an index on the 
 * inner/right relation (attrDesc2)
 */

Status Operators::INL(const string& result,           // Name of the output relation
                      const int projCnt,              // Number of attributes in the projection
                      const AttrDesc attrDescArray[], // The projection list (as AttrDesc)
                      const AttrDesc& attrDesc1,      // The left attribute in the join predicate
                      const Operator op,              // Predicate operator
                      const AttrDesc& attrDesc2,      // The left attribute in the join predicate
                      const int reclen)               // Length of a tuple in the output relation
{
  cout << "Algorithm: Indexed NL Join" << endl;

  // Relation names
  string relation1 = attrDesc1.relName;
  string relation2 = attrDesc2.relName;

  // Initialize heap file scans
  Status status;
  HeapFileScan hfs1(relation1, status);
  if (status != OK) {
    return status;
  }
  HeapFileScan hfs2(relation2, status);
  if (status != OK) {
    return status;
  }

  // Initialize output heap
  HeapFile outHfs(result, status);
  if (status != OK) {
    return status;
  }

  // Determine indexed attribute
  const AttrDesc* iaDesc, * niaDesc;
  HeapFileScan* niHfs, * iHfs;
  if (attrDesc1.indexed) {
    iaDesc = &attrDesc1;
    niaDesc = &attrDesc2;
    iHfs = &hfs1;
    niHfs = &hfs2;
  } else {
    iaDesc = &attrDesc2;
    niaDesc = &attrDesc1;
    iHfs = &hfs2;
    niHfs = &hfs1;
  }

  // Initialize index
  Index index(
    iaDesc->relName,
    iaDesc->attrOffset,
    iaDesc->attrLen,
    (Datatype)iaDesc->attrType,
    0,
    status
  );
  if (status != OK) {
    return status;
  }

  // Initialize scan. 
  int nihRecCnt = niHfs->getRecCnt();
  for (int nihIdx = 0; nihIdx < nihRecCnt; ++nihIdx) {
    RID niRid;
    Record niRecord;
    status = niHfs->scanNext(niRid, niRecord);
    if (status == FILEEOF) {
      break;
    }
    if (status != OK) {
      return status;
    }

    // Search through index to find record to join on 
    index.startScan((char*)niRecord.data + niaDesc->attrOffset);
    int ihRecCnt = iHfs->getRecCnt();
    for (int iIdx = 0; iIdx < ihRecCnt; ++iIdx) {
      RID iRid;
      status = index.scanNext(iRid);
      if (status == NOMORERECS) {
        break;
      }
      if (status != OK) {
        return status;
      }

      // Fetch join record from heap file
      Record iRecord;
      status = iHfs->getRandomRecord(iRid, iRecord);
      if (status != OK) {
        return status;
      }

      // Join attributes in new memory block
      char* newRecordMem = new char[reclen];
      for (int pIdx = 0; pIdx < projCnt; ++pIdx) {
        Record* inputRecord = strcmp(
            attrDescArray[pIdx].relName,
            iaDesc->relName)
          ? &niRecord
          : &iRecord;
        memcpy(
          newRecordMem,
          (char*)inputRecord->data + attrDescArray[pIdx].attrOffset,
          attrDescArray[pIdx].attrLen);
        newRecordMem += attrDescArray[pIdx].attrLen;
      }
      newRecordMem -= reclen;

      // Insert join record into results relation
      RID outRid;
      Record outRec = {newRecordMem, reclen};
      status = outHfs.insertRecord(outRec, outRid);
      if (status != OK) {
        return status;
      }
    }
  }

  return OK;
}

