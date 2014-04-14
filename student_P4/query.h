#ifndef QUERY_H
#define QUERY_H

#include "heapfile.h"
#include "index.h"

//
// Prototypes for query layer functions
//

//
// The class for encapsulating the query operators: selects and joins
// Projections are folded into the selects and joins
//
class Operators
{

public:
  // The select operator
  static Status Select(const string & result,      // name of the output relation
	               const int projCnt,          // number of attributes in the projection
		       const attrInfo projNames[], // the list of projection attributes
		       const attrInfo *attr,       // attribute used inthe selection predicate 
#ifdef BTREE_INDEX
		       const Operator op, 
		       const void *attrValue,
		       const Operator op2 = NOTSET, 
		       const void *attrValue2 = 0);
#else // hash index only
		       const Operator op,         // predicate operation
		       const void *attrValue);    // literal value in the predicate
#endif // BTREE_INDEX

   // The join operator
   static Status Join(const string & result,      // name of the output relation 
                      const int projCnt,          // number of attributes in the projection
	              const attrInfo projNames[], // the list of projection attributes
	              const attrInfo *attr1,      // left attr in the join predicate
	              const Operator op,          // the predicate operation 
	              const attrInfo *attr2);     // right attr in the join predicate


private: 

   // A simple scan select using a heap file scan
   static Status ScanSelect(const string & result,      // name of the output relation
	                    const int projCnt,          // number of attributes in the projection
                            const AttrDesc projNames[], // The projection list (as AttrDesc)
                            const AttrDesc *attrDesc,   // the attribute in the selection predicate
                            const Operator op,          // the predicate operation
                            const void *attrValue,      // a pointer to the literal value in the predicate
                            const int reclen);          // length of a tuple in the result relation

   // Select using an index
   static Status IndexSelect(const string & result,      // name of the output relation
  	                     const int projCnt,          // number of attributes in the projection
                             const AttrDesc projNames[], // The projection list (as AttrDesc)
                             const AttrDesc *attrDesc,   // the attribute in the selection predicate
                             const Operator op,          // the predicate operation
                             const void *attrValue,      // a pointer to the literal value in the predicate
#ifdef BTREE_INDEX
                             const Operator op2,
                             const void *attrValue2,
#endif // BTREE_INDEX
                             const int reclen);          // length of a tuple in the result relation

   // Function to match two record based on the predicate. Returns 0 if the two attributes 
   // are equal, a negative number if the left (attrDesc1) attribute is less that the right 
   // attribute, otherwise this function returns a positive number.
   static int matchRec(const Record & outerRec,     // Left record
                       const Record & innerRec,     // Right record
                       const AttrDesc & attrDesc1,  // Left attribute in the predicate
                       const AttrDesc & attrDesc2); // Right attribute in the predicate

   // The various join algorithms are declared below.
   // Simple nested loops
   static Status SNL(const string & result,          // output relation name
	             const int projCnt,              // number of attributes in the projection
                     const AttrDesc attrDescArray[], // The projection list (as AttrDesc)
                     const AttrDesc & attrDesc1,     // The left attribute in the join predicate
                     const Operator op,              // The join operation
                     const AttrDesc & attrDesc2,     // The left attribute in the join predicate
                     const int reclen);              // The lenght of a tuple in the result relation

   // indexed nested loops
   static Status INL(const string & result,          // output relation name
	             const int projCnt,              // number of attributes in the projection
                     const AttrDesc attrDescArray[], // The projection list (as AttrDesc)
                     const AttrDesc & attrDesc1,     // The left attribute in the join predicate
                     const Operator op,              // The join operation
                     const AttrDesc & attrDesc2,     // The left attribute in the join predicate
                     const int reclen);              // The lenght of a tuple in the result relation

   // Sort-merge join algorithm
   static Status SMJ(const string & result,          // output relation name
	             const int projCnt,              // number of attributes in the projection
                     const AttrDesc attrDescArray[], // The projection list (as AttrDesc)
                     const AttrDesc & attrDesc1,     // The left attribute in the join predicate
                     const Operator op,              // The join operation
                     const AttrDesc & attrDesc2,     // The left attribute in the join predicate
                     const int reclen);              // The lenght of a tuple in the result relation
};


// The class encapsulating the insert and delete operators
class Updates
{
public:
   // The insert operator
   static Status Insert(const string & relation,    // the relation into which to insert the tuple
	   	        const int attrCnt,          // number of attributes in the attrList
		        const attrInfo attrList[]); // the attribute list (with the attribute name and value)

   // The delete operator
   static Status Delete(const string & relation,    // the relation into which to insert the tuple
		        const string & attrName,    // the attribute in the predicate
		        const Operator op,          // the predicate operation
		        const Datatype type,        // the predicate type
		        const void *attrValue);     // the literal used in the predicate
}; 



#endif
