/*
*   SONEA Andreea - 333CB
*           Tema 3 Algoritmi Paraleli si Distribuiti
*/


#include "header.h"

/*
*  Calculeaza coordonatele de inceput pentru rand si coloana in matricea 
*  Sudoku ale patratului corespunzator nodului.
*/
void findSquareCoord(int square, int np, int* rowStart, int* colStart) {
	int findSquare = 0;
	int i, j;
    int dimSudoku = sqrt(np);

	for (i = 0; i < np; i += dimSudoku) {
		for (j = 0; j < np; j += dimSudoku) {
			if (findSquare == square) {
				*rowStart = i;
				*colStart = j;
				return;
			}
			findSquare++;
		}
	}  
}


/*
* Functie recursiva ce rezolva patratul corespunzator nodului.
* Sunt gasite coordonatele de inceput ale patratului pe care il rezolva nodul
* (rand, coloana). Se rezolva sudoku pentru respectivul patrat cautandu-se 
* casute goale doar de la acele coordonate.
* Atunci cand nu se mai gaseste nicio casuta goala se adauga solutia in 
* solutiile partiale si return pentru a cauta o alta solutie.
* Daca s-a gasit spatiu liber se genereaza toate numerele intre 1 si numarul 
* de procese si se verifica pe rand, coloana si in patrat daca se poate pune.
* Este apelata recursiv functia. Se reinitializeaza cu 0 casuta.
*/
int solveSquare(Node* node, int np) {
    int row, col, rowStart, colStart;
    int solved = 1;
    int x;
 
 	findSquareCoord(node->square, np, &rowStart, &colStart);

	for (row = rowStart; row < rowStart + node->dimSudoku; row++) {
        for (col = colStart; col < colStart + node->dimSudoku; col++) {
            if (node->s[row * np + col] == 0) {
            	solved = 0;
                break;
            }
        }
        if (solved == 0) {
        	break;
        }
	}
    if (solved == 1) {
    	// adaugare solutie
        addSolution(node, np);
        return;
    }
    // se genereaza numere intre 1 si np
    for (x = 1; x <= np; x++) {

        // verifica daca se poate atribui valoare x in (row, col)
        if (checkRow(node, np, row, x) &&
            checkCol(node, np, col, x) &&
            checkSquare(node, np, rowStart, colStart, x)) {

                // atribuire valoare
                node->s[row * np + col] = x;

                // avansare in backtraking
                solveSquare(node, np);

                // reinitializam pentru backtraking
                node->s[row * np + col] = 0;
        }
    }
    return;
}


/*
* Verifica daca un numar x mai exista pe rand.
* 0 - exista
* 1 - nu exista
*/
int checkRow(Node* node, int np, int row, int x) {
    int col;
    for (col = 0; col < np; col++)
        if (node->s[row * np + col] == x)
            return 0;
    return 1;
}
 

/*
* Verifica daca un numar x mai exista pe coloana.
* 0 - exista
* 1 - nu exista
*/
int checkCol(Node* node, int np, int col, int x) {
    int row;
    for (row = 0; row < np; row++)
        if (node->s[row * np + col] == x)
            return 0;
    return 1;
}


/*
* Verifica daca un numar x mai exista in casuta. Se primesc coordonatele de
* inceput pentru casuta(rand, coloana).
* 0 - exista
* 1 - nu exista
*/
int checkSquare(Node* node, int np, int startRow, int startCol, int x) {
    int row, col;
    for (row = startRow; row < startRow + node->dimSudoku; row++) {
        for (col = startCol; col < startCol + node->dimSudoku; col++) {
            if (node->s[row * np + col] == x) {
                return 0;
            }
        }
    }
    return 1;
}


/*
* Adauga in solutiile partiale, sudoku rezolvat. Solutiile partiale sunt 
* matrici de sudoku cu casute rezolvate, memorate intr-un vector.
*/
void addSolution(Node* node, int np) {
    int i, j, nr;

    node->nrPartialSol++;
    nr = node->nrPartialSol - 1;
    for (i = 0; i < np; i++) {
        for (j = 0; j < np; j++) {
            // copiaza s[i,j] in partialSol[nr, i, j]
            node->partialSol[(nr*np*np) + (i*np+j)] = node->s[i*np+j];
        }
    }
    node->partialSol[node->l - 1] = node->nrPartialSol;
}



