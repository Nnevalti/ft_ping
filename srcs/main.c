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

void check_root()
{
	if (getuid() != 0)
	{
		printf("Error: You must be root to run this program\n");
		exit(1);
	}
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
}

void	set_socket(env_t *env)
{
	int		sock_fd;
	struct timeval	timeout;

	timeout.tv_sec = RECV_TIMEOUT;
	timeout.tv_usec = 0;
	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
		fprintf(stderr, "Error: socket failed\n");
	if ((setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout))) == -1)
		fprintf(stderr, "Error: setsockopt failed\n");
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, (const void *)&env->ttl, sizeof(env->ttl)) == -1)
		fprintf(stderr, "Error: setsockopt failed\n");
	env->sockfd = sock_fd;
}

void ping_loop(env_t *env)
{
	// create a raw socket with ICMP protocol
	// get time of day
	// send ICMP packet
	// receive ICMP packet
	// get time of day
	// calculate time difference
	// print statistics
	// print error if any
	// exit

	// int ttl_val = 64, msg_count = 0, i, addr_len, flag = 1,
		// msg_received_count = 0;
	
	set_socket(env);
	printf("PING %s (%s): %d data bytes\n", env->hostname, env->addrstr, 56);
	if (gettimeofday(&env->start, NULL) == -1)
		fprintf(stderr, "Error: gettimeofday failed\n");
	struct timeval tv;
	gettimeofday(&tv, NULL);
	while (g_running[0])
	{
		printf("ping\n");
		sleep(PING_SLEEP_RATE);
	}
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
	return 0;
}