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

unsigned short calculate_checksum(void *buf, int len) {
	unsigned short *ptr = buf;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2) {
		sum += *ptr++;
	}

	if (len == 1) {
		sum += *(unsigned char*)ptr;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;

	return result;
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
