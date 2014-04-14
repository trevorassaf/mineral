#ifndef BUFMAP_H
#define BUFMAP_H

#include "db.h"

// define if debug output wanted
//#define DEBUGBUF

// Used by BufMap.  Class for locating buffer frame.
struct FileAndPage
{
  File*	file;    // pointer to a file object
  int	pageNo;  // page number within a file
  
  FileAndPage(File *f, int p)
    {
      file = f;
      pageNo = p;
    }

  bool  operator == (const FileAndPage & other) const
    {
      return *file == *(other.file) && pageNo == other.pageNo;
    }

  bool  operator < (const FileAndPage & other) const
    {
      if (*file == *(other.file)) return pageNo < other.pageNo;
      else return *file < *(other.file);
    }

};


// class to keep track of pages in the buffer pool
class BufMap 
{ 
public:   

  // insert entry into buffer pool, mapping (file,pageNo) to frameNo.
  // return OK or BUFMAPERROR if an error occured.
  const Status insert(File* file, const int pageNo, const unsigned int frameNo); 

  // Check if (file,pageNo) is currently in the buffer pool and return frameNo
  // if found.  Otherwise return BUFMAPNOTFOUND.
  const Status lookup(File* file, const int pageNo, unsigned int & frameNo) const; 

  // delete entry (file,pageNo) from buffer pool.  Return OK if pate was 
  // found.  Else return BUFMAPNOTFOUND.
  const Status remove(File* file, const int pageNo); 

private: 

  typedef map<FileAndPage, unsigned int> BufHashTable;
  BufHashTable hashTable;  // Hash table for locating frame

};

#endif // BUFMAP_H
