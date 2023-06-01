#include "main.h"
#include "watek_glowny.h"

int cmpfunc(const void *a, const void *b)
{
	packet_t **packet_a = (packet_t **)a;
	packet_t **packet_b = (packet_t **)b;
	return (*packet_a)->ts - (*packet_b)->ts;
}

void mainLoop()
{
	srandom(rank);
	int tag;
	int perc;

	while (stan != InFinish)
	{
		switch (stan)
		{
		case InRun:
			perc = random() % 100;
			if (perc < 25)
			{
				debug("Perc: %d", perc);
				println("Ubiegam się o sekcję krytyczną")
					debug("Zmieniam stan na wysyłanie");
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				ackCount = 0;
				for (int i = 0; i <= size - 1; i++)
					if (i != rank)
						sendPacket(pkt, i, REQUEST);
				changeState(InWant);
				free(pkt);
			}
			debug("Skończyłem myśleć");
			break;
		case InWant:
			println("Czekam na wejście do sekcji krytycznej")
				// tutaj zapewne jakiś muteks albo zmienna warunkowa
				// bo aktywne czekanie jest BUE
				if (ackCount == size - 1)
					changeState(InSection);
			break;
		case InSection:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w sekcji krytycznej")
				sleep(5);
			// if ( perc < 25 ) {
			debug("Perc: %d", perc);
			println("Wychodzę z sekcji krytyczneh")
				debug("Zmieniam stan na wysyłanie");
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = perc;
			for (int i = 0; i <= size - 1; i++)
				if (i != rank)
					sendPacket(pkt, (rank + 1) % size, RELEASE);
			changeState(InRun);
			free(pkt);
			//}
			break;
		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}

	while (state_new != IN_FINISH)
	{
		int x_without_us;
		int x_with_us;
		switch (state_new)
		{
		case CANT_GO_DONT_WANT:
		{
			l_clock++;
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = m;
			for (int i = 0; i <= size - 1; i++)
			{
				if (i != rank)
				{
					sendPacket(pkt, i, REQUEST);
				}
			}
			free(pkt);
			requests[requests_size] = pkt;
			requests_size++;
			qsort(requests, requests_size, sizeof(packet_t *), cmpfunc);
			l_clock_req = l_clock;
			changeStateNew(CANT_GO_DO_WANT);
			break;
		}
		case CANT_GO_DO_WANT:
		{

			for (int i = 0; i < size; i++)
			{
				if (timestamps[i] <= l_clock_req)
				{
					break;
				}
			}
			int req_count = 0;
			int m_sum = 0;
			for (int i = 0; i < requests_size; i++)
			{
				if (requests[i]->src != rank)
				{
					req_count++;
					m_sum += requests[i]->data;
				}
				else
				{
					break;
				}
			}
			if (req_count > S)
			{
				break;
			}
			if (m_sum >= M - m)
			{
				break;
			}
			changeStateNew(CAN_GO);
			break;
		}
		case CAN_GO:
		{
			x_without_us = x;
			int m_sum = 0;
			for (int i = 0; i < requests_size; i++)
			{
				if (requests[i]->src != rank)
				{
					m_sum += requests[i]->data;
				}
				else
				{
					break;
				}
			}
			x_without_us += m_sum;
			x_with_us = x_without_us + m;
			if (x_with_us <= X)
			{
				changeStateNew(INSIDE_X_OK);
				break;
			}
			else
			{
				changeStateNew(INSIDE_X_NO_OK);
				break;
			}
		}
		case INSIDE_X_OK:
		{
			sleep(random() % 5);
			pthread_mutex_lock(&l_clock_mut);
			l_clock = l_clock + 1;
			pthread_mutex_unlock(&l_clock_mut);
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = m;
			for (int i = 0; i <= size - 1; i++)
			{
				if (i != rank)
				{
					sendPacket(pkt, i, RELEASE);
				}
			}
			free(pkt);
			m += random() % max_random_m;
			changeStateNew(CANT_GO_DONT_WANT);
			break;
		}
		case INSIDE_X_NO_OK:
		{
			if (x_without_us < X)
			{
				sleep(random() % 5);
				pthread_mutex_lock(&l_clock_mut);
				l_clock = l_clock + 1;
				pthread_mutex_unlock(&l_clock_mut);
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = x_without_us + m;
				for (int i = 0; i <= size - 1; i++)
				{
					if (i != rank)
					{
						sendPacket(pkt, i, REPLACE);
					}
				}
				free(pkt);
				changeStateNew(CAN_GO);
			}
			else
			{
				changeStateNew(CAN_GO);
			}
			break;
		}
		}
	}
}
