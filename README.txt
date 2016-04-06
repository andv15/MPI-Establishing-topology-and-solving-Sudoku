/*
*	SONEA Andreea - 333CB
* 			Tema 3 Algoritmi Paraleli si Distribuiti
*/


	CREARE TOPOLOGIE

	- initializeaza date: id proces, nr solutii partiale, nr ramuri si aloca memorie pentru 
	vectori: adiacenta, nextHop, nr copii pe fiecare ramura, solutii partiale

	- citeste linie cu linie listele de adiacenta si isi insuseste doar linia corespunzatoare id

	- daca nu e radacina, asteapta sondaj de la parinte

	- trimite mesaj de sondaj pe toate ramurile mai putin parintelui pe care nu l-am considerat
	ramura. Mesajul de sondaj este o topologie goala care are primul element = SONDAJ(1024). 

	- daca e frunza, trimite topologia parintelui

	- daca nu e frunza, asteapta mesaj cu topologie de pe fiecare ramura
		- daca se primeste un mesaj de SONDAJ este stearsa legatura cu ramura din matricea de 
		adiacenta. Nu se mai trimite nimic inapoi pentru ca, desi ramura asteapta la randul ei 
		topologia de la nod, pe care il considera copil, va primii sondaj pentru ca era 
		considerata ramura, si la randul ei va sterge intrarea in matricea de adiacenta.
		- daca primeste topologie adauga datele din matricea de adiacenta primita

	- daca e radacina, afiseaza in fisier matricea completa

	- calculeaza pe fiecare ramura cati copii are(se foloseste functie recursiva). Functia 
	recursiva ce calculeaza numarul de copii ai nodului primit este apelata recursiv pentru 
	toate nodurile cu care are legatura nodul curent in afara de parinte si numara acele 
	legaturi.

	- completeaza tabela de rutare nextHop(se foloseste functie recursiva)
	- afiseaza tabela de rutare: pe ce ramura se poate ajunge in fiecare nod. Se adauga pentru 
	fiecare copil de pe acea ramura nextHop ca fiind vecinul nodului de pe ramura. In nodurile
	in care nu se poate ajunge prin vecini se considera ca se ajunge prin parinte. -1 pentru 
	nodul insusi.


	REZOLVARE SUDOKU

		Fiecare proces rezolva patratul corespunzator id-lui. Patratele se numara de la stanga 
	la dreapta, de sus in jos in matricea sudoku.

		-daca e radacina, citeste dimensiunea sudoku si matricea incompleta din fisier si 
	trimite pe toate ramurile sudoku necompletat(o matrice stocata intr-un vector de 
	dimensiune np*np).
		- rezolva patratul si adauga solutia partiala
		- daca nu e frunza, primeste solutii partiale de pe fiecare ramura si le combina cu 
	solutiile partiale pe care le are rezultand noi solutii partiale ale nodului curent
		- trimite solutiile partiale parintelui(un vector de dimensiune node->l stabilita la 
	inceput in care sunt puse una dupa alta matrici de sudoku de dimensiune nr_procese^2)

		- radacina afiseaza in fisierul "out.txt" prima solutie din solutiile partiale dupa
	combinarea solutiilor pe care le-a primit de pe fiecare ramura


	REZOLVARE PATRAT PROPRIU
	- functie recursiva (solveSquare)
	- sunt gasite coordonatele de inceput ale patratului pe care il rezolva nodul (rand, coloana)
	- se rezolva sudoku pentru respectivul patrat cautandu-se casute goale doar in acel patrat
	- atunci cand nu se mai gaseste nicio casuta goala se adauga solutia in solutiile partiale si
	se face return pentru a cauta o alta solutie
	- daca s-a gasit spatiu liber se genereaza toate numerele intre 1 si numarul de procese si se
	verifica pe rand, coloana si in patrat daca se poate pune
	- este apelata recursiv functia si sub apel se reinitializeaza cu 0 casuta pentru a putea fi
	generata o alta valoare



	COMBINARE SOLUTII PARTIALE

	- primeste un nod si solutiile partiale ale unei ramuri 
	- copiaza vectorul cu solutiile partiale ale nodului si il goleste pe cel original pentru a 
	putea fi adaugate noile solutii gasite
	- fiecare matrice din solutiile partiale ale ramurei va fi combinata cu fiecare matrice din 
	solutiile partiale ale nodului
	- combinarea este facuta in vectorul node->s ce memoreaza solutia

	Pasi:
	- se copiaza pe rand in solutie fiecare matrice a ramurei 
	- pentru fiecare matrice a ramurei se ia fiecare solutie partiala a nodului si se copiaza 
	valorile corespunzatoare in casutele goale ale solutiei
	- se verifica pe linie si pe coloana daca solutia e valida si, daca e, se adauga in vectorul 
	cu solutii partiale ale nodului. Verificarea in casuta nu este necesara pentru ca s-a facut
	la generare, iar 2 procese nu pot rezolva aceeasi casuta.
	- se restabileste solutia ca fiind solutia partiala curenta a ramurei pentru a putea fi 
	combinata si cu restul solutiilor partiale ale nodului
