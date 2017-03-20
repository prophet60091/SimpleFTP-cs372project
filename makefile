CFLAGS =  -fpic -lm

CXX = g++
CXXFLAGS += -Wall #turn on all warnings
CXXFLAGS += -pedantic-errors #enforce the standard
#CXXFLAGS += -g #debugging
#LDFLAGS = -libboost_date_time #libraries

#dependencies
OBJS = adventure.o

# The files to be compiled
SRCS =  adventure.c

#headers to be compiled
HEADERS =

#target: dependencies
#       rule to build
#
#Set the following to the name fo the output file ie file: or set default as well
adv: ${OBJS} ${HEADERS}
	${CXX} ${LDFLAGS} ${OBJS} -o main

${OBJS}: ${SRCS}
	${CXX} ${CXXFLAGS} -c $(@:.o=.c)


clean:
	rm -f *.o *.gcov *.gcda *.gcno *.so
