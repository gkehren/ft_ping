/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:31:45 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 03:37:50 by gkehren          ###   ########.fr       */
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

void	handle_sigint(int signal)
{
	(void)signal;
	if (g_ping.tries > 0)
		display_stats();
	close(g_ping.sockfd);
	free(g_ping.packet->data);
	free(g_ping.packet);
	free(g_ping.ip_address);
	free(g_ping.fqdn);
	free(g_ping.rtt);
	exit(0);
}

void	handle_sigalrm(int signal)
{
	(void)signal;
	if (g_ping.tries > 0)
		display_stats();
	close(g_ping.sockfd);
	free(g_ping.packet->data);
	free(g_ping.packet);
	free(g_ping.ip_address);
	free(g_ping.fqdn);
	free(g_ping.rtt);
	exit(0);
}

int	error_handler(const char *str, void *ptr)
{
	if (str != NULL)
		perror(str);
	if (ptr != NULL)
		free(ptr);
	close(g_ping.sockfd);
	free(g_ping.packet->data);
	free(g_ping.packet);
	free(g_ping.ip_address);
	free(g_ping.fqdn);
	free(g_ping.rtt);
	return (1);
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

void	init_packet(void)
{
	int	i;

	i = 0;
	while (i < g_ping.size_number)
	{
		g_ping.packet->data[i] = 0xA5;
		i++;
	}
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

void	display_address(void)
{
	if (g_ping.verbose == 1)
	{
		if (g_ping.numeric == 0)
			printf("PING %s (%s): %d data bytes, id 0x%x = %u\n", g_ping.fqdn,
				g_ping.ip_address, g_ping.size_number, g_ping.pid, g_ping.pid);
		else
			printf("PING %s (%s): %d data bytes, id 0x%x = %u\n",
				g_ping.ip_address, g_ping.ip_address, g_ping.size_number,
				g_ping.pid, g_ping.pid);
	}
	else
	{
		if (g_ping.numeric == 0)
			printf("PING %s (%s): %d data bytes\n",
				g_ping.fqdn, g_ping.ip_address, g_ping.size_number);
		else
			printf("PING %s (%s): %d data bytes\n", g_ping.ip_address,
				g_ping.ip_address, g_ping.size_number);
	}
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
