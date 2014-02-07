/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 * BOOTP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *			- name of bootfile
 *	Next step:	ARP
 *
 * RARP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *	Next step:	ARP
 *
 * ARP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *	We want:	- TFTP server ethernet address
 *	Next step:	TFTP
 *
 * DHCP:
 *
 *     Prerequisites:	- own ethernet address
 *     We want:		- IP, Netmask, ServerIP, Gateway IP
 *			- bootfilename, lease time
 *     Next step:	- TFTP
 *
 * TFTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *			- TFTP server ethernet address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * NFS:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * SNTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *	We want:	- network time
 *	Next step:	none
 */


#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <net.h>
#include "bootp.h"
#include "tftp.h"
#include "rarp.h"
#include "nfs.h"
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#include <miiphy.h>
#endif
#if defined(CONFIG_CMD_SNTP)
#include "sntp.h"
#endif
#if defined(CONFIG_CDP_VERSION)
#include <timestamp.h>
#endif
#if defined(CONFIG_CMD_DNS)
#include "dns.h"
#endif

#if defined(CONFIG_CMD_NET)

DECLARE_GLOBAL_DATA_PTR;

#ifndef	CONFIG_ARP_TIMEOUT
# define ARP_TIMEOUT		5000UL	/* Milliseconds before trying ARP again */
#else
# define ARP_TIMEOUT		CONFIG_ARP_TIMEOUT
#endif


#ifndef	CONFIG_NET_RETRY_COUNT
# define ARP_TIMEOUT_COUNT	5	/* # of timeouts before giving up  */
#else
# define ARP_TIMEOUT_COUNT	CONFIG_NET_RETRY_COUNT
#endif

/** BOOTP EXTENTIONS **/

IPaddr_t	NetOurSubnetMask=0;		/* Our subnet mask (0=unknown)	*/
IPaddr_t	NetOurGatewayIP=0;		/* Our gateways IP address	*/
IPaddr_t	NetOurDNSIP=0;			/* Our DNS IP address		*/
#if defined(CONFIG_BOOTP_DNS2)
IPaddr_t	NetOurDNS2IP=0;			/* Our 2nd DNS IP address	*/
#endif
char		NetOurNISDomain[32]={0,};	/* Our NIS domain		*/
char		NetOurHostName[32]={0,};	/* Our hostname			*/
char		NetOurRootPath[64]={0,};	/* Our bootpath			*/
ushort		NetBootFileSize=0;		/* Our bootfile size in blocks	*/

#ifdef CONFIG_MCAST_TFTP	/* Multicast TFTP */
IPaddr_t Mcast_addr;
#endif

/** END OF BOOTP EXTENTIONS **/

ulong		NetBootFileXferSize;	/* The actual transferred size of the bootfile (in bytes) */
uchar		NetOurEther[6];		/* Our ethernet address			*/
uchar		NetServerEther[6] =	/* Boot server enet address		*/
			{ 0, 0, 0, 0, 0, 0 };
