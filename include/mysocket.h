#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common_utils.h"

/* Inizializza e restituisce un puntatore a una struttura sockaddr_in con l'indirizzo e la porta specificati */
struct sockaddr_in *sa_init(char *address, uint16_t port);

/* Crea un socket con il tipo e il protocollo specificati, restituisce il descrittore del socket */
int Socket(struct sockaddr_in *sa, int type, int protocol);

/* Associa il socket all'indirizzo e alla porta specificati e inizia ad ascoltare le connessioni in arrivo */
void BindAndListen(int server, struct sockaddr *serverAddr, socklen_t addrLen, int backlog);

/* Inizializza l'insieme di descrittori di file (fd_set) con il socket server e imposta fdMax con il valore massimo di descrittore */
void InitializeFDSet(fd_set *allFDs, fd_set *readFDs, int server, int *fdMax);

/* Esegue la selezione su un set di descrittori di file per verificare la disponibilità di dati in lettura */
void Select(int currMax, fd_set *readFDs, int *error);

/* Accetta una nuova connessione sul socket server e restituisce il descrittore del nuovo socket */
int Accept(int server);

/* Invia un messaggio attraverso il socket specificato */
void Send(int *fd, void *msg, size_t msgSize, int flags);

/* Riceve un messaggio attraverso il socket specificato */
int Recv(int *fd, void *msg, size_t msgSize, int flags);

#endif
