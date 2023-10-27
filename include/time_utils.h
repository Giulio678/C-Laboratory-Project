#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stdio.h>    
#include <stdlib.h>   
#include <errno.h>    
#include <pthread.h>  
#include <string.h>
#include <unistd.h>   
#include <ctype.h>

#include "common_utils.h"

/* Restituisce una stringa rappresentante l'orario corrente nel formato "YYYY-MM-DD HH:MM:SS". */
char *Time();

/* Compara una data nel formato "YYYY-MM-DD" con la data corrente.
   Restituisce 1 se la data fornita è successiva a quella corrente, -1 se è precedente, e 0 se sono uguali. */
int CompareWithCurrentDate(const char *date_str);

#endif // TIME_UTILS_H