/*
* Afiseaza prima solutie partiala in fisier. Este apelata de radacina.
*/
void displayPartialSolutions(int* p, int nrPartialSol, int np, char* file) {
    int i, j, nr;
    int dim = sqrt(np);
    FILE* f = fopen(file, "w");

    fprintf(f, "%d\n", dim);
    for (nr = 0; nr < nrPartialSol; nr++) {
        for (i = 0; i < np; i++) {
            for (j = 0; j < np; j++) {
                fprintf(f, "%d ", p[(nr*np*np) + (i*np+j)]);
            }
            fprintf(f, "\n");  
        }
    }
    fclose(f);
}



/*
* Citeste dimensiunea sudoku si matricea incompleta din fisier.
* Este apelata de radacina.
*/

void readSudoku(Node* node, char* filename, int np) {
   FILE *file = fopen(filename, "r");
   int i, j;

   if (file != NULL) {
        fscanf(file, "%d", &node->dimSudoku);
        for (i = 0; i < np; i++) {
            for (j = 0; j < np; j++) {
                fscanf(file, "%d", &node->s[i*np+j]);
            }
        }
        fclose(file);
   }
}



/*
* Apelata doar de radacina. 
* Fiecare nod rezolva patratul corespunzator rank-ului(id-ului).
* Se trimite pe toate ramurile sudoku necompletat. Se rezolva patratul si se 
* retin toate solutiile partiale. Primeste solutii partiale de pe fiecare 
* ramura si se combina cu solutiile existente.
*/
void sudokuRoot(Node* node, int np) {
    int i, dest, rc;
    int tagSudoku = 2;
    int tagPartial = 3;
    MPI_Status Stat;

    // fiecare nod rezolva patratul corespunzator rank-ului(id-ului)
    node->square = node->id;

    // trimite pe toate ramurile sudoku necompletat
    for (i = 0; i < np; i++) {
        if (node->nrChilds[i] != 0) {
            dest = i;
            rc = MPI_Send(node->s, np*np, MPI_INT, dest, tagSudoku, 
                          MPI_COMM_WORLD);
        }   
    }  

    // rezolva patratul si adauga solutia partiala             
    solveSquare(node, np);

    // primeste solutii partiale de pe fiecare ramura
    for (i = 0; i < node->nrBranches; i++) {
        int* partialSolChild = (int *) calloc(node->l, sizeof(int));
        rc = MPI_Recv(partialSolChild, node->l, MPI_INT, 
                      MPI_ANY_SOURCE, tagPartial, MPI_COMM_WORLD, &Stat);
        int child = Stat.MPI_SOURCE;

        // combina solutiile primite cu solutiile existente
        combineAll(node, np, partialSolChild);

        free(partialSolChild);
    }
}



/*
* Apelata de noduri in afara de radacina. 
* Primeste sudoku necompletat de la parinte.
* Trimite pe toate ramurile sudoku necompletat. Rezolva patratul si retine
* toate solutiile partiale. Fiecare nod rezolva patratul corespunzator 
* rank-ului(id-ului). 
* Daca nu e frunza primeste solutii partiale de pe fiecare ramura si le combina
* cu solutiile existente. 
* Trimire noile solutii partiale parintelui.
*/
void sudokuNode(Node* node, int np) {
    int i, source, dest, rc;
    int tagSudoku = 2;
    int tagPartial = 3;
    float n = (float) np;
    MPI_Status Stat;

    // fiecare nod rezolva patratul corespunzator rank-ului(id-ului)
    node->square = node->id;
    node->dimSudoku = sqrt(n);

    // primeste sudoku necompletat de la parinte
    source = node->parent;
    rc = MPI_Recv(node->s, np*np, MPI_INT, source, tagSudoku, 
                  MPI_COMM_WORLD, &Stat);

    // trimite pe fiecare ramura sudoku necompletat(parintele nu e ramura)
    for (i = 0; i < np; i++) {
        if (node->nrChilds[i] != 0) {
            dest = i;
            rc = MPI_Send(node->s, np*np, MPI_INT, dest, tagSudoku, 
                          MPI_COMM_WORLD);
        }   
    }

    // rezolva patratul si adauga solutia partiala 
    solveSquare(node, np);

    if (node->nrBranches != 0) {
            // primeste solutii partiale de pe fiecare ramura
            for (i = 0; i < node->nrBranches; i++) {
                int* partialSolChild = (int *)calloc(node->l, sizeof(int));
                rc = MPI_Recv(partialSolChild, node->l, MPI_INT, MPI_ANY_SOURCE
                              ,tagPartial, MPI_COMM_WORLD, &Stat);
                int child = Stat.MPI_SOURCE;

                // combina solutiile primite cu solutiile existente
                combineAll(node, np, partialSolChild);

                free(partialSolChild);
            }
    }
    // trimite noile solutii partiale catre parinte
    dest = node->parent;
    rc = MPI_Send(node->partialSol, node->l, MPI_INT, dest, tagPartial, 
                  MPI_COMM_WORLD);
}



