# Progetto laboratorio 2 Biblioteca

## Disposizione file e cartelle

/ProgettoLab2
	
    /config
		-bib.conf

	/data
		-bib1.txt
		-bib2.txt
		-bib3.txt
		-bib4.txt
		-bib5.txt

	/include
		-common_utils.h
		-mysocket.h
		-mythreads.h
		-record_utils.h
		-time_utils.h
		-unboundedqueue.h

	/logs
		-nomebiblio.log

	/src
		-bibclient.c
		-bibserver.c
		-common_utils.c
		-mysocket.c
		-mythreads.c
		-record_utils.c
		-time_utils.c
		-unboundedqueue.c

    bibaccess
	Makefile
	README.md


## Cosa contengono i file 

bib.conf : file di configurazione dove vengono inseriti i dati del server a cui i client devono connettersi

bib*.txt : file txt che contengono le informazioni dei record che saranno inserite all'interno della biblioteca

common_utils.h : file di intestazione per la gestione di errori, allocazione risorse e apertura/chiusura file descriptor

mysocket.h:  file di intestazione che contiene le dichiarazioni di funzioni e aggiunta librerie necessarie per la gestione dei socket

mythreads.h: file di intestazione che contiene le dichiarazioni di funzioni e aggiunta librerie necessarie per la gestione dei thread

record_utils.h: file di intestazione che contiene le dichiarazioni di funzioni e strutture dati usate all'interno del programma

time_utils.h: file di intestazione che contiene le dichiarazioni di funzioni e aggiunta librerie necessarie per la gestione delle scadenze nella biblioteca

unboundedqueue.h: file di intestazione che contiene le dichiarazioni di funzioni e aggiunta librerie necessarie per la gestione della coda infinita

**I rispettivi file.c con gli stessi nomi sono usati per l'implementazione delle funzioni dichiarate nei file di intestazione**

bibserver.c: file dove viene implementato il server

bibclient.c : file dove viene implementato il client

nomebiblio.log : questi file di log contengono le informazioni relative alle richieste dei client ai server

bibaccess : file bash che controlla la correttezza dei file di log e stampa le relative informazioni

Makefile: file da eseguire per la compilazione e l'esecuzione del progetto

## Compilazione ed esecuzione

Per compilare ed eseguire i test: **make** (il makefile è impostato per eseguire le operazioni ***clean,all,test*** con questo comando)


Per compilare tutti i file.c: **make all** 
(questo comando prima da i permessi di rwx ai file e poi li compila)

Per rimuovere i file di log e di configurazione: **make clean**

Per eseguire i test: **make test**

#### Modifiche esecuzione
Questi sono gli argomenti passati al server nel for del test:
1. Il primo indica il nome della biblioteca che si vuole utilizzare (e.g pino).
2. Il secondo argomento è il percorso del file da dove vengono caricati i record (e.g data/bib1.txt)
3. Il terzo indica il numero di thread da creare e usare nel server

``` 
SERVER_ARGS = \
	"pino data/bib1.txt 5" \
	"gino data/bib2.txt 3" \
	"cane data/bib3.txt 1" \
	"pane data/bib4.txt 4" \
	"rane data/bib5.txt 2"
```

Se si volesse modificare dei parametri da passare al client basta modificare cosa gli argomenti(e.g autore ecc..)
```
@for i in 1 2 3 4 5 6 7 8; do \
		for j in 1 2 3 4 5; do \
			./$(CLIENT) --autore="Di Ciccio, Antonio" --titolo="Manuale di architettura pisana" --p & \
		done; \
		sleep 1; \
	done
```

* Opzione prestito: se si vuole eseguire un prestito serve specificare alla fine del comando l'opzione **--p**

* Per testare un solo server o un client
1. Eseguire il comando **make clean** e **make all** poi in ordine eseguire

```
valgrind ./bibserver nomebiblio data/bib1.txt nthread

valgrind ./bibclient --autore="Melis, Antonio Pietro Angelo"

```

* Per cambiare il segnale da inviare ai server (e.g SIGTERM)

```
@killall -TERM $(MAIN)
```














