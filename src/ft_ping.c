/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:31:45 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 16:59:38 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

void	init_packet(void)
{
	int	i;

	i = 0;
	while (i < g_ping.size_number - 1)
	{
		g_ping.packet->data[i] = g_ping.pattern;
		i++;
	}
	g_ping.packet->data[i] = '\0';
	g_ping.packet->header.type = ICMP_ECHO;
	g_ping.packet->header.code = 0;
	g_ping.packet->header.un.echo.id = g_ping.pid;
	g_ping.packet->header.un.echo.sequence = g_ping.tries;
	g_ping.packet->header.checksum = 0;
	g_ping.packet->header.checksum = calculate_checksum(g_ping.packet,
			g_ping.packet_size);
	gettimeofday(&g_ping.start_time, NULL);
}

int	handle_receive(uint8_t *buffer)
{
	struct iovec	iov[1];
	struct msghdr	msg;

	iov[0].iov_base = buffer;
	iov[0].iov_len = g_ping.size_number;
	msg = (struct msghdr){0};
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	if (recvmsg(g_ping.sockfd, &msg, 0) < 0)
	{
		if (errno == EINTR)
		{
			printf("Request timed out for icmp_seq %d\n", g_ping.tries);
			g_ping.tries++;
			g_ping.num_failures++;
		}
		else if (errno == EAGAIN)
		{
			g_ping.tries++;
			g_ping.num_failures++;
		}
		else
			return (error_handler("recvfrom", buffer));
	}
	return (0);
}

int	handle_packet(void)
{
	struct iphdr	*ip_header;
	struct icmphdr	*icmp_header;
	uint8_t			*buffer;

	buffer = malloc(sizeof(uint8_t) * g_ping.size_number);
	init_packet();
	if (sendto(g_ping.sockfd, g_ping.packet, g_ping.packet_size, 0,
			(struct sockaddr *)&g_ping.target_addr,
			sizeof(g_ping.target_addr)) < 0)
		return (error_handler("sendto", buffer));
	if (handle_receive(buffer) == 1)
		return (1);
	ip_header = (struct iphdr *)buffer;
	icmp_header = (struct icmphdr *)(buffer + sizeof(struct iphdr));
	display_packet(ip_header, icmp_header);
	free(buffer);
	return (0);
}

int	end_of_pings(void)
{
	display_stats();
	close(g_ping.sockfd);
	free(g_ping.packet->data);
	free(g_ping.packet);
	free(g_ping.ip_address);
	free(g_ping.fqdn);
	free(g_ping.rtt);
	if (g_ping.num_success > 0)
		return (0);
	else
		return (1);
}

int	ping(void)
{
	signal(SIGINT, handle_sigint);
	if (g_ping.w_timeout > 0)
	{
		signal(SIGALRM, handle_sigalrm);
		alarm(g_ping.w_timeout);
	}
	display_address();
	g_ping.packet = malloc(g_ping.packet_size);
	memset(g_ping.packet, 0, g_ping.packet_size);
	g_ping.packet->data = malloc(g_ping.size_number);
	while (1)
	{
		if (handle_packet() == 1)
			return (1);
		g_ping.tries++;
		usleep(1000000);
	}
	return (end_of_pings());
}
