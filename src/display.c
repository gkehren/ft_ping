/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 15:17:44 by gkehren           #+#    #+#             */
/*   Updated: 2024/02/05 18:56:03 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

const char	*get_icmp_type(uint8_t type)
{
	const char	*icmp_types[] = {
	[0] = "Echo Reply",
	[3] = "Destination Host Unreachable",
	[4] = "Source Quench",
	[5] = "Redirect Message",
	[8] = "Echo Request",
	[9] = "Router Advertisement",
	[10] = "Router Solicitation",
	[11] = "Time Exceeded",
	[12] = "Parameter Problem: Bad IP header",
	[13] = "Timestamp",
	[14] = "Timestamp Reply",
	[15] = "Information Request",
	[16] = "Information Reply",
	[17] = "Address Mask Request",
	[18] = "Address Mask Reply",
	};

	if (type <= 18)
		return (icmp_types[type]);
	else
		return ("Unknown");
}

void	display_packet(struct iphdr *ip_header, struct icmphdr *icmp_header)
{
	if (icmp_header == NULL || ip_header == NULL)
		return ;
	if (icmp_header->type == ICMP_ECHOREPLY)
	{
		g_ping.rtt[g_ping.num_success] = g_ping.elapsed_time;
		gettimeofday(&g_ping.end_time, NULL);
		g_ping.elapsed_time = get_elapsed_time(&g_ping.start_time,
				&g_ping.end_time);
		g_ping.num_success++;
		g_ping.total_rtt += g_ping.elapsed_time;
		if (g_ping.elapsed_time < g_ping.min_rtt)
			g_ping.min_rtt = g_ping.elapsed_time;
		if (g_ping.elapsed_time > g_ping.max_rtt)
			g_ping.max_rtt = g_ping.elapsed_time;
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			g_ping.size_number + 8, g_ping.ip_address, g_ping.tries,
			ip_header->ttl, g_ping.elapsed_time);
		ft_realloc(g_ping.num_success + 1);
	}
	else
	{
		printf("%d bytes from %s: %s\n", g_ping.size_number + 8,
			g_ping.ip_address, get_icmp_type(icmp_header->type));
		g_ping.num_failures++;
	}
}

void	display_address(void)
{
	if (g_ping.verbose == 1)
	{
		printf("PING %s (%s): %d data bytes, id 0x%x = %u\n", g_ping.fqdn,
			g_ping.ip_address, g_ping.size_number, g_ping.pid, g_ping.pid);
	}
	else
	{
		printf("PING %s (%s): %d data bytes\n",
			g_ping.fqdn, g_ping.ip_address, g_ping.size_number);
	}
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
