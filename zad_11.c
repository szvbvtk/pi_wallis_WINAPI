#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>


HANDLE mutex;

double suma = 1.0;

struct arg{
    int liczba_wyrazow;
    int indeks_pierwszego_wyrazu;
};


DWORD WINAPI *thread(LPVOID dane){
    struct arg *argumenty = (struct arg *)dane;
    

    int liczba_wyrazow = argumenty->liczba_wyrazow;
    int indeks_wyrazu = argumenty->indeks_pierwszego_wyrazu;
    double suma_tmp = 1;
    DWORD tid = GetCurrentThreadId();

    fprintf(stdout, "Thread #%lu size=%d first=%d\n", tid, liczba_wyrazow, indeks_wyrazu); 

    for(int i = indeks_wyrazu; i < indeks_wyrazu + liczba_wyrazow; i++){
        // suma_tmp *= (4.0 * i * i) / ((2 * i - 1.0) * (2 * i + 1.0)); //to jest chyba dokładniejsze (nw) ale w tym odkomentowanym są wyniki dokładnie jak w wyniku z polecenia
        suma_tmp *= (2.0*i / (2*i - 1.0)) * (2.0*i /(2*i + 1.0));
    }

    WaitForSingleObject(mutex, INFINITE);
    suma *= suma_tmp;
    ReleaseMutex(mutex);

    fprintf(stdout, "Thread #%lu prod=%0.20lf\n", tid, suma_tmp);
    // suma_tmp = 0;

    return 0;
}

int main(int argc, char **argv){

    if(argc < 3){
        fprintf(stderr, "Nieprawidłowa ilość argumentów\n"); 
        return 1;
    }

    char *endptr;
    long int n = strtol(argv[1], &endptr, 10);

    if(*endptr != '\0' || n < 1 || n > 1000000000){
        fprintf(stderr, "Nieprawidłowa ilość wyrazów\n"); 
        return 1;
    }


    int w = strtol(argv[2], &endptr, 10);

    if(*endptr != '\0' || w < 1 || w > 100){
        fprintf(stderr, "Nieprawidłowe ilość wątków\n"); 
        return 1;
    }

    int reszta = n % w;
    int liczba_wyrazow = n / w;

    // struct arg argumenty[w];
    struct arg* argumenty = malloc(w * sizeof (struct arg));

    int indeks_pierszego_wyrazu = 1;

    for(int i = 0; i < w; i++){
        argumenty[i].liczba_wyrazow = liczba_wyrazow;
        argumenty[i].indeks_pierwszego_wyrazu = indeks_pierszego_wyrazu;
        indeks_pierszego_wyrazu += liczba_wyrazow;
    }

    argumenty[w - 1].liczba_wyrazow += reszta;

    clock_t start, koniec;

    mutex = CreateMutex(NULL, FALSE, NULL);

    // HANDLE watki[w];
    HANDLE* watki = (HANDLE* )malloc(w * sizeof(HANDLE));
    DWORD* threadIds = (DWORD* )malloc(w * sizeof(DWORD));

    start = clock();
    for(int i = 0; i < w; i++){
        watki[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) thread, (LPVOID)&argumenty[i], 0, threadIds + i);
    }

    for(int i = 0; i < w; i++){
        WaitForSingleObject(watki[i], INFINITE);
        CloseHandle(watki[i]);
    }

    koniec = clock();

    // jesli odkomentuje to co zakomentowane to to zakomentowac
	free(watki);	
	free(argumenty);
    free(threadIds);
    CloseHandle(mutex);

    suma *= 2;
    fprintf(stdout, "w/Threads:  PI=%.20lf time=%f\n", suma, (float)(koniec - start) / CLOCKS_PER_SEC); 

    // bez wątków
    double sumab = 1.0;
    start = clock();
    for(int i = 1; i <= n; i++){
        sumab *= (2.0*i / (2*i - 1.0)) * (2.0*i /(2*i + 1.0));
    }
    koniec = clock();

    sumab *= 2;
    fprintf(stdout, "wo/Threads: PI=%.20lf time=%f\n", sumab, (float)(koniec - start) / CLOCKS_PER_SEC); 


    // fprintf(stdout, "wo/Threads: PI=%.30Lf\n", pi); 

    return 0;
}