#include <record_utils.h>

arg_t InitializeThreadArgs(int server, fd_set *allFDs, int *fdMax, Mutex *mutex, Queue_t *q, char *nomebiblio) {
    arg_t threadArgs;
    threadArgs.q = q;
    threadArgs.clients = allFDs;
    threadArgs.record = (Record *)Malloc(sizeof(Record), MAX_CATALOG_SIZE);
    threadArgs.num_record = 0;
    threadArgs.server = server;
    threadArgs.nomebiblio = (char *)Malloc(sizeof(char), 20);
    threadArgs.fdMax = fdMax;
    threadArgs.mutex = mutex;

    strcpy(threadArgs.nomebiblio, nomebiblio);
    strcat(threadArgs.nomebiblio, ".log");

    return threadArgs;
}


void OverwriteFile(const char *filename,void *args)
{
    arg_t *arg=(arg_t *)args;
    FILE *file;
    Fopen(filename, "w+",&file);

    int i=0;
    int j=0;

    
    for (i = 0; i < arg->num_record; i++) {
        for (j = 0; j < arg->record[i].num_campi; j++) {
            if(strcmp(arg->record[i].campi[j].nomecampo,"prestito")==0)
            {
                if(!(CompareWithCurrentDate(arg->record[i].campi[j].valore))) //Controllo se il prestito è da scrivere
                    fprintf(file,"%s: %s;", arg->record[i].campi[j].nomecampo, arg->record[i].campi[j].valore);
                
            }else fprintf(file,"%s: %s;", arg->record[i].campi[j].nomecampo, arg->record[i].campi[j].valore);
        }
        fprintf(file,"\n");
    }

    Fclose(&file);
}


char* strtrim(char* str) 
{
    int len = strlen(str);
    int i=0;
    int start = 0;
    int end = len - 1;

    if (len == 0 || len == 1) 
        return str;

    while (isspace(str[start])) start++;
    while (isspace(str[end])) end--;

    for (i = 0; i <= end - start; i++) 
        str[i] = str[start + i];

    str[i] = '\0';

    return str;
}

