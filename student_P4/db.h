/**************************************
 * DO NOT IN ANY WAY MODIFY THIS FILE *
 **************************************/

#ifndef DB_H
#define DB_H

#include <sys/types.h>
#include <string>
#include <functional>
#include <map>
#include <iostream>
#include "error.h"

using namespace std;

// define if debug output wanted

//#define DEBUGIO
//#define DEBUGFREE



class DB; // forward declaration
class Page; // forward declaration

// class definition for open files
class File {
  friend class DB;

 public:

  const Status allocatePage(int& pageNo);     // allocate a new page
  const Status disposePage(const int pageNo);       // release space for a page
  const Status readPage(const int pageNo,
		  Page* pagePtr) const;       // read page from file
  const Status writePage(const int pageNo,
		   const Page* pagePtr);      // write page to file
  const Status getFirstPage(int& pageNo) const;     // returns pageNo of first page

  bool operator == (const File & other) const
    {
      return fileName == other.fileName;
    }

  bool operator < (const File & other) const
    {
      return fileName < other.fileName;
    }

  void printName() const
  {
    cout << "File: " << fileName << endl;
  }
 private: 

  File(const string & fname);                   // initialize
  ~File();                  // deallocate file object

  static const Status create(const string & fileName);
  static const Status destroy(const string & fileName);

  const Status open();
  const Status close();

  // Check if the page is on the free list
  const Status onFreeList(const int pageNo, bool& onFL) const; 

  const Status intread(const int pageNo,
		 Page* pagePtr) const;        // internal file read
  const Status intwrite(const int pageNo,
		  const Page* pagePtr);       // internal file write

#ifdef DEBUGFREE
  void listFree();                      // list free pages
#endif

  string fileName;                    // The name of the file
  int openCnt;                        // # times file has been opened
  int unixFile;                       // unix file stream for file
};


class BufMgr;
extern BufMgr* bufMgr;


class DB {
 public:
  DB();                                 // initialize open file table
  ~DB();                                // clean up any remaining open files

  const Status createFile(const string & fileName) const;  // create a new file
  const Status destroyFile(const string & fileName) const; // destroy a file, 
                                                           // release all space
  const Status openFile(const string & fileName, File* & file);  // open a file
  const Status closeFile(File* file);         // close a file

 private:
  typedef map<string, File* > OpenFileMap;
  OpenFileMap   openFiles;    // list of open files
};


// structure of DB (header) page

typedef struct {
  int nextFree;                         // page # of next page on free list
  int firstPage;                        // page # of first page in file
  int numPages;                         // total # of pages in file
} DBPage;

#endif
