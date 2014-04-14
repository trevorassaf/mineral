
#ifndef ERROR_H
#define ERROR_H

// error/status codes defined here

// add error codes under appropriate heading
// -- remember to add corresponding message to
//    Error.print() also!

enum Status {

// no error

       OK = 0, NOTUSED1 = -999,

// File and DB errors

       BADFILEPTR, BADFILE, FILETABFULL, FILEOPEN, FILENOTOPEN,
       UNIXERR, BADPAGEPTR, BADPAGENO, FILEEXISTS,

// BufMgr and HashTable errors

       BUFMAPERROR, BUFMAPNOTFOUND, BUFFEREXCEEDED, PAGENOTPINNED,
       BADBUFFER, PAGEPINNED,

// Page errors
	
       NOSPACE,  NORECORDS,  ENDOFPAGE, INVALIDSLOTNO,

// HeapFile errors

       BADRID, BADRECPTR, BADSCANPARM, BADSCANID, SCANTABFULL, FILEEOF,

// Index errors
       BADINDEXPARM, 

// Hash Index errors
 
       RECNOTFOUND, BUCKETFULL, DIROVERFLOW, 
       NONUNIQUEENTRY, NOMORERECS, NOCHARIDX, 


// Btree index errors

       INDEXPAGEFULL, INDEXPAGENOTFULL, BADINDEXPAGEPOSITION, 
       INDEXLOADERROR, ENTRYNOTUNIQUE, EXPECTINGNONLEAFPAGE, SCANEXECUTING, 
       BADINDEXSCANPARM, NOSCANEXECUTING, ENDOFINDEXSCAN, NOTFOUNDINLEAF, 
       NOTFOUNDINNONLEAF, MERGEERROR, 

// SortedFile errors
 
       BADSORTPARM, INSUFMEM, 
	
// Catalog errors

       BADCATPARM, RELNOTFOUND, ATTRNOTFOUND,
       NAMETOOLONG, DUPLATTR, RELEXISTS, NOINDEX,
       INDEXEXISTS, ATTRTOOLONG,

// Utility errors

// Query errors

       ATTRTYPEMISMATCH, TMP_RES_EXISTS,

// do not touch filler -- add codes before it

       NOTUSED2
};


// definition of Error class

class Error {
 public:
  static void print(Status status);
};

#endif
