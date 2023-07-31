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

#define PACKET_SIZE 64
#define TIMEOUT 50000

void		handle_alarm(int signal);
void		ft_bzero(void *s, size_t n);
uint16_t	calculate_checksum(const void *data, size_t length);
double		get_elapsed_time(struct timeval *start_time, struct timeval *end_time);

#endif