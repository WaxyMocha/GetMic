SHELL = /bin/bash

CXXFLAGS = -Wall -pthread -lstdc++fs -std=c++17 -I../headers
CXX = g++
LIBS = -lfftw3 -lportaudio

GetMic: $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.cpp,%.o,$(wildcard */*.cpp))
	${CXX} ${CXXFLAGS} -o ../$@ $? ${CXXFLAGS} ${LIBS} 


#CSV.o OPUS.o WAV.o:
#	cd output; make -f Makefile
#
#CSV.o: output/CSV.cpp $(INCLUDES)/CSV.h
#	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
#
#OPUS.o: output/OPUS.cpp $(INCLUDES)/OPUS.h
#	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
#
#WAV.o: output/WAV.cpp $(INCLUDES)/WAV.h
#	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
