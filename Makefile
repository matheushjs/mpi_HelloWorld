all: vecsum integral pingpong

vecsum: vecsum.c
	mpicc -Wall -O2 -Wno-unused-result vecsum.c -o vecsum

integral: integral.c
	mpicc -Wall -O2 -Wno-unused-result integral.c -o integral -lm

pingpong: pingpong.c
	mpicc -Wall -O2 -Wno-unused-result pingpong.c -o pingpong

clean:
	find -name "*~" -type f -exec rm -vf '{}' \;
