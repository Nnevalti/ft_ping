#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

// convert an IP address to a string
char *ft_iptoa(struct sockaddr *sa)
{
    static char str[INET_ADDRSTRLEN];
    struct sockaddr_in *s = (struct sockaddr_in *)sa;

    inet_ntop(AF_INET, &s->sin_addr, str, sizeof(str));
    return str;
}

int main(int ac, char **av) {
    if (ac != 2) {
        printf("Usage: %s <hostname>\n", av[0]);
        return 1;
    }
    struct addrinfo* addr;
    int result = getaddrinfo(av[1], NULL, NULL, &addr);
    if (result != 0) {
        printf("Error from getaddrinfo: %s\n", gai_strerror(result));
        return 1;
    }

    result = getnameinfo(addr->ai_addr, addr->ai_addrlen, NULL, 0, NULL, 0, NI_NUMERICHOST);
    if (result != 0) {
        printf("Error from getnameinfo: %s\n", gai_strerror(result));
        return 1;
    }
    
    char host[1024];
    char *inputAddress = ft_iptoa(addr->ai_addr);
    struct sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    inet_pton(AF_INET, inputAddress, &(socketAddress.sin_addr));
    getnameinfo((struct sockaddr *)&socketAddress, sizeof(socketAddress), host, 1024, NULL, NULL, 0);
    printf("host=%s \n", host);

    struct sockaddr_in* internet_addr = (struct sockaddr_in*) addr->ai_addr;
    printf("%s is at: %s\n", av[1], inet_ntoa(internet_addr->sin_addr));
    return 0;
}