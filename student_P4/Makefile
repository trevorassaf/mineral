
# Makefile for Minirel
#

.SUFFIXES: .o .C

#
# Compiler and loader definitions
#

LD =		ld
LDFLAGS =	

CXX =		g++
CXXFLAGS =	-g -Wall -DDEBUG 
# CXXFLAGS =	-O -Wall -DDEBUG 

MAKEFILE =	Makefile

# Comment out if purify not desired
PURIFY =        purify -collector=/usr/ccs/bin/ld -g++

#
# list of all object and source files
#

# all the source files in this project
SRCS =		error.C heapfile.C index.C print.C insert.C select.C \
		scanselect.C indexselect.C snl.C smj.C inl.C join.C sort.C

# source files on which to run  make depend 
DSRCS =		error.C print.C insert.C select.C \
		scanselect.C indexselect.C snl.C smj.C inl.C join.C sort.C

# object files to link in to create the minirel program
MROBJS =	error.o heapfile.o index.o print.o insert.o select.o \
		scanselect.o indexselect.o snl.o smj.o inl.o join.o sort.o

# object files to link in to create the dbcreate program
DBOBJS =	print.o error.o heapfile.o index.o

# the libraries that are provided for this assignment
LIBS =		libsql.a libcat.a libmisc.a liblsm.a 
LIBSEC = 	libsql.a libcat.a libmisc.a libEC.a 

# rules for making the various executables
all:		minirel dbcreate dbdestroy

EC:		minirelEC dbcreateEC dbdestroyEC

minirel:	minirel.o $(MROBJS) $(LIBS)
		$(CXX) -o $@ $@.o $(MROBJS) $(LIBS) $(LDFLAGS) -lm

dbcreate:	dbcreate.o $(DBOBJS) liblsm.a libcat.a
		$(CXX) -o $@ $@.o $(DBOBJS) liblsm.a libcat.a $(LDFLAGS) -lm

dbdestroy:	dbdestroy.o
		$(CXX) -o $@ $@.o

minirelEC:	minirelEC.o $(MROBJS) $(LIBSEC) page.h
		$(CXX) -o $@ $@.o $(MROBJS) $(LIBSEC) page.cpp $(LDFLAGS) -lm

dbcreateEC:	dbcreateEC.o $(DBOBJS) libEC.a libcat.a page.h
		$(CXX) -o $@ $@.o $(DBOBJS) libEC.a libcat.a page.cpp $(LDFLAGS) -lm

dbdestroyEC:	dbdestroyEC.o
		$(CXX) -o $@ $@.o

#minirel.pure:	minirel.o $(MROBJS) $(LIBS)
#		$(PURIFY) $(CXX) -o $@ minirel.o $(MROBJS) $(LIBS) $(LDFLAGS) -lm
#
#dbcreate.pure:	dbcreate.o $(DBOBJS) $(LIBS)
#		$(PURIFY) $(CXX) -o $@ dbcreate.o $(DBOBJS) $(LDFLAGS) -lm

.C.o:
		$(CXX) $(CXXFLAGS) -c $<

clean:
		rm -f core *.bak *~ $(MROBJS) minirel.o dbcreate.o dbdestroy.o minirelEC.o dbcreateEC.o dbdestroyEC.o minirel dbcreate dbdestroy minirelEC dbcreateEC dbdestroyEC *.pure

depend:
	makedepend 	-I/usr/um/gnu/gcc/include/g++-3 \
			-I/usr/um/gnu/gcc/sparc-sun-solaris2.6/include \
			-I/usr/um/gnu/gcc/lib/gcc-lib/sparc-sun-solaris2.6/2.95.3/include\
			-f$(MAKEFILE) $(DSRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

