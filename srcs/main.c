#include "ft_ping.h"

void	sig_handler(int sig)
{
	if (sig == SIGQUIT)
	{
		g_running[1] = true;
	}
	if (sig == SIGINT) 
	{
		g_running[0] = false;
	}
	return;
}

void init_params(env_t *env, char *hostname)
{
	g_running[0] = true;
	g_running[1] = false;

	env->hostname = hostname;
	env->pid = getpid();

	env->ttl = MAX_TTL;
}

void dns_lookup(env_t *env) {
	int err;
	struct sockaddr_in *addr;

	memset(&env->hints, 0, sizeof(struct addrinfo));
	env->hints.ai_family = AF_INET; // IPv4
	env->hints.ai_socktype = SOCK_STREAM; // TCP protocol
	err = getaddrinfo(env->hostname, NULL, &env->hints, &env->res);
	if (err != 0) {
		fprintf(stderr, "ft_ping: cannot resolve %s: Unknown host\n", env->hostname);
		// fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}

	addr = (struct sockaddr_in *)env->res->ai_addr;
	inet_ntop(AF_INET, &(addr->sin_addr), env->addrstr, INET_ADDRSTRLEN);
	// print info on addr
	printf("PING %s (%s) %d(%d) bytes of data.\n", env->hostname, env->addrstr, PKT_SIZE, PKT_SIZE + 8);

}

void	set_socket(env_t *env)
{
	int		sock_fd;
	struct timeval	timeout;

	timeout.tv_sec = RECV_TIMEOUT;
	timeout.tv_usec = 0;

	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_fd < 0)
		fprintf(stderr, "Error: socket failed\n");
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, (const void *)&env->ttl, sizeof(env->ttl)) == -1)
		fprintf(stderr, "Error: setsockopt failed\n");
	if ((setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout))) == -1)
		fprintf(stderr, "Error: setsockopt failed\n");
	env->sockfd = sock_fd;
}

void init_packet(env_t *env)
{
	memset(&env->pkt, 0, sizeof(env->pkt));
		for (unsigned int i = 0; i < (PKT_SIZE - sizeof(env->pkt.hdr)); i++)
	{
		env->pkt.hdr_buf[i] = i + '0';
	}

	#if defined(__linux__)
		env->pkt.hdr->type = ICMP_ECHO;
		env->pkt.hdr->code = 0;
		env->pkt.hdr->un.echo.sequence = env->seq++;
		env->pkt.hdr->un.echo.id = env->pid;
		env->pkt.hdr->checksum = checksum(env->pkt, sizeof(env->pkt));
	#elif defined(__APPLE__) || defined(__MACH__)
		env->pkt.hdr.icmp_type = ICMP_ECHO;
		env->pkt.hdr.icmp_code = 0;
		env->pkt.hdr.icmp_seq = env->seq++;
		env->pkt.hdr.icmp_id = env->pid;
		env->pkt.hdr.icmp_cksum = checksum((unsigned short *)&env->pkt, sizeof(env->pkt));
	#endif
}

void ping_loop(env_t *env)
{
	// int ttl_val = 64, msg_count = 0, i, addr_len, flag = 1,
		// msg_received_count = 0;
	int err;

	set_socket(env);
	printf("PING %s (%s): %lu data bytes\n", env->hostname, env->addrstr, PKT_SIZE - (sizeof(env->pkt.hdr)));

	if (gettimeofday(&env->start, NULL) == -1)
		fprintf(stderr, "Error: gettimeofday failed\n");
	while (g_running[0])
	{
		init_packet(env);
		printf("ping\n");
		if ((err = sendto(env->sockfd, &env->pkt, sizeof(env->pkt), 0, env->res->ai_addr, env->res->ai_addrlen)) < 0) {
			fprintf(stderr, "Error: sendto failed\n");
			perror("sendto");
		}
		
		memset(&env->response.ret_hdr, 0, sizeof(env->response.ret_hdr));
		memset(&env->pkt.hdr_buf, 0, sizeof(env->pkt.hdr_buf));
		env->response.iov->iov_base = (void *)env->pkt.hdr_buf;
		env->response.iov->iov_len = sizeof(env->pkt.hdr_buf);
		env->response.ret_hdr.msg_name = NULL;
		env->response.ret_hdr.msg_namelen = 0;
		env->response.ret_hdr.msg_iov = env->response.iov;
		env->response.ret_hdr.msg_iovlen = 1;
		
		if ((err = recvmsg(env->sockfd, &env->response.ret_hdr, 0)) < 0) {
			fprintf(stderr, "Error: recvfrom failed\n");
			fprintf(stderr, "%s\n", gai_strerror(err));
			perror("recvmsg");
		}
		else {
			printf("pong\n");
			// if (gettimeofday(&env->end, NULL) == -1)
			// 	fprintf(stderr, "Error: gettimeofday failed\n");
			// print_stats(env);
		}		
		// if (g_running[1])
		// print_stats(env);
		printf("ping\n");
		sleep(PING_SLEEP_RATE);
	}
	// print_stats(env);
}

int main(int ac, char **av)
{
	opt_t opt;
	env_t env;

	check_root();
	opt = parse_opt(ac, av);
	handle_opt(opt);

	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	init_params(&env, opt.hostname);
	dns_lookup(&env);

	ping_loop(&env);
	freeaddrinfo(env.res);
	close(env.sockfd);
	return 0;
}