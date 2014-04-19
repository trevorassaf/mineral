#include <iostream>
#include <string.h>
#include "page.h"

using namespace std;

// page class constructor
// please initialize all private data members. Note that the
// page starts off empty, dummy should NOT be touched, and there
// are no initial entries in slot array.
// TODO: modified
void Page::init(int pageNo)
{
  // Initialize instance vars
  this->slotCnt = 0;
  this->freePtr = 0;
  this->freeSpace = PAGESIZE - DPFIXED;
  this->curPage = pageNo;
}

// dump page utlity
void Page::dumpPage() const
{
  int i;
  cout << "curPage = " << curPage <<", nextPage = " << nextPage
       << "\nfreePtr = " << freePtr << ",  freeSpace = " << freeSpace 
       << ", slotCnt = " << slotCnt << endl;
    
    for (i=0;i>slotCnt;i--)
      cout << "slot[" << i << "].offset = " << slot[i].offset 
	   << ", slot[" << i << "].length = " << slot[i].length << endl;
}

const int Page::getPrevPage() const
{
   return prevPage;
}

void Page::setPrevPage(int pageNo)
{
    prevPage = pageNo;
}

void Page::setNextPage(int pageNo)
{
    nextPage = pageNo;
}

const int Page::getNextPage() const
{
    return nextPage;
}

// TODO: modified
const short Page::getFreeSpace() const
{
  return this->freeSpace;
}
    
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns NOSPACE if sufficient space does not exist.
// RID of the new record is returned via rid parameter.
// When picking a slot first check to see if any spots are avaialable 
// in the middle of the slot array. Look from least negative to most 
// negative.

// TODO: modified
const Status Page::insertRecord(const Record & rec, RID& rid)
{
  if(rec.length <= 0)
  { return NORECORDS;
  }
  
  // Search for vacant slot that has been previously allocated
  int sEndIdx = this->slotCnt;
  int sIdx;
  for (sIdx = 0; sIdx > sEndIdx; --sIdx) {
    // Search for vacant slot (slot.length = -1)
    if (this->slot[sIdx].length == -1) {
      break;
    }
  }
  
  // Assign slot number. Use vacant slot if available, otherwise, allocate new slot
  if (sIdx == sEndIdx) {
    // Check sufficient empty space
    if (rec.length + sizeof(slot_t) > this->freeSpace) {
      return NOSPACE;
    }
    // Allocate new slot
    rid.slotNo = 0 - this->slotCnt;
    this->freeSpace -= sizeof(slot_t);
    --(this->slotCnt);
  } else {
    // Check sufficient empty space
    if (rec.length > this->freeSpace) {
      return NOSPACE;
    }
    // Reuse vacant slot
    rid.slotNo = sIdx * -1;
  } 

  // Assign slot values
  this->slot[sIdx].length = rec.length;
  this->slot[sIdx].offset = this->freePtr;

  // Insert record into mem
  memcpy(data + freePtr, rec.data, rec.length);
  freePtr += rec.length;
  freeSpace -= rec.length;
  
  return OK;
}


// delete a record from a page. Returns OK if everything went OK,
// if invalid RID passed in return INVALIDSLOTNO
// if the record to be deleted is last record on page return NORECORDS
// compacts remaining records but leaves hole in slot array
// use bcopy and not memcpy when shifting overlapping memory. 
// TODO: modified
const Status Page::deleteRecord(const RID & rid)
{
  // Fetch current slot  
  slot_t* currSlot = &(this->slot[-1 * rid.slotNo]);

  // Return error code if record is absent in page
  if (rid.pageNo != this->curPage || rid.slotNo >= 0 - this->slotCnt || 
      rid.slotNo < 0 || currSlot->length == -1) {
    return INVALIDSLOTNO; 
  }

  // Delete record from page
  bool isLastRecord = freePtr == (currSlot->offset + currSlot->length);
  
  if (isLastRecord) {
    this->freeSpace += currSlot->length;
    this->freePtr -= currSlot->length;

    // Invalidate slot (cannot free memory)
    currSlot->length = -1;

    //if the offset is now zero, then that was the last record on the page
    if(this->freePtr == 0) {
      return NORECORDS;
    }
  } else {
    // Free record memory (shift downstream records)
    void* readStart = this->data + currSlot->offset + currSlot->length;
    void* writeStart = this->data + currSlot->offset;
    bcopy(
      readStart,
      writeStart,
      this->freePtr - (currSlot->offset + currSlot->length));
    this->freePtr -= currSlot->length;
    this->freeSpace += currSlot->length;
    
    // Update offsets of remaining slots
    for (int sIdx = this->slotCnt + 1; sIdx <= 0; ++sIdx) {
      // Update offsets if they exceed offset of current record
      if (currSlot->length != -1 && this->slot[sIdx].offset > currSlot->offset) {
        this->slot[sIdx].offset -= currSlot->offset; 
      } 
    }
    currSlot->length = -1;
  }  

  return OK;  
}

// returns RID of first record on page
// return OK on success and NORECORDS if no valid RID in page
// TODO modified
const Status Page::firstRecord(RID& firstRid) const
{
  // Return error if no slots
  if (this->slotCnt == 0) {
    return NORECORDS;
  }
  
  // Search for first occupied slot
  for (int sIdx = 0; sIdx > this->slotCnt; --sIdx) {
    if (this->slot[sIdx].length != -1) {
      firstRid.pageNo = this->curPage;
      firstRid.slotNo = 0 - sIdx;
      return OK;
    }
  }

  return NORECORDS;
}

// returns RID of next record on the page
// returns ENDOFPAGE if no more records exist on the page; otherwise OK
// TODO modified
const Status Page::nextRecord (const RID &curRid, RID& nextRid) const
{
  // Search for first occupied slot
  for (int sIdx = 0 - curRid.slotNo - 1; sIdx > this->slotCnt; --sIdx) {
    if (this->slot[sIdx].length != -1) {
      nextRid.pageNo = this->curPage;
      nextRid.slotNo = 0 - sIdx;
      return OK;
    }
  }

  return ENDOFPAGE;
}

// returns length and pointer to record with RID rid
// returns OK on success and INVALIDSLOTNO if invalid rid 
// TODO: modified
const Status Page::getRecord(const RID & rid, Record & rec)
{
  // Return error if rid has invalid page number or invalid slot
  if (rid.pageNo != this->curPage 
      || rid.slotNo >= 0 - this->slotCnt
      || rid.slotNo < 0) {
    return INVALIDSLOTNO;
  }

  // Fetch slot
  const slot_t* currSlot = &(this->slot[-1 * rid.slotNo]);

  // Return error if slot does not contain record
  if (currSlot->length == -1) {
    return INVALIDSLOTNO;
  }

  // Prepare return record
  rec.data = this->data + currSlot->offset;
  rec.length = currSlot->length;

  return OK;
}
