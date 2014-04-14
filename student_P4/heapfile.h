#ifndef HEAPFILE_H
#define HEAPFILE_H

#include "buf.h"
#include "page.h"
#include "datatypes.h"

extern DB db;

// define if debug output wanted
//#define DEBUGREL

// Some constant definitions
const unsigned MAXNAMESIZE = 50;

struct HeaderPage
{
  char fileName[MAXNAMESIZE];   // name of file
  int		firstPage;	// pageNo of first data page in file
  int		lastPage;	// pageNo of last data page in file
  int		pageCnt;	// number of pages
  int		recCnt;		// record count
};


// class definition of heapFile
class HeapFile {
protected:

  File* 	file;           // pointer to underlying DB File object
  HeaderPage*   headerPage;	// pointer to header page in buffer pool
  int		headerPageNo;	// page number of header page

public:

  // initialize
  HeapFile(const string & name, Status& returnStatus);

  // destructor
  ~HeapFile();

  // return number of records in file
  const int getRecCnt() const;

  // insert record into file
  const Status insertRecord(const Record & rec, RID& outRid); 

  // delete record from file
  const Status deleteRecord(const RID & rid);
};


class HeapFileScan : public HeapFile
{
public:

  // initiate a filtered scan
  HeapFileScan(const string & name,
	       const int offset, 
	       const int length,  
	       const Datatype type, 
	       const char* filter, 
	       const Operator op,
	       Status & status);

  HeapFileScan(const string & name,
	       Status & status);

  // end filtered scan
  ~HeapFileScan();

  // Start a scan (with an optional predicate)
  const Status startScan(const int offset, 
			 const int length,  
			 const Datatype type, 
			 const char* filter, 
			 const Operator op);

  // end the scan
  const Status endScan();


  // set a marker to the current record
  Status setMarker(); 

  // move the scan back to the marked position. And retrieve the 
  // record that was marked.
  Status gotoMarker(RID& rid, Record & rec); 

  // read record from file, returning pointer and length
  const Status getRecord(const RID & rid, Record & rec);

  // marks current page of scan dirty
  const Status markDirty(const RID & rid);

  // return next record
  const Status scanNext(RID& outRid);

  // Goto the next record and return both the rid and the record
  const Status scanNext(RID& outRid, Record& rec);

  // read record from file, returning pointer and length
  const Status getRandomRecord(const RID &rid, Record & rec);

private:

  RID   curRec;            // rid of last record returned
  Page* curPage;	   // pointer to pinned page in buffer pool
  int   curPageNo;	   // page number of pinned page
  bool  dirtyFlag;	   // true if page has been updated
  int   offset;            // byte offset of filter attribute
  int   length;            // length of filter attribute
  Datatype type;           // datatype of filter attribute
  const char* filter;      // comparison value of filter
  Operator op;             // comparison operator of filter

  RID   mark;              // last marked spot (RID) in the file.

  const bool matchRec(const Record & rec) const;
};

#endif
