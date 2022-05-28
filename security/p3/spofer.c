#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

//This checksum code is taken from Computer and Internet Security by Wenliang Du
unsigned short in_cksum (unsigned short *buf, int length)
{
   unsigned short *w = buf;
   int nleft = length;
   int sum = 0;
   unsigned short temp=0;

   /*
    * The algorithm uses a 32 bit accumulator (sum), adds
    * sequential 16 bit words to it, and at the end, folds back all
    * the carry bits from the top 16 bits into the lower 16 bits.
    */
   while (nleft > 1)  {
       sum += *w++;
       nleft -= 2;
   }

   /* treat the odd byte at the end, if any */
   if (nleft == 1) {
        *(u_char *)(&temp) = *(u_char *)w ;
        sum += temp;
   }

   /* add back carry outs from top 16 bits to low 16 bits */
   sum = (sum >> 16) + (sum & 0xffff);  // add hi 16 to low 16
   sum += (sum >> 16);                  // add carry
   return (unsigned short)(~sum);
}

struct ethheader {
  u_char  ether_dhost[6]; /* destination host address */
  u_char  ether_shost[6]; /* source host address */
  u_short ether_type;     /* protocol type (IP, ARP, RARP, etc) */
};
struct ipheader {
  unsigned char      iph_ihl:4, //IP header length
                     iph_ver:4; //IP version
  unsigned char      iph_tos; //Type of service
  unsigned short int iph_len; //IP Packet length (data + header)
  unsigned short int iph_ident; //Identification
  unsigned short int iph_flag:3, //Fragmentation flags
                     iph_offset:13; //Flags offset
  unsigned char      iph_ttl; //Time to Live
  unsigned char      iph_protocol; //Protocol type
  unsigned short int iph_chksum; //IP datagram checksum
  struct  in_addr    iph_sourceip; //Source IP address
  struct  in_addr    iph_destip;   //Destination IP address
};
struct icmpheader {
  unsigned char icmp_type; // ICMP message type
  unsigned char icmp_code; // Error code
  unsigned short int icmp_chksum; //Checksum for ICMP Header and data
  unsigned short int icmp_id;     //Used for identifying request
  unsigned short int icmp_seq;    //Sequence number
};

int main(int argc, char *argv[]){
	
	char buffer[1024];//buffer for creating the packet
	int actual_len=0;
	memset(buffer,0,1024);
	
	int sock =socket(AF_INET,SOCK_RAW,IPPROTO_RAW);//creating a raw socket to send the custom packet
	if(sock<0){printf("Error at socket()\n");return 2;}
	int x=1;
	int r=setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&x,sizeof(x));//setting the socket so we can send IP packets with custom header
	if(r<0){printf("Error at setsockopt()\n");return 2;}
	
	struct sockaddr_in dest;
	memset((char*)&dest,0,sizeof(dest));
	dest.sin_family=AF_INET;//set IPv4, instead of IPv6 etc
	inet_aton("8.8.8.8",&dest.sin_addr);//set the destination address
	
	struct icmpheader *icmphdr=(struct icmpheader *)(buffer+sizeof(struct ipheader));
	icmphdr->icmp_type=8;//8 is echo request,0 is echo reply
	icmphdr->icmp_chksum=in_cksum((unsigned short *)icmphdr,sizeof(struct icmpheader));//calculate the checksum of ICMP by using the checksum function from the book, which is written in accordence to the standart
	struct ipheader *iphdr=(struct ipheader *)buffer;
	iphdr->iph_ver=4;//set IPv4
	iphdr->iph_ihl=5;//set lenght as 5 4-bytes = 20bytes
	//iphdr->iph_ihl=15;
	iphdr->iph_ttl=20;
	iphdr->iph_sourceip.s_addr=inet_addr("10.9.0.5");//set the source and dest addresses in IP header
	iphdr->iph_destip.s_addr=inet_addr("8.8.8.8");
	iphdr->iph_protocol=IPPROTO_ICMP;//indicate ICMP in the IP header
	iphdr->iph_len=htons(sizeof(struct ipheader)+sizeof(struct icmpheader));
	
	actual_len=sizeof(struct ipheader)+sizeof(struct icmpheader);
	r=sendto(sock,buffer,actual_len,0,(struct sockaddr *)&dest,sizeof(dest));//send the packet which is created in the buffer to the "dest" on the "sock" raw socket
	if(r<0){printf("Error at sendto()\n");return 2;}
	
	return 0;
}
