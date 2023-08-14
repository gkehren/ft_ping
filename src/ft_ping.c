#include "../include/ft_ping.h"

extern t_ping ft_ping;

void handle_sigint(int signal)
{
	(void)signal;
	if (ft_ping.tries > 0)
		display_stats();
	close(ft_ping.sockfd);
	free(ft_ping.packet->data);
	free(ft_ping.packet);
	free(ft_ping.ip_address);
	free(ft_ping.fqdn);
	free(ft_ping.rtt);
	exit(0);
}

void handle_sigalrm(int signal)
{
	(void)signal;
	if (ft_ping.tries > 0)
		display_stats();
	close(ft_ping.sockfd);
	free(ft_ping.packet->data);
	free(ft_ping.packet);
	free(ft_ping.ip_address);
	free(ft_ping.fqdn);
	free(ft_ping.rtt);
	exit(0);
}

int ping()
{
	uint8_t buffer[ft_ping.size_number];
	struct iovec iov[1];
	iov[0].iov_base = &buffer;
	iov[0].iov_len = sizeof(buffer);

	struct msghdr msg = {0};
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	signal(SIGINT, handle_sigint);
	if (ft_ping.w_timeout > 0)
	{
		signal(SIGALRM, handle_sigalrm);
		alarm(ft_ping.w_timeout);
	}

	if (ft_ping.verbose == 1)
		printf("PING %s (%s): %d data bytes, id 0x%x = %u\n", ft_ping.numeric == 0 ? ft_ping.fqdn : ft_ping.ip_address, ft_ping.ip_address, ft_ping.size_number, ft_ping.pid, ft_ping.pid);
	else
		printf("PING %s (%s): %d data bytes\n", ft_ping.numeric == 0 ? ft_ping.fqdn : ft_ping.ip_address, ft_ping.ip_address, ft_ping.size_number);

	ft_ping.packet = malloc(sizeof(ft_ping.packet));
	ft_ping.packet->data = malloc(ft_ping.size_number);

	while (1)
	{
		for (int i = 0; i < ft_ping.packet_size; i++)
		{
			ft_ping.packet->data[i] = 0xA;
			printf("%02x ", ft_ping.packet->data[i]);
		}
		printf("\n");
		ft_ping.packet->header.type = ICMP_ECHO;
		ft_ping.packet->header.code = 0;
		ft_ping.packet->header.un.echo.id = ft_ping.pid;
		ft_ping.packet->header.un.echo.sequence = ft_ping.tries;
		ft_ping.packet->header.checksum = 0;
		ft_ping.packet->header.checksum = calculate_checksum(&ft_ping.packet->header, sizeof(struct icmphdr));

		gettimeofday(&ft_ping.start_time, NULL);
		// Send the ICMP packet to the target address
		if (sendto(ft_ping.sockfd, ft_ping.packet, ft_ping.packet_size, 0, (struct sockaddr *)&ft_ping.target_addr, sizeof(ft_ping.target_addr)) < 0)
		{
			perror("sendto");
			close(ft_ping.sockfd);
			free(ft_ping.packet->data);
			free(ft_ping.packet);
			free(ft_ping.ip_address);
			free(ft_ping.fqdn);
			free(ft_ping.rtt);
			return 1;
		}

		// Wait for an ICMP packet to be received
		if (recvmsg(ft_ping.sockfd, &msg, 0) < 0)
		{
			if (errno == EINTR)
			{
				printf("Request timed out for icmp_seq %d\n", ft_ping.tries);
				ft_ping.tries++;
				ft_ping.num_failures++;
				continue;
			}
			else if (errno == EAGAIN)
			{
				ft_ping.tries++;
				ft_ping.num_failures++;
				continue;
			}
			else
			{
				perror("recvfrom");
				close(ft_ping.sockfd);
				free(ft_ping.packet->data);
				free(ft_ping.packet);
				free(ft_ping.ip_address);
				free(ft_ping.fqdn);
				free(ft_ping.rtt);
				return 1;
			}
		}

		struct iphdr *ip_header = (struct iphdr *)buffer;
		struct icmphdr *icmp_header = (struct icmphdr *)(buffer + sizeof(struct iphdr));

		if (icmp_header->type == ICMP_ECHOREPLY)
		{
			ft_ping.rtt[ft_ping.num_success] = ft_ping.elapsed_time;
			gettimeofday(&ft_ping.end_time, NULL);
			ft_ping.elapsed_time = get_elapsed_time(&ft_ping.start_time, &ft_ping.end_time);
			ft_ping.num_success++;
			ft_ping.total_rtt += ft_ping.elapsed_time;
			if (ft_ping.elapsed_time < ft_ping.min_rtt)
				ft_ping.min_rtt = ft_ping.elapsed_time;
			if (ft_ping.elapsed_time > ft_ping.max_rtt)
				ft_ping.max_rtt = ft_ping.elapsed_time;
			printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				ft_ping.size_number, ft_ping.ip_address, ft_ping.tries, ip_header->ttl, ft_ping.elapsed_time);

			ft_realloc(ft_ping.num_success + 1);
		}
		else
		{
			printf("Received ICMP packet with type %d\n", icmp_header->type);
			ft_ping.num_failures++;
		}

		ft_ping.tries++;
		usleep(1000000);
	}

	display_stats();

	close(ft_ping.sockfd);
	free(ft_ping.packet->data);
	free(ft_ping.packet);
	free(ft_ping.ip_address);
	free(ft_ping.fqdn);
	free(ft_ping.rtt);
	if (ft_ping.num_success > 0)
		return 0;
	else
		return 1;
}
