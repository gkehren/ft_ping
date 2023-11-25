/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:31:33 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 04:22:07 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

int	ft_strcmp(const char *s1, const char *s2)
{
	int	i;

	i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

void	free_realloc(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
	close(g_ping.sockfd);
	free(g_ping.packet->data);
	free(g_ping.packet);
	free(g_ping.ip_address);
	free(g_ping.fqdn);
	free(g_ping.rtt);
	exit(1);
}

void	ft_realloc(int size)
{
	double	*tmp;
	int		i;

	tmp = (double *)malloc(size * sizeof(double));
	if (!tmp)
		free_realloc(NULL);
	i = 0;
	while (i < g_ping.num_success)
	{
		tmp[i] = g_ping.rtt[i];
		i++;
	}
	free(g_ping.rtt);
	g_ping.rtt = (double *)malloc(size * sizeof(double));
	if (!g_ping.rtt)
		free_realloc(tmp);
	i = 0;
	while (i < g_ping.num_success)
	{
		g_ping.rtt[i] = tmp[i];
		i++;
	}
	free(tmp);
}

void	compute_stats(void)
{
	double	mean_rtt;
	double	sum_squared_diff;
	double	variance_rtt;
	double	diff;
	int		i;

	if (g_ping.num_success > 1)
	{
		mean_rtt = g_ping.total_rtt / g_ping.num_success;
		sum_squared_diff = 0.0;
		i = 0;
		while (i < g_ping.num_success)
		{
			diff = g_ping.rtt[i] - mean_rtt;
			sum_squared_diff += diff * diff;
			i++;
		}
		variance_rtt = sum_squared_diff / (double)g_ping.num_success;
		g_ping.stddev_rtt = sqrt(variance_rtt);
	}
	else
		g_ping.stddev_rtt = 0.0;
}

void	display_stats(void)
{
	compute_stats();
	printf("--- %s ping statistics ---\n", g_ping.ip_address);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
		g_ping.tries, g_ping.num_success,
		(g_ping.tries - g_ping.num_success) * 100 / g_ping.tries);
	if (g_ping.num_success > 0)
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			g_ping.min_rtt, g_ping.total_rtt / g_ping.num_success,
			g_ping.max_rtt, g_ping.stddev_rtt / g_ping.num_success);
}

uint16_t	calculate_checksum(void *data, int length)
{
	uint32_t	sum;
	uint16_t	*ptr;
	uint16_t	last_byte;

	sum = 0;
	ptr = data;
	while (length > 1)
	{
		sum += *ptr++;
		length -= 2;
	}
	if (length == 1)
	{
		last_byte = 0;
		*((uint8_t *)&last_byte) = *((uint8_t *)ptr);
		sum += last_byte;
	}
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	return ((uint16_t)(~sum));
}

double	get_elapsed_time(struct timeval *start_time, struct timeval *end_time)
{
	return ((end_time->tv_sec - start_time->tv_sec) * 1000.0
		+ (end_time->tv_usec - start_time->tv_usec) / 1000.0);
}
