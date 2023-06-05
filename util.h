#ifndef UTILH
#define UTILH
#include "main.h"

typedef struct
{
    int ts;
    int src;

    int data;
} packet_t;
#define NITEMS 3

#define ACK 1
#define REQUEST 2
#define RELEASE 3
#define REPLACE 4
#define FINISH 5
#define APP_PKT 6

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    CANT_GO_DONT_WANT,
    CANT_GO_DO_WANT,
    CAN_GO,
    IN_FINISH
} state_t_new;
extern state_t stan;
extern state_t_new state_new;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t l_clock_mut;
extern pthread_mutex_t timestamps_mut;
extern pthread_mutex_t requests_mut;
extern pthread_mutex_t new_message_mut;
extern pthread_cond_t new_message_cond;
extern int new_message;

void changeStateNew(state_t_new);
#endif
