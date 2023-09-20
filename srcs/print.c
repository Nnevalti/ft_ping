#include "ft_ping.h"

void print_stats(env_t *env, unsigned int ret)
{
	double time_elapsed = (env->receive.tv_sec - env->send.tv_sec) * 1000.0;
	time_elapsed += (env->receive.tv_usec - env->send.tv_usec) / 1000.0;

	printf("%u bytes from %s (%s): icmp_seq=%d ttl=%d time=%.3f ms\n", ret, env->hostname, env->addrstr, env->seq - 1, env->ttl, time_elapsed);

	if (time_elapsed < env->min_rtt || env->seq == 1)
		env->min_rtt = time_elapsed;
	if (time_elapsed > env->max_rtt)
		env->max_rtt = time_elapsed;

	if (env->seq == 1)
	{
		env->rtt = (double *)malloc(sizeof(double) + 1);
		env->rtt[0] = time_elapsed;
		env->rtt[1] = -1;
	}
	else
	{
		env->rtt = (double *)realloc(env->rtt, sizeof(double) * (env->seq + 1));
		env->rtt[env->seq - 1] = time_elapsed;
		env->rtt[env->seq] = -1;
	}
}

void	print_ttl(env_t *env, unsigned int ret)
{
	char	str[INET_ADDRSTRLEN];
	
	printf("Request timeout for icmp_seq %d\n", env->seq - 1);
	printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.3f ms\n\n", ret, env->hostname, inet_ntop(AF_INET, (void*)&env->addrstr, str, INET_ADDRSTRLEN), env->seq - 1, env->ttl, 0.0);
}

void print_errors(env_t *env)
{
	struct ip *ip = (struct ip *)(env->response.iov->iov_base);
	struct icmp *icmp = (struct icmp *)(env->response.iov->iov_base + (ip->ip_hl << 2));
	
	printf("From gateway (%s) icmp_seq=%d type=%d code=%d\n", env->addrstr, env->seq - 1, icmp->icmp_type, icmp->icmp_code);
}

void print_stats_rtt(env_t *env)
{
	printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", env->min_rtt, env->avg_rtt, env->max_rtt, env->stddev_rtt);
}

void print_final_stats(env_t *env)
{
	if (gettimeofday(&env->end, NULL) == -1)
		exit_clean(env, "gettimeofday failed");
	printf("\n--- %s ping statistics ---\n", env->hostname);
	printf("%d packets transmitted, %d received, %f%% packet loss\n", env->pkt_sent, env->pkt_recv, (env->pkt_sent - env->pkt_recv) / env->pkt_sent * 100.0);
	printf("rtt min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", env->min_rtt, env->avg_rtt, env->max_rtt, env->stddev_rtt);
}