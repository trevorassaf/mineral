#ifndef UTILITY_H
#define UTILITY_H


#include <string>
#include "error.h"

//
// Prototypes for utility layer functions
//
class Utilities
{
public:

   // A bulkload utility
   static Status Load(const string & relation,      // the relation being loaded 
                      const string & fileName);     // the file containing the data for loading.

   // A utility to print a relation
   static Status Print(string relation);
   
   // Quit the database and perform any necessary cleanup
   static void Quit(void);

};

#endif
