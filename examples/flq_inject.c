#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <sys/time.h> 

#include <lorcon2/lorcon.h> 
#include <lorcon2/lorcon_packasm.h> 
#include <lorcon2/lorcon_ieee80211.h>
#include <lorcon2/lorcon_forge.h>


typedef void (*inject_func_t)() ;


int g_channel_type = LORCON_CHANNEL_BASIC ;

char *interface = "wlp8s0" ;
uint8_t *mac = "\x00\x16\xEA\x12\x34\x56";

lorcon_driver_t *drvlist, *driver; // Needed to set up interface/context
lorcon_t *context; // LORCON context

lcpa_metapack_t *metapack; // metapack for LORCON packet assembly 
lorcon_packet_t *txpack; // The raw packet to be sent
lorcon_channel_t lorcon_channel ;//fflq

// Supported Rates  
uint8_t rates[] = "\x8c\x12\x98\x24\xb0\x48\x60\x6c"; // 6,9,12,18,24,36,48,54

// Beacon Interval
int interval = 100;
// Capabilities
int capabilities = 0x0421; 
//int capabilities = 0x4101; //fflq



//lcpf_action_no_ack
void lcpf_action_no_ack(struct lcpa_metapack *pack, unsigned int fcflags, 
		unsigned int duration, uint8_t *mac1, uint8_t *mac2, 
		uint8_t *mac3, uint8_t *mac4, unsigned int fragment, 
		unsigned int sequence) 
{
	//lcpf_80211headers(metapack, WLAN_FC_TYPE_MGMT, WLAN_FC_SUBTYPE_CFEND, 
	//		0x80, 0, mac, mac, mac, 0, 0, 0) ; 
	lcpf_80211headers(pack, WLAN_FC_TYPE_MGMT, WLAN_FC_SUBTYPE_CFEND, 
			fcflags, duration, mac1, mac2, mac3, mac4, fragment, sequence) ; 
}


void flq_inject()
{
	//printf("*fflq %s\n", __func__) ;
	metapack = lcpa_init();
	//lcpf_action_no_ack(metapack, 0x00, 0, mac, mac, mac, 0, 0, 0) ;
	lcpf_action_no_ack(metapack, 0x80, 0, mac, mac, mac, 0, 0, 0) ;
	lcpf_add_ie(metapack, 1, sizeof(rates)-1, rates); 

	txpack = (lorcon_packet_t *)lorcon_packet_from_lcpa(context, metapack);
    //lorcon_packet_set_mcs(txpack, 1, 1, 1, 1); //fflq
	lorcon_inject(context,txpack) ;
	lcpa_free(metapack);
}


int main(int argc, char *argv[]) {
	if (argc > 1)	g_channel_type = atoi(argv[1]) ;

	driver = lorcon_auto_driver(interface) ;
	context = lorcon_create(interface, driver) ;
	lorcon_open_injmon(context) ; // Create Monitor Mode Interface
	//printf("*fflq, fd %d\n", ((struct flq_lorcon*)context)->inject_fd) ;

	lorcon_channel.channel = 64 ;
	lorcon_channel.type = g_channel_type ;
	//lorcon_set_complex_channel(context, &lorcon_channel) ;
	printf("[+] Using channel: %d type %d flags %d\n\n", 
			lorcon_channel.channel, lorcon_channel.type, 0);

	//fflq, same if use setted channel, not above lorcon_set_complex_channel
	/*
	inject_func_t inject_funcs[20] = {
		[LORCON_CHANNEL_BASIC] = inject_noht, 
		[LORCON_CHANNEL_HT20] = inject_ht20, 
		[LORCON_CHANNEL_HT40M] = inject_noht, 
		[LORCON_CHANNEL_HT40P] = inject_noht, 
		[LORCON_CHANNEL_VHT80] = inject_noht, 
		[LORCON_CHANNEL_VHT160] = inject_noht, 
	} ;
	*/

	int count = 0 ;
	while(1) {
		//if (!inject_funcs[g_channel_type])	break ;
		//inject_funcs[g_channel_type]() ;
		flq_inject() ;
	
		printf("\033[K\r");
		printf("[+] Sent %d frames, %d", ++count, g_channel_type);
		fflush(stdout) ;

		usleep(1000000); //fflq
	}

	// Close the interface
	lorcon_close(context);
	// Free the LORCON Context
	lorcon_free(context);	
	
	return 0;
}




