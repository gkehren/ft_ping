/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_arg.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 16:32:36 by gkehren           #+#    #+#             */
/*   Updated: 2023/11/25 16:59:21 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

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
		if (next_arg == NULL)
		{
			printf("ft_ping: option requires an argument -- 'p'\n");
			printf("Try 'ft_ping -?' for more information.\n");
			return (1);
		}
		if (next_arg[0] == '-')
		{
			printf("ft_ping: error in pattern near %s\n", next_arg);
			return (1);
		}
		g_ping.pattern = next_arg[0];
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
