#ifndef MYTHREADS_H_
#define MYTHREADS_H_

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "common_utils.h"

typedef pthread_t Thread;
typedef pthread_mutex_t Mutex;
typedef pthread_cond_t Cond;

/* Crea un nuovo thread.
   Parametri:
   - thread: Puntatore a una variabile pthread_t dove verrà memorizzato l'ID del nuovo thread.
   - function: Puntatore alla funzione che il nuovo thread eseguirà.
   - arg: Argomento da passare alla funzione.
*/
void ThreadCreate(Thread *thread, void *(*function)(void *), void *arg);

/* Aspetta che un thread termini.
   Parametri:
   - thread: Puntatore al thread da attendere.
   - buff: Puntatore a una variabile dove verrà memorizzato il valore di ritorno del thread.
*/
void ThreadJoin(Thread thread, void **buff);

/* Inizializza un mutex per la sincronizzazione dei thread. */
void MutexInit(Mutex *mutex);

/* Blocca un mutex per l'accesso esclusivo. */
void MutexLock(Mutex *mutex);

/* Sblocca un mutex precedentemente bloccato. */
void MutexUnlock(Mutex *mutex);

/* Distrugge un mutex, rilasciando le risorse associate. */
void MutexDestroy(Mutex *mutex);

/* Prova a bloccare un mutex con timeout. */
void MutexTrylock(Cond *cond, Mutex *mutex, const struct timespec *abstime);

/* Inizializza una variabile di condizione per la sincronizzazione dei thread. */
void CondInit(Cond *cond);

/* Aspetta su una variabile di condizione, sbloccando il mutex associato fino a quando la condizione viene segnalata. */
void CondWait(Cond *cond, Mutex *mutex);

/* Segnala una variabile di condizione per svegliare un thread in attesa. */
void CondSignal(Cond *cond);

/* Distrugge una variabile di condizione, rilasciando le risorse associate. */
void CondDestroy(Cond *cond);

/* Segnala una variabile di condizione per svegliare tutti i thread in attesa. */
void CondBroadcast(Cond *cond);

/* Aspetta su una variabile di condizione con timeout. */
void CondTimedWait(Cond *cond, Mutex *mutex, const struct timespec *abstime);

#endif /* MYTHREADS_H_ */
