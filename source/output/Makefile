SHELL = /bin/bash

CXXFLAGS = -Wall -pthread -lstdc++fs -std=c++17
CXX = g++
INCLUDES = ../../headers
LIBS = -lfftw3 -lportaudio

all: CSV.o OPUS.o WAV.o

CSV.o: CSV.cpp $(INCLUDES)/CSV.h
	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<

OPUS.o: OPUS.cpp $(INCLUDES)/OPUS.h
	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<

WAV.o: WAV.cpp $(INCLUDES)/WAV.h
	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
