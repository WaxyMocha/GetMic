SHELL = /bin/bash

CXXFLAGS = -Wall -pthread -lstdc++fs -std=c++17
CXX = g++
INCLUDES = ../headers
LIBS = -lfftw3 -lportaudio

GetMic: GetMic.o settings.o thread.o CSV.o OPUS.o WAV.o
	${CXX} ${CXXFLAGS} -I${INCLUDES} -o ../$@ $? ${CXXFLAGS} ${LIBS}

GetMic.o: GetMic.cpp $(INCLUDES)/GetMic.h
	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<

settings.o: settings.cpp $(INCLUDES)/settings.h
	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<

thread.o: thread.cpp $(INCLUDES)/thread.h
	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<

CSV.o OPUS.o WAV.o:
	cd output; make -f Makefile
#
#CSV.o: output/CSV.cpp $(INCLUDES)/CSV.h
#	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
#
#OPUS.o: output/OPUS.cpp $(INCLUDES)/OPUS.h
#	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
#
#WAV.o: output/WAV.cpp $(INCLUDES)/WAV.h
#	${CXX} ${CXXFLAGS} -I${INCLUDES} -c $<
