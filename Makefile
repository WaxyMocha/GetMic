all: fftw3 Portaudio

fftw3:
	chmod +x ./fftw/configure
	cd fftw; ./configure --disable-doc
	cd fftw; make -f ./Makefile
	cd fftw; make install -f ./Makefile DESTDIR=$(shell pwd)/fftw/build prefix=""

Portaudio:
	chmod +x ./portaudio/configure
	cd portaudio; ./configure
	cd portaudio; make -f ./Makefile
	cd portaudio; make install -f ./Makefile DESTDIR=$(shell pwd)/portaudio/build prefix="_"

clean:
	cd fftw; make clean -f ./Makefile
	cd fftw; rm -fr build

	cd portaudio; make clean -f ./Makefile
	cd portaudio; rm -fr build_
