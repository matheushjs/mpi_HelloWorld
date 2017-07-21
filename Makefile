all:
	mpicc -Wall -O2 -Wno-unused-result vecsum.c -o vecsum

clean:
	find -name "*~" -type f -exec rm -vf '{}' \;
