/*
*	SONEA Andreea - 333CB
* 			Tema 3 Algoritmi Paraleli si Distribuiti
*/


#include "header.h"




main(int argc, char *argv[])  {
	int np, rank;  
	MPI_Status Stat;
	Node node;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// initializeaza datele si aloca memorie vectorilor
	init(&node, np, rank);

	// citeste lista adiacenta
	readAdList(&node, argv[1], np);

	// calculare topologie
	if (rank == ROOT) {
		topologyRoot(&node, np);
	}
	else {
			topologyNode(&node, np);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	// afiseaza nextHop
	displayNextHop(node, np);

	if (rank == ROOT) {
		readSudoku(&node, argv[2], np);
		sudokuRoot(&node, np);
        displayPartialSolutions(node.partialSol, 1, np, argv[3]);
	}
	else {
			sudokuNode(&node, np);
	}

	MPI_Finalize();

}