/*
 * GSM_M66.h
 *
 * Created: 14-08-2017 13:13:06
 *  Author: Sarthak
 */ 

#include <asf.h>
#include <string.h>
#ifndef GSM_M66_H_
#define GSM_M66_H_

unsigned char searchForgsm[20];
unsigned char searchForgsm_1[20];
unsigned char searchForgsm_2[10];
unsigned char searchForgsm_3[13];
unsigned char searchForgsm_4[20];

//! Buffer counter
int volatile rx_i,rx_i2,rx_i3,rx_i4;
uint8_t  rx_ack;
//! Buffer write index
int volatile rx_wr_i;


#define AT_ERROR_SET					"AT+CMEE=1\r\n"						// Set Error Check
//#define AT_ERROR_SET					"AT+CMEE=2\r\n"						// Set Error Check
#define AT_ECHO_OFF						"ATE0\r\n"							// Echo OFF
#define AT_IMEI_NO						"AT+GSN\r\n"						// Get IMEI Number
#define AT_SIM_PIN_QUERY				"AT+CPIN?\r\n"						// Check GSM PIN Connection
#define AT_SIM_CCID						"AT+QCCID\r\n"						// GET SIM CCID
#define AT_GSM_STATUS					"AT+CPAS\r\n"
#define AT_NETWORK_REG_STATUS			"AT+COPS?\r\n"						// Check Network Registration status
#define AT_NETWORK_REG					"AT+CREG?\r\n"						// Specifies the SIM is registered to which type of Network
#define AT_SERVICE_PROVIDER_NAME		"AT+QSPN?\r\n"						// GET SERVICE PROVIDER NAME
#define AT_SIGNAL_QUALITY				"AT+CSQ\r\n"						// Get Signal Strength


#define AT_GPRS_ATTACH_STATUS			"AT+CGATT?\r\n"						// Check GPRS Attach Status
#define AT_NETWORK_GPRS_REG				"AT+CGREG?\r\n"						// Check GPRS Attach Status
#define AT_GPRS_ATTACH		 			"AT+CGATT=1\r\n"					// Attach GPRS
#define AT_GET_PDP_CONTEXT				"AT+CGDCONT?\r\n"					// Get PDP Context
#define AT_SET_PDP_CONTEXT				"AT+CGDCONT=1,\"IP\",\"WWW\"\r\n"					// Get PDP Context
#define AT_ACTIVATE_PDP					"AT+CGACT=1,1\r\n"

#define AT_GET_REGISTER_TCPSTACK		"AT+QIREGAPP\r\n"
//#define AT_REGISTER_TCPSTACK			"AT+QIREGAPP=\"airtelgprs.com\",\"\",\"\"\r\n"
#define AT_REGISTER_TCPSTACK			"AT+QIREGAPP=\"www\",\"\",\"\"\r\n"
//#define AT_REGISTER_TCPSTACK			"AT+QIREGAPP=\"internet\",\"\",\"\"\r\n"
#define AT_LOCAL_PORT					"AT+QILPORT=TCP,0\r\n"
#define AT_ACTIVATE_GPRS				"AT+QIACT\r"
#define AT_SERVER_OPEN					"AT+QIOPEN=\"TCP\",\"104.199.133.120\",3003\r\n"
#define AT_NTP_OPEN_1					"AT+QIOPEN=\"UDP\",\"202.71.136.67\",123\r\n"
#define AT_NTP_OPEN_2					"AT+QIOPEN=\"UDP\",\"202.65.114.202\",123\r\n"
#define AT_SEND_DATA					"AT+QISEND\r\n"
#define AT_SOCKET_CLOSE					"AT+QICLOSE\r\n"



#define AT_POWER_DOWN_MODE_0			"AT+QPOWD=0\r\n"
#define AT_POWER_DOWN_MODE_1			"AT+QPOWD=1\r\n"


