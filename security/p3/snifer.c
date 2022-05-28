#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <arpa/inet.h>

/*
struct pcap_pkthdr {
	struct timeval ts;  	//time stamp 
	bpf_u_int32 caplen;  	//length of portion present 
	bpf_u_int32 len;  	//length this packet (off wire) 
}
*/
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
struct tcpheader {
    u_short tcp_sport;               /* source port */
    u_short tcp_dport;               /* destination port */
    u_int   tcp_seq;                 /* sequence number */
    u_int   tcp_ack;                 /* acknowledgement number */
    u_char  tcp_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->tcp_offx2 & 0xf0) >> 4)
    u_char  tcp_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short tcp_win;                 /* window */
    u_short tcp_sum;                 /* checksum */
    u_short tcp_urp;                 /* urgent pointer */
};

void pkt_rcvd(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
	//this function is called for each captured packet that fits in the BPF set beforehand
	//printf("Got a packet.\n");
	const struct ipheader *iphdr=(struct ipheader*)(packet+SIZE_ETHERNET); //casting the ip header part of the buffer, which is after ethernet header
	int ip_size=(iphdr->iph_ihl)*4;//printf("IP_size:%d\n",ip_size); //iph_ihl is the amount of 4-byte words, thus multiplied by 4
	const struct tcpheader *tcphdr=(struct tcpheader*)(packet+SIZE_ETHERNET+ip_size);//casting the tcp header part of the buffer, which is after ethernet and ip header
	int tcp_size=TH_OFF(tcphdr)*4;//printf("TCP_size:%d\n",tcp_size);
	printf("DATA:%s\n",(packet+SIZE_ETHERNET+ip_size+tcp_size));
    	printf("Source: %s\n",inet_ntoa(iphdr->iph_sourceip));
    	printf("Desti.: %s\n\n",inet_ntoa(iphdr->iph_destip));
	
}


int main(int argc, char *argv[]){
	int e;
	int promisc=1;
	if(argc>1 && atoi(argv[1])==2){//enable/disable promisc mode
		printf("promisc disabled\n");promisc=0;
	}
	
	pcap_t *handle;
	char errbuff[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	//char filterexp[]="";
	//char filterexp[]="icmp && host 10.9.0.5 && host 8.8.8.8";
	//char filterexp[]="tcp dst portrange 10-100";
	char filterexp[]="tcp port 23";//telnet
	bpf_u_int32 net;
	
	handle=pcap_open_live("br-2a0d781115d4",BUFSIZ,promisc,1000,errbuff);//create the handle on NIC
	if(handle==NULL){printf("Error at pcap_open_live.\n");printf("%s\n",errbuff);return 2;}
	e=pcap_compile(handle,&fp,filterexp,0,net);//compile the text BPF format to computer-format
	if(e==-1){printf("Error at filter compilation.\n");return 2;}
	pcap_setfilter(handle,&fp);//apply the BPF to the handle
	if(e==-1){printf("Error at setting filter.\n");return 2;}
	
	pcap_loop(handle,-1,pkt_rcvd,NULL);//listen on the handle(NIC) in a loop, when packet recieved call pkt_rcvd(packet)
	pcap_close(handle);
	return 0;

}



