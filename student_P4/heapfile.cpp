#include "heapfile.h"
#include "error.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

HeapFile::HeapFile(const string & name, Status& returnStatus)
{
   // Solution Starts
    Status 	status;
    Page*	pagePtr;

    // The constructor gets run in two different cases.
    // In the first case, the file is new and the header page
    // must be initialized.  This case is detected via a failure
    // in the db->openfile() call.  In the second case, the
    // file already exists and all that must be done is to fetch
    // the header page into the buffer pool

    // try to open the file
    status = db.openFile(name, file);
    if (status != OK)
    {
	// file doesn't exist. First create it.
	status = db.createFile(name);
	if (status != OK) 
	{
		cerr << "createfile failed\n";
		returnStatus = status;
		return;
	}
	// then open it
        status = db.openFile(name, file);
	if (status != OK) 
	{
		cerr << "openfile failed\n";
		returnStatus = status;
		return;
	}
	// allocate and initialize the header page  
	status = bufMgr->allocPage(file, headerPageNo, pagePtr);
	if (status != OK) 
	{
		cerr << "allocation of header page failed\n";
		returnStatus = status;
		return;
	}
	headerPage = (HeaderPage*) pagePtr;
        strncpy(headerPage->fileName, name.c_str(), MAXNAMESIZE); // copy in file name
	headerPage->firstPage = -1;
	headerPage->lastPage = -1;
	headerPage->pageCnt = 0;
	headerPage->recCnt = 0;
    }
    else
    {
	// file already exists.  get header page into the buffer pool
	status = file->getFirstPage(headerPageNo);
	if (status != OK) 
	{
		cerr << "fetch of first page failed\n";
		returnStatus = status;
	}
	status = bufMgr->readPage(file, headerPageNo, pagePtr);
	if (status != OK) 
	{
		cerr << "read of first page failed\n";
		returnStatus = status;
	}
	headerPage = (HeaderPage*) pagePtr;
    }

    returnStatus = OK;
   // Solution Ends
}

HeapFile::~HeapFile()
{
   // Solution Starts
    Status status;

    status = bufMgr->unPinPage(file, headerPageNo, true);
    if (status != OK) cerr << "error in unpin of header page\n";

    status = db.closeFile(file);
    if (status != OK) cerr << "error in closeFile call\n";
   // Solution Ends
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
   // Solution Starts
  return headerPage->recCnt;
   // Solution Ends
}

// Insert a record into the file
const Status HeapFile::insertRecord(const Record & rec, RID& outRid)
{
   // Solution Starts
    Page* 	lastPage;
    int		lastPageNo;
    Page*	newPage;
    int		newPageNo;
    Status	status, unpinstatus;
    RID		rid;

    // check if file is empty
    if (headerPage->lastPage == -1)
    {
	// file is empty.  need to get a data page
	status = bufMgr->allocPage(file, newPageNo, newPage);
	if (status != OK) return status;
	// initialize the empty page
	newPage->init(newPageNo);

	// set up header page pointers properly
    	headerPage->firstPage = headerPage->lastPage = newPageNo;
	headerPage->pageCnt++;
	// set up forward and backward pointers
	newPage->setNextPage(-1);
	newPage->setPrevPage(-1); 

	// unpin the page so that we can fall cleanly into following case
	status = bufMgr->unPinPage(file, newPageNo, true);
    }
    // try and add the record onto the last page. 
    // start by getting the last page into the buffer pool

    lastPageNo = headerPage->lastPage;
    status = bufMgr->readPage(file, lastPageNo, lastPage);
    if (status != OK) return status;

    // then try to insert the record
    status = lastPage->insertRecord(rec, rid);
    if (status == OK) 
    {
	// insert was successful. unpin page and return
        status = bufMgr->unPinPage(file, lastPageNo, true);
	headerPage->recCnt++;
	outRid = rid;
	return status;
    }
    else
    if (status == NOSPACE)
    {
	// current page was full.  allocate a new page
	status = bufMgr->allocPage(file, newPageNo, newPage);
	if (status != OK) 
	{
	    // unpin the last page
            unpinstatus = bufMgr->unPinPage(file, lastPageNo, true);
	    return status;
	}
	// initialize the empty page
	newPage->init(newPageNo);

	// modify header page contents properly
    	headerPage->lastPage = newPageNo;
	headerPage->pageCnt++;

	// link up new page appropriately
	lastPage->setNextPage(newPageNo);  // set forward pointer
	newPage->setPrevPage(lastPageNo); // backwards pointer
	newPage->setNextPage(-1); // no next page

        status = bufMgr->unPinPage(file, lastPageNo, true);
	if (status != OK) 
	{
	    // unpin the last page
            unpinstatus = bufMgr->unPinPage(file, newPageNo, true);
	    return status;
	}
        // now try to insert the record
        status = newPage->insertRecord(rec, rid);
        if (status == OK) 
        {
	    // insert was successful. unpin page and return
            status = bufMgr->unPinPage(file, newPageNo, true);
	    headerPage->recCnt++;
	    outRid = rid;
	    return status;
        }
	else return status;
    }
    else return status;
   // Solution Ends
}

