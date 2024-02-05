/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkehren <gkehren@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/25 02:28:45 by gkehren           #+#    #+#             */
/*   Updated: 2024/02/05 20:54:55 by gkehren          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <signal.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <netdb.h>
# include <errno.h>
# include <limits.h>
# include <math.h>

typedef struct s_packet
{
	struct icmphdr	header;
	char			*data;
}	t_packet;

typedef struct s_ping
{
	int						sockfd;
	struct sockaddr_in		target_addr;
	t_packet				*packet;
	uint8_t					*buffer;
	struct timeval			start_time;
	struct timeval			end_time;
	struct timeval			timeout;
	double					elapsed_time;
	int						tries;
	int						ttl;
	int						num_pings;
	int						num_success;
	int						num_failures;
	double					min_rtt;
	double					max_rtt;
	double					total_rtt;
	double					stddev_rtt;
	double					*rtt;
	char					*ip_address;
	char					*fqdn;
	int						pid;
	int						verbose;
	int						help;
	int						size_number;
	int						packet_size;
	int						pattern;
}	t_ping;

int					ping(void);
void				display_packet(struct iphdr *ip_header,
						struct icmphdr *icmp_header);
void				display_address(void);
void				display_stats(void);
void				ft_realloc(int size);
void				ft_free_split(char **arg);
char				*ft_strdup(const char *s1);
char				**ft_split(char **s, char c);
uint16_t			calculate_checksum(void *data, int length);
int					ft_strcmp(const char *s1, const char *s2);
double				get_elapsed_time(struct timeval *start_time,
						struct timeval *end_time);
void				handle_sigint(int signal);
int					error_handler(const char *str, void *ptr);
int					end_of_pings(void);
char				*ft_strndup(const char *s, size_t n);
int					ft_atoi(const char *str);
size_t				ft_strlen(const char *s);
int					init_socket(void);
int					parse_one_arg(char *arg);

#endif
