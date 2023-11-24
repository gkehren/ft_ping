/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:25:29 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 00:08:53 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

t_ping	g_ping;

int	ft_atoi(const char *str)
{
	int	i;
	int	nbr;
	int	sign;

	i = 0;
	nbr = 0;
	sign = 1;
	while (str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] != '\0' && str[i] >= '0' && str[i] <= '9')
	{
		nbr = (nbr * 10) + str[i] - 48;
		i++;
	}
	return (nbr * sign);
}

int	get_next_arg(char *arg)
{
	int	i;

	if (arg == NULL)
	{
		printf("ft_ping: option requires an argument -- '%c'\n", arg[1]);
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	i = 0;
	while (arg[i])
	{
		if (arg[i] < '0' || arg[i] > '9')
		{
			printf("ft_ping: invalid value ('%s')\n", arg);
			return (1);
		}
		i++;
	}
	return (0);
}

int	parse_one_arg(char *arg, char *next_arg)
{
	if (ft_strcmp(arg, "-?") == 0)
		g_ping.help = 1;
	else if (ft_strcmp(arg, "-v") == 0)
		g_ping.verbose = 1;
	else if (arg[0] != '-' && g_ping.fqdn == NULL)
		g_ping.fqdn = ft_strdup(arg);
	else if (ft_strcmp(arg, "-f") == 0)
		g_ping.flood = 1;
	else if (ft_strcmp(arg, "-l") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		g_ping.preload = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-n") == 0)
		g_ping.numeric = 1;
	else if (ft_strcmp(arg, "-w") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		if (ft_atoi(next_arg) <= 0)
		{
			printf("ft_ping: option value too small: '%s'\n", next_arg);
			return (1);
		}
		g_ping.w_timeout = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-W") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		if (ft_atoi(next_arg) <= 0)
		{
			printf("ft_ping: option value too small: '%s'\n", next_arg);
			return (1);
		}
		g_ping.linger = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-p") == 0)
	{
		// need to handle -p differently because the next arg is a pattern
	}
	else if (ft_strcmp(arg, "-r") == 0)
		g_ping.ignore_routing = 1;
	else if (ft_strcmp(arg, "-s") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		g_ping.size_number = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-T") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		g_ping.tos = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-ttl") == 0)
	{
		if (get_next_arg(next_arg) == 1)
			return (1);
		g_ping.ttl = ft_atoi(next_arg);
	}
	else if (ft_strcmp(arg, "-ip-timestamp") == 0)
		g_ping.ip_timestamp = 1;
	else if (arg[0] == '-')
	{
		printf("ft_ping: invalid value ('%c')\n", arg[1]);
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	return (0);
}

void	ft_free_split(char **arg)
{
	int	i;

	i = 0;
	while (arg[i])
	{
		free(arg[i]);
		i++;
	}
	free(arg);
}

void	parse_args(char **argv)
{
	int		i;
	char	**arg;

	i = 0;
	arg = ft_split(argv, ' ');
	while (arg[i])
	{
		if (parse_one_arg(arg[i], arg[i + 1]) == 1)
		{
			ft_free_split(arg);
			free(g_ping.fqdn);
			exit(1);
		}
		i++;
	}
	ft_free_split(arg);
}

int	init_socket(void)
{
	struct addrinfo		hints;
	struct addrinfo		*result;
	int					status;
	struct sockaddr_in	*ipv4;

	g_ping.pid = htons(getpid());
	g_ping.num_pings = 5;
	g_ping.min_rtt = DBL_MAX;
	g_ping.tries = 0;
	g_ping.num_success = 0;
	g_ping.num_failures = 0;
	g_ping.rtt = malloc(sizeof(double));
	g_ping.packet_size = g_ping.size_number + 8;
	hints = (struct addrinfo){0};
	hints.ai_family = AF_INET;
	status = getaddrinfo(g_ping.fqdn, NULL, &hints, &result);
	if (status != 0)
	{
		printf("ft_ping: unknown host\n");
		free(g_ping.ip_address);
		free(g_ping.fqdn);
		free(g_ping.rtt);
		exit(1);
	}
	ipv4 = (struct sockaddr_in *)result->ai_addr;
	g_ping.ip_address = ft_strdup(inet_ntoa(ipv4->sin_addr));
	freeaddrinfo(result);
	g_ping.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_ping.sockfd < 0)
	{
		perror("socket");
		free(g_ping.ip_address);
		free(g_ping.fqdn);
		free(g_ping.rtt);
		return (1);
	}
	if (setsockopt(g_ping.sockfd, IPPROTO_IP, IP_TTL,
			&g_ping.ttl, sizeof(g_ping.ttl)) < 0)
	{
		perror("setsockopt");
		free(g_ping.ip_address);
		free(g_ping.packet->data);
		free(g_ping.packet);
		close(g_ping.sockfd);
		free(g_ping.fqdn);
		free(g_ping.rtt);
		return (1);
	}
	if (g_ping.linger > 0)
		g_ping.timeout.tv_sec = g_ping.linger;
	else
		g_ping.timeout.tv_sec = 1;
	g_ping.timeout.tv_usec = 0;
	if (setsockopt(g_ping.sockfd, SOL_SOCKET, SO_RCVTIMEO,
			&g_ping.timeout, sizeof(g_ping.timeout)) < 0)
	{
		perror("setsockopt");
		free(g_ping.ip_address);
		free(g_ping.packet->data);
		free(g_ping.packet);
		close(g_ping.sockfd);
		free(g_ping.fqdn);
		free(g_ping.rtt);
		return (1);
	}
	if (g_ping.ignore_routing == 1)
	{
		status = 1;
		if (setsockopt(g_ping.sockfd, IPPROTO_IP, IP_HDRINCL,
				&status, sizeof(status)) < 0)
		{
			perror("setsockopt");
			free(g_ping.ip_address);
			free(g_ping.packet->data);
			free(g_ping.packet);
			close(g_ping.sockfd);
			free(g_ping.fqdn);
			free(g_ping.rtt);
			return (1);
		}
	}
	if (g_ping.tos > 0)
	{
		if (setsockopt(g_ping.sockfd, IPPROTO_IP, IP_TOS,
				&g_ping.tos, sizeof(g_ping.tos)) < 0)
		{
			perror("setsockopt");
			free(g_ping.ip_address);
			free(g_ping.packet->data);
			free(g_ping.packet);
			close(g_ping.sockfd);
			free(g_ping.fqdn);
			free(g_ping.rtt);
			return (1);
		}
	}
	memset(&g_ping.target_addr, 0, sizeof(g_ping.target_addr));
	g_ping.target_addr.sin_family = AF_INET;
	g_ping.target_addr.sin_addr.s_addr = inet_addr(g_ping.ip_address);
	return (0);
}

int	main(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	g_ping.verbose = 0;
	g_ping.help = 0;
	g_ping.fqdn = NULL;
	g_ping.ttl = 64;
	g_ping.size_number = 56;
	g_ping.packet_size = 64;
	g_ping.linger = 0;
	g_ping.w_timeout = 0;
	g_ping.numeric = 0;
	g_ping.ignore_routing = 0;
	g_ping.tos = 0;
	g_ping.pattern = 0xA;
	parse_args(argv);
	if (g_ping.help == 1)
	{
		printf("Usage: %s [options] <destination>\n", argv[0]);
		printf("Send ICMP ECHO_REQUEST packets to network hosts\n\n");
		printf(" Options:\n");
		printf("  -v\t\tverbose output\n");
		printf("  -?\t\tgive this help list\n");
		printf("\n");
		free(g_ping.fqdn);
		return (0);
	}
	if (init_socket() == 1)
		return (1);
	else
		return (ping());
}
