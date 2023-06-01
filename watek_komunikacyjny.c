#include "main.h"
#include "watek_komunikacyjny.h"

static int cmpfunc(const void *a, const void *b)
{
	packet_t **packet_a = (packet_t **)a;
	packet_t **packet_b = (packet_t **)b;
	return (*packet_a)->ts - (*packet_b)->ts;
}

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t *pkt = malloc(sizeof(packet_t));
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (state_new != IN_FINISH)
    {
        debug("czekam na recv");
        MPI_Recv(pkt, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&l_clock_mut);
        if (pkt->ts > l_clock)
        {
            l_clock = pkt->ts + 1;
        }
        else
        {
            l_clock++;
        }
        pthread_mutex_unlock(&l_clock_mut);
        pthread_mutex_lock(&timestamps_mut);
        if (pkt->ts > timestamps[pkt->src])
        {
            timestamps[pkt->src] = pkt->ts;
        }
        pthread_mutex_unlock(&timestamps_mut);

        switch (status.MPI_TAG)
        {
        case REQUEST:
            pthread_mutex_lock(&requests_mut);
            requests[requests_size] = pkt;
			requests_size++;
			qsort(requests, requests_size, sizeof(packet_t *), cmpfunc);
            pthread_mutex_unlock(&requests_mut);
            l_clock += 1;
            packet_t *pkt2 = malloc(sizeof(packet_t));
            for (int i = 0; i <= size - 1; i++)
            {
                if (i != rank)
                {
                    sendPacket(pkt2, i, ACK);
                }
            }
            break;
        case RELEASE:
            pthread_mutex_lock(&requests_mut);
            int index_of_request = 0;
            while (requests[index_of_request]->src != pkt->src)
            {
                index_of_request++;
            }
            free(requests[index_of_request]);
            for (int i = index_of_request + 1; i < requests_size; i++)
            {
                requests[i] = requests[i - 1];
            }
            requests_size--;
            pthread_mutex_unlock(&requests_mut);
            x -= pkt->data;
            break;
        case REPLACE:
            x -= pkt->data;
            break;
        }
    }
    // while (stan != InFinish)
    //     {
    //     MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //     pthread_mutex_lock(&l_clock_mut);
    //     if (pakiet.ts > l_clock)
    //     {
    //         l_clock = pakiet.ts + 1;
    //     }
    //     else
    //     {
    //         l_clock++;
    //     }
    //     pthread_mutex_unlock(&l_clock_mut);

    //     switch (status.MPI_TAG)
    //     {
    //     case REQUEST:
    //         debug("Ktoś coś prosi. A niech ma!")
    //             sendPacket(0, status.MPI_SOURCE, ACK);
    //         break;
    //     case ACK:
    //         debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
    //         ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
    //         break;
    //     default:
    //         break;
    //     }
    //     }
}
