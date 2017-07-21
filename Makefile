all:
	mpicc -Wall -O2 -Wno-unused-result prog.c -o prog

clean:
	find -name "*~" -type f -exec rm -vf '{}' \;
