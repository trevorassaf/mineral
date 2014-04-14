#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "index.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Constructor for the Index class. The arguments passed in are 
//     'name' -- file name of the relation
//     'offset', 'length', 'type' -- describing the attribute indexed
//     'unique' -- flag for enforcing uniquess of the entries

Index::Index(const string & name, 
	     const int offset, 
	     const int length, 
	     const Datatype type, 
	     const int unique, 
	     Status& status)
{
  Page* pagePtr;
  file = 0;

  // For this assignment turn off hash indices on strings
  if (type == STRING ) 
  {
     status = NOCHARIDX;
     return;
  }
   
   
  if(name.empty()) {
    status = BADFILE;
    return;
  }
  if (offset < 0 || length < 1) {
    status = BADINDEXPARM;
    return;
  }

  if (type != STRING && type != INTEGER && type != DOUBLE){
    status = BADINDEXPARM;
    return;
  }
  if (type == INTEGER && length != sizeof(int)
      || type == DOUBLE && length != sizeof(double)) {
    status = BADINDEXPARM;
    return;
  }

  curBuc = NULL;

  recSize = length + sizeof(RID);     // size of the index entry
  numSlots = (PAGESIZE - 2*sizeof(short)) / recSize;   // # entries on a page

  status = OK;

  // get name of the index file by concatenating relation name and
  // the offset of the attribute

  ostringstream outputString;
  outputString << name << '.' << offset << ends;
  string indexName(outputString.str());

  // The constructor runs in two cases. In the first case, an index 
  // doesn't exist as detected by db.openFile. The relation file 
  // should be scanned and each tuple is inserted to the index. In
  // the second case, an index already exists. Only the headerPage
  // needs to read in and pinned down in the buffer pool.

  file = 0;
  status = db.openFile(indexName, file);
  if (status != OK) { // index doesn't exit. Create it

    status = db.createFile(indexName);
    if (status != OK)
      return;

    // open the file
    status = db.openFile(indexName, file);
    if (status != OK)
      return;

    // allocate and initialize the header page
    status = bufMgr->allocPage(file, headerPageNo, (Page*&)headerPage);
    if (status != OK)
      return;
    strcpy(headerPage->fileName, name.c_str());

    headerPage->offset = offset;
    headerPage->length = length;
    headerPage->type = type;
    headerPage->depth = 0;
    headerPage->unique = unique;
    dirSize = 1;

    // allocate and initialize the first bucket pointed by dir[0]
    int pageNo;
    Bucket* bucket;
    status = bufMgr->allocPage(file, pageNo, (Page*&)bucket);
    if (status != OK)
      return;
    bucket->depth = 0;
    bucket->slotCnt = 0;
    headerPage->dir[0] = pageNo;

    status = bufMgr->unPinPage(file, pageNo, true);
    if (status != OK)
      return;

    // build index by scanning the relation file and inserting every
    // tuple
    
    HeapFileScan heapFileScan(name, offset, length, type, NULL, EQ, status);

    if (status != OK)
      return;

    Record rec;
    RID   rid;

    while(1) {
      status = heapFileScan.scanNext(rid);
      if (status == FILEEOF)
	{
	  status = OK;
	  break;
	}
      status = heapFileScan.getRecord(rid, rec);
      if (status != OK)
	return;
      status = insertEntry((char *)rec.data + offset, rid);
      if (status != OK)
	return;
    }
  }
  else  { // file already exists.  get header page into the buffer pool

    status = file->getFirstPage(headerPageNo);
    if (status != OK) 
      {
	cerr << "fetch of first page failed\n";
        return;
      }
    status = bufMgr->readPage(file, headerPageNo, pagePtr);
    if (status != OK) 
      {
	cerr << "read of first page failed\n";
	return;
      }
    headerPage = (iHeaderPage*) pagePtr;
    dirSize = (int)pow(2.0, headerPage->depth);
  }
}

Index::~Index() {

  // if file pointer is not set then simply return.
  if (!file) return; 

  Status status = bufMgr->unPinPage(file, headerPageNo, true);
  if (status != OK)
    return;
  if (curBuc != NULL) {
    status = bufMgr->unPinPage(file, curPageNo, true);
    if (status != OK) cerr << "error in unpin of index page\n";
  }

  status = db.closeFile(file);
}

// return a hashvalue hashed from the attribute 'value' according to 
// the current depth of the directory

