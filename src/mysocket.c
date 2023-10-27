#include <mysocket.h>

struct sockaddr_in *sa_init(char *address, uint16_t port) {
    struct sockaddr_in *sa = (struct sockaddr_in *)Malloc(sizeof(struct sockaddr_in),1);
    CHECK_ERROR_NULL(sa,"Sockaddr");
    memset((void *)sa, 0, sizeof(struct sockaddr_in));

    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    if(address)  //Se l'indirizzo passato è una stringa
        sa->sin_addr.s_addr=inet_addr(address);
    else
        sa->sin_addr.s_addr=htonl(INADDR_ANY);
    return sa;
}

int Socket(struct sockaddr_in *sa, int type, int protocol) {
    int server=socket(sa->sin_family, type, protocol);
    CHECK_ERROR_DESCRIPTOR(server,"Socket creation",cleanup(NULL,server));
    int val=1;
    
    if(setsockopt(server,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&val,sizeof(val)) == -1) //Setto le opzioni per evitare problemi nel bind
    {
        perror("Socket option");
        Close(server);
    }

    return server;
}

void BindAndListen(int server, struct sockaddr *serverAddr, socklen_t addrLen, int backlog) {
    CHECK_ERROR_DESCRIPTOR(bind(server, serverAddr, addrLen), "Socket bind", Close(server));
    CHECK_ERROR_DESCRIPTOR(listen(server, backlog), "Socket listen", Close(server));
}

void InitializeFDSet(fd_set *allFDs, fd_set *readFDs, int server, int *fdMax) {
    FD_ZERO(allFDs);
    FD_SET(server, allFDs);
    *fdMax = server;
}

void Select(int currMax, fd_set *readFDs, int *error) {
    if (select(currMax + 1, readFDs, NULL, NULL, NULL) == -1) {
        perror("");
        *error = 1;
    }
}

int Accept(int server) {
    int clientFD = accept(server, NULL, NULL);
    CHECK_ERROR_DESCRIPTOR(clientFD, "Socket Accept", cleanup(NULL,clientFD));
    return clientFD;
}

void Send(int *fd, void *msg, size_t msgSize, int flags) {
    int result = send(*fd, msg, msgSize, flags);
    CHECK_ERROR_NEGATIVE(result, "Error Send");
}

int Recv(int *fd, void *msg, size_t msgSize, int flags) {
    int result = recv(*fd, msg, msgSize, flags);

    if (result == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Nessun dato disponibile al momento
            return 0; // O restituisci un valore che indichi l'assenza di dati
        } else {
            perror("Error Receive");
            fprintf(stderr, "Errore errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }

    return result;
}