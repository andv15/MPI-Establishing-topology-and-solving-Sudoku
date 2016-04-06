/*
*	SONEA Andreea - 333CB
* 			Tema 3 Algoritmi Paraleli si Distribuiti
*/


#include "header.h"



/*
* Initializeaza valorile pentru id, nr solutii partiale, nr ramuri si aloca
* memorie pentru vectorii: adiacenta, nextHop, nr copii pe fiecare ramura, 
* solutii partiale
*/
void init(Node *node, int np, int rank) {

	node->a = (int *)calloc(np * np, sizeof(int));
	node->nrChilds = (int *)calloc(np + 1, sizeof(int));
	node->nextHop = (int *)calloc(np, sizeof(int));
	node->nrBranches = 0;
	node->id = rank;

	// alocare matrice sudoku
	node->s = (int *)calloc(np * np, sizeof(int));
	node->l = 10000000;
	node->partialSol = (int *)calloc(node->l, sizeof(int));
	node->nrPartialSol = 0;
}



/*
* Citeste linie cu linie listele de adiacenta si isi insuseste doar linia
* corespunzatoare id. Completeaza in matricea de adiacenta linia procesului
* curent
*/
void readAdList(Node *node, char* filename, int np) {

   FILE *file = fopen(filename, "r");
   int nrLine = 0;
   char line[128];
   char* token;

   if (file != NULL) {
      while (fgets(line, sizeof(line), file) != NULL) {
      	if ( nrLine == node->id && line != NULL) {
      		// am gasit linia din fisierul listei de adiacenta pentru proces
      		// citeste rank-ul(id nodului)
         	token = strtok (line, " -");
         	token = strtok (NULL, " -\n");
         	while(token != NULL) {
         		int neighbor = atoi(token);
         		node->a[node->id * np + neighbor] = 1;
				token = strtok (NULL, " -\n");
         	}
      	}
        nrLine++;
      }
      fclose(file);
   }
}



/*
* Functie recursiva ce calculeaza numarul de copii ai nodului primit.
* Este apelata recursiv pentru toate nodurile cu care are legatura in
* afara de parinte.
*/
int recursiveCalc(int* a, int id, int parent, int np) {
	int i, aux = 0;

	for (i = 0; i < np; i++) {
		if (a[id*np + i] == 1 && i != parent)  {
			aux += recursiveCalc(a, i, id, np);
		}
	}
	return 1 + aux;
}




/*
* Calculeaza numarul de copii pe fiecare ramura a nodului. Pentru fiecare 
* ramura este apelata o functie recursiva ce calculeaza numarul de copii 
* a ramurei respective. Parintele nu este considerat o ramura.
*/
void calculation(Node* node, int np) {
	int i;

	for (i = 0; i < np; i++) {
		node->nrChilds[i] = 0;
		if (node->a[node->id * np + i] == 1 && i != node->parent) {
			node->nrChilds[i] += recursiveCalc(node->a, i, node->id, np);
		}
	}
}


/*
* Adauga recursiv intrari in tabela de rutare. Primeste un copil al nodului
* si adauga nextHop. Este apelata recursiv pentru toate ramurile copilului.
*/
void recursiveAddNextHop(Node* node, int* a, int idChild, int parent, 
						 int np, int nextHop) {
	int i;

	for (i = 0; i < np; i++) {
		if (a[idChild*np + i] == 1 && i != parent)  {
			node->nextHop[i] = nextHop;
			recursiveAddNextHop(node, a, i, idChild, np, nextHop);
		}
	}
	return;
}


/*
* Adauga in tabela de rutare nodurile la care se poate ajunge printr-o ramura.
* Este adaugat intrarea pentru ramura primita si este apelata o functie 
* recursiva ce adauga intrari pentru ramurile copilului si asmd.
*/
void addNextHop(Node* node, int np, int* aChild, int idChild) {
	int i;

	node->nextHop[idChild] = idChild;
	for (i = 0; i < np; i++) {
		if (aChild[idChild*np+i] == 1 && i != node->id){
			node->nextHop[i] = idChild;
			recursiveAddNextHop(node, aChild, i, idChild, np, idChild);
		}
	}
}


