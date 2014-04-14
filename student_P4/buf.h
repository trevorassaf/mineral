#ifndef BUF_H
#define BUF_H

#include "db.h"
#include "bufMap.h"

// define if debug output wanted
//#define DEBUGBUF


class BufMgr;  //forward declaration of BufMgr class 

// class for maintaining information about buffer pool frame
class BufDesc {
    friend class BufMgr;
private:
  File* file;   // pointer to file object
  int   pageNo; // page within file
  int   pinCnt; // number of times this page has been pinned
  bool dirty;   // true if dirty;  false otherwise
  bool valid;   // true if page is valid
  bool refbit;  // has this buffer frame been referenced recently?

  void Clear() {  // initialize buffer frame for a new user
    	pinCnt = 0;
	file = NULL;
	pageNo = -1;
    	dirty = refbit = false;
	valid = false;
  };

  void Set(File* filePtr, int pageNum) { 
      file = filePtr;
      pageNo = pageNum;
      pinCnt = 1;
      dirty = false;
      refbit = true;
      valid = true;
  }

  BufDesc() {
      Clear();
  }
};


struct BufStats
{
  unsigned accesses;    // Total number of accesses to buffer pool
  unsigned diskreads;   // Number of pages read from disk 
  unsigned diskwrites;  // Number of pages written back to disk

  void clear()
    {
      accesses = diskreads = diskwrites = 0;
    }
      
  BufStats()
    {
      clear();
    }
};

inline ostream & operator << (ostream & os, const BufStats & stats)
{
  os << "accesses = " << stats.accesses
     << ", disk reads = " << stats.diskreads
     << ", disk writes = " << stats.diskwrites << endl;

  return os;
}


class BufMgr 
{
private:
  unsigned int	 clockHand;  // clock hand for clock algorithm
  BufMap         bufMap;     // mapping of (File, page) to frame
  BufDesc	 *bufTable;  // vector of status info, 1 per page
  unsigned int   numBufs;    // Number of pages in buffer pool
  BufStats       bufStats;   // Statistics about buffer pool usage


  const Status allocBuf(unsigned int & frame);   // allocate a free frame.  

  void advanceClock()
    {
      clockHand = (clockHand + 1) % numBufs;
    }

public:
  Page	         *bufPool;   // actual buffer pool

  BufMgr(const unsigned int bufs);
  ~BufMgr();

  const Status readPage(File* file, const int PageNo, Page*& page);
  const Status unPinPage(File* file, const int PageNo, const bool dirty);
  const Status allocPage(File* file, int& PageNo, Page*& page); 
                        // allocates a new, empty page 
  const Status flushFile(File* file); // writing out all dirty pages of the file
  const Status disposePage(File* file,
			   const int PageNo); // dispose of page in file

  unsigned numUnpinnedPages();  // Return the number of pages that are currently unpinned

  void printSelf(void); // Print the buffer pool contents

  const BufStats & getBufStats() const // Get buffer pool usage
    {
      return bufStats;
    }

  void clearBufStats() 
    {
      bufStats.clear();
    }
};

#endif
