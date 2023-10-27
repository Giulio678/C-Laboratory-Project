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
#include <fcntl.h> //per mettere un blocco sull'accesso al file
#include <poll.h>
#include <unboundedqueue.h>
#include <mythreads.h>
#include <common_utils.h>
#include <mysocket.h>
#include <time_utils.h>
#include <record_utils.h>

#define MAX_FDS 100 // Definisco il massimo numero di file descriptor da gestire

// Variabile per tracciare i segnali SIGINT e SIGTERM
volatile sig_atomic_t sigint_received = 0;


char type;//Serve per scrivere alla fine del log file se la richiesta era QUERY o LOAN
int contaprestito=0;
int numlogs=0;

void SignalHandler(int sig);
void UpdateMax(fd_set set, int* max);
void* worker(void* args);
void SaveData(const char *name,long int port,const char *address);
int ReadConfFile();
void SetupSignalHandlers();
void checkArgs(int argc,char *name);


int main(int argc, char* argv[]) {

    checkArgs(argc,argv[0]);  // Verifica gli argomenti passati al programma

    int i=0;
    int error=0;
    int num_workers = atoi(argv[3]);  // Ottiene il numero di worker dai parametri

    // Registra gli handlers per SIGINT e SIGTERM
    SetupSignalHandlers();

    // Inizializza il mutex
    Mutex m;
    MutexInit(&m);

    FILE *file;
    Fopen("config/bib.conf", "a", &file);  // Apertura del file di configurazione
    Fclose(&file);

    srand(time(NULL));  // Inizializzazione del generatore di numeri casuali

    int portNumber = ReadConfFile();  // Legge il file di configurazione e ottiene il numero di porta
    struct sockaddr_in *sa = sa_init(NULL, portNumber);  // Inizializza la struttura sockaddr_in
    CHECK_ERROR_NULL(sa, "Sockaddr init");

    SaveData(argv[1], portNumber, inet_ntoa(sa->sin_addr));  // Salva i dati del server

    int serverSocket = Socket(sa, SOCK_STREAM, 0);  // Crea il socket del server
    CHECK_ERROR_DESCRIPTOR(serverSocket, "Server Socket",cleanup(NULL,serverSocket));

    BindAndListen(serverSocket, (struct sockaddr*)sa, sizeof(*sa), 10);  // Fa il binding e mette il socket in ascolto

    fd_set allFDs, readFDs;  // Inizializza i set di file descriptor,readFDs mi serve per tenere una copia di allFDs poi nel while
    int fdMax;

    int flags = fcntl(serverSocket, F_GETFL, 0);  // Imposta il socket del server come non bloccante
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

    Queue_t* q = initQueue();  // Inizializza la coda

    InitializeFDSet(&allFDs, &readFDs, serverSocket, &fdMax);  // Inizializza i set di file descriptor

    arg_t threadArgs;  // Argomenti del thread

    char *logs=(char *)Malloc(sizeof(char),20);  // Allocazione memoria per il percorso del file di log
    strcpy(logs,"logs/");
    strcat(logs,argv[1]);
    strcat(logs,"\0");
    threadArgs = InitializeThreadArgs(serverSocket, &allFDs, &fdMax, &m, q, logs);  // Inizializza gli argomenti dei thread
    myFree(logs);

    ReadRecordFromFile(argv[2], &threadArgs);  // Legge i record dal file

    if (num_workers <= 0 || num_workers > MAX_WORKERS) {
        fprintf(stderr, "Il numero di thread deve essere compreso tra 1 e %d\n", MAX_WORKERS);
        exit(1);
    }

    Thread workers[num_workers];  // Crea un array di thread

    for (i = 0; i < num_workers; i++) {
        ThreadCreate(&workers[i], worker, &threadArgs);  // Crea i thread worker
    }

    int numOpenFD = 0;  // Numero attuale di file descriptor aperti

    while (!sigint_received) {
        MutexLock(&m);  // Blocca il mutex
        readFDs = allFDs;  // Copia i set di file descriptor
        int currMax = fdMax;  // Ottiene il massimo file descriptor
        MutexUnlock(&m);  // Sblocca il mutex

        Select(currMax, &readFDs, &error);  // Esegue la select

        if (!error) {
            for (i = 0; i < currMax + 1; i++) {
                if (FD_ISSET(i, &readFDs)) {

                    if ((i == serverSocket)) {  // Nuova connessione
                        int clientSocket = Accept(serverSocket);
                        printf("Un client è arrivato %s\n"," ");
                        numOpenFD++;
                        MutexLock(&m);
                        FD_SET(clientSocket, &allFDs);  // Aggiunge il nuovo socket ai file descriptor
                        if (clientSocket > fdMax)
                            fdMax = clientSocket;
                        MutexUnlock(&m);
                    } else {  // Richiesta da un client esistente
                        int* client = (int*)Malloc(sizeof(int), 1);
                        *client = i;
                        push(q, client);  // Aggiunge la richiesta alla coda
                    }
                }
            }
        }
    }


    //Metto in coda -1 cosi da dire ai workers di fermarsi
    for(i = 0; i < num_workers; i++)
    {
        int *end = (int *)Malloc(sizeof(int), 1);
        *end = -1;
        push(q, end);
    }

    //Aspetto che i thread finiscano
    for(i = 0; i < num_workers; i++)
    {
        ThreadJoin(workers[i], NULL);
    }

    // Chiudi i file descriptor che sono sempre aperti
    for(i = 0; i < numOpenFD; i++)
    {
        if (FD_ISSET(i, &allFDs))
            Close(i);
    }

    //Scrivo in fondo al file di log il tipo di richiesta fatta dal client
    FILE *logfile;
    Fopen(threadArgs.nomebiblio,"a+",&logfile);
    if(type=='L')
        fprintf(logfile,"\nLOAN %d",contaprestito);
    else fprintf(logfile,"\nQUERY %d",numlogs);
    Fclose(&logfile);


    OverwriteFile(argv[2], &threadArgs); //Sovrascrive il file (e.g bib1.txt) con i nuovi dati dopo l'esecuzione

    //Libero le risorse
    myFree(threadArgs.record);
    myFree(threadArgs.nomebiblio);
    myFree(sa);
    deleteQueue(q);
    MutexDestroy(&m);

    return 0;
}

