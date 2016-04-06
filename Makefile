all: build

build: main.c header.h node.c sudoku.c
	mpicc -o main main.c header.h node.c sudoku.c -lm

run:
	mpirun -np 4 ./main "topologie" "sudoku/2/2-1.txt" "out.txt"

clean:
	rm main
	rm out.txt

