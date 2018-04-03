all:
	cd source; make -f Makefile

clean:
	@find . -type f -name '*.o' -delete
