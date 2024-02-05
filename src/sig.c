/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sig.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 15:23:05 by gkehren           #+#    #+#             */
/*   Updated: 2024/02/05 20:52:56 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ft_ping.h"

extern t_ping	g_ping;

void	handle_sigint(int signal)
{
	(void)signal;
	if (g_ping.tries > 0)
		display_stats();
	if (g_ping.buffer != NULL)
		free(g_ping.buffer);
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
	return (-1);
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