IPaddr_t	NetOurIP;		/* Our IP addr (0 = unknown)		*/
IPaddr_t	NetServerIP;		/* Server IP addr (0 = unknown)		*/
volatile uchar *NetRxPacket;		/* Current receive packet		*/
int		NetRxPacketLen;		/* Current rx packet length		*/
unsigned	NetIPID;		/* IP packet ID				*/
uchar		NetBcastAddr[6] =	/* Ethernet bcast address		*/
			{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uchar		NetEtherNullAddr[6] =
			{ 0, 0, 0, 0, 0, 0 };
#ifdef CONFIG_API
void		(*push_packet)(volatile void *, int len) = 0;
#endif
#if defined(CONFIG_CMD_CDP)
uchar		NetCDPAddr[6] =		/* Ethernet bcast address		*/
			{ 0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc };
#endif
int		NetState;		/* Network loop state			*/
#ifdef CONFIG_NET_MULTI
int		NetRestartWrap = 0;	/* Tried all network devices		*/
static int	NetRestarted = 0;	/* Network loop restarted		*/
static int	NetDevExists = 0;	/* At least one device configured	*/
#endif

/* XXX in both little & big endian machines 0xFFFF == ntohs(-1) */
ushort		NetOurVLAN = 0xFFFF;		/* default is without VLAN	*/
ushort		NetOurNativeVLAN = 0xFFFF;	/* ditto			*/

char		BootFile[128];		/* Boot File name			*/

#if defined(CONFIG_CMD_PING)
IPaddr_t	NetPingIP;		/* the ip address to ping		*/

static void PingStart(void);
#endif

#if defined(CONFIG_CMD_CDP)
static void CDPStart(void);
#endif

#if defined(CONFIG_CMD_SNTP)
IPaddr_t	NetNtpServerIP;		/* NTP server IP address		*/
int		NetTimeOffset=0;	/* offset time from UTC			*/
#endif

#ifdef CONFIG_NETCONSOLE
void NcStart(void);
int nc_input_packet(uchar *pkt, unsigned dest, unsigned src, unsigned len);
#endif

volatile uchar	PktBuf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];

volatile uchar *NetRxPackets[PKTBUFSRX]; /* Receive packets			*/

static rxhand_f *packetHandler;		/* Current RX packet handler		*/
static thand_f *timeHandler;		/* Current timeout handler		*/
static ulong	timeStart;		/* Time base value			*/
static ulong	timeDelta;		/* Current timeout value		*/
volatile uchar *NetTxPacket = 0;	/* THE transmit packet			*/

static int net_check_prereq (proto_t protocol);

/**********************************************************************/

IPaddr_t	NetArpWaitPacketIP;
IPaddr_t	NetArpWaitReplyIP;
uchar	       *NetArpWaitPacketMAC;	/* MAC address of waiting packet's destination	*/
uchar	       *NetArpWaitTxPacket;	/* THE transmit packet			*/
int		NetArpWaitTxPacketSize;
uchar		NetArpWaitPacketBuf[PKTSIZE_ALIGN + PKTALIGN];
ulong		NetArpWaitTimerStart;
int		NetArpWaitTry;

void ArpRequest (void)
{
}

void ArpTimeoutCheck(void)
{
	ulong t;

	if (!NetArpWaitPacketIP)
		return;

	t = get_timer(0);

	/* check for arp timeout */
	if ((t - NetArpWaitTimerStart) > ARP_TIMEOUT) {
		NetArpWaitTry++;

		if (NetArpWaitTry >= ARP_TIMEOUT_COUNT) {
			puts ("\nARP Retry count exceeded; starting again\n");
			NetArpWaitTry = 0;
			NetStartAgain();
		} else {
			NetArpWaitTimerStart = t;
			ArpRequest();
		}
	}
}

static void
NetInitLoop(proto_t protocol)
{
	static int env_changed_id = 0;
	bd_t *bd = gd->bd;
	int env_id = get_env_id ();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		NetCopyIP(&NetOurIP, &bd->bi_ip_addr);
		NetOurGatewayIP = getenv_IPaddr ("gatewayip");
		NetOurSubnetMask= getenv_IPaddr ("netmask");
		NetServerIP = getenv_IPaddr ("serverip");
		NetOurNativeVLAN = getenv_VLAN("nvlan");
		NetOurVLAN = getenv_VLAN("vlan");
#if defined(CONFIG_CMD_DNS)
		NetOurDNSIP = getenv_IPaddr("dnsip");
#endif
		env_changed_id = env_id;
	}

	return;
}

/**********************************************************************/
/*
 *	Main network processing loop.
 */

int
NetLoop(proto_t protocol){
}

/**********************************************************************/

static void
startAgainTimeout(void)
{
	NetState = NETLOOP_RESTART;
}

static void
startAgainHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	/* Totally ignore the packet */
}

void NetStartAgain (void)
{

}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

void
NetSetHandler(rxhand_f * f)
{
	packetHandler = f;
}


