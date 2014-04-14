#ifndef DATATYPES_H
#define DATATYPES_H

// These are the data types that minirel understands
/*
enum DataType {
      SQLChar,      // SQL Char data type
      SQLInteger,   // SQL Integer data type
      SQLDouble     // SQL double data type
   };
*/

// Given out in part2
enum Datatype {INTEGER=0, DOUBLE=1, STRING=2 };

// A list of operations that are supported in predicates in minirel.
enum Operator { LT, LTE, EQ, GTE, GT, NE, NOTSET };  // scan operators

#endif // DATATYPES_H

