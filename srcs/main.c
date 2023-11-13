#include "ft_ping.h"

void sig_handler(int sig)
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

	if (!env->opt.ttl)
		env->ttl = MAX_TTL;

	env->pkt_sent = 0;
	env->pkt_recv = 0;
	env->seq = 0;

	env->min_rtt = 0.0f;
	env->max_rtt = 0.0f;
	env->avg_rtt = 0.0f;
	env->stddev_rtt = 0.0f;
	env->rtt = NULL;
}

void send_ping(env_t *env)
{
	int err;

	init_send(env);
	if ((err = sendto(env->sockfd, &env->pkt, sizeof(env->pkt), 0, env->res->ai_addr, env->res->ai_addrlen)) < 0)
		fprintf(stderr, "ping: sendto: %s\n", strerror(errno));
	if (gettimeofday(&env->send, NULL) < 0)
		exit_clean(env, "gettimeofday failed");
	env->pkt_sent++;
}

#if defined(__APPLE__) || defined(__MACH__)
void recv_ping(env_t *env)
{
	int ret;

	init_recv(env);
	ret = recvmsg(env->sockfd, &env->response.ret_hdr, 0);
	if (ret > 0)
	{
		if (ntohs(env->pkt.hdr.icmp_id) == env->pid && env->seq - 1 == ntohs(env->pkt.hdr.icmp_seq))
		{
			struct ip *ip = (struct ip *)(env->response.iov->iov_base);
			struct icmp *icmp = (struct icmp *)(env->response.iov->iov_base + (ip->ip_hl << 2));
			if (gettimeofday(&env->receive, NULL) == -1)
				exit_clean(env, "gettimeofday failed");

			if (icmp->icmp_type == ICMP_ECHOREPLY)
			{
				env->pkt_recv++;
				print_stats(env, ret);
				if (env->opt.audible)
					printf("\a");
			}
			else if (env->opt.verbose)
			{
				check_icmp_errors(icmp);
			}
			else {
				printf("An error occured for icmp_seq %d. Activate verbose mode to see the error\n", env->seq - 1);
			}
		}
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			printf("Recvmsg timeout for icmp_seq %d\n", env->seq - 1);
		else
		{
			printf("recvmsg failed: %s\n", strerror(errno));
			exit_clean(env, "recvmsg failed");
		}
	}
}
#elif defined(__linux__)
void recv_ping(env_t *env)
{
	int ret;

	init_recv(env);
	ret = recvmsg(env->sockfd, &env->response.ret_hdr, 0);
	if (ret > 0)
	{
		if (ntohs(env->pkt.hdr.un.echo.id) == env->pid && env->seq - 1 == ntohs(env->pkt.hdr.un.echo.sequence))
		{
			struct iphdr *ip = (struct iphdr *)(env->response.iov->iov_base);
			struct icmphdr *icmp = (struct icmphdr *)(env->response.iov->iov_base + (ip->ihl << 2));
			if (gettimeofday(&env->receive, NULL) == -1)
				exit_clean(env, "gettimeofday failed");

			if (icmp->type == ICMP_ECHOREPLY)
			{
				env->pkt_recv++;
				print_stats(env, ret);
				if (env->opt.audible)
					printf("\a");
			}
			else if (env->opt.verbose)
			{
				check_icmp_errors(icmp);
			}
			else {
				printf("An error occured for icmp_seq %d. Activate verbose mode to see the error\n", env->seq - 1);
			}
		}
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			printf("Recvmsg timeout for icmp_seq %d\n", env->seq - 1);
		else
		{
			printf("recvmsg failed: %s\n", strerror(errno));
			exit_clean(env, "recvmsg failed");
		}
	}
}
#endif

void calculate_stats(env_t *env)
{
	env->avg_rtt = (env->min_rtt + env->max_rtt) / 2.0;
	env->stddev_rtt = 0.0;
	if (env->pkt_recv != 0)
	{
		double squared_diff_sum = 0.0;

		for (int i = 0; env->rtt[i] != -1; i++)
		{
			double diff = env->rtt[i] - env->avg_rtt;
			squared_diff_sum += diff * diff;
		}

		double variance = squared_diff_sum / env->pkt_recv;
		env->stddev_rtt = sqrt(variance);
	}
}

void ping_loop(env_t *env)
{
	set_socket(env);
	printf("PING %s (%s): %lu data bytes\n", env->hostname, env->addrstr, PKT_SIZE - (sizeof(env->pkt.hdr)));

	if (gettimeofday(&env->start, NULL) < 0)
		exit_clean(env, "gettimeofday failed");

	while (g_running[0])
	{
		if (env->opt.count && env->pkt_sent == env->count) {
			g_running[0] = false;
			break;
		}

		send_ping(env);
		recv_ping(env);
		calculate_stats(env);
		if (g_running[1])
		{
			g_running[1] = false;
			print_stats_rtt(env);
		}

		if (env->opt.interval)
			usleep(env->interval * 1000000);
		else
			usleep(PING_SLEEP_RATE * 1000000);
	}
	if (gettimeofday(&env->end, NULL) == -1)
		exit_clean(env, "gettimeofday failed");

	if (env->pkt_recv > 0)
		print_final_stats(env);
}

int main(int ac, char **av)
{
	env_t env;

	check_root();
	env.opt = parse_opt(ac, av, &env);
	handle_opt(env.opt);

	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	init_params(&env, env.opt.hostname);
	dns_lookup(&env);

	ping_loop(&env);
	free(env.rtt);
	freeaddrinfo(env.res);
	close(env.sockfd);
	return 0;
}