// delete record from file. leaves page unpinned
const Status HeapFile::deleteRecord(const RID & rid)
{
   // Solution Starts
    Page* page;
    Status status;
    Error error;
    int pageNo = rid.pageNo; // extract page number from rid

    status = bufMgr->readPage(file, pageNo, page); // read the page
    if (status != OK) 
    {
      cerr << "readPage call in deleteRecord encountered error" << endl;
	Error err;
	err.print(status);
    	return status;
    }
    status = page->deleteRecord(rid); // delete the record from the page
    if ( (status != OK) && ( status != NORECORDS))
    {
      cerr << "page->deleteRecord call in deleteRecord encountered error" << endl;
	Error err;
	err.print(status);
    }

    headerPage->recCnt--;

    // At this point we should check if the whole page can be deallocated.
    if (status == NORECORDS)
      {
	// It can.
	// Get the previous page
	int prevPageNo = page->getPrevPage();

	if (prevPageNo != -1)
	  {
	    Page *prevPage;

	    // Prev page is not the header page.
	    if ((status = bufMgr->readPage(file, prevPageNo, prevPage)) != OK)
	      return status;

	    // Change the next page ptr of prev page
	    prevPage->setNextPage(page->getNextPage());

	    // Unpin the prev page
	    if ((status = bufMgr->unPinPage(file, prevPageNo, true)) != OK)
	      return status;
	  }
	else
	  headerPage->firstPage = page->getNextPage();

	int nextPageNo = page->getNextPage();
	if (nextPageNo != -1)
	  {
	    Page *nextPage;

	    // This is not the last page in the file
	    if ((status = bufMgr->readPage(file, nextPageNo, nextPage)) != OK)
	      return status;

	    // Change the prev page ptr of next page
	    nextPage->setPrevPage(page->getPrevPage());

	    // Unpin the next page
	    if ((status = bufMgr->unPinPage(file, nextPageNo, true)) != OK)
	      return status;
	  }
	else
	  headerPage->lastPage = page->getPrevPage();	    

	// Now dispose this page
	headerPage->pageCnt--;
	if ((status = bufMgr->unPinPage(file, pageNo, true)) != OK)
	  return status;

	status = bufMgr->disposePage(file, pageNo);
	if ( status != OK )
	  {
	    cerr << status << "failed \n";
	  error.print(status);
	  }
      }
    else
      status = bufMgr->unPinPage(file, pageNo, true);

    return status;
   // Solution Ends
}

HeapFileScan::HeapFileScan(const string & name,
                           Status & status) : HeapFile(name, status)
{
  curPage = NULL;
  curPageNo = 0;
  dirtyFlag = false;
  filter = NULL;
}

HeapFileScan::HeapFileScan(const string & name,
                           const int offset_,
                           const int length_,
                           const Datatype type_,
                           const char* filter_,
                           const Operator op_,
                           Status & status) : HeapFile(name, status)
{
  status = startScan(offset_, length_, type_, filter_, op_);
}