const Status Index::hashIndex(const void *attr, int& hashvalue)
{
    int index;
    const char *value = (const char *)attr;

    switch (headerPage->type)
    {
       case STRING:
            {
               // For strings "add" the values of the characters, stopping 
               // at the first character that is 0
               index = value[0];
               for (int i = 1; i < headerPage->length; i++) 
                   if (!value[i]) 
                      index += value[i];
                   else break;
            }
            break;
       case DOUBLE:
            {
               // Treat doubles as strings of length sizeof (double)
               index = value[0];
               for (int i = 1; i < headerPage->length; i++) index += value[i];
            }
            break;
       case INTEGER:
            // copy the integer value to avoid byte-alignment errors.
            memcpy (&index, value, sizeof(int));
            break;
       default: 
            return BADINDEXPARM;
    }

    // hashvalue is the lower bits of 'index'. Number of bits is determined
    // by the depth of the directory

    index=abs(index);   
    int mask = 1 << headerPage->depth;
    hashvalue = index % mask;

    return OK;
  }

// Insert an <attribute, rid> pair into the index. Return OK if the 
// entry is inserted and DIROVERFLOW if the directory isn't large
// enough to hold the indices.

const Status Index::insertEntry(const void *value, RID rid) 
{ 
  Bucket* bucket;
  Status status;
  Bucket *newBucket;
  int newPageNo;       
  char  data[PAGESIZE*2];
  int counter;
  int index;

  // If the 'unique' flag is set, scan the index to see if the
  // <attribute, rid> pair already exists

  if (headerPage->unique == UNIQUE) {
    RID outRid;

    if((status = startScan(value)) != OK)
      return status;
    while ((status = scanNext(outRid)) != NOMORERECS) {
      if (status != OK)
	return status;    
      if (!memcmp(&outRid, &rid, sizeof(RID)))
	return NONUNIQUEENTRY;
    }
    if((status = endScan()) != OK)
      return status;
  }
 
  // Get the bucket containing the entry into buffer pool
  status = hashIndex(value, index);
  int pageNo = headerPage->dir[index];

#ifdef DEBUGIND
  cout << "Inserting entry " << *(int*)value << " to bucket " 
    << pageNo << endl;
#endif 
 
  status = bufMgr->readPage(file, pageNo, (Page*&)bucket);
  if (status != OK) 
    return status;

  // the bucket needs to be splitted if the number of entries
  // on the bucket equals the maximum
  if (bucket->slotCnt == numSlots) { // splitting bucket
    
    // allocate a new bucket
    status = bufMgr->allocPage(file, newPageNo, (Page*&)newBucket);  
    if (status != OK)
      return status;

    // Initialize this newly allocated bucket
    newBucket->depth = ++(bucket->depth);
    newBucket->slotCnt = 0;

    // Copy all (value, rid) pairs in the old bucket and the new
    // entry to a temporary area
    memcpy(data, bucket->data, numSlots*recSize);
    memcpy(&(data[numSlots*recSize]), value, headerPage->length);
    memcpy(&(data[numSlots*recSize + headerPage->length]), &rid, sizeof(RID));

    counter = bucket->slotCnt + 1;
    bucket->slotCnt = 0;

    // the directory needs to be doubled if the depth of the bucket
    // being splitted equals the depth of the directory

    if (bucket->depth > headerPage->depth) { // doubling directory

      // The directory is doubled and the lower half of the directory
      // is copied to the upper half

      int newDirSize = 2 * dirSize;
      if (newDirSize > DIRSIZE)
	return DIROVERFLOW;
      for (int i = 0; i < dirSize; i++)
	headerPage->dir[i + dirSize] = headerPage->dir[i];

      dirSize = newDirSize;
      (headerPage->depth)++;
      headerPage->dir[index + (1 << (bucket->depth - 1))] = newPageNo;

    } else {

      // reset the appropriate directories to the new bucket
      int oldindex = index % (1 << (bucket->depth - 1));
      int newindex = oldindex + (1 << (bucket->depth - 1));
      for (int j = 0; j < dirSize; j++) 
	if ((j % (1 << (bucket->depth))) == newindex)
	  headerPage->dir[j] = newPageNo;
    }

#ifdef DEBUGIND
      printDir();
#endif 

    // call insertEntry recursively to insert all (value, rid) 
    // pairs in the temporary area to the index

    for (int k = 0; k < counter; k++) {
      value = &(data[k * recSize]);
      rid = * ((RID *)((char *)value + headerPage->length));
      status = insertEntry(value, rid);
      if (status != OK)
	return status;
    }
    status = bufMgr->unPinPage(file, newPageNo, true);
    if (status != OK)
      return status;

    } else { 
    // There is sufficient free space in the bucket. Insert (value, rid) here

    int offset = (bucket->slotCnt) * recSize;
    memcpy(&(bucket->data[offset]), value, headerPage->length);
    memcpy(&(bucket->data[offset+headerPage->length]), &rid, sizeof(RID));
    (bucket->slotCnt)++;
  }

  status = bufMgr->unPinPage(file, pageNo, true);
  return status;
}

#ifdef DEBUGIND
void Index::printDir() {

  cout << "printing directory...\n";
  cout << "depth is " << headerPage->depth << endl;
  for (int i = 0; i < dirSize; i++) 
    cout << i << "\tpoints to bucket " << headerPage->dir[i] << endl;
  cout << endl;
}
#endif 

