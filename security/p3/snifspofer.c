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

#define SIZE_ETHERNET 14
struct ethheader {
  u_char  ether_dhost[6]; /* destination host address */
  u_char  ether_shost[6]; /* source host address */
  u_short ether_type;     /* protocol type (IP, ARP, RARP, etc) */
};
struct ipheader {
  unsigned char      iph_ihl:4, //IP header length 	rightmost
                     iph_ver:4; //IP version		leftmost
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



void pkt_rcvd(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
	//this place is very similar to spofer.c, but it has some differences
	const struct ipheader *ip1=(struct ipheader*)(packet+SIZE_ETHERNET); 
	int ip_size=(ip1->iph_ihl)*4;
    	//printf("Source: %s\n",inet_ntoa(iphdr->iph_sourceip));
    	//printf("Desti.: %s\n\n",inet_ntoa(iphdr->iph_destip));
    	const struct icmpheader *icmp1=(struct icmpheader*)(packet+SIZE_ETHERNET+ip_size); 
    	
    	if(icmp1->icmp_type==8){//here it checks if the packet is an ICMP echo request message, if it is not, it dismisses the packet
    		int ssize=0;
    		ssize+=ntohs(ip1->iph_len)-20-8;//this size is the remaining of the packet after the ICMP header.lenght from the ip header is extracted which is ipheader+ippayload. then ip header size and icmp header size is subtracted to find the remaining data size.
    		//printf("F:%s\n",inet_ntoa(ip1->iph_sourceip));printf("T:%s\n",inet_ntoa(ip1->iph_destip));
    		char buffer[1024];
		int actual_len=0;
		memset(buffer,0,1024);
		
		char *data=(char *)(packet+SIZE_ETHERNET+ip_size+sizeof(struct icmpheader));
		
		int sock =socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
		if(sock<0){printf("Error at socket()\n");}
		int x=1;
		int r=setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&x,sizeof(x));
		if(r<0){printf("Error at setsockopt()\n");}
		
		struct sockaddr_in dest;
		memset((char*)&dest,0,sizeof(dest));
		dest.sin_family=AF_INET;
		inet_aton(inet_ntoa(ip1->iph_sourceip),&dest.sin_addr);//destination addr for socket is set as the source of the caught packet
		
		struct icmpheader *icmphdr=(struct icmpheader *)(buffer+sizeof(struct ipheader));
		icmphdr->icmp_type=0;
		icmphdr->icmp_id=icmp1->icmp_id;//ICMP id and seq are identical as the caught packet
		icmphdr->icmp_seq=icmp1->icmp_seq;
		memcpy((buffer+sizeof(struct ipheader)+sizeof(struct icmpheader)),data,ssize);//the part exluding the headers are copied, which is the payload. calculated above.
		icmphdr->icmp_chksum=in_cksum((unsigned short *)icmphdr,sizeof(struct icmpheader));//checksum calculated for ICMP
		
		struct ipheader *iphdr=(struct ipheader *)buffer;
		iphdr->iph_ver=4;
		iphdr->iph_ihl=5;
		iphdr->iph_ttl=20;
		iphdr->iph_sourceip.s_addr=inet_addr(inet_ntoa(ip1->iph_destip));//destination and source address of our packet are set in reverse of the caught packet
		iphdr->iph_destip.s_addr=inet_addr(inet_ntoa(ip1->iph_sourceip));
		iphdr->iph_protocol=IPPROTO_ICMP;
		//iphdr->iph_len=htons(sizeof(struct ipheader)+sizeof(struct icmpheader));
	
		actual_len=sizeof(struct ipheader)+sizeof(struct icmpheader)+ssize;
		r=sendto(sock,buffer,actual_len,0,(struct sockaddr *)&dest,sizeof(dest));//spoofed reply is sent for the caught ICMP echo request
		if(r<0){printf("Error at sendto()\n");}
		printf("ECHO SPOTTED!\n");
    	}else{
    		printf("Not echo! It is %d.\n",icmp1->icmp_type);//if the icmp packet is not echo request, just print what it is to inform the user
    	}
    	
	
}


int main(int argc, char *argv[]){
	int e;
	int promisc=1;
	if(argc>1 && atoi(argv[1])==2){//enable/disable promisc mode
		printf("promisc disabled\n");promisc=0;
	}
	//this is same as the snifer.c
	pcap_t *handle;
	char errbuff[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filterexp[]="icmp";
	bpf_u_int32 net;
	
	handle=pcap_open_live("br-2a0d781115d4",BUFSIZ,promisc,1000,errbuff);
	if(handle==NULL){printf("Error at pcap_open_live.\n");printf("%s\n",errbuff);return 2;}
	e=pcap_compile(handle,&fp,filterexp,0,net);
	if(e==-1){printf("Error at filter compilation.\n");return 2;}
	pcap_setfilter(handle,&fp);
	if(e==-1){printf("Error at setting filter.\n");return 2;}
	
	pcap_loop(handle,-1,pkt_rcvd,NULL);
	pcap_close(handle);
	return 0;

}



