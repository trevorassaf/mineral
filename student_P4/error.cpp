#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "error.h"

using namespace std;

void Error::print(Status status)
{
  cerr << "Error: ";
  switch(status) {

    // no error

    case OK:           cerr << "no error"; break;

    // File and DB errors

    case BADFILEPTR:   cerr << "bad file pointer"; break;
    case BADFILE:      cerr << "bad filename"; break;
    case FILETABFULL:  cerr << "open file table full"; break;
    case FILEOPEN:     cerr << "file open"; break;
    case FILENOTOPEN:  cerr << "file not open"; break;
    case UNIXERR:      perror("Unix error"); break;
    case BADPAGEPTR:   cerr << "bad page pointer"; break;
    case BADPAGENO:    cerr << "bad page number"; break;
    case FILEEXISTS:   cerr << "file exists already"; break;

    // BufMgr and BufMap errors

    case BUFMAPERROR: cerr << "buffer map error"; break;
    case BUFMAPNOTFOUND: cerr << "entry not found"; break;
    case BUFFEREXCEEDED: cerr << "buffer pool full"; break;
    case PAGENOTPINNED: cerr << "page not pinned"; break;
    case BADBUFFER: cerr << "buffer pool corrupted"; break;
    case PAGEPINNED: cerr << "page still pinned"; break;

    // Page class errors

    case NOSPACE: cerr << "no space on page for record"; break;
    case NORECORDS: cerr << "page is empty - no records"; break;
    case ENDOFPAGE: cerr << "last record on page"; break;
    case INVALIDSLOTNO: cerr << "invalid slot number"; break;

    // Heap file errors

    case BADRID:       cerr << "bad record id"; break;
    case BADRECPTR:    cerr << "bad record pointer"; break;
    case BADSCANPARM:  cerr << "bad scan parameter"; break;
    case BADSCANID:    cerr << "bad scan id"; break;
    case SCANTABFULL:  cerr << "scan table full"; break;
    case FILEEOF:      cerr << "end of file encountered"; break;

    // Index errors

    case BADINDEXPARM: cerr << "bad index parameter"; break;

    // Hash Index errors

    case RECNOTFOUND:  cerr << "no such record"; break;
    case BUCKETFULL:   cerr << "bucket full"; break;
    case DIROVERFLOW:  cerr << "directory is full"; break;
    case NONUNIQUEENTRY: cerr << "nonunique entry"; break;
    case NOMORERECS:   cerr << "no more records"; break;
    case NOCHARIDX:   cerr << "hash indices on CHAR is not supported"; break;

    // B+-tree Index errors

    case INDEXPAGEFULL: cerr << "the index page is full"; break;
    case INDEXPAGENOTFULL: cerr << "the index page is not full"; break;
    case BADINDEXPAGEPOSITION: cerr << "bad index page position"; break;
    case INDEXLOADERROR: cerr << "error loading the index"; break;
    case ENTRYNOTUNIQUE: cerr << "entry is not unique"; break;
    case EXPECTINGNONLEAFPAGE: cerr << "expecting a non-leaf page"; break;
    case SCANEXECUTING: cerr << "only one scan supported at a time"; break;
    case BADINDEXSCANPARM: cerr << "bad index scan parameter(s)"; break;
    case NOSCANEXECUTING: cerr << "no scan is executing"; break;
    case ENDOFINDEXSCAN: cerr << "end of index scan"; break;
    case NOTFOUNDINLEAF: cerr << "entry not found in leaf page"; break;
    case NOTFOUNDINNONLEAF: cerr << "entry not found in non-leaf page"; break;
    case MERGEERROR: cerr << "Merge error"; break;


    // Sorted file errors

    case BADSORTPARM:  cerr << "bad sort parameter"; break;
    case INSUFMEM:     cerr << "insufficient memory"; break;

    // Catalog errors

    case BADCATPARM:   cerr << "bad catalog parameter"; break;
    case RELNOTFOUND:  cerr << "relation not in catalog"; break;
    case ATTRNOTFOUND: cerr << "attribute not in catalog"; break;
    case NAMETOOLONG:  cerr << "name too long"; break;
    case ATTRTOOLONG:  cerr << "attributes too long"; break;
    case DUPLATTR:     cerr << "duplicate attribute names"; break;
    case RELEXISTS:    cerr << "relation exists already"; break;
    case NOINDEX:      cerr << "no index exists"; break;
    case INDEXEXISTS:  cerr << "index exists already"; break;

      // Query errors

  case ATTRTYPEMISMATCH: cerr << "attribute type mismatch"; break;
  case TMP_RES_EXISTS:   cerr << "temporary result relation exists"; break;

    default:           cerr << "undefined error status: " << status;
  }
  cerr << endl;
}

