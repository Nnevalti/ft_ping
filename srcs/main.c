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

void check_icmp_errors(struct icmp *icmp)
{
	switch (icmp->icmp_type)
	{
	case ICMP_ECHOREPLY:
		printf("Echo Reply\n");
		break;
	case ICMP_UNREACH:
		switch (icmp->icmp_code)
		{
		case ICMP_UNREACH_NET:
			printf("Destination Net Unreachable\n");
			break;
		case ICMP_UNREACH_HOST:
			printf("Destination Host Unreachable\n");
			break;
		case ICMP_UNREACH_PROTOCOL:
			printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_UNREACH_PORT:
			printf("Destination Port Unreachable\n");
			break;
		case ICMP_UNREACH_NEEDFRAG:
			printf("frag needed and DF set (MTU %d)\n",
						 ntohs(icmp->icmp_nextmtu));
			break;
		case ICMP_UNREACH_SRCFAIL:
			printf("Source Route Failed\n");
			break;
		case ICMP_UNREACH_FILTER_PROHIB:
			printf("Communication prohibited by filter\n");
			break;
		default:
			printf("Dest Unreachable, Bad Code: %d\n",
						 icmp->icmp_code);
			break;
		}
		break;
	case ICMP_SOURCEQUENCH:
		printf("Source Quench\n");
		break;
	case ICMP_REDIRECT:
		switch (icmp->icmp_code)
		{
		case ICMP_REDIRECT_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIRECT_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIRECT_TOSNET:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIRECT_TOSHOST:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Bad Code: %d", icmp->icmp_code);
			break;
		}
		printf("(New addr: %s)\n", inet_ntoa(icmp->icmp_gwaddr));
		break;
	case ICMP_ECHO:
		printf("Echo Request\n");
		break;
	case ICMP_TIMXCEED:
		switch (icmp->icmp_code)
		{
		case ICMP_TIMXCEED_INTRANS:
			printf("Time to live exceeded\n");
			break;
		case ICMP_TIMXCEED_REASS:
			printf("Frag reassembly time exceeded\n");
			break;
		default:
			printf("Time exceeded, Bad Code: %d\n",
						 icmp->icmp_code);
			break;
		}
		break;
	case ICMP_TSTAMP:
		printf("Timestamp\n");
		break;
	case ICMP_TSTAMPREPLY:
		printf("Timestamp Reply\n");
		break;
	case ICMP_IREQ:
		printf("Information Request\n");
		break;
	case ICMP_IREQREPLY:
		printf("Information Reply\n");
		break;
	case ICMP_MASKREQ:
		printf("Address Mask Request\n");
		break;
	case ICMP_MASKREPLY:
		printf("Address Mask Reply\n");
		break;
	case ICMP_ROUTERADVERT:
		printf("Router Advertisement\n");
		break;
	case ICMP_ROUTERSOLICIT:
		printf("Router Solicitation\n");
		break;
	default:
		printf("Bad ICMP type: %d\n", icmp->icmp_type);
	}
}

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
			}
			else if (env->opt.verbose)
			{
				check_icmp_errors(icmp);
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
		send_ping(env);
		recv_ping(env);
		calculate_stats(env);
		if (g_running[1])
		{
			g_running[1] = false;
			print_stats_rtt(env);
		}
		usleep(PING_SLEEP_RATE * 1000000);
	}
	if (gettimeofday(&env->end, NULL) == -1)
		exit_clean(env, "gettimeofday failed");

	print_final_stats(env);
}

int main(int ac, char **av)
{
	env_t env;

	check_root();
	env.opt = parse_opt(ac, av);
	handle_opt(env.opt);

	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	init_params(&env, env.opt.hostname);
	dns_lookup(&env);

	ping_loop(&env);
	freeaddrinfo(env.res);
	close(env.sockfd);
	return 0;
}