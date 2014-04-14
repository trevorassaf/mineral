#include <iostream>
#include <string.h>
#include "page.h"

using namespace std;

// page class constructor
// please initialize all private data members. Note that the
// page starts off empty, dummy should NOT be touched, and there
// are no initial entries in slot array.
void Page::init(int pageNo)
{
    /* Solution Here */
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

const short Page::getFreeSpace() const
{
    /* Solution Here */
}
    
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns NOSPACE if sufficient space does not exist.
// RID of the new record is returned via rid parameter.
// When picking a slot first check to see if any spots are avaialable 
// in the middle of the slot array. Look from least negative to most 
// negative.

const Status Page::insertRecord(const Record & rec, RID& rid)
{
    /* Solution Here */
}


// delete a record from a page. Returns OK if everything went OK,
// if invalid RID passed in return INVALIDSLOTNO
// if the record to be deleted is last record on page return NORECORDS
// compacts remaining records but leaves hole in slot array
// use bcopy and not memcpy when shifting overlapping memory. 
const Status Page::deleteRecord(const RID & rid)
{
    /* Solution Here */
}

// returns RID of first record on page
// return OK on success and NORECORDS if no valid RID in page
const Status Page::firstRecord(RID& firstRid) const
{
    /* Solution Here */
}

// returns RID of next record on the page
// returns ENDOFPAGE if no more records exist on the page; otherwise OK
const Status Page::nextRecord (const RID &curRid, RID& nextRid) const
{
    /* Solution Here */
}

// returns length and pointer to record with RID rid
// returns OK on success and INVALIDSLOTNO if invalid rid 
const Status Page::getRecord(const RID & rid, Record & rec)
{
    /* Solution Here */
}
