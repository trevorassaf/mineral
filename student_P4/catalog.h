#ifndef CATALOG_H
#define CATALOG_H

#include "datatypes.h"
#include "heapfile.h"
// #include "index.h"

// -------------------- These enums are defined in datatypes.h -------------
// -- These are the data types that minirel understands
// -- enum Datatype { INTEGER=0, DOUBLE=1, STRING=2 };
// 
// -- A list of operations that are supported in predicates in minirel.
// -- enum Operator { LT, LTE, EQ, GTE, GT, NE, NOTSET };  // scan operators
// -------------------------------------------------------------------------

// define if debug output wanted
//#define DEBUGCAT


#define RELCATNAME   "relcat"           // name of relation catalog
#define ATTRCATNAME  "attrcat"          // name of attribute catalog
#define RELNAME      "relname"          // name of indexed field in rel/attrcat
#define MAXNAME      32                 // length of relName, attrName
#define MAXSTRINGLEN 255                // max. length of string attribute


// schema of relation catalog:
//   relation name : char(32)           <-- lookup key
//   attribute count : integer(4)
//   index count : integer(4)
typedef struct {
  char relName[MAXNAME];                // relation name
  int attrCnt;                          // number of attributes
  int indexCnt;                         // number of indexed attrs
} RelDesc;


// Description of an attribute. This structure is produced by the SQL
// parser and is used in two cases: In the "create relation" command 
// to send to the catalogs the name and type of the parsed attribute,
// and in the "insert into" command to send to the insert command
// the value of the attribute (using the attrValue field).
typedef struct {
  char relName[MAXNAME];                // relation name
  char attrName[MAXNAME];               // attribute name
  int  attrType;                        // INTEGER, DOUBLE, or STRING
  int  attrLen;                         // length of attribute in bytes

  void *attrValue;                      // ptr to binary value (used 
                                        //    by the parser for insert into 
                                        //    statements)
                                        //  [In some versions of the SQL 
                                        //   parser, attrValue is also used 
                                        //   to hold the default value 
                                        //   specified during the create table
                                        //   command]
                                        //   
} attrInfo; 


// The class implementing the Relation Catalogs. Inherited from the 
// heapfiles class that is used to store the relation catalog tuples.
class RelCatalog : public HeapFileScan {
 public:
  // open relation catalog
  RelCatalog(Status &status);

  // get relation descriptor for a relation
  const Status getInfo(const string & rName, RelDesc& record);

  // add information to catalog
  const Status addInfo(RelDesc & record);

  // remove tuple from catalog
  const Status removeInfo(const string & rName);

  // create a new relation
  const Status createRel(const string & rName, 
		   const int attrCnt, 
		   const attrInfo attrList[]);

  // destroy a relation
  const Status destroyRel(const string & rName);

  // add index to a relation
  const Status addIndex(const string & rName, const string & attrName);

  // drop index from a relation
  const Status dropIndex(const string & rName, const string & attrName);

  // print catalog information
  const Status help(const string & rName);          // relation may be NULL

  // get rid of catalog
  ~RelCatalog();
};


// schema of attribute catalog:
//   relation name : char(32)           <-- lookup keys
//   attribute name : char(32)          <--
//   attribute number : integer(4)
//   attribute type : integer(4)  (type is Datatype actually)
//   attribute size : integer(4)
//   index flag : integer(4)
typedef struct {
  char relName[MAXNAME];                // relation name
  char attrName[MAXNAME];               // attribute name
  int attrOffset;                       // attribute offset
  int attrType;                         // attribute type
  int attrLen;                          // attribute length
  int indexed;                          // TRUE if indexed
} AttrDesc;


// The class implementing the Attribute Catalogs. Inherited from the 
// heapfiles class that is used to store the attribute catalog tuples.
class AttrCatalog : public HeapFileScan {
 friend class RelCatalog;

 public:
  // open attribute catalog
  AttrCatalog(Status &status);

  // get attribute catalog tuple
  const Status getInfo(const string & rName, 
		       const string & attrName, 
		       AttrDesc &record);

  // add information to catalog
  const Status addInfo(AttrDesc & record);

  // remove tuple from catalog
  const Status removeInfo(const string & rName, const string & attrName);

  // get all attributes of a relation
  const Status getRelInfo(const string & rName, 
			  int &attrCnt, 
			  AttrDesc *&attrs);

  // delete all information about a relation
  const Status dropRelation(const string & rName);

  // add index to a relation
  const Status addIndex(const string & rName, const string & attrName);

  // drop index from a relation
  const Status dropIndex(const string & rName, const string & attrName);

  // close attribute catalog
  ~AttrCatalog();
};


// extern variables that are instantianted in the main program.
extern RelCatalog  *relCat;   // Pointer to the relational catalog object
extern AttrCatalog *attrCat;  // Pointer to the attribute catalog object
extern Error error;           // The system error manager.

#endif