const Status HeapFileScan::startScan(const int offset_,
                                     const int length_,
                                     const Datatype type_,
                                     const char* filter_,
                                     const Operator op_)
{
  // Solution Starts
  curPage = NULL;
  curPageNo = 0;
  dirtyFlag = false;
  curRec.reset();

  if (!filter_) {                        // no filtering requested
    filter = NULL;
    return OK;
  }

  if ((offset_ < 0 || length_ < 1) ||
      (type_ != STRING && type_ != INTEGER && type_ != DOUBLE) ||
      (type_ == INTEGER && length_ != sizeof(int)
       || type_ == DOUBLE && length_ != sizeof(double)) ||
      (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ !=
 NE))
    {
      return BADSCANPARM;
    }

  offset = offset_;
  length = length_;
  type = type_;
  filter = filter_;
  op = op_;
 // Solution Ends

  return OK;
}

const Status HeapFileScan::endScan()
{
  // Solution Starts
  // generally must unpin last page of the scan
  Status status;
  if (curPage != NULL)
    if ((status = bufMgr->unPinPage(file, curPageNo, dirtyFlag)) != OK)
       return status; 

  curPage = NULL;
  curPageNo = 0;
  dirtyFlag = false;
  curRec.reset();

  // Solution Ends

  return OK;
}

HeapFileScan::~HeapFileScan()
{
  endScan();
}

const Status HeapFileScan::scanNext(RID& outRid)
{
   // Solution Starts
    Status 	status = OK;
    RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;

    if (curPageNo < 0) {curRec.reset(); return FILEEOF;}  // already at EOF!

    // special case the record of the first page of the file
    if (curPageNo == 0) 
    {
    	// need to get the first page of the file
	curPageNo = headerPage->firstPage;
	dirtyFlag = false;
	if (curPageNo == -1) {curRec.reset(); return FILEEOF;} // file is empty
	 
	// read the first page of the file
        status = bufMgr->readPage(file, curPageNo, curPage); 
        if (status != OK) return status;
	else
	{
	    // get the first record off the page
	    status  = curPage->firstRecord(tmpRid);
	    curRec = tmpRid;
	    if (status == NORECORDS) 
	    {
    	    	curPageNo = -1; // in case called again
		curPage = NULL; // for endScan()
	    	curRec.reset();  // reset the curRec
                return FILEEOF;  // first page had no records
	    }
	    // get pointer to record
	    status = curPage->getRecord(tmpRid, rec);
	    if (status != OK) return status;
	    // see if record matches predicate
            if (matchRec(rec) == true)  
	    {
		outRid = tmpRid;
		return OK;
	    }
	}
    }
    // Default case. already have a page pinned in the buffer pool.
    // First see if it has any more records on it.  If so, return
    // next one. Otherwise, get the next page of the file
    for(;;) 
    {
	// Loop, looking for a record that satisfied the predicate.
	// First try and get the next record off the current page
     	status  = curPage->nextRecord(curRec, nextRid);
	if (status == OK) curRec = nextRid;
	else if (status == ENDOFPAGE)
	{
	    // get the page number of the next page in the file
	    nextPageNo = curPage->getNextPage();
	    if (nextPageNo == -1) {curRec.reset(); return FILEEOF;} // end of file

	    // unpin the current page
    	    status = bufMgr->unPinPage(file,curPageNo, 
			dirtyFlag);
	    if (status != OK) return status;
	 
	    // get prepared to read the next page
	    curPageNo = nextPageNo;
	    dirtyFlag = false;

	    // read the next page of the file
            status = bufMgr->readPage(file,curPageNo,curPage);
            if (status != OK) return status;

	    // get the first record off the page
	    status  = curPage->firstRecord(curRec);
	    if (status == NORECORDS) 
	    {
    	       curPageNo = -1; // in case, called again
	       curPage = NULL; // for endScan()
	       curRec.reset();  // reset the curRec
	       return FILEEOF;  // first page had no records
	    }
	}
	// curRec points at a valid record
	// see if the record satisfies the scan's predicate 
	// get a pointer to the record
	status = curPage->getRecord(curRec, rec);
	if (status != OK) return status;
	// see if record matches predicate
	if (matchRec(rec) == true)  
	{
	    // return rid of the record
	    outRid = curRec;
	    return OK;
	}
    }
   // Solution Ends
}

