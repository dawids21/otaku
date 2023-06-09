#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srandom(time(NULL) + rank);
	state_t_new previous_state = CANT_GO_DONT_WANT;
	while (state_new != IN_FINISH)
	{
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
			println("Ubiegam się o wejscie");
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
			pthread_mutex_lock(&requests_mut);
			int should_break = FALSE;
			for (int i = 0; i < size; i++)
			{
				if (i != rank && timestamps[i] <= l_clock_req && finished[i] == 0)
				{
					should_break = TRUE;
					break;
				}
			}
			pthread_mutex_unlock(&requests_mut);
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
			println("Sprawdzam warunki wejscia (req_count: %d, m_sum: %d)", req_count, m_sum);
			if (req_count >= S)
			{
				break;
			}
			if (m_sum > M - m)
			{
				break;
			}
			changeStateNew(CAN_GO);
			break;
		}
		case CAN_GO:
		{
			println("Sprawdzam czy trzeba wymienic przedstawiciela");
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
				println("Jestem w sekcji krytycznej nie wymieniam przedstawiciela");
				sleep(random() % 5);
				println("Opuszczam sekcje krytyczna");
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
					println("Koncze dzialanie m za duze by wejsc");
					break;
				}
				changeStateNew(CANT_GO_DONT_WANT);
				debug("wychodze");
				break;
			}
			else
			{
				if (x_without_us <= X)
				{
					println("Jestem w sekcji krytycznej wymieniam przedstawiciela");
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
					sleep(random() % 5);
					println("Opuszczam sekcje krytyczna");
					pthread_mutex_lock(&l_clock_mut);
					l_clock = l_clock + 1;
					pthread_mutex_unlock(&l_clock_mut);
					pkt = malloc(sizeof(packet_t));
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
						println("Koncze dzialanie m za duze by wejsc");
						break;
					}
					changeStateNew(CANT_GO_DONT_WANT);
					debug("wychodze");
					break;
				}
				else
				{
					println("Ktos musi wymienic przedstawiciela");
				}
			}
		}
		}
		pthread_mutex_lock(&new_message_mut);
		new_message = FALSE;
		pthread_cond_signal(&new_message_cond);
		pthread_mutex_unlock(&new_message_mut);
	}
	packet_t *pkt = malloc(sizeof(packet_t));
	pkt->data = m;
	for (int i = 0; i <= size - 1; i++)
	{
		if (i != rank)
		{
			sendPacket(pkt, i, FINISH);
		}
	}
}
