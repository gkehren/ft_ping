/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/24 23:25:29 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 17:00:20 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

t_ping	g_ping;

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

void	init_g_ping(char **argv)
{
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
	g_ping.pattern = 'A';
	parse_args(argv);
}

int	main(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("ft_ping: missing host operand\n");
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	init_g_ping(argv);
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
