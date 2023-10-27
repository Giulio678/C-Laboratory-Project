#include <time_utils.h>

char *Time() {
    time_t current_timestamp = time(NULL);
    char *timestamp_str = (char *)Malloc(100,1); // Alloca memoria per la stringa
    struct tm *tm_info = localtime(&current_timestamp);
    strftime(timestamp_str, 100, "%d-%m-%Y %H:%M:%S", tm_info);

    return timestamp_str;
}

int CompareWithCurrentDate(const char *date_str) {
    int day, month, year, hour, minute, second;
    struct tm input_date;
    memset(&input_date, 0, sizeof(struct tm));
    if (sscanf(date_str, "%d-%d-%d %d:%d:%d", &day, &month, &year, &hour, &minute, &second) != 6) {
        printf("Error in data parsing\n");
        return 0;
    }

    input_date.tm_mday = day;
    input_date.tm_mon = month-1; // Mese da 0 a 11
    input_date.tm_year = year - 1900; // Anno da 1900
    input_date.tm_hour = hour-1; //Ora da 0 a 23
    input_date.tm_min = minute;
    input_date.tm_sec = second;

     // Imposta il fuso orario sulla timezone UTC
    input_date.tm_isdst = 0; // 0 indica la timezone UTC

    time_t input_timestamp = mktime(&input_date);

    time_t current_timestamp = time(NULL);
    struct tm tm = *localtime(&current_timestamp);

    if (current_timestamp - input_timestamp > 2592000) { // questi sono 30 giorni (30*24*60*60):2592000 secondi mettere 30 se si volesse fare il test per 30 secondi
        return 1; // La data è maggiore di almeno 30 secondi rispetto alla data attuale
    } else {
        return 0; // La data è uguale alla data attuale o è maggiore ma meno di 30 secondi
    }
}