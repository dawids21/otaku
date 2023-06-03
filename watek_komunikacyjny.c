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
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (state_new != IN_FINISH)
    {
        pthread_mutex_lock(&new_message_mut);
        while (new_message == TRUE)
        {
            pthread_cond_wait(&new_message_cond, &new_message_mut);
        }
        pthread_mutex_unlock(&new_message_mut);
        packet_t *pkt = malloc(sizeof(packet_t));
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
            pthread_mutex_lock(&l_clock_mut);
            packet_t *pkt2 = malloc(sizeof(packet_t));
            pkt2->ts = pkt->ts;
            pkt2->src = pkt->src;
            pkt2->data = pkt->data;
            requests[requests_size] = pkt2;
			requests_size++;
			qsort(requests, requests_size, sizeof(packet_t *), cmpfunc);
            l_clock += 1;
            pthread_mutex_unlock(&l_clock_mut);
            pthread_mutex_unlock(&requests_mut);
            pkt2 = malloc(sizeof(packet_t));
            sendPacket(pkt2, pkt->src, ACK);
            free(pkt2);
            break;
        case RELEASE:
            pthread_mutex_lock(&requests_mut);
            int index_of_request = 0;
            while (requests[index_of_request]->src != pkt->src)
            {
                index_of_request++;
            }
            packet_t *request_to_free = requests[index_of_request];
            for (int i = index_of_request + 1; i < requests_size; i++)
            {
                requests[i] = requests[i - 1];
            }
            requests_size--;
            pthread_mutex_unlock(&requests_mut);
            x += pkt->data;
            free(request_to_free);
            break;
        case REPLACE:
            x -= pkt->data;
            break;
        }
        free(pkt);
        pthread_mutex_lock(&new_message_mut);
        new_message = TRUE;
        pthread_cond_signal(&new_message_cond);
        pthread_mutex_unlock(&new_message_mut);
    }
}
