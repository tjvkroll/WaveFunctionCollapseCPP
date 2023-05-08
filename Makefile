CPP = g++ 

# add --coverage to CFLAGS for gcov

CDEFS=
CFLAGS= -g -O2 -std=c++17 -fopenmp -Wall $(INCLUDE_DIRS) $(CDEFS)
LFLAGS= -c -g -O2 -std=c++17 -fopenmp -Wall $(INCLUDE_DIRS) $(CDEFS)

LIBS=

PRODUCT= DensityFinder 

CFILES= DensityFinder.cpp wfc.cpp block.cpp

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	${PRODUCT}

clean:
	-rm -f *.o *.NEW *~ *.gcov *.gcda *.gcno
	-rm -f ${PRODUCT} ${DERIVED} ${GARBAGE}

DensityFinder: DensityFinder.cpp wfc.o block.o
	$(CPP) $(CFLAGS) -o $@ $^ $(LIB_DIRS) -lm

wfc.o: wfc.cpp wfc.h block.o
	$(CPP) $(LFLAGS) $<

block.o: block.cpp block.h
	$(CPP) $(LFLAGS) $<
