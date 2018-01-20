all: fftw3 Portaudio

fftw3:
	chmod +x ./fftw/configure
	cd fftw; ./configure
	cd fftw; make -f ./Makefile

Portaudio:
	chmod +x ./portaudio/configure
	cd portaudio; ./configure
	cd portaudio; make -f ./Makefile
