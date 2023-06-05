#include "main.h"
#include "util.h"

MPI_Datatype MPI_PAKIET_T;

state_t_new state_new = CANT_GO_DONT_WANT;

pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t l_clock_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t timestamps_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t requests_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t new_message_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_message_cond = PTHREAD_COND_INITIALIZER;
int new_message = TRUE;

struct tagNames_t
{
    const char *name;
    int tag;
} tagNames[] = {
    {"pakiet aplikacyjny", APP_PKT}, 
    {"finish", FINISH}, {"potwierdzenie", ACK}, 
    {"prośbę o sekcję krytyczną", REQUEST}, 
    {"zwolnienie sekcji krytycznej", RELEASE},
    {"wymieniam", REPLACE}};

const char *const tag2string(int tag)
{
    for (int i = 0; i < sizeof(tagNames) / sizeof(struct tagNames_t); i++)
    {
        if (tagNames[i].tag == tag)
            return tagNames[i].name;
    }
    return "<unknown>";
}

void inicjuj_typ_pakietu()
{
    int blocklengths[NITEMS] = {1, 1, 1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void sendPacket(packet_t *pkt, int destination, int tag)
{
    pthread_mutex_lock(&l_clock_mut);
    l_clock++;
    pkt->src = rank;
    pkt->ts = l_clock;
    pthread_mutex_unlock(&l_clock_mut);
    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
}

void changeStateNew(state_t_new newState)
{
    pthread_mutex_lock(&stateMut);
    if (state_new == IN_FINISH)
    {
        pthread_mutex_unlock(&stateMut);
        return;
    }
    state_new = newState;
    pthread_mutex_unlock(&stateMut);
}