// scan the next record and also return the actual record
const Status HeapFileScan::scanNext(RID& outRid, Record& rec)
{
   Status status;

   if ((status = scanNext(outRid)) != OK) return status;
   if ((status = getRecord(outRid, rec)) != OK) return status;
   return OK;
}

// returns pointer to record identified by rid.  page is left pinned
// and the scan logic is required to unpin the page 

const Status HeapFileScan::getRecord(const RID & rid, Record & rec)
{
   // Solution Starts
    Page* page;
    Status status;

    if (rid.pageNo != curPageNo)
    {
      cerr << "page# in rid (" << rid.pageNo 
	   << ") does not match current page# of scan (" 
	   << curPageNo << ")" << endl;
	return BADPAGENO;
    }
    page = curPage;
    status = page->getRecord(rid, rec);

    return status;
   // Solution Ends
}

// mark record identified by rid dirty.  page is left pinned.

const Status HeapFileScan::markDirty(const RID & rid)
{
   // Solution Starts
    if (rid.pageNo != curPageNo)
    {
      cerr << "page# in rid (" << rid.pageNo 
	   << ") does not match current page# of scan (" 
	   << curPageNo << ")" << endl;
      return BADPAGENO;
    }
    dirtyFlag = true;
    return OK;
   // Solution Ends
}

const bool HeapFileScan::matchRec(const Record & rec) const
{

    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

  double diff = 0;                       // < 0 if attr < fltr
  switch(type) {

    case INTEGER:
      int iattr, ifltr;                 // word-alignment problem possible
      memcpy(&iattr,
	     (char *)rec.data + offset,
	     length);
      memcpy(&ifltr,
	     filter,
	     length);
      diff = iattr - ifltr;
      break;

    case DOUBLE:
      double fattr, ffltr;               // word-alignment problem possible
      memcpy(&fattr,
	     (char *)rec.data + offset,
	     length);
      memcpy(&ffltr,
	     filter,
	     length);
      diff = fattr - ffltr;
      break;

    case STRING:
      diff = strncmp((char *)rec.data + offset,
		    filter,
		    length);
      break;
  }

  switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    case NOTSET: assert(!"invalid op"); break;
    default: break;
  }

  return false;

}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned

const Status HeapFileScan::getRandomRecord(const RID &  rid, 
					   Record & rec)
{
   // Solution Starts
    Page* page;
    Status status;

    if ((curPageNo > 0) && 
	(rid.pageNo != curPageNo))
    {
	// there is a page pinned which is not the desired page, unpin it
        status = bufMgr->unPinPage(file, curPageNo, 
		dirtyFlag);
	if (status != OK) return status;
	curPageNo = -1;
	curPage   = NULL;
    }

    if (curPageNo < 1) {        // no current page?
	// get prepared to read the desired page
	curPageNo = rid.pageNo; // extract page # from rid
	dirtyFlag = false;

	// read desired page
	status = bufMgr->readPage(file, curPageNo, 
				  curPage);
	if (status != OK) return status;
    }

    page = curPage;
    status = page->getRecord(rid, rec);
    curRec.pageNo = rid.pageNo;
    curRec.slotNo = rid.slotNo;

    return status;
   // Solution Ends
}

// set a marker to the current record
Status HeapFileScan::setMarker() 
{
   mark.pageNo = curRec.pageNo;
   mark.slotNo = curRec.slotNo;

   return OK;
}

// move the scan back to the marked position.
Status HeapFileScan::gotoMarker(RID &  rid, Record& rec) 
{
   rid = mark;
   return getRandomRecord(mark, rec);
}
