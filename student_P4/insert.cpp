#include "catalog.h"
#include "query.h"
#include "index.h"
#include <cassert>
#include <vector>
#include <cstring>
#include "utility.h"

/*
 * Inserts a record into the specified relation
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

Status Updates::Insert(const string& relation,      // Name of the relation
                       const int attrCnt,           // Number of attributes specified in INSERT statement
                       const attrInfo attrList[])   // Value of attributes specified in INSERT statement
{
    // Insert record into heap-file
    // Create record from insert data
    // Fetch attribute meta-data (stored in rd)
    int relAttrCnt;
    AttrDesc* attrs;
    Status status = attrCat->getRelInfo(relation, relAttrCnt, attrs);
    // Handle errors on schema fetch
    if (status != OK) {
      return status;
    }
    
    // TODO: Ensure number of inserted attributes equals number of attributes in record
    assert(relAttrCnt == attrCnt);

    // Construct new record from attribute data
    // Calculate number of bytes in record
    int rBytes = 0;
    for (int i = 0; i < relAttrCnt; ++i) {
      rBytes += attrs->attrLen;
      ++attrs;
    }
    attrs -= relAttrCnt;

    // Initialize new record
    char* rData = new char[rBytes];
    
    // Initialize index vector
    vector<int> v1;

    for (int i = 0; i < relAttrCnt; ++i) {
      bool check = false;
      for (int j = 0; j < attrCnt; ++j) {
        if (!strcmp(attrs->attrName, attrList[j].attrName)) {
          // Copy inserted data into record 
          memcpy(rData + attrs->attrOffset, attrList[j].attrValue, attrs->attrLen);
          check = true;
          if(attrs->indexed) {
            v1.push_back(i);
          }
          break;
        }
        //TODO: Figure out how to handle this
        assert(check)
      }
      ++attrs;
    }
    attrs -= relAttrCnt;

    // Create record with rData
    Record record = {rData, rBytes};

    // Obtain heapfile
    HeapFile hf(relation, status);
    if (status != OK) {
      return status;
    }
    
    // Insert record into heapfile
    RID rid;
    status = hf.insertRecord(record, rid);
    if (status != OK) {
      return status;
    }

    // Identify indexes on this relation
    for (int i = 0; i < v1.size(); ++i) {
      // Obtain index associated with this attribute
      Index index(
        relation,
        attrs[v1[i]].attrOffset,
        attrs[v1[i]].attrLen,
        (Datatype)attrs[v1[i]].attrType,
        0,
        status
      );
      if (status != OK) {
        return status;
      }

      // Insert entry into index
      char* ikValue = new char[attrs[v1[i]].attrLen];
      memcpy(ikValue, rData + attrs[v1[i]].attrOffset, attrs[v1[i]].attrLen);
      index.insertEntry(ikValue, rid);
    }
    return OK;
}
