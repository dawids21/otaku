#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;

    int data; /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3 // wielkość sekcji krytycznej - u nas wielkość pomieszczenia +

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK 1
#define REQUEST 2
#define RELEASE 3
#define REPLACE 4
#define FINISH 5
#define APP_PKT 6

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    InRun,
    InMonitor,
    InWant,
    InSection,
    InFinish
} state_t;
typedef enum
{
    CANT_GO_DONT_WANT,
    CANT_GO_DO_WANT,
    CAN_GO,
    INSIDE_X_OK,
    INSIDE_X_NO_OK,
    IN_FINISH
} state_t_new;
extern state_t stan;
extern state_t_new state_new;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t l_clock_mut; // mutex, broni dostępu do zmiennej zegara
extern pthread_mutex_t timestamps_mut; // mutex, broni dostępu do zmiennej zegara
extern pthread_mutex_t requests_mut; // mutex, broni dostępu do zmiennej zegara

/* zmiana stanu, obwarowana muteksem */
void changeState(state_t);

void changeStateNew(state_t_new);
#endif