/*
* Primeste un nod si solutiile partiale ale unei ramuri. 
* Se copiaza solutiile partiale ale nodului si se goleste vectorul cu solutiile
* partiale ale nodului.
* Se ia fiecare matrice sudoku din solutiile partiale ale ramurei si se copiaza
* pe rand in matricea sudoku a nodului. Pentru fiecare matrice a ramurei se ia
* fiecare solutie partiala a nodului si se copiaza in casutele goale ale
* solutiei, valoare corespunzatoare din solutia partiala a nodului.
* Se verifica pe linie si pe coloana daca solutia e valida si daca da se adauga 
* in solutia partiala a nodului. 
* Se restabileste matricea solutiei ca fiind solutia partiala curenta a ramurei
* pentru a putea combina si cu restul solutiilor partiale ale nodului.
*/
void combineAll(Node* node, int np, int* partial) {
    int r, c, nr1, nr2;
    int dim = node->dimSudoku;
    int nrP1 = node->nrPartialSol;
    int nrP2 = partial[node->l - 1];
    int* nodePartialSol = node->partialSol;

    node->partialSol = (int *)calloc(node->l, sizeof(int));
    node->nrPartialSol = 0;

    for (nr2 = 0; nr2 < nrP2; nr2++) {
        // copiaza in sudoku matricea ramurei
        for (r = 0; r < np; r++) {
            for (c = 0; c < np; c++) {
                node->s[r*np+c] = partial[nr2* np*np + r*np+c];
            }
        }

        for (nr1 = 0; nr1 < nrP1; nr1++) {
            // copiaza in sudoku in casute goale matricea nodului
            for (r = 0; r < np; r++) {
                for (c = 0; c < np; c++) {
                    if (node->s[r*np+c] == 0) {
                        node->s[r*np+c] = nodePartialSol[nr1*np*np + r*np+c];
                    }
                }
            }
            // verifica daca combinarea matricilor e solutie valida
            if (validSolution(node, np) == 1) {
                addSolution(node, np);
            }
            // restabileste valoarea -> matricea ramurei fara imbinarea cu
            // matricea nodului
            for (r = 0; r < np; r++) {
                for (c = 0; c < np; c++) {
                    node->s[r*np+c] = partial[nr2* np*np + r*np+c];
                }
            }
        }
    }

    free(nodePartialSol);
}


/*
* Verifica daca matricea cu solutia sudoku memorata de nod este valida,
* verificandu-se pe linii si pe coloane daca exista duplicate. In casuta 
* nu mai este necesara verificarea pentru ca a fost facuta la generare.
*/
int validSolution(Node* node, int np) {
    int row, col;
    int x;

    for (row = 0; row < np; row++) {
        for (col = 0; col < np; col++) {
            x = node->s[row*np+col];
            if (x != 0) {
                node->s[row*np+col] = 0;
                if (!checkCol(node, np, col, x) || 
                    !checkRow(node, np, row, x)) {
                            return 0;
                }
                node->s[row*np+col] = x;
            }
        }
    }
    return 1;
}