void ReadRecordFromFile(const char *filename,arg_t *arg) {
    FILE *file;
    Fopen(filename, "r",&file);

    char record[MAX_RECORD_LENGTH];

    int n_rec=arg->num_record;

    while (fgets(record, sizeof(record), file) != NULL) {
        
        // Divisione della riga in campi separati da punto e virgola
        char *token = strtok(record, ";");
        n_rec=arg->num_record;
        (arg->record[n_rec]).num_campi=0;
        int n_campi=(arg->record[n_rec]).num_campi;

        while (token != NULL) {
            // Divisione del campo in nomecampo e valore
            char *delim = strchr(token, ':');

            if (delim != NULL) {
                *delim = '\0';
                n_campi=(arg->record[n_rec]).num_campi;

                memset(((arg->record[n_rec]).campi[n_campi]).nomecampo,0,sizeof(((arg->record[n_rec]).campi[n_campi]).nomecampo));
                memset(((arg->record[n_rec]).campi[n_campi]).valore,0,sizeof(((arg->record[n_rec]).campi[n_campi]).valore));


                strcpy(((arg->record[n_rec]).campi[n_campi]).nomecampo,strtrim(token));
                strcpy(((arg->record[n_rec]).campi[n_campi]).valore,strtrim(delim+1));
                (arg->record[n_rec]).num_campi++;

            }
            token = strtok(NULL, ";");
        }

        if((arg->record[n_rec]).num_campi>0)
        {
            arg->num_record++;
        }
        
    }

    Fclose(&file);
}
int *SearchRecord(void *args, Record r, int flag, int *contaprestito) {
    arg_t *arg = ((arg_t *)args);
    int i = 0;
    int j = 0;
    int k = 0;
    int reset = 0; //Condizione se trovo il prestito
    int found = 0; //Contatore di quanti campi ho trovato
    int update = 0; //Controllo se ho aggiornato il campo prestito per vedere nel caso dovessi fare un prestito se devo aggiungere al record il campo o solo aggiornarlo
    int ok = 1 ;  //Condizione per fermarmi se ho trovato un campo giusto e l'altro sbagliato (e.g stesso autore ma titolo diverso dalla richiesta)
    int piu_autori=0;  //Condizione per controllare se trovo un autore e se verificata aspetto ad andare subito avanti perchè ce ne potrebbe essere un'altro dopo
    int *ever_found = (int *)Calloc(arg->num_record, sizeof(int)); //Array che mi serve per tenere traccia dei record trovati

    for (i = 0; i < arg->num_record; i++) {
        // Ciclo sugli array di campi
        while (j < arg->record[i].num_campi && k < r.num_campi && ok) {

            if(strcmp(arg->record[i].campi[j].nomecampo,"autore")==0)
            {
                piu_autori=1;
            }
            
            // Se il nome del campo è uguale
            if (strcmp(arg->record[i].campi[j].nomecampo, r.campi[k].nomecampo) == 0) {
                if (flag) {
                    if (strcmp(arg->record[i].campi[j].nomecampo, "prestito") == 0 && ever_found[i]) {
                        // Se il campo è "prestito" e l'elemento è già stato trovato
                        if (CompareWithCurrentDate(arg->record[i].campi[j].valore)) {
                            char *t = Time();
                            strcpy(arg->record[i].campi[j].valore, t);
                            (*contaprestito)++; //Aggiorno numero di prestiti
                            ++found; //Aggiorno found perchè ho trovato un campo
                            myFree(t);
                            reset = 1; //Imposto il valore di reset perche nel caso trovi il prestito ma non trovo tutti i campi richiesti imposto ever_found a 0
                        } else {
                            ever_found[i] = 0;
                        }
                        update = 1;  //Setto update a 1 perchè vuol dire che il campo prestito esiste e non lo devo creare
                    }
                }

                if (!update) {
                    // Se il valore del campo è uguale
                    if (strcmp(arg->record[i].campi[j].valore, r.campi[k].valore) == 0) {
                        
                        ever_found[i] = 1;
                        ++found; //Aggiorno found perchè ho trovato un campo
                    } else {
                        if (found) {  //Se ho trovato un campo con lo stesso nome della richiesta( e.g autore ) ma con un valore diverso
                            ever_found[i] = 0;
                            if(!piu_autori)
                                ok = 0;
                        }
                    }
                }
                if(ever_found[i]) //Controllo se ho trovato qualcosa vado avanti
                {
                    k++;
                }

                piu_autori=0;
            }
            j++;
        }

        j = 0;
        k = 0;

        if (found == r.num_campi - 1 && flag && (!update)) {
            // Se sono stati trovati tutti gli elementi e l'opzione è attiva
            char *now = Time();
            strcpy(arg->record[i].campi[arg->record[i].num_campi].nomecampo, "prestito");
            strcpy(arg->record[i].campi[arg->record[i].num_campi].valore, now);
            myFree(now);
            arg->record[i].num_campi++;
            (*contaprestito)++;
            ++found;
            ever_found[i] = 1;
        }

        if (found < r.num_campi) { //Se ho trovato meno campi rispetto alla richiesta
            found = 0;

            ever_found[i] = 0;
            if (reset)
                (*contaprestito)--; //Rimetto il campo prestito al valore precedente
        }

        found=0;
        update = 0;
        reset = 0;
    }
    return ever_found;  //Ritorno l'array con le posizioni degli elementi trovati
}

void PrintLibrary(void *args) {
    arg_t *arg = (arg_t *)args;
    int i = 0, j = 0;

    // Stampa dei record e dei campi
    for (i = 0; i < arg->num_record; i++) {
        printf("Record %d:\n", i + 1);
        for (j = 0; j < arg->record[i].num_campi; j++) {
            printf("%s: %s\n", arg->record[i].campi[j].nomecampo, arg->record[i].campi[j].valore);
        }
        printf("\n");
    }
}

char *RebuildRecord(Record *r) {
    int j = 0;
    char *buf = (char *)Malloc(sizeof(char), 1000);
    buf[0] = '\0';

    for (j = 0; j < r->num_campi; j++) {
        // Costruzione di una stringa rappresentante il record
        strcat(buf, r->campi[j].nomecampo);
        strcat(buf, ":");
        strcat(buf, r->campi[j].valore);
        strcat(buf, ";");
    }
    strcat(buf, "\0");
    return buf;
}

void ProcessRequest_msg(message request_msg, int *flag_prestito, Record *tmp) {
    char temp[500];
    strcpy(temp, request_msg.data);

    if (request_msg.type == 'L') {
        *flag_prestito = 1;
    }

    char *token = strtok(request_msg.data, ";");  // Delimitatore di campo
    while (token != NULL) {
        char *delim = strchr(token, ':');  //Delimitatore di valore
        if (delim != NULL) {
            int num_campi = tmp->num_campi;
            *delim = '\0';
            memset(tmp->campi[num_campi].nomecampo, 0, sizeof(tmp->campi[num_campi].nomecampo));
            memset(tmp->campi[num_campi].valore, 0, sizeof(tmp->campi[num_campi].valore));
            strcpy(tmp->campi[num_campi].nomecampo, token);
            strcpy(tmp->campi[num_campi].valore, delim + 1);
            tmp->num_campi++;
        }
        token = strtok(NULL, ";");
    }
}