void
NetSetTimeout(ulong iv, thand_f * f)
{
	if (iv == 0) {
		timeHandler = (thand_f *)0;
	} else {
		timeHandler = f;
		timeStart = get_timer(0);
		timeDelta = iv;
	}
}


void
NetSendPacket(volatile uchar * pkt, int len)
{

}

int
NetSendUDPPacket(uchar *ether, IPaddr_t dest, int dport, int sport, int len)
{

}

#if defined(CONFIG_CMD_PING)
static ushort PingSeqNo;

int PingSend(void)
{
	static uchar mac[6];
	volatile IP_t *ip;
	volatile ushort *s;
	uchar *pkt;

	/* XXX always send arp request */

	memcpy(mac, NetEtherNullAddr, 6);

	debug("sending ARP for %08lx\n", NetPingIP);

	NetArpWaitPacketIP = NetPingIP;
	NetArpWaitPacketMAC = mac;

	pkt = NetArpWaitTxPacket;
	pkt += NetSetEther(pkt, mac, PROT_IP);

	ip = (volatile IP_t *)pkt;

	/*
	 *	Construct an IP and ICMP header.  (need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE_NO_UDP + 8);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_p     = 0x01;		/* ICMP */
	ip->ip_sum   = 0;
	NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
	NetCopyIP((void*)&ip->ip_dst, &NetPingIP);	   /* - "" - */
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);

	s = &ip->udp_src;		/* XXX ICMP starts here */
	s[0] = htons(0x0800);		/* echo-request, code */
	s[1] = 0;			/* checksum */
	s[2] = 0;			/* identifier */
	s[3] = htons(PingSeqNo++);	/* sequence number */
	s[1] = ~NetCksum((uchar *)s, 8/2);

	/* size of the waiting packet */
	NetArpWaitTxPacketSize = (pkt - NetArpWaitTxPacket) + IP_HDR_SIZE_NO_UDP + 8;

	/* and do the ARP request */
	NetArpWaitTry = 1;
	NetArpWaitTimerStart = get_timer(0);
	ArpRequest();
	return 1;	/* waiting */
}

static void
PingTimeout (void)
{
	eth_halt();
	NetState = NETLOOP_FAIL;	/* we did not get the reply */
}

static void
PingHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	IPaddr_t tmp;
	volatile IP_t *ip = (volatile IP_t *)pkt;

	tmp = NetReadIP((void *)&ip->ip_src);
	if (tmp != NetPingIP)
		return;

	NetState = NETLOOP_SUCCESS;
}

static void PingStart(void)
{
#if defined(CONFIG_NET_MULTI)
	printf ("Using %s device\n", eth_get_name());
#endif	/* CONFIG_NET_MULTI */
	NetSetTimeout (10000UL, PingTimeout);
	NetSetHandler (PingHandler);

	PingSend();
}
#endif

#if defined(CONFIG_CMD_CDP)

#define CDP_DEVICE_ID_TLV		0x0001
#define CDP_ADDRESS_TLV			0x0002
#define CDP_PORT_ID_TLV			0x0003
#define CDP_CAPABILITIES_TLV		0x0004
#define CDP_VERSION_TLV			0x0005
#define CDP_PLATFORM_TLV		0x0006
#define CDP_NATIVE_VLAN_TLV		0x000a
#define CDP_APPLIANCE_VLAN_TLV		0x000e
#define CDP_TRIGGER_TLV			0x000f
#define CDP_POWER_CONSUMPTION_TLV	0x0010
#define CDP_SYSNAME_TLV			0x0014
#define CDP_SYSOBJECT_TLV		0x0015
#define CDP_MANAGEMENT_ADDRESS_TLV	0x0016

#define CDP_TIMEOUT			250UL	/* one packet every 250ms */

static int CDPSeq;
static int CDPOK;

ushort CDPNativeVLAN;
ushort CDPApplianceVLAN;

static const uchar CDP_SNAP_hdr[8] = { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x0C, 0x20, 0x00 };

