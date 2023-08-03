#ifndef FT_PING_H
# define FT_PING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <errno.h>
#include <float.h>
#include <math.h>

#define PACKET_SIZE 64
#define TIMEOUT 50000

typedef struct s_packet
{
	struct icmphdr	header;
	char			data[PACKET_SIZE - sizeof(struct icmphdr)];
}	t_packet;

typedef struct s_ping
{
	int						packet_size;
	int						timeout;
	int						sockfd;
	struct sockaddr_in		target_addr;
	t_packet				packet;
	struct timeval			start_time;
	struct timeval			end_time;
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
}	t_ping;

int					ping();
void				display_stats();
void				handle_alarm(int signal);
char				*ft_strdup(const char *s1);
char				**ft_split(char const *s, char c);
unsigned short		calculate_checksum(void *buf, int len);
int					ft_strcmp(const char *s1, const char *s2);
double				get_elapsed_time(struct timeval *start_time, struct timeval *end_time);

#endif
