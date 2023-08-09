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

void exit_clean(env_t *env, char *msg)
{
	if (msg)
		fprintf(stderr, "Error: %s\n", msg);
	if (env->res)
		freeaddrinfo(env->res);
	if (env->sockfd)
		close(env->sockfd);
	exit(errno);
}

void init_params(env_t *env, char *hostname)
{
	g_running[0] = true;
	g_running[1] = false;

	env->hostname = hostname;
	env->pid = getpid();

	env->ttl = MAX_TTL;

	env->pkt_sent = 0;
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
		exit(errno);
	}

	addr = (struct sockaddr_in *)env->res->ai_addr;
	inet_ntop(AF_INET, &(addr->sin_addr), env->addrstr, INET_ADDRSTRLEN);
}

void	set_socket(env_t *env)
{
	int		sock_fd;
	struct timeval	timeout;

	timeout.tv_sec = RECV_TIMEOUT;
	timeout.tv_usec = 0;

	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_fd < 0)
		exit_clean(env, "socket failed");
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, (const void *)&env->ttl, sizeof(env->ttl)) == -1)
		exit_clean(env, "setsockopt failed");
	if ((setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout))) == -1)
		exit_clean(env, "setsockopt failed");
	env->sockfd = sock_fd;
}

void init_send(env_t *env)
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

void init_recv(env_t *env)
{
	memset(&env->response.ret_hdr, 0, sizeof(env->response.ret_hdr));
	memset(&env->pkt.hdr_buf, 0, sizeof(env->pkt.hdr_buf));
	env->response.iov->iov_base = (void *)env->pkt.hdr_buf;
	env->response.iov->iov_len = sizeof(env->pkt.hdr_buf);
	env->response.ret_hdr.msg_iov = env->response.iov;
	env->response.ret_hdr.msg_iovlen = 1;
}

void send_ping(env_t *env)
{
	int err;

	init_send(env);
	if ((err = sendto(env->sockfd, &env->pkt, sizeof(env->pkt), 0, env->res->ai_addr, env->res->ai_addrlen)) < 0)
		exit_clean(env, "sendto failed");
	if (gettimeofday(&env->send, NULL) < 0)
		exit_clean(env, "gettimeofday failed");
	env->pkt_sent++;
}


void print_stats(env_t *env, unsigned int ret)
{
	double time_elapsed = (env->receive.tv_sec - env->send.tv_sec) * 1000.0;
	time_elapsed += (env->receive.tv_usec - env->send.tv_usec) / 1000.0;
	printf("%u bytes from %s (%s): icmp_seq=%d ttl=%d time=%.3f ms\n", ret, env->hostname, env->addrstr, env->seq - 1, env->ttl, time_elapsed);
	// min and max rtt
	if (time_elapsed < env->min_rtt || env->seq == 1)
		env->min_rtt = time_elapsed;
	if (time_elapsed > env->max_rtt)
		env->max_rtt = time_elapsed;
}

void recv_ping(env_t *env)
{
	int err;

	init_recv(env);
	if ((err = recvmsg(env->sockfd, &env->response.ret_hdr, 0)) < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			printf("PING: Timeout for icmp_seq %d\n", env->seq);
		else
			exit_clean(env, "recvmsg failed");
	}
	else {
		// if (env->response.ret_hdr.msg_namelen != env->res->ai_addrlen)
		// 	exit_clean(env, "recvmsg failed");
		if (env->seq - 1 != env->pkt.hdr.icmp_seq)
			exit_clean(env, "recvmsg failed");
		if (gettimeofday(&env->receive, NULL) == -1)
			exit_clean(env, "gettimeofday failed");
		print_stats(env, err);
		env->pkt_recv++;
	}
}

void calculate_stats(env_t *env)
{
	env->avg_rtt = (env->min_rtt + env->max_rtt) / 2.0;
	env->mdev_rtt = env->max_rtt - env->min_rtt;
}

void print_final_stats(env_t *env)
{
	printf("\n--- %s ping statistics ---\n", env->hostname);
	printf("%d packets transmitted, %d received, %f%% packet loss\n", env->pkt_sent, env->pkt_recv, (env->pkt_sent - env->pkt_recv) / env->pkt_sent * 100.0);
	printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", env->min_rtt, env->avg_rtt, env->max_rtt, env->mdev_rtt);
}

void ping_loop(env_t *env)
{
	// int flag = 1, msg_received_count = 0;

	set_socket(env);
	printf("PING %s (%s): %lu data bytes\n", env->hostname, env->addrstr, PKT_SIZE - (sizeof(env->pkt.hdr)));

	if (gettimeofday(&env->start, NULL) < 0)
		exit_clean(env, "gettimeofday failed");

	while (g_running[0])
	{
		send_ping(env);
		recv_ping(env);
		calculate_stats(env);
		// if (g_running[1])
		// print_stats(env);
		sleep(PING_SLEEP_RATE);
	}
	print_final_stats(env);
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