static ushort CDP_compute_csum(const uchar *buff, ushort len)
{
	ushort csum;
	int     odd;
	ulong   result = 0;
	ushort  leftover;
	ushort *p;

	if (len > 0) {
		odd = 1 & (ulong)buff;
		if (odd) {
			result = *buff << 8;
			len--;
			buff++;
		}
		while (len > 1) {
			p = (ushort *)buff;
			result += *p++;
			buff = (uchar *)p;
			if (result & 0x80000000)
				result = (result & 0xFFFF) + (result >> 16);
			len -= 2;
		}
		if (len) {
			leftover = (signed short)(*(const signed char *)buff);
			/* CISCO SUCKS big time! (and blows too):
			 * CDP uses the IP checksum algorithm with a twist;
			 * for the last byte it *sign* extends and sums.
			 */
			result = (result & 0xffff0000) | ((result + leftover) & 0x0000ffff);
		}
		while (result >> 16)
			result = (result & 0xFFFF) + (result >> 16);

		if (odd)
			result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
	}

	/* add up 16-bit and 17-bit words for 17+c bits */
	result = (result & 0xffff) + (result >> 16);
	/* add up 16-bit and 2-bit for 16+c bit */
	result = (result & 0xffff) + (result >> 16);
	/* add up carry.. */
	result = (result & 0xffff) + (result >> 16);

	/* negate */
	csum = ~(ushort)result;

	/* run time endian detection */
	if (csum != htons(csum))	/* little endian */
		csum = htons(csum);

	return csum;
}

int CDPSendTrigger(void)
{
	volatile uchar *pkt;
	volatile ushort *s;
	volatile ushort *cp;
	Ethernet_t *et;
	int len;
	ushort chksum;
#if defined(CONFIG_CDP_DEVICE_ID) || defined(CONFIG_CDP_PORT_ID)   || \
    defined(CONFIG_CDP_VERSION)   || defined(CONFIG_CDP_PLATFORM)
	char buf[32];
#endif

	pkt = NetTxPacket;
	et = (Ethernet_t *)pkt;

	/* NOTE: trigger sent not on any VLAN */

	/* form ethernet header */
	memcpy(et->et_dest, NetCDPAddr, 6);
	memcpy(et->et_src, NetOurEther, 6);

	pkt += ETHER_HDR_SIZE;

	/* SNAP header */
	memcpy((uchar *)pkt, CDP_SNAP_hdr, sizeof(CDP_SNAP_hdr));
	pkt += sizeof(CDP_SNAP_hdr);

	/* CDP header */
	*pkt++ = 0x02;				/* CDP version 2 */
	*pkt++ = 180;				/* TTL */
	s = (volatile ushort *)pkt;
	cp = s;
	*s++ = htons(0);			/* checksum (0 for later calculation) */

	/* CDP fields */
#ifdef CONFIG_CDP_DEVICE_ID
	*s++ = htons(CDP_DEVICE_ID_TLV);
	*s++ = htons(CONFIG_CDP_DEVICE_ID);
	sprintf(buf, CONFIG_CDP_DEVICE_ID_PREFIX "%pm", NetOurEther);
	memcpy((uchar *)s, buf, 16);
	s += 16 / 2;
#endif

#ifdef CONFIG_CDP_PORT_ID
	*s++ = htons(CDP_PORT_ID_TLV);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, CONFIG_CDP_PORT_ID, eth_get_dev_index());
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_CAPABILITIES
	*s++ = htons(CDP_CAPABILITIES_TLV);
	*s++ = htons(8);
	*(ulong *)s = htonl(CONFIG_CDP_CAPABILITIES);
	s += 2;
#endif

#ifdef CONFIG_CDP_VERSION
	*s++ = htons(CDP_VERSION_TLV);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, CONFIG_CDP_VERSION);
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_PLATFORM
	*s++ = htons(CDP_PLATFORM_TLV);
	memset(buf, 0, sizeof(buf));
	strcpy(buf, CONFIG_CDP_PLATFORM);
	len = strlen(buf);
	if (len & 1)	/* make it even */
		len++;
	*s++ = htons(len + 4);
	memcpy((uchar *)s, buf, len);
	s += len / 2;
