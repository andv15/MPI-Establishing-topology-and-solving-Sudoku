/*
*	SONEA Andreea - 333CB
* 			Tema 3 Algoritmi Paraleli si Distribuiti
*/

#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define SONDAJ 1024
#define END	   512
#define ROOT   0



typedef struct node {

	int *a;				// matrice adiacenta
	int parent;			// parinte
	int *nrChilds; 		// nr de copii pe fiecare ramura:
						//		1 - ramura ce duce in frunza
						//		0 - nu exista ramura catre acel nod
						//	  > 1 - ramura duce in nod ce are alte ramuri
	int *nextHop;		// tabela rutare
	int nrBranches;		// nr de ramuri
	int id;				// nr procesului

	int dimSudoku;		// dimensiune sudoku
	int *s;				// matrice sudoku
	int *partialSol;	// matrici de solutii partiale
	int square;			// id-ul patratului la care lucreaza
	long l;				// dimensiunea maxima a solutiei partiale
	int nrPartialSol;	// nr de solutii partiale

} Node;


void init(Node *node, int np, int rank);
void readAdList(Node *node, char* filename, int np);
void topologyRoot(Node* node, int np);
void topologyNode(Node* node, int np);

int  recursiveCalc(int* a, int id, int parent, int np);
void calculation(Node* node, int np);

void addNextHop(Node* node, int np, int* aChild, int child);
void doNextHop(Node* node, int np);
void displayNextHop(Node node, int np);

void displayMatrix(int* m, int np);


void readSudoku(Node* node, char* filename, int np);
void sudokuRoot(Node* node, int np);
void sudokuNode(Node* node, int np);
void findSquareCoord(int square, int np, int* rowStart, int* colStart);
int  solveSquare(Node* node, int np);
int  checkRow(Node* node, int np, int row, int x);
int  checkCol(Node* node, int np, int col, int x);
int  checkSquare(Node* node, int np, int startRow, int startCol, int x);
void addSolution(Node* node, int np);
void displayPartialSolutions(int* p, int nrPartialSol, int np, char* file);
void combineAll(Node* node, int np, int* partial);
int  validSolution(Node* node, int np);