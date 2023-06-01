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
		switch (state_new)
		{
		case CANT_GO_DONT_WANT:
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
			requests[requests_size] = pkt;
			requests_size++;
			qsort(requests, requests_size, sizeof(packet_t *), cmpfunc);
			l_clock_req = l_clock;
			changeStateNew(CANT_GO_DO_WANT);
			break;
		case CANT_GO_DO_WANT:
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
		}
	}
}
