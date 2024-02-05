/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_arg.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 16:32:36 by gkehren           #+#    #+#             */
/*   Updated: 2024/02/05 17:52:54 by gkehren          ###   ########.fr       */
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

int	parse_one_arg(char *arg)
{
	if (ft_strcmp(arg, "-?") == 0)
		g_ping.help = 1;
	else if (ft_strcmp(arg, "-v") == 0)
		g_ping.verbose = 1;
	else if (arg[0] != '-' && g_ping.fqdn == NULL)
		g_ping.fqdn = ft_strdup(arg);
	else if (arg[0] == '-')
	{
		printf("ft_ping: invalid value ('%c')\n", arg[1]);
		printf("Try 'ft_ping -?' for more information.\n");
		return (1);
	}
	return (0);
}
