#include "main.h"
#include "watek_glowny.h"

static int cmpfunc(const void *a, const void *b)
{
	packet_t **packet_a = (packet_t **)a;
	packet_t **packet_b = (packet_t **)b;
	int diff_ts = (*packet_a)->ts - (*packet_b)->ts;
	if (diff_ts != 0)
	{
		return diff_ts;
	}
	return (*packet_a)->src - (*packet_b)->src;
}

void mainLoop()
{
	srandom(rank);
	int tag;
	int perc;

	// while (stan != InFinish)
	// {
	// 	switch (stan)
	// 	{
	// 	case InRun:
	// 		perc = random() % 100;
	// 		if (perc < 25)
	// 		{
	// 			debug("Perc: %d", perc);
	// 			println("Ubiegam się o sekcję krytyczną")
	// 				debug("Zmieniam stan na wysyłanie");
	// 			packet_t *pkt = malloc(sizeof(packet_t));
	// 			pkt->data = perc;
	// 			ackCount = 0;
	// 			for (int i = 0; i <= size - 1; i++)
	// 				if (i != rank)
	// 					sendPacket(pkt, i, REQUEST);
	// 			changeState(InWant);
	// 			free(pkt);
	// 		}
	// 		debug("Skończyłem myśleć");
	// 		break;
	// 	case InWant:
	// 		println("Czekam na wejście do sekcji krytycznej")
	// 			// tutaj zapewne jakiś muteks albo zmienna warunkowa
	// 			// bo aktywne czekanie jest BUE
	// 			if (ackCount == size - 1)
	// 				changeState(InSection);
	// 		break;
	// 	case InSection:
	// 		// tutaj zapewne jakiś muteks albo zmienna warunkowa
	// 		println("Jestem w sekcji krytycznej")
	// 			sleep(5);
	// 		// if ( perc < 25 ) {
	// 		debug("Perc: %d", perc);
	// 		println("Wychodzę z sekcji krytyczneh")
	// 			debug("Zmieniam stan na wysyłanie");
	// 		packet_t *pkt = malloc(sizeof(packet_t));
	// 		pkt->data = perc;
	// 		for (int i = 0; i <= size - 1; i++)
	// 			if (i != rank)
	// 				sendPacket(pkt, (rank + 1) % size, RELEASE);
	// 		changeState(InRun);
	// 		free(pkt);
	// 		//}
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// 	sleep(SEC_IN_STATE);
	// }
	state_t_new previous_state = CANT_GO_DONT_WANT;
	while (state_new != IN_FINISH)
	{
		sleep(1);
		pthread_mutex_lock(&new_message_mut);
		while (new_message == FALSE && state_new == previous_state)
		{
			pthread_cond_wait(&new_message_cond, &new_message_mut);
		}
		previous_state = state_new;
		pthread_mutex_unlock(&new_message_mut);
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
				sendPacket(pkt, i, REQUEST);
			}
			l_clock_req = l_clock;
			changeStateNew(CANT_GO_DO_WANT);
			break;
		}
		case CANT_GO_DO_WANT:
		{
			debug("ubiegam sie o wejscie");
			pthread_mutex_lock(&timestamps_mut);
			int should_break = FALSE;
			for (int i = 0; i < size; i++)
			{
				if (i != rank && timestamps[i] <= l_clock_req)
				{
					should_break = TRUE;
					break;
				}
			}
			pthread_mutex_unlock(&timestamps_mut);
			if (should_break)
			{
				break;
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
			debug("wchodze");
			pthread_mutex_lock(&requests_mut);
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
			pthread_mutex_unlock(&requests_mut);
			x_without_us += m_sum;
			x_with_us = x_without_us + m;
			if (x_with_us <= X)
			{
				debug("jestem w srodku x ok");
				sleep(random() % 5);
				pthread_mutex_lock(&l_clock_mut);
				l_clock = l_clock + 1;
				pthread_mutex_unlock(&l_clock_mut);
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = m;
				for (int i = 0; i <= size - 1; i++)
				{
					sendPacket(pkt, i, RELEASE);
				}
				free(pkt);
				m += random() % max_random_m;
				if (m > M)
				{
					changeStateNew(IN_FINISH);
					debug("koncze");
					break;
				}
				changeStateNew(CANT_GO_DONT_WANT);
				debug("wychodze");
				break;
			}
			else
			{
				if (x_without_us < X)
				{
					debug("jestem w srodku wymieniam x");
					sleep(random() % 5);
					pthread_mutex_lock(&l_clock_mut);
					l_clock = l_clock + 1;
					pthread_mutex_unlock(&l_clock_mut);
					packet_t *pkt = malloc(sizeof(packet_t));
					pkt->data = x_without_us + m;
					for (int i = 0; i <= size - 1; i++)
					{
						sendPacket(pkt, i, REPLACE);
					}
					free(pkt);
				}
				break;
			}
		}
		}
		pthread_mutex_lock(&new_message_mut);
		new_message = FALSE;
		pthread_cond_signal(&new_message_cond);
		pthread_mutex_unlock(&new_message_mut);
	}
}
