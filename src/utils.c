#include "../include/ft_ping.h"

extern t_ping ft_ping;

int	ft_strcmp(const char *s1, const char *s2)
{
	int	i;

	i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

void	ft_realloc(int size)
{
	double	*tmp;
	int		i;

	tmp = (double *)malloc(size * sizeof(double));
	if (!tmp)
	{
		close(ft_ping.sockfd);
		free(ft_ping.ip_address);
		free(ft_ping.fqdn);
		free(ft_ping.rtt);
		exit(1);
	}
	i = 0;
	while (i < ft_ping.num_success)
	{
		tmp[i] = ft_ping.rtt[i];
		i++;
	}
	free(ft_ping.rtt);
	ft_ping.rtt = (double *)malloc(size * sizeof(double));
	if (!ft_ping.rtt)
	{
		close(ft_ping.sockfd);
		free(ft_ping.ip_address);
		free(ft_ping.fqdn);
		free(ft_ping.rtt);
		free(tmp);
		exit(1);
	}
	i = 0;
	while (i < ft_ping.num_success)
	{
		ft_ping.rtt[i] = tmp[i];
		i++;
	}
	free(tmp);
}

void	display_stats()
{
	if (ft_ping.num_success > 1)
	{
		// TODO stddev is wrong
		double mean_rtt = ft_ping.total_rtt / ft_ping.num_success;
		double sum_squared_diff = 0.0;
		for (int i = 0; i < ft_ping.num_success; i++)
		{
			double diff = ft_ping.rtt[i] - mean_rtt;
			sum_squared_diff += diff * diff;
		}
		double variance_rtt = sum_squared_diff / (double)ft_ping.num_success;
		ft_ping.stddev_rtt = sqrt(variance_rtt);
	}
	else
		ft_ping.stddev_rtt = 0.0;

	printf("--- %s ping statistics ---\n", ft_ping.ip_address);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
		ft_ping.tries, ft_ping.num_success, (ft_ping.tries - ft_ping.num_success) * 100 / ft_ping.tries);
	if (ft_ping.num_success > 0)
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ft_ping.min_rtt, ft_ping.total_rtt / ft_ping.num_success, ft_ping.max_rtt, ft_ping.stddev_rtt / ft_ping.num_success);
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

double get_elapsed_time(struct timeval *start_time, struct timeval *end_time)
{
	return (end_time->tv_sec - start_time->tv_sec) * 1000.0 + (end_time->tv_usec - start_time->tv_usec) / 1000.0;
}

