/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:31:45 by gkehren           #+#    #+#             */
/*   Updated: 2024/02/05 20:52:50 by gkehren          ###   ########.fr       */
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

int	handle_receive_error(uint8_t *buffer)
{
	if (errno == EINTR)
	{
		printf("Request timed out for icmp_seq %d\n", g_ping.tries);
		g_ping.tries++;
		g_ping.num_failures++;
		return (0);
	}
	else if (errno == EAGAIN)
	{
		g_ping.tries++;
		g_ping.num_failures++;
		return (0);
	}
	return (error_handler("recvfrom", buffer));
}

int	handle_receive(uint8_t *buffer)
{
	struct iovec	iov[1];
	struct msghdr	msg;
	ssize_t			num_bytes;

	iov[0].iov_base = buffer;
	iov[0].iov_len = g_ping.size_number;
	msg = (struct msghdr){0};
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	num_bytes = recvmsg(g_ping.sockfd, &msg, 0);
	if (num_bytes < 0)
	{
		if (handle_receive_error(buffer) == 0)
			return (0);
		else
			return (-1);
	}
	return (num_bytes);
}

int	handle_packet(void)
{
	struct iphdr	*ip_header;
	struct icmphdr	*icmp_header;
	int				num_bytes;

	g_ping.buffer = malloc(sizeof(uint8_t) * g_ping.size_number);
	init_packet();
	if (sendto(g_ping.sockfd, g_ping.packet, g_ping.packet_size, 0,
			(struct sockaddr *)&g_ping.target_addr,
			sizeof(g_ping.target_addr)) < 0)
		return (error_handler("sendto", g_ping.buffer));
	num_bytes = handle_receive(g_ping.buffer);
	if (num_bytes == -1)
		return (-1);
	else if ((size_t)num_bytes > sizeof(struct iphdr) + sizeof(struct icmphdr))
	{
		ip_header = (struct iphdr *)g_ping.buffer;
		icmp_header = (struct icmphdr *)(g_ping.buffer + sizeof(struct iphdr));
		display_packet(ip_header, icmp_header);
	}
	free(g_ping.buffer);
	g_ping.buffer = NULL;
	return (0);
}

int	ping(void)
{
	signal(SIGINT, handle_sigint);
	display_address();
	g_ping.packet = malloc(g_ping.packet_size);
	memset(g_ping.packet, 0, g_ping.packet_size);
	g_ping.packet->data = malloc(g_ping.size_number);
	while (1)
	{
		if (handle_packet() == -1)
			return (1);
		g_ping.tries++;
		usleep(1000000);
	}
	return (end_of_pings());
}