// Funzione per il gestore dei segnali
void SignalHandler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        sigint_received = 1;
    }
}

//Funzione per aggiornare il massimo tra i file descriptor
void UpdateMax(fd_set set, int* max) {
    while (!FD_ISSET(*max, &set)) (*max)--;
}


void* worker(void* args) {

    arg_t *arg=(arg_t*)args;  // Ottiene gli argomenti dal parametro void*

    Queue_t* q = ((arg_t*)args)->q;  // Ottiene la coda dalla struttura arg_t
    int* fdMax = ((arg_t*)args)->fdMax;  // Ottiene il massimo file descriptor
    fd_set* clients = ((arg_t*)args)->clients;  // Ottiene il set di file descriptor
    Mutex* m = ((arg_t*)args)->mutex;  // Ottiene il mutex
    int i=0;

    //Messaggi
    message request_msg;  // Messaggio in entrata
    message response_msg;  // Messaggio in uscita
    memset(&response_msg, 0, sizeof(message));  // Inizializza il messaggio di risposta

    int notfound=1;

    //File di log
    FILE *logFile;

    while (1) {

        Record tmp;
        int num_campi=0;
        tmp.num_campi=num_campi;
        int flag_prestito=0;
        int change=contaprestito;
        char *messaggio=(char *)Malloc(sizeof(char),5000);
        messaggio[0]='\0';
        int skip=0; //Se si verifica un errore durante l'apertura del file di log salto la parte di scrittura
        int *ever_found=NULL;
        int* fd = (int*)pop(q);  // Ottiene un descrittore di file dalla coda

        if(*fd==-1)
        {
            myFree(messaggio);
            myFree(fd);
            break;
        }

        int n=Recv(fd, &request_msg, sizeof(message), 0);  // Riceve il messaggio dal cliente

        if (n > 0) {
            
            MutexLock(m);  // Blocca il mutex

            type=request_msg.type;  // Ottiene il tipo di richiesta

            ProcessRequest_msg(request_msg,&flag_prestito,&tmp);  // Elabora la richiesta del client

            ever_found=SearchRecord(args,tmp,flag_prestito,&contaprestito);  // Cerca i record nella biblioteca

            if(Fopen2(arg->nomebiblio, "a+",&logFile)==NULL)
            {
                memset(&response_msg, 0, sizeof(message));
                response_msg.type=MSG_ERROR;
                strcpy(response_msg.data,"Error Fopen logfile");
                Send(fd, &response_msg, sizeof(message), 0);
                skip=1;

            }  // Apre il file di log in modalità append

            
            for(i=0;i<arg->num_record;i++)
            {
                if(ever_found[i]!=0)
                {   
                    notfound=0;

                    strcpy(messaggio, RebuildRecord(&arg->record[i]));  // Ricostruisce il record in una stringa
                    
                    response_msg.type=MSG_RECORD;
                    response_msg.length=strlen(messaggio);
                    strcpy(response_msg.data,messaggio);
                    Send(fd, &response_msg, sizeof(message), 0); // Invia il messaggio di risposta al client

                    if(!skip)
                    {
                    if(flag_prestito)
                    {
                        if(change != contaprestito)  // Controlla se ci sono nuovi prestiti
                        { 
                            fprintf(logFile, "%.*s", N, response_msg.data); //Metto in questo formato senno response_msg.data non veniva riconosciuto come stringa
                            fprintf(logFile,"\n");
                        }

                    }else{
                        fprintf(logFile, "%.*s", N, response_msg.data);
                        fprintf(logFile,"\n");
                    }
                    }

                    numlogs++;
                }
            }

            myFree(messaggio);
            myFree(ever_found);

            MutexUnlock(m);  // Sblocca il mutex
            if(!skip)
                Fclose(&logFile);  // Chiude il file di log

            //Messaggio se non ho trovato nessun record all'interno della biblioteca
            if(notfound)
            {
                response_msg.type=MSG_NO;
                Send(fd, &response_msg, sizeof(message), 0);  // Invia un messaggio di "nessun risultato trovato"
            }
        
            response_msg.type=MSG_FINISHED;
            Send(fd, &response_msg, sizeof(message), 0); // Messaggio di fine ricezione dati da parte del client
            skip=0;
        }

        if (n == 0) {
            MutexLock(m);
            FD_CLR(*fd, clients);
            UpdateMax(*clients, fdMax); // Aggiorna il massimo tra i file descriptor
            MutexUnlock(m);
        }

        myFree(fd);  // Libera la memoria del descrittore di file

    }
    return NULL;
}





