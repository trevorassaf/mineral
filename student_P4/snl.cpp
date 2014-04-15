#include "catalog.h"
#include "query.h"
#include "sort.h"
#include "index.h"
#include <cstring>
#include <string>
#include <iostream>

Status Operators::SNL(const string& result,           // Output relation name
                      const int projCnt,              // Number of attributes in the projection
                      const AttrDesc attrDescArray[], // Projection list (as AttrDesc)
                      const AttrDesc& attrDesc1,      // The left attribute in the join predicate
                      const Operator op,              // Predicate operator
                      const AttrDesc& attrDesc2,      // The left attribute in the join predicate
                      const int reclen)               // The length of a tuple in the result relation
{
  cout << "Algorithm: Simple NL Join" << endl;

  // Obtain relation names
  string relation1 = attrDesc1.relName;
  string relation2 = attrDesc2.relName;
  Status status;
  
  // Initialize input heap file scans
  HeapFileScan hfs1(relation1, status);
  if (status != OK) {
    return status;
  }

  // Initialize result heap file
  HeapFile outputHfs(result, status);
  if (status != OK) {
    return status;
  }

  //TODO: Decide if order matters  
  // Perform heap file scans
  int numRecs1 = hfs1.getRecCnt();
  
  for (int i = 0; i < numRecs1; ++i) {
    RID rid1;
    Record record1;
    status = hfs1.scanNext(rid1, record1);
    
    //If file has ended, then finish join
    if (status == FILEEOF) {
     break;
    }
    else if (status != OK) {
      return status;
    }

    //Begin inner heap scan for each tuple in outer heap file
    HeapFileScan hfs2(relation2, status);
    if (status != OK) {
      return status;
    }
    int numRecs2 = hfs2.getRecCnt();
    
    for (int j = 0; j < numRecs2; ++j) {
      RID rid2;
      Record record2;
      status = hfs2.scanNext(rid2, record2);
      
      //If inner heap file has ended, then reset the scan and break
      if (status == FILEEOF) {
        hfs2.endScan();
        break;
      }
      else if (status != OK) {
        return status;
      }

      // Join check
      int comparison = matchRec(
        record1,
        record2,
        attrDesc1,
        attrDesc2);
      if ((op == LT && comparison < 0) ||
          (op == LTE && comparison <= 0) ||
          (op == EQ && comparison == 0) ||
          (op == GT && comparison > 0) ||
          (op == GTE && comparison >= 0) ||
          (op == NE && comparison != 0)) {
        
        // Records satisfy join criterion, perform join
        int offset = 0;
        char* rData = new char[reclen];
        for (int adaIdx = 0; adaIdx < projCnt; ++adaIdx) {
          // Select record associated with this particular projection attribute
          //  differentiate based on relation name
          Record* inputRecord = strcmp(attrDescArray[adaIdx].relName, relation1.c_str())
            ? &record2
            : &record1;
          memcpy(rData + offset, (char*)inputRecord->data + attrDescArray[adaIdx].attrOffset, attrDescArray[adaIdx].attrLen);
          offset += attrDescArray[adaIdx].attrLen;
        }
        Record newRecord = {rData, reclen};
        RID newRid;
        outputHfs.insertRecord(newRecord, newRid);
      } 
    }
  }

  return OK;
}

