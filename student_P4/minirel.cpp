#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "catalog.h"
#include "query.h"

// Global variables
DB db;                 // a handle for the DB class
Error error;           // a handle for the error class

BufMgr *bufMgr;        // pointer to the buffer manager 
RelCatalog *relCat;    // pointer to the relation catalogs 
AttrCatalog *attrCat;  // pointer to the attribute catalogs

extern FILE* yyin;     // input file for the parser. The parser reads 
                       //    SQL statments from this file 

// The driver for the minirel database server
int main(int argc, char **argv)
{
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " dbname" << endl;
    return 1;
  }

  // If the command line contains a file, then read SQL statements
  // from the file, else read input from stdin.
  if (argc > 2)
    if (!(yyin = fopen (argv[2], "r")))
    {
        cerr << "Error in opening file: <" << argv[2] << ">" << endl;
        perror(0);
        exit (-1);
    }

  if (chdir(argv[1]) < 0) {
    perror("chdir");
    exit(1);
  }

  // create buffer manager
  bufMgr = new BufMgr(32);
  
  // open relation and attribute catalogs
  Status status;
  relCat = new RelCatalog(status);
  if (status == OK)
    attrCat = new AttrCatalog(status);
  if (status != OK) {
    error.print(status);
    exit(1);
  }

  cout << "Welcome to Minirel" << endl;

  // Keep parsing query. The parse routine reads in SQL commands, and 
  // calls appropriate Minirel functions for executing the SQL commands
  extern void parse();
  parse();

  // Cleanup before shutting down the server
  delete relCat;
  delete attrCat;
  delete bufMgr;

  return 0;
}
