/*
 *    pinger.c
 *    This is a ping imitation program
 *    It will send an ICMP ECHO packet to the server of
 *    your choice and listen for an ICMP REPLY packet
 *    Have fun!
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <string.h>
#include <unistd.h>

#include "wifi/wifi_interface.h"
#include "gprs/gprs_interface.h"


char dst_addr[20];
char src_addr[20];

unsigned short in_cksum(unsigned short *, int);
void usage();
char* getip();
char* toip(char*);

char *wifiComGetCurrentIpStr(void);
char *gprsComGetCurrentIpStr(void);

int ping(char* to, int timeout)
{
    struct iphdr* ip;
    struct iphdr* ip_reply;
    struct icmphdr* icmp;
    struct sockaddr_in connection;
    char* packet;
    char* buffer;
    int sockfd;
    int optval;
    int addrlen;
    fd_set fds;
    int siz;
    char POSip[20] = "";

    //printf("!!!!!!!!!!!!!!!!!entering on ping\n");

    int ret = 1;

    int selectResult = 0;
    struct timeval tv;

    //printf("wifiComGetCurrentState() = %d, gprsComGetCurrentState() = %d\n", wifiComGetCurrentState(), gprsComGetCurrentState());

    if(wifiComGetCurrentStateToApplication() == W_CONNECTED)
    {
    	strcpy(POSip, wifiComGetCurrentIpStr());
    }
    else if(gprsComGetCurrentStateToApplication() == G_PPP_CONNECTED)
    {
    	strcpy(POSip, gprsComGetCurrentIpStr());
    }
    else
    {
    	return 1;
    }

    //printf("Source address: %s\n", POSip);
    //printf("Destination address: %s\n", to);
    /*
     * allocate all necessary memory
    */
    packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
    /****************************************************************/

    ip = (struct iphdr*) packet;
    icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));

    sprintf(src_addr, POSip);
    sprintf(dst_addr, to);

    /*
     *  here the ip packet is set up
     */
    ip->ihl          = 5;
    ip->version          = 4;
    ip->tos          = 0;
    ip->tot_len          = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id           = htons(0);
    ip->frag_off     = 0;
    ip->ttl          = 64;
    ip->protocol     = IPPROTO_ICMP;
    ip->saddr            = inet_addr(src_addr);
    ip->daddr            = inet_addr(dst_addr);
    ip->check            = in_cksum((unsigned short *)ip, sizeof(struct iphdr));

    //printf("ip->saddr : %s,  ip->daddr : %s-> \n",  (char*)ip->daddr, (char*)ip->saddr);

//    if(ip->saddr == INADDR_NONE || ip->daddr == INADDR_NONE)
//    {
//    	printf("anything is wrong..\n");
//    }
//    else
//    {
//    	printf("IPs is ok....\n");
//    }

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
    perror("socket");
    exit(EXIT_FAILURE);
    }

    /*
     *  IP_HDRINCL must be set on the socket so that
     *  the kernel does not attempt to automatically add
     *  a default ip header to the packet
     */

    setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));

    /*
     *  here the icmp packet is created
     *  also the ip checksum is generated
     */
    icmp->type           = ICMP_ECHO;
    icmp->code           = 0;
    icmp->un.echo.id     = random();
    icmp->un.echo.sequence   = 0;
    icmp-> checksum      = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));


    connection.sin_family = AF_INET;
    connection.sin_addr.s_addr = inet_addr(dst_addr);

    /*
     *  now the packet is sent
     */

    sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(struct sockaddr));
    //printf("Sent %d byte packet to %s\n", ip->tot_len, dst_addr);

    /*
     *  now we listen for responses
     */
    addrlen = sizeof(connection);

    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    selectResult = select(sockfd+1, &fds, NULL, NULL, &tv);

    if(selectResult)
    {
		if (( siz = recvfrom(sockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0, (struct sockaddr *)&connection, &addrlen)) == -1)
		{
			perror("recv");
		}
		else
		{
			ret = 0;
			//printf("Received %d byte reply from %s:\n", siz , dst_addr);
			ip_reply = (struct iphdr*) buffer;
			//printf("ID: %d\n", ntohs(ip_reply->id));
			//printf("TTL: %d\n", ip_reply->ttl);
		}
    }

    free(packet);
    free(buffer);
    close(sockfd);
    return ret;
}

void usage()
{
    fprintf(stderr, "\nUsage: pinger [destination] <-s [source]>\n");
    fprintf(stderr, "Destination must be provided\n");
    fprintf(stderr, "Source is optional\n\n");
}

char* getip()
{
    char buffer[256];
    struct hostent* h;

    gethostname(buffer, 256);
    h = gethostbyname(buffer);

    return inet_ntoa(*(struct in_addr *)h->h_addr);

}

/*
 * return the ip address if host provided by DNS name
 */
char* toip(char* address)
{
    struct hostent* h;
    h = gethostbyname(address);
    return inet_ntoa(*(struct in_addr *)h->h_addr);
}

/*
 * in_cksum --
 * Checksum routine for Internet Protocol
 * family headers (C Version)
 */
unsigned short in_cksum(unsigned short *addr, int len)
{
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;
    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }
    /* mop up an odd byte, if necessary */
    if (nleft == 1)
    {
      *(u_char *) (&answer) = *(u_char *) w;
      sum += answer;
    }
    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);       /* add hi 16 to low 16 */
    sum += (sum >> 16);               /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}
