#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "catalog.h"

// Global variables
DB db;                 // a handle for the DB class
Error error;           // a handle for the error class

BufMgr *bufMgr;        // pointer to the buffer manager 
RelCatalog *relCat;    // pointer to the relation catalogs 
AttrCatalog *attrCat;  // pointer to the attribute catalogs

#define CALL(c)    {Status s;if((s=c)!=OK){error.print(s);exit(1);}}


// Driver program for creating the database
int main(int argc, char *argv[])
{
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " dbname" << endl;
    return 1;
  }

  // 
  // All minirel relations for a specific database are stored in a 
  // Unix directory that has the same name as the database. 
  // Corresponding to each HeapFile in the system, a file is created 
  // in this subdirectory
  //
  // create database subdirectory and chdir there
  if (mkdir(argv[1], S_IRUSR | S_IWUSR | S_IXUSR
	             | S_IRGRP | S_IWGRP | S_IXGRP) < 0) {
    perror("mkdir");
    exit(1);
  }

  if (chdir(argv[1]) < 0) {
    perror("chdir");
    exit(1);
  }

  // create buffer manager
  bufMgr = new BufMgr(10000);
  
  // open relation and attribute catalogs
  Status status;

  relCat = new RelCatalog(status);
  if (status == OK)
    attrCat = new AttrCatalog(status);
  if (status != OK) {
    error.print(status);
    exit(1);
  }

#define BOOTSTRAP_CATALOGS 
#ifdef BOOTSTRAP_CATALOGS 

  // add tuples describing relcat and attrcat to relation catalog
  // and attribute catalog

  RelDesc rd;
  AttrDesc ad;

  strcpy(rd.relName, RELCATNAME);
  rd.attrCnt = 3;
  rd.indexCnt = 0;
  CALL(relCat->addInfo(rd));

  // Create the attribute records for the relcat relation.
  //    Warning the attribute names must be all lower case to 
  //    allow the user to query the catalogs (the parser only
  //    passes back lowercase names)

  strcpy(ad.relName, RELCATNAME);
  strcpy(ad.attrName, "relname");
  ad.attrOffset = 0;
  ad.attrType = STRING;
  ad.attrLen = sizeof rd.relName;
  ad.indexed = 0;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "attrcnt");
  ad.attrOffset += sizeof rd.relName;
  ad.attrType = INTEGER;
  ad.attrLen = sizeof rd.attrCnt;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "indexcnt");
  ad.attrOffset += sizeof rd.attrCnt;
  ad.attrType = INTEGER;
  ad.attrLen = sizeof rd.indexCnt;
  CALL(attrCat->addInfo(ad));

  // Just as the relcat information is added to the catalogs,
  // add the attrcat name. Note that the variable ATTRCATNAME is 
  // the name of the attrCat relation. 
  // The attributes of this relation should correspond to the variables 
  // of the struct AttrDesc.

  // Solution Begins 
  // Warning the attribute names must be all lower case to 
  // allow the user to query the catalogs (the parser only
  // passes back lowercase names)
  strcpy(rd.relName, ATTRCATNAME);
  rd.attrCnt = 6;
  rd.indexCnt = 0;
  CALL(relCat->addInfo(rd))

  strcpy(ad.relName, ATTRCATNAME);
  strcpy(ad.attrName, "relname");
  ad.attrOffset = 0;
  ad.attrType = STRING;
  ad.attrLen = sizeof ad.relName;
  ad.indexed = 0;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "attrname");
  ad.attrOffset += sizeof ad.relName;
  ad.attrType = STRING;
  ad.attrLen = sizeof ad.attrName;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "attroffset");
  ad.attrOffset += sizeof ad.attrName;
  ad.attrType = INTEGER;
  ad.attrLen = sizeof ad.attrOffset;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "attrtype");
  ad.attrOffset += sizeof ad.attrOffset;
  ad.attrType = INTEGER;
  ad.attrLen = sizeof ad.attrType;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "attrlen");
  ad.attrOffset += sizeof ad.attrType;
  ad.attrType = INTEGER;
  ad.attrLen = sizeof ad.attrLen;
  CALL(attrCat->addInfo(ad));

  strcpy(ad.attrName, "indexed");
  ad.attrOffset += sizeof ad.attrLen;
  ad.attrType = INTEGER;
  ad.attrLen = sizeof ad.indexed;
  CALL(attrCat->addInfo(ad));
  // Solution Ends 

#endif

  delete relCat;
  delete attrCat;

  delete bufMgr;

  cout << "Database " << argv[1] << " created" << endl;

  return 0;
}
