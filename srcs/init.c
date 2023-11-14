#include "ft_ping.h"

void set_socket(env_t *env)
{
	int sock_fd;
	struct timeval timeout;

	if (env->opt.timeout) {
		timeout.tv_sec = (int)env->timeout;
		timeout.tv_usec = ((env->timeout - timeout.tv_sec) * 1000000);
	}
	else {
		timeout.tv_sec = RECV_TIMEOUT;
		timeout.tv_usec = 0;
	}

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
	env->pkt.hdr.type = ICMP_ECHO;
	env->pkt.hdr.code = 0;
	env->pkt.hdr.un.echo.sequence = htons(env->seq);
	env->pkt.hdr.un.echo.id = htons(env->pid);
	env->pkt.hdr.checksum = checksum((unsigned short *)&env->pkt, sizeof(env->pkt));
#elif defined(__APPLE__) || defined(__MACH__)
	env->pkt.hdr.icmp_type = ICMP_ECHO;
	env->pkt.hdr.icmp_code = 0;
	env->pkt.hdr.icmp_seq = htons(env->seq);
	env->pkt.hdr.icmp_id = htons(env->pid);
	env->pkt.hdr.icmp_cksum = checksum((unsigned short *)&env->pkt, sizeof(env->pkt));
#endif
	env->seq++;
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