/*
* Adauga in tabela de rutare ruta catre parinte si rute prin parinte
* catre toate nodurile care nu au rute stabilite in urma adaugarii 
* tuturor nodurilor in se putea ajunge prin ramuri.
* -1 pentru el insusi
*/
void doNextHop(Node* node, int np) {
	int i;

	// adaugare ruta catre parinte
	node->nextHop[node->parent] = node->parent;
	// ruta catre el insusi e -1(eroare)
	node->nextHop[node->id] = -1;

	// in toate nodurile pe care nu le are ca vecini si nici nu se pot
	// ajunge la ele prin vecini se va ajunge prin parinte
	for (i = 0; i < np; i++) {
		if (node->nextHop[i] == 0) {
			node->nextHop[i] = node->parent;
		}
	}
}


/*
* Afiseaza vectorul nextHop reprezentand nodul prin care se poate
* ajunge la fiecare nod din retea.
*/
void displayNextHop(Node node, int np) {
	int i;
	char* buff = (char *)calloc(10*np, sizeof(char));
	char str[15];

	// id
	sprintf(str, "id = %d nextHop: ", node.id);
	strcpy(buff, str);

	for (i = 0; i < np; i++) {
		sprintf(str, "%d ", node.nextHop[i]);
		strcat(buff, str);
	}

	// afiseaza nextHop
	printf("%s\n", buff);
	free(buff);
}



/*
* Apelata doar de procesul cu id 0. 
* Trimite mesaj de sondaj pe toate ramurile. Mesajul de sondaj este o 
* topologie goala care are primul element = SONDAJ(1024). 
*
* Asteapta mesaj cu topologie de pe fiecare ramura. Daca se primeste un 
* mesaj de SONDAJ este stearsa legatura cu ramura din matricea de adiacenta.
* Nu se mai trimite nimic inapoi pentru ca ramura asteapta la randul ei 
* topologia si primeste sondaj pentru ca era considerata ramura.
* Daca e topologie adauga datele din matricea de adiacenta primita.

* Afiseaza in fisier matricea completa, calculeaza pe fiecare ramura cati 
* copii are si completeaza tabela de rutare nextHop.
*/
void topologyRoot(Node* node, int np) {
	int i, j, x;
	int* aChild = (int *)calloc(np * np, sizeof(int));
	int child;

	int rank = node->id;
	int dest, source, tag = 1, rc;  
	MPI_Status Stat;

	node->parent = -1;
	// trimite sondaj la vecini
	for (i = 0; i < np; i++) {
		if (node->a[i] == 1) {
			node->nrBranches++;
			dest = i;
			// send sondaj -> topologie goala cu SONDAJ
			aChild[0] = SONDAJ;
			rc = MPI_Send(aChild, np*np, MPI_INT, dest, tag, MPI_COMM_WORLD);
		}	
	}

	// asteapta de pe fiecare ramura matricea
	int nrc = node->nrBranches;
	while (nrc > 0) {
		rc = MPI_Recv(aChild, np*np, MPI_INT, 
					  MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &Stat);
		child = Stat.MPI_SOURCE;

		if (aChild[0] == SONDAJ) {
			// sterge copilul(ciclu)
			node->a[rank * np + child] = 0;
			node->nrBranches--;
		}
		else {
				// adauga next hop pentru ramura de pe care a primit matricea
				addNextHop(node, np, aChild, child);

				// adauga matricea de adiacenta primita
				for (i = 0; i < np; i++) {
					for (j = 0; j < np; j++) {
						node->a[i*np+j] = node->a[i*np+j] || aChild[i*np+j];
					}
				}
		}
		nrc--;
	}
		
	// afiseaza matricea
	displayMatrix(node->a, np);

	// calculare pe fiecare ramura cati copii are
	calculation(node, np);

	// nextHop pentru el insusi
	node->nextHop[node->id] = -1;

	free(aChild);
}



