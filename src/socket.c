/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 15:59:31 by gkehren           #+#    #+#             */
/*   Updated: 2024/02/05 20:56:18 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

void	free_init(const char *str)
{
	if (str != NULL)
		perror(str);
	if (g_ping.sockfd > 0)
		close(g_ping.sockfd);
	free(g_ping.ip_address);
	free(g_ping.fqdn);
	free(g_ping.rtt);
	exit(1);
}

void	set_socket_option(void)
{
	if (setsockopt(g_ping.sockfd, IPPROTO_IP, IP_TTL,
			&g_ping.ttl, sizeof(g_ping.ttl)) < 0)
		free_init("setsockopt");
	g_ping.timeout.tv_sec = 1;
	g_ping.timeout.tv_usec = 0;
	if (setsockopt(g_ping.sockfd, SOL_SOCKET, SO_RCVTIMEO,
			&g_ping.timeout, sizeof(g_ping.timeout)) < 0)
		free_init("setsockopt");
}

void	init_socket2(void)
{
	g_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_ping.sockfd < 0)
		free_init("socket");
	set_socket_option();
	memset(&g_ping.target_addr, 0, sizeof(g_ping.target_addr));
	g_ping.target_addr.sin_family = AF_INET;
	g_ping.target_addr.sin_addr.s_addr = inet_addr(g_ping.ip_address);
}

void	malloc_g_ping(void)
{
	g_ping.buffer = NULL;
	g_ping.pid = htons(getpid());
	g_ping.num_pings = 5;
	g_ping.min_rtt = LONG_MAX;
	g_ping.tries = 0;
	g_ping.num_success = 0;
	g_ping.num_failures = 0;
	g_ping.rtt = malloc(sizeof(double));
	g_ping.total_rtt = 0;
	g_ping.min_rtt = LONG_MAX;
	g_ping.max_rtt = LONG_MIN;
	g_ping.packet_size = g_ping.size_number + 8;
}

int	init_socket(void)
{
	struct addrinfo		hints;
	struct addrinfo		*result;
	int					status;
	struct sockaddr_in	*ipv4;

	malloc_g_ping();
	hints = (struct addrinfo){0};
	hints.ai_family = AF_INET;
	status = getaddrinfo(g_ping.fqdn, NULL, &hints, &result);
	if (status != 0)
	{
		printf("ft_ping: unknown host\n");
		free_init(NULL);
	}
	ipv4 = (struct sockaddr_in *)result->ai_addr;
	g_ping.ip_address = ft_strdup(inet_ntoa(ipv4->sin_addr));
	freeaddrinfo(result);
	init_socket2();
	return (0);
}
