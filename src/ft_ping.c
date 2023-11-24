/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:31:45 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 00:08:16 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

const char	*get_icmp_type(uint8_t type)
{
	switch (type)
	{
		case 0: return "Echo Reply";
		case 3: return "Destination Host Unreachable";
		case 4: return "Source Quench";
		case 5: return "Redirect Message";
		case 8: return "Echo Request";
		case 9: return "Router Advertisement";
		case 10: return "Router Solicitation";
		case 11: return "Time Exceeded";
		case 12: return "Parameter Problem: Bad IP header";
		case 13: return "Timestamp";
		case 14: return "Timestamp Reply";
		case 15: return "Information Request";
		case 16: return "Information Reply";
		case 17: return "Address Mask Request";
		case 18: return "Address Mask Reply";
		default: return "Unknown";
	}
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

int	ping()
{
	uint8_t			buffer[g_ping.size_number];
	struct iovec	iov[1];

	iov[0].iov_base = &buffer;
	iov[0].iov_len = sizeof(buffer);
	struct msghdr msg = {0};
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	signal(SIGINT, handle_sigint);
	if (g_ping.w_timeout > 0)
	{
		signal(SIGALRM, handle_sigalrm);
		alarm(g_ping.w_timeout);
	}
	if (g_ping.verbose == 1)
		printf("PING %s (%s): %d data bytes, id 0x%x = %u\n", g_ping.numeric == 0 ? g_ping.fqdn : g_ping.ip_address, g_ping.ip_address, g_ping.size_number, g_ping.pid, g_ping.pid);
	else
		printf("PING %s (%s): %d data bytes\n", g_ping.numeric == 0 ? g_ping.fqdn : g_ping.ip_address, g_ping.ip_address, g_ping.size_number);
	g_ping.packet = malloc(g_ping.packet_size);
	memset(g_ping.packet, 0, g_ping.packet_size);
	g_ping.packet->data = malloc(g_ping.size_number);
	while (1)
	{
		for (int i = 0; i < g_ping.size_number; i++)
			g_ping.packet->data[i] = 0xA5;
		g_ping.packet->header.type = ICMP_ECHO;
		g_ping.packet->header.code = 0;
		g_ping.packet->header.un.echo.id = g_ping.pid;
		g_ping.packet->header.un.echo.sequence = g_ping.tries;
		g_ping.packet->header.checksum = 0;
		g_ping.packet->header.checksum = calculate_checksum(g_ping.packet, g_ping.packet_size);
		gettimeofday(&g_ping.start_time, NULL);
		// Send the ICMP packet to the target address
		if (sendto(g_ping.sockfd, g_ping.packet, g_ping.packet_size, 0, (struct sockaddr *)&g_ping.target_addr, sizeof(g_ping.target_addr)) < 0)
		{
			perror("sendto");
			close(g_ping.sockfd);
			free(g_ping.packet->data);
			free(g_ping.packet);
			free(g_ping.ip_address);
			free(g_ping.fqdn);
			free(g_ping.rtt);
			return (1);
		}
		// Wait for an ICMP packet to be received
		if (recvmsg(g_ping.sockfd, &msg, 0) < 0)
		{
			if (errno == EINTR)
			{
				printf("Request timed out for icmp_seq %d\n", g_ping.tries);
				g_ping.tries++;
				g_ping.num_failures++;
				continue ;
			}
			else if (errno == EAGAIN)
			{
				g_ping.tries++;
				g_ping.num_failures++;
				continue ;
			}
			else
			{
				perror("recvfrom");
				close(g_ping.sockfd);
				free(g_ping.packet->data);
				free(g_ping.packet);
				free(g_ping.ip_address);
				free(g_ping.fqdn);
				free(g_ping.rtt);
				return (1);
			}
		}
		struct iphdr	*ip_header = (struct iphdr *)buffer;
		struct icmphdr	*icmp_header = (struct icmphdr *)(buffer + sizeof(struct iphdr));
		if (icmp_header->type == ICMP_ECHOREPLY)
		{
			g_ping.rtt[g_ping.num_success] = g_ping.elapsed_time;
			gettimeofday(&g_ping.end_time, NULL);
			g_ping.elapsed_time = get_elapsed_time(&g_ping.start_time, &g_ping.end_time);
			g_ping.num_success++;
			g_ping.total_rtt += g_ping.elapsed_time;
			if (g_ping.elapsed_time < g_ping.min_rtt)
				g_ping.min_rtt = g_ping.elapsed_time;
			if (g_ping.elapsed_time > g_ping.max_rtt)
				g_ping.max_rtt = g_ping.elapsed_time;
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				g_ping.size_number + 8, g_ping.ip_address, g_ping.tries, ip_header->ttl, g_ping.elapsed_time);
			ft_realloc(g_ping.num_success + 1);
		}
		else
		{
			printf("%d bytes from %s: %s\n", g_ping.size_number + 8, g_ping.ip_address, get_icmp_type(icmp_header->type));
			g_ping.num_failures++;
		}
		g_ping.tries++;
		usleep(1000000);
	}
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
