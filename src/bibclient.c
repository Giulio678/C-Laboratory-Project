#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>

#include <mysocket.h>
#include <common_utils.h>

#define N 500

#define MSG_QUERY 'Q'
#define MSG_RECORD 'R'
#define MSG_LOAN 'L'
#define MSG_NO 'N'
#define MSG_ERROR 'E'

typedef struct {
    char type;
    unsigned int length;
    char data[N];
}message;

//Struttura dove vado ad inserire i dati del server per la connessione
typedef struct { 
    char name[100];
    char address[100];
    int port;
} ServerConfig;

//Funzione che legge il file di configurazione e mette i dati dentro la struttura per la connessione
int ReadConfFile(ServerConfig servers[], int *numServers) {
    FILE *file ;
    Fopen("config/bib.conf", "r",&file);

    *numServers = 0;
    while (fscanf(file, "%s %d %s", servers[*numServers].name, &servers[*numServers].port, servers[*numServers].address) == 3 && *numServers < 100) {
        (*numServers)++;
    }

    Fclose(&file);
    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    char buff[N];
    buff[0]='\0';

    int i=0;
    int loan=0;

    ServerConfig servers[100];  // Supponiamo che ci siano al massimo 100 server nel file

    int numServers=0;
    
    //Specifico le opzioni che il programma puo accettare da linea di comando
   struct option long_options[] = {
        {"autore", required_argument, NULL, 'a'},
        {"titolo", required_argument, NULL, 't'},
        {"editore", required_argument, NULL, 'e'},
        {"anno", required_argument, NULL, 'n'},
        {"nota", required_argument, NULL, 'o'},
        {"collocazione", required_argument, NULL, 'c'},
        {"luogo_pubblicazione", required_argument, NULL, 'l'},
        {"descrizione_fisica", required_argument, NULL, 'd'},
        {"p", no_argument, NULL, 'p'},
        {"volume", required_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

   while ((opt = getopt_long(argc, argv, "a:t:e:n:o:c:l:d:p:v:", long_options,NULL)) != -1) {
        switch (opt) {
            case 'a':
                // Gestione opzione autore
                strcat(buff,"autore:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 't':
                // Gestione opzione titolo
                strcat(buff,"titolo:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'e':
                // Gestione opzione editore
                strcat(buff,"editore:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'n':
                // Gestione opzione anno
                strcat(buff,"anno:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'o':
                // Gestione opzione nota
                strcat(buff,"nota:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'c':
                // Gestione opzione collocazione
                strcat(buff,"collocazione:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'l':
                // Gestione opzione luogo_pubblicazione
                strcat(buff,"luogo_pubblicazione:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'd':
                // Gestione opzione descrizione_fisica
                strcat(buff,"descrizione_fisica:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;
            case 'p':
                // Gestione opzione prestito
                strcat(buff,"prestito: da fare;");
                loan=1;
                break;
            case 'v':
                // Gestione opzione volume
                strcat(buff,"volume:");
                strcat(buff,optarg);
                strcat(buff,";");
                break;

            default:
                fprintf(stderr, "Usage: %s --option=<option>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    ReadConfFile(servers, &numServers);

    if( numServers == 0)
    {
        printf("No server active\n");
        exit(EXIT_FAILURE);
    }



// Ciclo attraverso tutti i server
for (i = 0; i < numServers; i++) {
    // Stampa le informazioni del server (porta, nome, indirizzo)
    printf("%d ",servers[i].port);
    printf("%s ",servers[i].name);
    printf("%s\n",servers[i].address);

    // Inizializza la struttura sockaddr_in per la connessione al server
    struct sockaddr_in *sa = sa_init(servers[i].address, servers[i].port);
    CHECK_ERROR_NULL(sa, "Sockaddr init");

    // Crea un socket per la comunicazione con il server
    int clientSocket = Socket(sa, SOCK_STREAM, 0);
    CHECK_ERROR_DESCRIPTOR(clientSocket, "Server Socket",cleanup(NULL,clientSocket));

    // Imposta un timeout per la connessione (5 secondi)
    int timeout_ms = 5000;
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    CHECK_ERROR_DESCRIPTOR(setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)),"Timeout set error",Close(clientSocket));

    int conn = 0;
    // Prova a connettersi al server con gestione di errori specifici
    while ((conn = connect(clientSocket, (struct sockaddr *)sa, sizeof(*sa))) == -1 && (errno == ECONNREFUSED || errno == EAGAIN || errno == EINPROGRESS)){
        usleep(200000);
    }
    CHECK_ERROR_DESCRIPTOR(conn,"Connection socket error",Close(clientSocket));

    printf("Connected to server\n");

    // Prepara un messaggio di richiesta al server
    message query_msg;
    memset(&query_msg, 0, sizeof(message));
    if(loan)
        query_msg.type=MSG_LOAN;
    else
        query_msg.type = MSG_QUERY;

    query_msg.length = strlen(buff);
    strcpy(query_msg.data,buff);

    // Invia il messaggio al server
    Send(&clientSocket, &query_msg, sizeof(message),0);

    int keep_receiving = 1;

    // Continua a ricevere messaggi dal server finché necessario
    while (keep_receiving) {
        message response_msg;
        memset(&response_msg, 0, sizeof(message));

        ssize_t bytes_received=0;

        // Prova a ricevere un messaggio dal server
        while ((bytes_received = Recv(&clientSocket, &response_msg, sizeof(message),0))==0) {
            usleep(50000);
        }

        // Gestione delle diverse tipologie di messaggi ricevuti
        if (response_msg.type == MSG_RECORD) {
            printf("Answer from server:\n");
            printf("Type: %c\n", response_msg.type);
            printf("Length: %u\n", response_msg.length);
            printf("Data: %s\n", response_msg.data);
        } else if (response_msg.type == MSG_NO) {
            printf("No record satisfies the query.\n");
        } else if (response_msg.type == MSG_ERROR) {
            printf("Error from server: %s\n", response_msg.data);
        } else {
            printf("End of message\n");
            keep_receiving=0;
        }
    }

    // Libera la memoria della struttura sockaddr_in
    myFree(sa);
    // Chiude il socket
    Close(clientSocket);
}

    

    return 0;

}