struct flq_lorcon {
	char drivername[32];

	char *ifname;
	char *vapname;

	pcap_t *pcap;

	/* Only capture_fd is assumed to be selectable */
	int inject_fd, ioctl_fd, capture_fd;

	int packets_sent;
	int packets_recv;

	int dlt;

	int channel;
    int channel_ht_flags;

	char errstr[LORCON_STATUS_MAX];

	uint8_t original_mac[6];

	/*
	int timeout_ms;

	void *auxptr;

    void *userauxptr;

	lorcon_handler handler_cb;
	void *handler_user;

	int (*close_cb)(lorcon_t *context);
	
	int (*openinject_cb)(lorcon_t *context);
	int (*openmon_cb)(lorcon_t *context);
	int (*openinjmon_cb)(lorcon_t *context);

	int (*ifconfig_cb)(lorcon_t *context, int state);

	int (*setchan_cb)(lorcon_t *context, int chan);
	int (*getchan_cb)(lorcon_t *context);

    int (*setchan_ht_cb)(lorcon_t *context, lorcon_channel_t *channel);
	int (*getchan_ht_cb)(lorcon_t *context, lorcon_channel_t *ret_channel);

	int (*sendpacket_cb)(lorcon_t *context, lorcon_packet_t *packet);
	int (*getpacket_cb)(lorcon_t *context, lorcon_packet_t **packet);

	int (*setdlt_cb)(lorcon_t *context, int dlt);
	int (*getdlt_cb)(lorcon_t *context);

	lorcon_wep_t *wepkeys;

	int (*getmac_cb)(lorcon_t *context, uint8_t **mac);
	int (*setmac_cb)(lorcon_t *context, int len, uint8_t *mac);

    int (*pcap_handler_cb)(u_char *user, const struct pcap_pkthdr *h,
            const u_char *bytes);
*/
};





/*
void inject_origin_codes()
{
	while(1) {
		metapack = lcpa_init();
		
		//NOHT
		//lcpf_action_no_ack(metapack, 0x00, 0, mac, mac, mac, 0, 0, 0) ;

		//HT20
		lcpf_action_no_ack(metapack, 0x80, 0, mac, mac, mac, 0, 0, 0) ;
		
		//fflq key to disp HTC: fcflags=0x80 and set ie rates
		// Append IE Tag 1 for rates, fflq must have or no pcap
        lcpf_add_ie(metapack, 1, sizeof(rates)-1, rates); 

		// Append IE Tag 0 for SSID
		//lcpf_add_ie(metapack, 0, strlen(ssid),ssid);

		// Append IE Tag 3 for Channel 
		//lcpf_add_ie(metapack, 3, 1, &channel);

		// Append IE Tags 42/47 for ERP Info 
		//lcpf_add_ie(metapack, 42, 1, "\x05");
		//lcpf_add_ie(metapack, 47, 1, "\x05");
	
		// Convert the LORCON metapack to a LORCON packet for sending
		txpack = (lorcon_packet_t *) lorcon_packet_from_lcpa(context, metapack);

        //lorcon_packet_set_mcs(txpack, 1, mcs, shortgi, ht40); //fflq
        //lorcon_packet_set_mcs(txpack, 1, 0, 0, 1); //fflq
		
		// Send and exit if error
		if (lorcon_inject(context,txpack) < 0 )	return -1;

		printf("\033[K\r");
		printf("[+] Sent %d frames, Hit CTRL + C to stop...", count++);
		fflush(stdout) ;

		// Free the metapack
		lcpa_free(metapack);

		usleep(1000000); //fflq
	}

}
*/