#endif

#ifdef CONFIG_CDP_TRIGGER
	*s++ = htons(CDP_TRIGGER_TLV);
	*s++ = htons(8);
	*(ulong *)s = htonl(CONFIG_CDP_TRIGGER);
	s += 2;
#endif

#ifdef CONFIG_CDP_POWER_CONSUMPTION
	*s++ = htons(CDP_POWER_CONSUMPTION_TLV);
	*s++ = htons(6);
	*s++ = htons(CONFIG_CDP_POWER_CONSUMPTION);
#endif

	/* length of ethernet packet */
	len = (uchar *)s - ((uchar *)NetTxPacket + ETHER_HDR_SIZE);
	et->et_protlen = htons(len);

	len = ETHER_HDR_SIZE + sizeof(CDP_SNAP_hdr);
	chksum = CDP_compute_csum((uchar *)NetTxPacket + len, (uchar *)s - (NetTxPacket + len));
	if (chksum == 0)
		chksum = 0xFFFF;
	*cp = htons(chksum);

	(void) eth_send(NetTxPacket, (uchar *)s - NetTxPacket);
	return 0;
}

static void
CDPTimeout (void)
{
	CDPSeq++;

	if (CDPSeq < 3) {
		NetSetTimeout (CDP_TIMEOUT, CDPTimeout);
		CDPSendTrigger();
		return;
	}

	/* if not OK try again */
	if (!CDPOK)
		NetStartAgain();
	else
		NetState = NETLOOP_SUCCESS;
}

static void
CDPDummyHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	/* nothing */
}

static void
CDPHandler(const uchar * pkt, unsigned len)
{
	const uchar *t;
	const ushort *ss;
	ushort type, tlen;
	uchar applid;
	ushort vlan, nvlan;

	/* minimum size? */
	if (len < sizeof(CDP_SNAP_hdr) + 4)
		goto pkt_short;

	/* check for valid CDP SNAP header */
	if (memcmp(pkt, CDP_SNAP_hdr, sizeof(CDP_SNAP_hdr)) != 0)
		return;

	pkt += sizeof(CDP_SNAP_hdr);
	len -= sizeof(CDP_SNAP_hdr);

	/* Version of CDP protocol must be >= 2 and TTL != 0 */
	if (pkt[0] < 0x02 || pkt[1] == 0)
		return;

	/* if version is greater than 0x02 maybe we'll have a problem; output a warning */
	if (pkt[0] != 0x02)
		printf("** WARNING: CDP packet received with a protocol version %d > 2\n",
				pkt[0] & 0xff);

	if (CDP_compute_csum(pkt, len) != 0)
		return;

	pkt += 4;
	len -= 4;

	vlan = htons(-1);
	nvlan = htons(-1);
	while (len > 0) {
		if (len < 4)
			goto pkt_short;

		ss = (const ushort *)pkt;
		type = ntohs(ss[0]);
		tlen = ntohs(ss[1]);
		if (tlen > len) {
			goto pkt_short;
		}

		pkt += tlen;
		len -= tlen;

		ss += 2;	/* point ss to the data of the TLV */
		tlen -= 4;

		switch (type) {
			case CDP_DEVICE_ID_TLV:
				break;
			case CDP_ADDRESS_TLV:
				break;
			case CDP_PORT_ID_TLV:
				break;
			case CDP_CAPABILITIES_TLV:
				break;
			case CDP_VERSION_TLV:
				break;
			case CDP_PLATFORM_TLV:
				break;
			case CDP_NATIVE_VLAN_TLV:
				nvlan = *ss;
				break;
			case CDP_APPLIANCE_VLAN_TLV:
				t = (const uchar *)ss;
				while (tlen > 0) {
					if (tlen < 3)
						goto pkt_short;

					applid = t[0];
					ss = (const ushort *)(t + 1);

#ifdef CONFIG_CDP_APPLIANCE_VLAN_TYPE
					if (applid == CONFIG_CDP_APPLIANCE_VLAN_TYPE)
						vlan = *ss;
#else
					vlan = ntohs(*ss);	/* XXX will this work; dunno */
#endif
					t += 3; tlen -= 3;
				}
				break;
			case CDP_TRIGGER_TLV:
				break;
			case CDP_POWER_CONSUMPTION_TLV:
				break;
			case CDP_SYSNAME_TLV:
				break;
			case CDP_SYSOBJECT_TLV:
				break;
			case CDP_MANAGEMENT_ADDRESS_TLV:
				break;
		}
	}

	CDPApplianceVLAN = vlan;
	CDPNativeVLAN = nvlan;

	CDPOK = 1;
	return;

 pkt_short:
	printf("** CDP packet is too short\n");
	return;
}