// delete <attribute, rid> pair from the index. return OK if deleted
// return RECNOTFOUND if such a pair does not exist in the index

const Status Index::deleteEntry(const void* value, const RID & rid) 
{
  Status status;
  int index;
  Bucket* bucket;

  status = hashIndex(value, index);

  // read in the bucket that might have the entry in it
  int pageNo = headerPage->dir[index];
  status = bufMgr->readPage(file, pageNo, (Page*&)bucket);
  if (status != OK) 
    return status;

  // scan the bucket for the entry. Delete it if found
  for(int i = 0; i < bucket->slotCnt; i++) {
    status = matchRec(bucket, value, i);
    if (status == OK) {
      if (!memcmp(&rid, &(bucket->data[i*recSize + headerPage->length]), 
		 sizeof(RID))) {

	// the entry is found. Decrease the entry counts in the bucket
	// and copy the last entry in the bucket to the slot occupied
	// by the deleted entry

	(bucket->slotCnt)--;
	memcpy(&(bucket->data[i*recSize]), 
	       &(bucket->data[recSize*(bucket->slotCnt)]),recSize);
	status = bufMgr->unPinPage(file, pageNo, true);
	return status;
      }
    }
  }

  status = bufMgr->unPinPage(file, pageNo, false);
  if (status != OK)
    return status;
  return RECNOTFOUND;
}

// Test if the index entry in 'bucket' matches the attribute 'value.
// 'offset' indicates the location of the entry in the bucket

const Status Index::matchRec(const Bucket* bucket, 
			     const void* value, 
			     const int offset) 
{
  const char* tmp = &(bucket->data[offset*recSize]);

  switch(headerPage->type) {
  case INTEGER:
    if (*(int *)value == *(int *)tmp)
      return OK;
    break;
  case DOUBLE:
    {
       // Take care of byte misallignments 
       double inVal, inTmp;
       memcpy (&inVal, value, sizeof (double));
       memcpy (&inTmp, tmp, sizeof (double));
       // if (*(double *)value == *(double*)tmp) return OK;
       if (inVal == inTmp) return OK;
    }
    break;
  case STRING:
    if (!memcmp(value, tmp, headerPage->length))
      return OK;
    break;
  default:
    break;
  }
  return RECNOTFOUND;
}

#ifdef DEBUGIND
void Index::printBucs() {

  Bucket* bucket;

  cout << endl << "printing indices"<< endl;
  for (int i = 0; i < dirSize; i++) {
    int pageNo = headerPage->dir[i];
    cout << "page " << pageNo << ": ";
    Status status = bufMgr->readPage(file, pageNo, (Page*&)bucket);
    if (status != OK)
      cout << "error dumping indices"<<endl;
    else {
      for (int j = 0; j < bucket->slotCnt; j++)
	cout << *(int*)&(bucket->data[j*recSize]) << "\t";
      cout << endl;
    }
    status = bufMgr->unPinPage(file, pageNo, false);
  }
}
#endif 

// start a scan of the entries with attribute 'value'. return SCANTABFULL
// if too many scans are open at the same time

const Status Index::startScan(const void* scanValue)
{
  int hashvalue;

  curValue = scanValue;

  // read in and pin down the bucket containing entries with 'value'
  // in the buffer pool
  Status status = hashIndex(curValue, hashvalue);
  if (status != OK) 
    return status;

  int pageNo = curPageNo = headerPage->dir[hashvalue];
  status = bufMgr->readPage(file, pageNo, 
			    (Page*&)curBuc);
  if (status != OK) 
    return status;

  curOffset = 0;
  
  return OK;
}

// return the next entry with attribute 'value'. return NOMORERECS if
// it reaches the end of the bucket

const Status Index::scanNext(RID& outRid) 
{
  int& offset = curOffset;
  Bucket* buc = curBuc;
  const void* value = curValue;

  while (offset < buc->slotCnt) {
    if (matchRec(buc, value, offset) == OK)
      break;
    offset++;
  }

  if (offset == buc->slotCnt)
    return NOMORERECS;
  else {
    outRid = *(RID *)&(buc->data[offset*recSize + headerPage->length]);
    offset++;
    return OK;
  }
}

// unpin the bucket and clear the scan table entry.
const Status Index::endScan() 
{
  Status status;

  if (curBuc != NULL)
  {
    status = bufMgr->unPinPage(file, curPageNo, false);
    curBuc = NULL;
    return status;
  }
  else return OK;
}

/*
// find a free entry in the scan table
int Index::findFree()
{
  int i;
  for(i = 0; i < MAXOPENSCANS; i++)
  {
    if (openScans[i].valid == false) return i;
  }
  return -1;
}
*/
