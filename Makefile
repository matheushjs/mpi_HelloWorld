all: vecsum integral

vecsum: vecsum.c
	mpicc -Wall -O2 -Wno-unused-result vecsum.c -o vecsum

integral: integral.c
	mpicc -Wall -O2 -Wno-unused-result integral.c -o integral -lm

clean:
	find -name "*~" -type f -exec rm -vf '{}' \;