static void CDPStart(void)
{
#if defined(CONFIG_NET_MULTI)
	printf ("Using %s device\n", eth_get_name());
#endif
	CDPSeq = 0;
	CDPOK = 0;

	CDPNativeVLAN = htons(-1);
	CDPApplianceVLAN = htons(-1);

	NetSetTimeout (CDP_TIMEOUT, CDPTimeout);
	NetSetHandler (CDPDummyHandler);

	CDPSendTrigger();
}
#endif


void
NetReceive(volatile uchar * inpkt, int len)
{
}

/**********************************************************************/

static int net_check_prereq (proto_t protocol)
{
	switch (protocol) {
		/* Fall through */
#if defined(CONFIG_CMD_PING)
	case PING:
		if (NetPingIP == 0) {
			puts ("*** ERROR: ping address not given\n");
			return (1);
		}
		goto common;
#endif
#if defined(CONFIG_CMD_SNTP)
	case SNTP:
		if (NetNtpServerIP == 0) {
			puts ("*** ERROR: NTP server address not given\n");
			return (1);
		}
		goto common;
#endif
#if defined(CONFIG_CMD_DNS)
	case DNS:
		if (NetOurDNSIP == 0) {
			puts("*** ERROR: DNS server address not given\n");
			return 1;
		}
		goto common;
#endif
#if defined(CONFIG_CMD_NFS)
	case NFS:
#endif
	case NETCONS:
	case TFTP:
		if (NetServerIP == 0) {
			puts ("*** ERROR: `serverip' not set\n");
			return (1);
		}
#if defined(CONFIG_CMD_PING) || defined(CONFIG_CMD_SNTP)
    common:
#endif

		if (NetOurIP == 0) {
			puts ("*** ERROR: `ipaddr' not set\n");
			return (1);
		}
		/* Fall through */

	case DHCP:
	case RARP:
	case BOOTP:
	case CDP:
		if (memcmp (NetOurEther, "\0\0\0\0\0\0", 6) == 0) {
#ifdef CONFIG_NET_MULTI
			extern int eth_get_dev_index (void);
			int num = eth_get_dev_index ();

			switch (num) {
			case -1:
				puts ("*** ERROR: No ethernet found.\n");
				return (1);
			case 0:
				puts ("*** ERROR: `ethaddr' not set\n");
				break;
			default:
				printf ("*** ERROR: `eth%daddr' not set\n",
					num);
				break;
			}

			NetStartAgain ();
			return (2);
#else
			puts ("*** ERROR: `ethaddr' not set\n");
			return (1);
#endif
		}
		/* Fall through */
	default:
		return (0);
	}
	return (0);		/* OK */
}
/**********************************************************************/

int
NetCksumOk(uchar * ptr, int len)
{
	return !((NetCksum(ptr, len) + 1) & 0xfffe);
}


unsigned
NetCksum(uchar * ptr, int len)
{
	ulong	xsum;
	ushort *p = (ushort *)ptr;

	xsum = 0;
	while (len-- > 0)
		xsum += *p++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return (xsum & 0xffff);
}

int
NetEthHdrSize(void)
{
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	return ((myvlanid & VLAN_IDMASK) == VLAN_NONE) ? ETHER_HDR_SIZE : VLAN_ETHER_HDR_SIZE;
}

