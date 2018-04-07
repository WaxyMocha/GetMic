SHELL = /bin/bash

CXXFLAGS = -Wall -lrt -lm -lasound -ljack -pthread -lstdc++fs -std=c++17 -Iheaders
CXX = g++
LIBS = -lfftw3 -lportaudio

GetMic: $(patsubst %.cpp,%.o,$(shell find source -type f -name "*.cpp"))
	${CXX} ${CXXFLAGS} -o $@ $? ${CXXFLAGS} ${LIBS} 

clean:
	@find . -type f -name '*.o' -delete
	@rm GetMic
