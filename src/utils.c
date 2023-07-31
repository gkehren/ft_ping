#include "../include/ft_ping.h"

void ft_bzero(void *s, size_t n)
{
	size_t i;
	char *ptr;
	i = 0;
	ptr = (char *)s;
	while (i < n)
	{
		ptr[i] = '\0';
		i++;
	}
	s = (void *)ptr;
}

uint16_t calculate_checksum(const void *data, size_t length)
{
	uint32_t sum = 0;
	const uint16_t *ptr = data;

	while (length > 1)
	{
		sum += *ptr++;
		length -= 2;
	}

	if (length == 1)
		sum += *(uint8_t *)ptr;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);

	return ~sum;
}

void handle_alarm(int signal)
{
	(void)signal;
	// Do nothing
}

double get_elapsed_time(struct timeval *start_time, struct timeval *end_time)
{
	return (end_time->tv_sec - start_time->tv_sec) * 1000.0 + (end_time->tv_usec - start_time->tv_usec) / 1000.0;
}
