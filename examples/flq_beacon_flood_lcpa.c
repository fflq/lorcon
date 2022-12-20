/*
	beacon_flood_lcpa.c 
	by brad.antoniewicz@foundstone.com	

	simple IEEE 802.11 beacon flooder using LORCON2's 
	packet assembly functionality

    also demonstrates setting MCS values and HT channel
    parsing

*/

#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include <sys/time.h> // Needed for timestamp

#include <lorcon2/lorcon.h> // For LORCON 
#include <lorcon2/lorcon_packasm.h> // For metapack packet assembly
#include <lorcon2/lorcon_ieee80211.h>

void usage(char *argv[]) {
	printf("\t-s <SSID>\tSSID to flood\n");
	printf("\t-i <int> \tInterface\n");
	printf("\t-c <channel>\tChannel\n");
    printf("\t-m <mcs>\tTransmit frames at a specific MCS\n");
    printf("\t-g          \tEnable Short-GI frame timing\n");
    printf("\t-H          \tTransmit frames at HT40\n");
	printf("\nExample:\n");
	printf("\t%s -s brad -i wlan0 -c 1\n\n",argv[0]);
}
int main(int argc, char *argv[]) {

	char *interface = NULL, *ssid = NULL;
	int c;
    int channel, ch_flags;
	unsigned int count = 0;

    int mcs = -1;
    int shortgi = 0;
    int ht40 = 0;

	lorcon_driver_t *drvlist, *driver; // Needed to set up interface/context
	lorcon_t *context; // LORCON context

	lcpa_metapack_t *metapack; // metapack for LORCON packet assembly 
	lorcon_packet_t *txpack; // The raw packet to be sent

	/* 
		These are needed for the actual beacon frame
	*/
		
	// BSSID and source MAC address
	//uint8_t *mac = "\x00\xDE\xAD\xBE\xEF\x00";
	uint8_t *mac = "\x00\x16\xEA\x12\x34\x56";

	// Timestamp
        struct timeval time; 
        uint64_t timestamp; 
	
	// Supported Rates  
	uint8_t rates[] = "\x8c\x12\x98\x24\xb0\x48\x60\x6c"; // 6,9,12,18,24,36,48,54

	// Beacon Interval
	int interval = 100;

	// Capabilities
	int capabilities = 0x0421; 
	//int capabilities = 0x4101; //fflq

	lorcon_channel_t lorcon_channel ;//fflq
	char buf[1024] ; int r ;

	printf ("%s - Simple 802.11 Beacon Flooder\n", argv[0]);
	printf ("-----------------------------------------------------\n\n");

	/* 
		This handles all of the command line arguments
	*/
	
	while ((c = getopt(argc, argv, "i:s:hc:m:Hg")) != EOF) {
		switch (c) {
			case 'i': 
				interface = strdup(optarg);
				break;
			case 's': 
				if ( strlen(strdup(optarg)) < 255 ) {
					ssid = strdup(optarg);
				} else {
					printf("ERROR: SSID Length too long! Should not exceed 255 characters\n");
					return -1;
				}
				break;
			case 'c':
                //if (lorcon_parse_ht_channel(optarg, &channel, &ch_flags) == 0) {
    			r = sscanf(optarg, "%u%16s", &channel, buf);
				printf("r %d, %u %s\n", r, channel, buf) ;
                if (lorcon_parse_ht_channel(optarg, &lorcon_channel) != 0) {
                    printf("ERROR: Unable to parse channel\n");
                    return -1;
                }
				break;
            case 'm':
                if (sscanf(optarg, "%d", &mcs) != 1) {
                    printf("ERROR: Unable to parse MCS value\n");
                    return -1;
                }

                if (mcs < 0 || mcs > 15) {
                    printf("ERROR: Expected MCS between 0 and 15\n");
                    return -1;
                }

                break;

            case 'H':
                ht40 = 1;

                if (mcs < 0)
                    mcs = 0;

                break;

            case 'g':

                shortgi = 1;

                if (mcs < 0)
                    mcs = 0;

                break;

			case 'h':
				usage(argv);
                return -1;
				break;
			default:
				usage(argv);
				break;
			}
	}

	if ( interface == NULL || ssid == NULL ) { 
		printf ("ERROR: Interface, channel, or SSID not set (see -h for more info)\n");
		return -1;
	}

	printf("[+] Using interface %s\n",interface);
	
	/*	
	 	The following is all of the standard interface, driver, and context setup
	*/

	// Automatically determine the driver of the interface
	
	if ( (driver = lorcon_auto_driver(interface)) == NULL) {
		printf("[!] Could not determine the driver for %s\n",interface);
		return -1;
	} else {
		printf("[+]\t Driver: %s\n",driver->name);
	}

	// Create LORCON context
	if ((context = lorcon_create(interface, driver)) == NULL) {
			printf("[!]\t Failed to create context");
			return -1; 
	}

	// Create Monitor Mode Interface
	if (lorcon_open_injmon(context) < 0) {
		printf("[!]\t Could not create Monitor Mode interface!\n");
		return -1;
	} else {
		printf("[+]\t Monitor Mode VAP: %s\n",lorcon_get_vap(context));
		lorcon_free_driver_list(driver);
	}

	// Set the channel we'll be injecting on
	//lorcon_set_ht_channel(context, channel, ch_flags);
	//printf("[+]\t Using channel: %d flags %d\n\n", channel, ch_flags);
	
	// fflq
	//lorcon_set_channel(context, lorcon_channel.channel);
	lorcon_set_complex_channel(context, &lorcon_channel) ;
	printf("[+]\t Using channel: %d type %d flags %d\n\n", 
			lorcon_channel.channel, lorcon_channel.type, ch_flags);

    if (mcs != -1) {
        printf("[+]\t Using MCS %u Short-GI %u HT40 %u\n",
                mcs, shortgi, ht40);
    }

	/* 
		The following is the packet creation and sending code
	*/

	// Keep sending frames until interrupted
	while(1) {

		// Create timestamp
		gettimeofday(&time, NULL);
		timestamp = time.tv_sec * 1000000 + time.tv_usec;

		// Initialize the LORCON metapack	
		metapack = lcpa_init();
		
		// Create a Beacon frame from 00:DE:AD:BE:EF:00
		//lcpf_beacon(metapack, mac, mac, 0x00, 0x0, 0x00, 0x00, timestamp, interval, capabilities);
		//lcpf_data(metapack, 9, mac, mac, mac, 0, 0, 0x00, 0x00);
		//lcpf_qos_data(metapack, 9, mac, mac, mac, mac, 0, 0x00, 0x00);
		//lcpf_assocreq(metapack, mac, mac, 0, 0, 0, 0x00, 0x00);

		//lcpf_action_no_ack
		lcpf_80211headers(metapack, WLAN_FC_TYPE_MGMT, WLAN_FC_SUBTYPE_CFEND, 
				0x80, 0, mac, mac, mac, 0, 0, 0) ; 

		//void lcpf_assocreq(struct lcpa_metapack *pack, uint8_t *dst, uint8_t *src, 
		//	uint8_t *bssid, int framecontrol, int duration, int fragment,
		//	int sequence, uint16_t capabilities, uint16_t listenint)
		//void lcpf_beacon(struct lcpa_metapack *pack, uint8_t *src, uint8_t *bssid, int framecontrol, 
		//	int duration, int fragment, int sequence, uint64_t timestamp, int beacon, int capabilities) 
		//lcpf_80211headers(pack, WLAN_FC_TYPE_MGMT, WLAN_FC_SUBTYPE_BEACON, framecontrol, duration,
		//			  chunk, src, bssid, NULL, fragment, sequence);
		
		/*fflq
		// Append IE Tag 0 for SSID
		lcpf_add_ie(metapack, 0, strlen(ssid),ssid);

		// Most of the following IE tags are not needed, but added here as examples
		*/

		//fflq key to disp HTC: fcflags=0x80 and set ie rates
		// Append IE Tag 1 for rates
        lcpf_add_ie(metapack, 1, sizeof(rates)-1, rates); 

		// Append IE Tag 3 for Channel 
		//lcpf_add_ie(metapack, 3, 1, &channel);

		// Append IE Tags 42/47 for ERP Info 
		//lcpf_add_ie(metapack, 42, 1, "\x05");
		//lcpf_add_ie(metapack, 47, 1, "\x05");
	
		// Convert the LORCON metapack to a LORCON packet for sending
		txpack = (lorcon_packet_t *) lorcon_packet_from_lcpa(context, metapack);

        //lorcon_packet_set_mcs(txpack, 1, mcs, shortgi, ht40); //fflq
        lorcon_packet_set_mcs(txpack, 1, 0, 0, 1); //fflq
		/*
        if (mcs != -1) {
            lorcon_packet_set_mcs(txpack, 1, mcs, shortgi, ht40); //fflq
        }
		*/
		
		// Send and exit if error
		if ( lorcon_inject(context,txpack) < 0 ) 
			return -1;

        // Wait interval before next beacon
        usleep(interval * 1000);

		// Print nice and pretty
		printf("\033[K\r");
		printf("[+] Sent %d frames, Hit CTRL + C to stop...", count);
		fflush(stdout);
		count++;

		// Free the metapack
		lcpa_free(metapack);

		usleep(1000000); //fflq
	}

	/* 
	 	The following is all of the standard cleanup stuff
	*/

	// Close the interface
	lorcon_close(context);

	// Free the LORCON Context
	lorcon_free(context);	
	
	return 0;
}