//Funzione che salva i dati del server sul file di configurazione
void SaveData(const char *name,long int port,const char *address) {
    FILE *file;

    Fopen("config/bib.conf", "a",&file);
    printf("%s %ld %s\n", name, port, address);
    fprintf(file, "%s %ld %s\n", name, port, address);

    Fclose(&file);
}

//Funzione di generazione porta socket e controllo porta già in uso 
int ReadConfFile() {

    FILE *file;
    Fopen("config/bib.conf", "r", &file);
    srand(time(NULL));
    int portNumber = (rand() % 9 + 1) * 1111; //Genero da 1111 a 9999
    int ports[10];
    int i = 0;
    int j = 0;
    int done = 0;
    int duplicatePort = 0;
    
    while (fscanf(file, "%*s %d %*s", &ports[i]) == 1) {
        i++;
    }

    Fclose(&file);

    if (i > 0) {
        while (!done) {
            duplicatePort = 0;
            j = 0;
            while (j < i && !duplicatePort) {
                if (ports[j] == portNumber)
                {   
                    duplicatePort = 1;
                }
                j++;
            }

            if (!duplicatePort) {
                done = 1;
            } else {
                portNumber = (rand() % 9 + 1) * 1111; //Se la porta è duplicata rigenero
            }
        }
    }

    return portNumber;
}

//Gestisco segnali
void SetupSignalHandlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandler;
    CHECK_ERROR_ZERO(sigaction(SIGINT, &sa, NULL),"SIGINT init");
    CHECK_ERROR_ZERO(sigaction(SIGTERM, &sa, NULL),"SIGTERM init");
}

//Controllo gli argomenti passati
void checkArgs(int argc,char *name) {
    if (argc < 4) {
        printf("Usage: %s nomebiblio bib.txt nthread\n",name);
        exit(EXIT_FAILURE);
    }
}