int
NetSetEther(volatile uchar * xet, uchar * addr, uint prot)
{
	Ethernet_t *et = (Ethernet_t *)xet;
	ushort myvlanid;

	myvlanid = ntohs(NetOurVLAN);
	if (myvlanid == (ushort)-1)
		myvlanid = VLAN_NONE;

	memcpy (et->et_dest, addr, 6);
	memcpy (et->et_src, NetOurEther, 6);
	if ((myvlanid & VLAN_IDMASK) == VLAN_NONE) {
	et->et_protlen = htons(prot);
		return ETHER_HDR_SIZE;
	} else {
		VLAN_Ethernet_t *vet = (VLAN_Ethernet_t *)xet;

		vet->vet_vlan_type = htons(PROT_VLAN);
		vet->vet_tag = htons((0 << 5) | (myvlanid & VLAN_IDMASK));
		vet->vet_type = htons(prot);
		return VLAN_ETHER_HDR_SIZE;
	}
}

void
NetSetIP(volatile uchar * xip, IPaddr_t dest, int dport, int sport, int len)
{
	IP_t *ip = (IP_t *)xip;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
	 *	(need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = htons(IP_HDR_SIZE + len);
	ip->ip_id    = htons(NetIPID++);
	ip->ip_off   = htons(IP_FLAGS_DFRAG);	/* Don't fragment */
	ip->ip_ttl   = 255;
	ip->ip_p     = 17;		/* UDP */
	ip->ip_sum   = 0;
	NetCopyIP((void*)&ip->ip_src, &NetOurIP); /* already in network byte order */
	NetCopyIP((void*)&ip->ip_dst, &dest);	   /* - "" - */
	ip->udp_src  = htons(sport);
	ip->udp_dst  = htons(dport);
	ip->udp_len  = htons(8 + len);
	ip->udp_xsum = 0;
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);
}

void copy_filename (char *dst, char *src, int size)
{
	if (*src && (*src == '"')) {
		++src;
		--size;
	}

	while ((--size > 0) && *src && (*src != '"')) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

#endif

#if defined(CONFIG_CMD_NFS) || defined(CONFIG_CMD_SNTP) || defined(CONFIG_CMD_DNS)
/*
 * make port a little random, but use something trivial to compute
 */
unsigned int random_port(void)
{
	return 1024 + (get_timer(0) % 0x8000);;
}
#endif

void ip_to_string (IPaddr_t x, char *s)
{
	x = ntohl (x);
	sprintf (s, "%d.%d.%d.%d",
		 (int) ((x >> 24) & 0xff),
		 (int) ((x >> 16) & 0xff),
		 (int) ((x >> 8) & 0xff), (int) ((x >> 0) & 0xff)
	);
}

IPaddr_t string_to_ip(char *s)
{
	IPaddr_t addr;
	char *e;
	int i;

	if (s == NULL)
		return(0);

	for (addr=0, i=0; i<4; ++i) {
		ulong val = s ? simple_strtoul(s, &e, 10) : 0;
		addr <<= 8;
		addr |= (val & 0xFF);
		if (s) {
			s = (*e) ? e+1 : e;
		}
	}

	return (htonl(addr));
}

void VLAN_to_string(ushort x, char *s)
{
	x = ntohs(x);

	if (x == (ushort)-1)
		x = VLAN_NONE;

	if (x == VLAN_NONE)
		strcpy(s, "none");
	else
		sprintf(s, "%d", x & VLAN_IDMASK);
}

ushort string_to_VLAN(char *s)
{
	ushort id;

	if (s == NULL)
		return htons(VLAN_NONE);

	if (*s < '0' || *s > '9')
		id = VLAN_NONE;
	else
		id = (ushort)simple_strtoul(s, NULL, 10);

	return htons(id);
}

IPaddr_t getenv_IPaddr (char *var)
{
	return (string_to_ip(getenv(var)));
}

ushort getenv_VLAN(char *var)
{
	return (string_to_VLAN(getenv(var)));
}