/*
* Apelata de procese cu id diferit de 0. 
* Asteapta sondaj de la parinte.
* Trimite mesaj de sondaj pe toate ramurile mai putin parintelui. Mesajul 
* de sondaj este o topologie goala care are primul element = SONDAJ(1024).
*
* Daca e frunza si nu are ramuri in afara de parinte trimite topologia sa
* parintelui.
*
* Asteapta mesaj cu topologie de pe fiecare ramura. Daca se primeste un 
* mesaj de SONDAJ este stearsa legatura cu ramura din matricea de adiacenta.
* Nu se mai trimite nimic inapoi pentru ca ramura asteapta la randul ei 
* topologia si primeste sondaj pentru ca era considerata ramura.
* Daca e topologie adauga datele din matricea de adiacenta primita.

* Calculeaza pe fiecare ramura cati copii are si completeaza tabela de 
* rutare nextHop.
*/
void topologyNode(Node* node, int np) {
	int i, j, x;
	int* aChild = (int *)calloc(np * np, sizeof(int));
	int child;

	int rank = node->id;
	int dest, source, tag = 1, rc;  
	MPI_Status Stat;

	// asteapta sondaj de la parinte
	rc = MPI_Recv(aChild, np*np, MPI_INT, MPI_ANY_SOURCE, tag, 
				  MPI_COMM_WORLD, &Stat);
	node->parent = Stat.MPI_SOURCE;

	// trimite sondaj la vecini mai putin la parinte
	for (i = 0; i < np; i++) {
		if (node->a[rank * np + i] == 1 && i != node->parent) {
			dest = i;
			aChild[0] = SONDAJ;
			rc = MPI_Send(aChild, np*np, MPI_INT, dest, tag, MPI_COMM_WORLD);
			node->nrBranches++;
		}			
	}

	if (node->nrBranches == 0) {
		// trimite topologia la parinte daca e frunza
		dest = node->parent;
		rc = MPI_Send(node->a, np*np, MPI_INT, dest, tag, MPI_COMM_WORLD);	
	}
	else {
			// asteapta de la copii SONDAJ sau TOPOLOGIE
			int nrc = node->nrBranches;
			while (nrc > 0) {
				rc = MPI_Recv(aChild, np*np, MPI_INT, 
							  MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &Stat);
				child = Stat.MPI_SOURCE;
				if (aChild[0] == SONDAJ) {
					// sterge copilul pentru ca a primit sondaj
					node->a[rank * np + child] = 0;
					node->nrBranches--;
				}
				else {
						// adauga next hop pentru ramura de pe care a primit
						// matrice cu topologie
						addNextHop(node, np, aChild, child);
						
						// adauga datele noi in topologie
						for (i = 0; i < np; i++) {
							for (j = 0; j < np; j++) {
								node->a[i*np+j] = node->a[i*np+j] || 
												  aChild[i*np+j];
							}
						}
				}
				nrc--;
			}

			// trimite catre parinte matricea cu topologie
			dest = node->parent;
			rc = MPI_Send(node->a, np*np, MPI_INT, dest, tag, MPI_COMM_WORLD);
	}
	// calculare pe fiecare ramura cati copii are
	calculation(node, np);

	// calculare nextHop pentru nodurile ce se ajunge prin parinte
	// -1 pentru el insusi
	doNextHop(node, np);

	free(aChild);
}


/*
* Afiseaza o matrice primita ca parametru.
*/
void displayMatrix(int* m, int np) {
	int i, j;

	for (i = 0; i < np; i++) {
		for (j = 0; j < np; j++) {
			printf("%d ", m[i*np+j]);
		}
		printf("\n");
	}
	printf("\n");		
}
