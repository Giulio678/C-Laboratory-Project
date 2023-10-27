#ifndef RECORD_UTILS_H
#define RECORD_UTILS_H

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

#include "common_utils.h"
#include "time_utils.h"
#include "mythreads.h"
#include "unboundedqueue.h"

#define MSG_QUERY 'Q'
#define MSG_RECORD 'R'
#define MSG_LOAN 'L'
#define MSG_NO 'N'
#define MSG_ERROR 'E'
#define MSG_FINISHED 'F'

#define MAX_RECORD_LENGTH 1024
#define MAX_CATALOG_SIZE 1000
#define MAX_FIELD_LENGTH 128

#define N 500 //messaggio dal client

#define MAX_WORKERS 10 // Numero massimo di thread worker

typedef struct{  //Struttura di comunicazione messaggi tra client e server
    char type;        // Tipo di messaggio
    unsigned int length;   // Lunghezza del campo dati
    char data[N];     // Dati del messaggio
}message;

typedef struct {
    char nomecampo[256];   // Nome di un campo
    char valore[256];      // Valore associato al campo
} Campo;

typedef struct {
    Campo campi[15];   // Array di campi
    int num_campi;     // Numero effettivo di campi nel record
} Record;

typedef struct{
    Queue_t *q;           // Puntatore alla coda
    fd_set *clients;      // Puntatore all'insieme di file descriptor dei client
    Record *record;       // Puntatore a un array di record
    int server;           // File descriptor del server
    int num_record;       // Numero totale di record
    char *nomebiblio;     // Puntatore al nome della biblioteca
    int *fdMax;           // Puntatore al massimo file descriptor
    Mutex *mutex;         // Puntatore a una variabile di sincronizzazione
}arg_t; // Struttura condivisa tra i thread

/* Inizializza la struttura degli argomenti dei thread. */
arg_t InitializeThreadArgs(int server, fd_set *allFDs, int *fdMax, Mutex *mutex, Queue_t *q, char *nomebiblio);

/* Sovrascrive un file con i nuovi dati. */
void OverwriteFile(const char *filename, void *args);

/* Rimuove gli spazi bianchi all'inizio e alla fine di una stringa. */
char *strtrim(char *str);

/* Legge i record da un file e li carica nella struttura dati condivisa. */
void ReadRecordFromFile(const char *filename, arg_t *arg);

/* Cerca un record in base ai campi specificati e restituisce gli indici dei record corrispondenti. */
int *SearchRecord(void *args, Record r, int flag, int *contaprestito);

/* Stampa l'elenco completo della biblioteca. */
void PrintLibrary(void *args);

/* Ricostruisce un record in una stringa formattata. */
char *RebuildRecord(Record *r);

/* Processa una richiesta di messaggio e imposta eventuali flag di prestito. */
void ProcessRequest_msg(message request_msg, int *flag_prestito, Record *tmp);

/* Rimuove un record dalla struttura dati condivisa. */
void RemoveRecord(void *args, int index);

#endif // RECORD_UTILS_H
