#include "../include/ft_ping.h"

int	ft_strcmp(const char *s1, const char *s2)
{
	int	i;

	i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
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