#define TATA_DOCOMO			"TATA DOCOMO"
#define RELIANCE			"RELIANCE"
#define AIRTEL				"AIRTEL"
#define AIRCEL				"AIRCEL"
#define VODAFONE			"VODAFONE"
#define BSNL				"BSNL"
#define IDEA				"IDEA"
#define IDEA_NEW			"!DEA"
#define MTNL				"MTNL"
#define LOOP				"LOOP"
#define UNINOR				"UNINOR"
#define RELIANCE_JIO		"JIO"
#define MOBILE012			"012MOBILE"
#define ORANGE				"ORANGE"
#define GOLAN				"GOLAN"
#define HOME_CELL			"HOME CELLULAR"
#define CELLCOM				"CELLCOM"
#define PELEPHONE			"PELEPHONE"
#define HOTMOBILE			"HOT MOBILE"
#define RAMI_LEVI			"RAMI LEVI"
#define YOUPHONE			"YOUPHONE"


#define TATADOCOMO_APN			"tata.docomo.internet"
#define RELIANCE_APN			"rcomnet"
#define AIRTEL_APN				"airtelgprs.com"
#define AIRCEL_APN				"aircelgprs.pr"
#define VODAFONE_APN			"www"
#define BSNL_APN				"bsnlnet"
#define IDEA_APN				"internet"
#define MTNL_APN				"mtnl.net"
#define LOOP_APN				"www"
#define UNINOR_APN				"uninor"
#define RELIANCE_JIO_APN		"jionet"
#define MOBILE012_APN			"uinternet"
#define ORANGE_APN				"uinternet"
#define GOLAN_APN				"internet.golantelecom.net.il"
#define HOME_CELL_APN			"hcminternet"
#define CELLCOM_APN				"internetg"
#define PELEPHONE_APN			"internet.pelephone.net.il"
#define HOTMOBILE_APN			"net.hotm"
#define RAMI_LEVI_APN			"internet.rl"
#define YOUPHONE_APN			"data.youphone.co.il"


/************************************************************************/
/*       LCD DEBUG STATEMENTS                                           */
#define LCD_BAUD_SET				"GSM_Baud_Set"
#define LCD_ERROR_SET				"GSM-Error-Set"
#define LCD_ECHO_OFF				"GSM-Echo-Off"
#define LCD_IMEI_NO					"GSM-IMEI-NO"
#define LCD_SIM_PIN_QUERY			"SIM-PIN-Status"
#define LCD_SIM_CCID				"SIM-CCID"
#define LCD_GSM_STATUS				"SIM-Card-Status"
#define LCD_NETWORK_REG_STATUS		"Network-Reg-Status"
#define LCD_NETWORK_REG				"NETWORK-REG"
#define LCD_SERVICE_PROVIDER_NAME	"Service-Provider"
#define LCD_SIGNAL_QUALITY			"Signal-Quality"
#define LCD_GPRS_ATTACH_STATUS		"GPRS-Status"
#define LCD_GET_REGISTER_TCPSTACK	"CHECK-APN"
#define LCD_REGISTER_TCPSTACK		"SET-APN"
#define LCD_ACTIVATE_GPRS			"ACTIVATE_GPRS"
#define LCD_QISTAT					"Status"
#define LCD_COLP					"GSM_CALL_Resp"
#define LCD_QCOLP					"GSM_ALPHA_Resp"
#define LCD_QEXTUNSOL				"GSM_ENABLE_URC"
#define LCD_QINISTAT				"SIM_READY"
/************************************************************************/

void gsm_m66_off(void);

void gsm_m66_on(void);



void configure_GSM_uart(const usart_serial_options_t *options);

void gsm_rx_handler(uint8_t instance);

void GSM_M66_configure_baudrate(void);

void gsm_user_write(uint8_t *data,uint16_t usr_len);

int GSM_check_acknowledge( void );

uint8_t GSM_Send_data_to_Server(uint8_t *buf_snd,uint16_t send_len);

uint8_t GSM_M66_init(void);

void GSM_rx_Buffer_reset(void);

uint8_t GSM_Socket_Open(char *proto,uint8_t *ip,uint16_t port,uint8_t dns_state);

uint8_t GSM_Socket_Close(void);

void GSM_rx_off( void );

void GSM_rx_on( void );

uint16_t ntp_request(uint8_t *ntp_buf);

uint8_t get_ntp_time(void);

uint8_t get_network_time(void);

uint8_t gsm_call(char *number);

uint8_t GSM_Rss(void);

uint8_t get_default_apn(uint8_t *buf,uint8_t *sp);

uint8_t gsm_dns_set(uint8_t dns_flg);

uint8_t switch_intial(void);


#endif /* GSM_M66_H_ */