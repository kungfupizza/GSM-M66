/*
 * GSM_M66.c
 *
 * Created: 14-08-2017 13:13:31
 *  Author: Sarthak
 */ 
#include <GSM_M66.h>
#include <main.h>
uint8_t rec_data;

static usart_rs232_options_t hf_ptr115200 ={
	.baudrate = 115200,
	.charlength=USART_CHSIZE_8BIT_gc,
	.paritytype=0x00,
	.stopbits= false,
};


void configure_GSM_uart(const usart_serial_options_t *options)
{
    usart_serial_init(&USARTE0,options);
	GSM_rx_Buffer_reset();
	memset(searchForgsm_1,0,20);
	memcpy(searchForgsm_1,"\r\nALREADYCONNECT\r\n",18);
	memset(searchForgsm,0,20);
	memcpy(searchForgsm,"\r\nOK\r\n",6);
	memset(searchForgsm_2,0,10);
	memcpy(searchForgsm_2,"ERROR",5);
	memset(searchForgsm_3,0,13);
	memcpy(searchForgsm_3,"+PDP DEACT\r\n",12);
	memset(searchForgsm_4,0,20);
	memcpy(searchForgsm_4,"\r\nCONNECT FAIL\r\n",16);


}

void GSM_rx_on( void )
{
    usart_set_rx_interrupt_level(&USARTE0, USART_INT_LVL_HI);  // Enable RX interrupt
}

/*! \brief RX interrupt disable
 *
 *  \param    void
 *
 *  \retval   void
 */
void GSM_rx_off( void )
{
    usart_set_rx_interrupt_level(&USARTE0, USART_INT_LVL_OFF); // Disable RX interrupt
}


ISR(USARTE0_RXC_vect)
{
	rx_buffer[ rx_wr_i] = usart_getchar(&USARTE0);
	if((searchForgsm[rx_i] == rx_buffer[ rx_wr_i]) || (searchForgsm_1[rx_i] == rx_buffer[ rx_wr_i]))                   //Test response match
	{
		rx_i++;
		if((!searchForgsm[rx_i])||(!searchForgsm_1[rx_i]))                      //End of new_message string...received new message!
		{
			rx_i = 0;
			rx_ack = 1;
		}
	}
	if((searchForgsm_2[rx_i2] == rx_buffer[ rx_wr_i]))                   //Test response match
	{
		rx_i2++;
		if((!searchForgsm_2[rx_i2]))                      //End of new_message string...received new message!
		{
			rx_i2 = 0;
			rx_ack = 2;
		}
	}
	if((searchForgsm_4[rx_i4] == rx_buffer[ rx_wr_i]))                   //Test response match
	{
		rx_i4++;
		if((!searchForgsm_4[rx_i4]))                      //End of new_message string...received new message!
		{
			rx_i4 = 0;
			rx_ack = 2;
		}
	}
	if((searchForgsm_3[rx_i3] == rx_buffer[ rx_wr_i]))                   //Test response match
	{
		rx_i3++;
		if((!searchForgsm_3[rx_i3]))                      //End of new_message string...received new message!
		{
			rx_i3 = 0;
			gsm_reinit = 1;
		}
	}
	else
	{
		rx_i3 = 0;
	}
	rx_wr_i++;                                   //Always read something
}

void GSM_rx_Buffer_reset( void )
{
	rx_i = rx_wr_i = 0;           //Init variables
	rx_i4=0;
	rx_ack = 0;     //Zero overflow flag
	rx_i = rx_i2 = rx_i3 = rx_i4 =0;
	memset(rx_buffer,0,RXBUFFER_SIZE);
}



void GSM_M66_configure_baudrate(void)
{
	gsm_user_write("AT\r\n",4);
}

int GSM_check_acknowledge( void )
{
	if(rx_ack==1)                      //End of new_message string...received new message!
	{
		rx_ack = 0;
		return 1;
	}
	if(rx_ack==2)                      //End of new_message string...received new message!
	{
		return 1;
	}
	return 0;
}


void gsm_user_write(uint8_t *data,uint16_t usr_len)
{
	for(int i=0 ;i<usr_len;i++ )
	{
		usart_serial_putchar(&USARTE0, data[i] );
	}	
}

void gsm_m66_off(void)
{
	ioport_set_pin_level(GSM_PWRKEY,false);
	delay_ms(800);
	ioport_set_pin_level(GSM_PWRKEY,true);
	delay_s(4);
	delay_s(4);
	delay_s(4);
	delay_s(4);
}


void gsm_m66_on(void)
{
	delay_ms(103);
	ioport_set_pin_low(GSM_PWRKEY);
	delay_s(1);
	delay_ms(400);
}


uint8_t GSM_M66_init(void)
{
	uint8_t *ret,*ret1;
	uint8_t copy_len;
	int gsm_retry=0;
	uint8_t gsm_sp[30]= {0};
	configure_GSM_uart(&hf_ptr115200);
	GSM_rx_on();

	char lcd_gsm[30];
	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_BAUD_SET);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}

	for(int i=0;i<5;i++)
	{
		ether_syncretry =1;
		ethernet_break(true,comm_syn_break,0x1388);
		GSM_M66_configure_baudrate();
		while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
		GSM_rx_Buffer_reset();
		delay_ms(1);
	}
	ethernet_break(false,comm_syn_break,0x1388);
	if(ether_syncretry==0)
	{
		user_debug_1("Baud Rate Not Set\r\n");
		return 1;
	}


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_ERROR_SET);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =1;
	user_debug_1(AT_ERROR_SET);
	ethernet_break(true,comm_syn_break,0x7fff);
	gsm_user_write(AT_ERROR_SET,strlen(AT_ERROR_SET));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("%s",rx_buffer);
		user_debug_1("AT_ERROR NOT SET\r\n");
		return 1;
	}
	
	user_debug("ERROR: %s",rx_buffer);
	GSM_rx_Buffer_reset();


	ether_syncretry =1;
	user_debug_1("ATI\r\n");
	ethernet_break(true,comm_syn_break,0x7fff);
	gsm_user_write("ATI\r\n",strlen("ATI\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("%s",rx_buffer);
		user_debug_1("ATI NOT SET\r\n");
		return 1;
	}

	user_debug("ATI: %s",rx_buffer);
	GSM_rx_Buffer_reset();

	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_ECHO_OFF);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =1;
	user_debug_1(AT_ECHO_OFF);
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write(AT_ECHO_OFF,strlen(AT_ECHO_OFF));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("ECHO NOT OFF\r\n");
		return 1;
	}
	user_debug("ECHO_OFF: %s",rx_buffer);
	GSM_rx_Buffer_reset();


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_IMEI_NO);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =1;
	ethernet_break(true,comm_syn_break,0xffff);
	user_debug_1(AT_IMEI_NO);
	gsm_user_write(AT_IMEI_NO,strlen(AT_IMEI_NO));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not IMEI_NO\r\n");
		user_debug_1("IMEI NO: %s",rx_buffer);
		return 1;
	}
	user_debug("IMEI NO: %s",rx_buffer);
	ret = (uint8_t *)strstr(&rx_buffer[2],"\r");
	if(ret)
	{
		copy_len=ret-rx_buffer-2;
		memcpy(gsm_tmp.gsm_imei,&rx_buffer[2],copy_len);		
	}
//	user_debug_1("IMEI NO: %s",gsm_obj.gsm_imei);
	GSM_rx_Buffer_reset();

	gsm_retry=10;
CHECK_SIMPIN:

	if(gsm_retry<=0)
	{
		GSM_rx_Buffer_reset();
		goto GSM_POWER_DOWN;
	}
	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_SIM_PIN_QUERY);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	ethernet_break(true,comm_syn_break,0xffff);
	user_debug_1(AT_SIM_PIN_QUERY);
	gsm_user_write(AT_SIM_PIN_QUERY,strlen(AT_SIM_PIN_QUERY));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		gsm_retry -=1;
		delay_s(1);
		user_debug_1("Not SIM_PIN_QUERY\r\n");
		goto CHECK_SIMPIN;
	}
	user_debug("SIM_PIN: %s",rx_buffer);
	if(!strstr(rx_buffer,"+CPIN: READY"))
	{
		gsm_retry -=1;
		GSM_rx_Buffer_reset();
		delay_s(1);
		goto CHECK_SIMPIN;
	}
	GSM_rx_Buffer_reset();
//	user_debug_1("AT_SIM_PIN_QUERY\r\n");
	
	gsm_retry=10;
CHECK_INIT:
	if(gsm_retry<=0)
	{
		GSM_rx_Buffer_reset();
		goto GSM_POWER_DOWN;
	}
	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_QINISTAT);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	user_debug_1("AT+QINISTAT\r\n");
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write("AT+QINISTAT\r\n",strlen("AT+QINISTAT\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("AT_ERROR NOT SET\r\n");
			delay_s(1);
		gsm_retry -=1;
		goto CHECK_INIT;
	}
	

	user_debug("QINISTAT: %s",rx_buffer);
	if(strstr(rx_buffer,"+QINISTAT: 3")){}
	else
	{
		gsm_retry-=1;
		GSM_rx_Buffer_reset();
		delay_s(1);
		goto CHECK_INIT;
	}
	GSM_rx_Buffer_reset();


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_SIM_CCID);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =1;
	ethernet_break(true,comm_syn_break,0x7fff);
	user_debug_1(AT_SIM_CCID);
	gsm_user_write(AT_SIM_CCID,strlen(AT_SIM_CCID));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not SIM_CCID\r\n");
		return 1;
	}
//	user_debug_1("AT_SIM_CCID\r\n");
	user_debug("CCID: %s",rx_buffer);
	ret = (uint8_t *)strstr(&rx_buffer[2],"\r");
	if(ret)
	{
		copy_len=ret-rx_buffer-2;
		memcpy(gsm_tmp.sim_ccid,&rx_buffer[2],copy_len);		
	}
//	user_debug_1("CCID: %s",gsm_obj.sim_ccid);
	GSM_rx_Buffer_reset();

gsm_retry=60;
CHECK_CSQ:
		if(gsm_retry<=0)
		{
			GSM_rx_Buffer_reset();
			goto GSM_POWER_DOWN;
		}
		memset(lcd_gsm,0,30);
		strcpy(lcd_gsm,LCD_SIGNAL_QUALITY);
		for (int i=0;i<strlen(lcd_gsm);i++)
		{
			lcd_gsm[i]=lcd_gsm[i]-0x20;
		}
		LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
		if(gsm_init_done==0)
		{
			lcd_row_no = lcd_row_no + 10;
		}
		ether_syncretry =4;
		user_debug_1(AT_SIGNAL_QUALITY);
		ethernet_break(true,comm_syn_break,0xffff);
		gsm_user_write(AT_SIGNAL_QUALITY,strlen(AT_SIGNAL_QUALITY));
		while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
		ethernet_break(false,comm_syn_break,0x1388);
		if((ether_syncretry==0) || (rx_ack==2))
		{
			user_debug_1("Not SIGNAL_QUALITY\r\n");
			user_debug_1("%s\r\n",rx_buffer);
			delay_s(1);
			GSM_rx_Buffer_reset();
			gsm_retry -=1;
			goto CHECK_CSQ;
		}
		user_debug("%s",rx_buffer);
		if(strstr(rx_buffer,"+CSQ: 99"))
		{
			GSM_rx_Buffer_reset();
			delay_s(1);
			gsm_retry -=1;
			goto CHECK_CSQ;
		}
		GSM_rx_Buffer_reset();


gsm_retry=50;
CHECK_NETWORK:
		if(gsm_retry<=0)
		{
			GSM_rx_Buffer_reset();
			goto GSM_POWER_DOWN;
		}
		memset(lcd_gsm,0,30);
		strcpy(lcd_gsm,LCD_NETWORK_REG);
		for (int i=0;i<strlen(lcd_gsm);i++)
		{
			lcd_gsm[i]=lcd_gsm[i]-0x20;
		}
		LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
		if(gsm_init_done==0)
		{
			lcd_row_no = lcd_row_no + 10;
		}
		ether_syncretry =4;
		user_debug_1(AT_NETWORK_REG);
		ethernet_break(true,comm_syn_break,0xffff);
		gsm_user_write(AT_NETWORK_REG,strlen(AT_NETWORK_REG));
		while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
		ethernet_break(false,comm_syn_break,0x1388);
		if((ether_syncretry==0) || (rx_ack==2))
		{
			user_debug_1("Not NETWORK_REG\r\n");
			user_debug_1("%s\r\n",rx_buffer);
			delay_s(3);
			GSM_rx_Buffer_reset();
			gsm_retry -=1;
			goto CHECK_NETWORK;
		}
		user_debug_1("AT_NETWORK_REG\r\n");
		user_debug("%s\r\n",rx_buffer);
		if(strstr(rx_buffer,"+CREG: 0,1")){}
		else if (strstr(rx_buffer,"+CREG: 0,5")){}
		else
		{
			GSM_rx_Buffer_reset();
			delay_s(4);
			gsm_retry -=1;
			goto CHECK_NETWORK;
		}
		GSM_rx_Buffer_reset();


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_SERVICE_PROVIDER_NAME);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =2;
	user_debug_1(AT_SERVICE_PROVIDER_NAME);
	ethernet_break(true,comm_syn_break,0x7fff);
	gsm_user_write(AT_SERVICE_PROVIDER_NAME,strlen(AT_SERVICE_PROVIDER_NAME));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not SERVICE_PROVIDER_NAME\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		GSM_rx_Buffer_reset();
		return 1;
	}
	user_debug_1("AT_SERVICE_PROVIDER_NAME\r\n");
	user_debug("NETWORK %s",rx_buffer);
	int j=0;
	ret = (uint8_t *)strstr(rx_buffer,"+QSPN: ");
	if(ret)
	{
		ret += 7;
		ret1 = (uint8_t *)strchr(ret,13);
		if(ret1)
		{
			copy_len=ret1-ret;
			while (copy_len>j)
			{
				gsm_sp[j] = toupper(ret[j]);
				j++;
			}
			//			memcpy(gsm_sp,ret,copy_len);
		}
	}
	GSM_rx_Buffer_reset();


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_GPRS_ATTACH_STATUS);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	user_debug_1(AT_GPRS_ATTACH);
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write(AT_GPRS_ATTACH,strlen(AT_GPRS_ATTACH));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not GPRS_ATTACH\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		return 1;
	}
	user_debug("GPRS %s\r\n",rx_buffer);
	user_debug_1("AT_GPRS_ATTACH\r\n");
	GSM_rx_Buffer_reset();

gsm_retry=10;
	CHECK_GPRSNETWORK:

		if(gsm_retry<=0)
		{
			GSM_rx_Buffer_reset();
			goto GSM_POWER_DOWN;
		}
		memset(lcd_gsm,0,30);
		strcpy(lcd_gsm,LCD_NETWORK_REG);
		for (int i=0;i<strlen(lcd_gsm);i++)
		{
			lcd_gsm[i]=lcd_gsm[i]-0x20;
		}
		LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
		if(gsm_init_done==0)
		{
			lcd_row_no = lcd_row_no + 10;	
		}
		ether_syncretry =4;
		user_debug_1(AT_NETWORK_GPRS_REG);
		ethernet_break(true,comm_syn_break,0xffff);
		gsm_user_write(AT_NETWORK_GPRS_REG,strlen(AT_NETWORK_GPRS_REG));
		while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
		ethernet_break(false,comm_syn_break,0x1388);
		if((ether_syncretry==0) || (rx_ack==2))
		{
			user_debug_1("Not AT_NETWORK_GPRS_REG\r\n");
			user_debug_1("%s\r\n",rx_buffer);
			GSM_rx_Buffer_reset();
			gsm_retry-=1;
			delay_s(1);
			goto CHECK_GPRSNETWORK;
		}
		user_debug_1("AT_NETWORK_GPRS_REG\r\n");
		user_debug("GPRS_REG %s\r\n",rx_buffer);
		if(strstr(rx_buffer,"+CGREG: 0,1")){}
		else if (strstr(rx_buffer,"+CGREG: 0,5")){}
		else
		{
			GSM_rx_Buffer_reset();
			gsm_retry-=1;
			delay_s(1);
			goto CHECK_GPRSNETWORK;
		}
		GSM_rx_Buffer_reset();



	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_REGISTER_TCPSTACK);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	memset(ethernet_buffer,0,BUFFER_SIZE);
	copy_len = sprintf(ethernet_buffer,"AT+QICSGP=1,");
	if((strlen(gsm_obj.gsm_apn)) && (strncmp(gsm_obj.sim_ccid,gsm_tmp.sim_ccid,20)==0))
	{
		copy_len += sprintf(&ethernet_buffer[copy_len],"\"%s\"",gsm_obj.gsm_apn);
	}
	else
	{
		copy_len += get_default_apn(&ethernet_buffer[copy_len],gsm_sp);
	}
	if(strlen(gsm_obj.gsm_username))
	{
		copy_len += sprintf(&ethernet_buffer[copy_len],",\"%s\"",gsm_obj.gsm_username);
	}
	if(strlen(gsm_obj.gsm_pass))
	{
		copy_len += sprintf(&ethernet_buffer[copy_len],",\"%s\"",gsm_obj.gsm_pass);
	}
	copy_len += sprintf(&ethernet_buffer[copy_len],"\r\n");
	user_debug("%s",ethernet_buffer);
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write(ethernet_buffer,copy_len);
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("NOT REGISTER_APN\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		GSM_rx_Buffer_reset();
		return 1;
	}
	user_debug("APN %s\r\n",rx_buffer);
	user_debug_1("AT_REGISTER_APN\r\n");
	GSM_rx_Buffer_reset();


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_QISTAT);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =2;
	user_debug_1("AT+QISTAT\r\n");
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write("AT+QISTAT\r\n",strlen("AT+QISTAT\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("QISTAT NOT\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		return 1;
	}
	delay_s(1);
	user_debug("QISTAT1 %s\r\n",rx_buffer);
	if(strstr(rx_buffer,"IP INITIAL"))
	{
		goto STEP1;
	}
	else
	{
		if(switch_intial())
		{
			goto GSM_POWER_DOWN;
		}
	}
	GSM_rx_Buffer_reset();

STEP1:
	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_GET_REGISTER_TCPSTACK);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	user_debug_1(AT_GET_REGISTER_TCPSTACK);
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write(AT_GET_REGISTER_TCPSTACK,strlen(AT_GET_REGISTER_TCPSTACK));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("NOT GET_REGISTER_TCPSTACK\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		GSM_rx_Buffer_reset();
		return 1;
	}
	user_debug("GET TCP %s\r\n",rx_buffer);
	user_debug_1("AT_GET_REGISTER_TCPSTACK\r\n");
	GSM_rx_Buffer_reset();

STEP2:

	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_ACTIVATE_GPRS);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =20;
	user_debug_1(AT_ACTIVATE_GPRS);
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write(AT_ACTIVATE_GPRS,strlen(AT_ACTIVATE_GPRS));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not ACTIVATE_GPRS\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		GSM_rx_Buffer_reset();
		goto GSM_POWER_DOWN;
	}
	user_debug("ACT_GPRS %s\r\n",rx_buffer);
	user_debug_1("AT_ACTIVATE_GPRS\r\n");
	GSM_rx_Buffer_reset();


STEP3:

	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_COLP);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	user_debug_1("AT+COLP=1\r\n");
	ethernet_break(true,comm_syn_break,0x7fff);
	gsm_user_write("AT+COLP=1\r\n",strlen("AT+COLP=1\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("NOT SET Calling Response\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		return 1;
	}
	user_debug("COLP %s\r\n",rx_buffer);
	user_debug_1("SET Calling Response\r\n");
	GSM_rx_Buffer_reset();


	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_QCOLP);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;
	}
	ether_syncretry =3;
	user_debug_1("AT+QCOLP=1\r\n");
	ethernet_break(true,comm_syn_break,0x7fff);
	gsm_user_write("AT+QCOLP=1\r\n",strlen("AT+QCOLP=1\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("NOT SET Alpha Response\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		return 1;
	}
	user_debug("QCOLP %s\r\n",rx_buffer);
	user_debug_1("SET Alpha Response\r\n");
	GSM_rx_Buffer_reset();
	
	memset(lcd_gsm,0,30);
	strcpy(lcd_gsm,LCD_QEXTUNSOL);
	for (int i=0;i<strlen(lcd_gsm);i++)
	{
		lcd_gsm[i]=lcd_gsm[i]-0x20;
	}
	LCD_String(lcd_row_no,0,lcd_gsm,GREEN);
	if(gsm_init_done==0)
	{
		lcd_row_no = lcd_row_no + 10;	
	}
	ether_syncretry =2;
	ethernet_break(true,comm_syn_break,0x7fff);
	gsm_user_write("AT+QEXTUNSOL=\"UR\",1\r\n",strlen("AT+QEXTUNSOL=\"UR\",1\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("NOT SET URC Response\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		return 1;
	}
	user_debug_1("SET URC Response\r\n");
	GSM_rx_Buffer_reset();


	gsm_init_done=0;
	return 0;

	GSM_POWER_DOWN:	
		ioport_set_pin_level(GSM_PWRKEY,true);
		delay_s(1);
/*		ether_syncretry =2;
		user_debug_1("AT+QPOWD=1\r\n");
		GSM_rx_Buffer_reset();
		ethernet_break(true,comm_syn_break,0xffff);
		gsm_user_write("AT+QPOWD=1\r\n",strlen("AT+QPOWD=1\r\n"));
		while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
		ethernet_break(false,comm_syn_break,0x1388);
		delay_s(1);
		if((ether_syncretry==0) || (rx_ack==2))
		{
			user_debug_1("Not AT+QPOWD\r\n");
			user_debug_1("%s\r\n",rx_buffer);
			return 2;
		}
		user_debug_1("%s\r\n",rx_buffer);
		user_debug_1("QPOWD\r\n");
		GSM_rx_Buffer_reset();
*/		return 2;
}

uint8_t switch_intial(void)
{
	ether_syncretry =1;
	user_debug_1("AT+QIDEACT\r\n");
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write("AT+QIDEACT\r\n",strlen("AT+QIDEACT\r\n"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	delay_s(1);
	if(ether_syncretry==0)
	{
		user_debug_1("Not DEACT\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		return 1;
	}
	user_debug("DEACT %s\r\n",rx_buffer);
	user_debug_1("IP INITIAl\r\n");
	GSM_rx_Buffer_reset();
	
}

uint8_t GSM_Send_data_to_Server(uint8_t *buf_snd,uint16_t send_len)
{
	uint8_t cmd_buf[20];
	GSM_rx_Buffer_reset();
	memset(searchForgsm,0,20);
	memcpy(searchForgsm,">",1);
	memset(cmd_buf,0,20);
	comm_timer =5;
	ethernet_comm_break(true,comm__break,0xffff);
	sprintf(cmd_buf,"AT+QISEND=%d\r\n",send_len);
	gsm_user_write(cmd_buf,strlen(cmd_buf));
	while ((!GSM_check_acknowledge()) && (comm_timer>0));
	ethernet_comm_break(false,comm__break,0xffff);
	memcpy(searchForgsm,"\r\nOK\r\n",6);
	if((comm_timer==0) || (rx_ack==2))
	{
		GSM_rx_Buffer_reset();
		user_debug_1("SEND Prompt Not came\r\n");
		return 1;
	}
	
	comm_timer =3;
	ethernet_comm_break(true,comm__break,0xffff);
	gsm_user_write(buf_snd,send_len);
	while ((!GSM_check_acknowledge()) && (comm_timer>0));
	ethernet_comm_break(false,comm__break,0xffff);
	if((comm_timer==0) || (rx_ack==2))
	{
		GSM_rx_Buffer_reset();
		user_debug_1("AT_SEND_DATA Timer overflow\r\n");
		return 1;
	}

	user_debug_1("Data send\r\n");
	GSM_rx_Buffer_reset();
	return 0;
	
}


//uint8_t GSM_Socket_Open(uint8_t *sck_ip,uint16_t sck_len){
uint8_t GSM_Socket_Open(char *proto,uint8_t *ip,uint16_t port,uint8_t dns_state){

	uint16_t sck_len=0;
	memset(searchForgsm_1,0,20);
	memcpy(searchForgsm_1,"\r\nALREADYCONNECT\r\n",18);
	GSM_rx_Buffer_reset();
	memset(searchForgsm,0,20);
	memcpy(searchForgsm,"\r\nCONNECTOK\r\n",13);
	user_debug_1("GSM SEND DATA Task\r\n");
	comm_timer =10;
	memset(ethernet_buffer,0,BUFFER_SIZE);
	sck_len=sprintf(ethernet_buffer,"AT+QIOPEN=\"%s\",",proto);
	if(dns_state)
	{
		sck_len=sck_len + sprintf(&ethernet_buffer[sck_len],"\"%s\",",ip);		
	}
	else
	{
		sck_len=sck_len + sprintf(&ethernet_buffer[sck_len],"\"%d.%d.%d.%d\",",ip[0],ip[1],ip[2],ip[3]);		
	}
	sck_len=sck_len + sprintf(&ethernet_buffer[sck_len],"%d\r\n",port);
	user_debug_1("%s",ethernet_buffer);
	ethernet_comm_break(true,comm__break,0xffff);
	gsm_user_write(ethernet_buffer,sck_len);
	while ((!GSM_check_acknowledge()) && (comm_timer>0));
	ethernet_comm_break(false,comm__break,0xffff);
	memset(searchForgsm_1,0,20);
	memcpy(searchForgsm_1,"\r\nALREADYCONNECT\r\n",18);
	memset(searchForgsm,0,20);
	memcpy(searchForgsm,"\r\nOK\r\n",6);
	if((comm_timer==0) || (rx_ack==2))
	{
		if(rx_ack==2)
		{
			GSM_Socket_Close();
		}
		GSM_rx_Buffer_reset();

		comm_timer =2;
		user_debug_1("AT+QISTAT\r\n");
		ethernet_comm_break(true,comm__break,0xffff);
		gsm_user_write("AT+QISTAT\r\n",strlen("AT+QISTAT\r\n"));
		while ((!GSM_check_acknowledge()) && (comm_timer>0));
		ethernet_comm_break(false,comm__break,0xffff);
		if((comm_timer==0) || (rx_ack==2))
		{
			user_debug_1("QISTAT NOT\r\n");
			user_debug_1("%s\r\n",rx_buffer);
		}
		user_debug("%s",rx_buffer);
		user_debug_1("AT_SOCKET_OPEN Timer overflow\r\n");
		return 1;

	}

	GSM_rx_Buffer_reset();
	user_debug_1("Socket Connect\r\n");
	delay_ms(10);
	return 0;		
}

uint8_t GSM_Socket_Close(void)
{
	comm_timer =1;
	ethernet_comm_break(true,comm__break,0xffff);
	gsm_user_write(AT_SOCKET_CLOSE,strlen(AT_SOCKET_CLOSE));
	while ((!GSM_check_acknowledge()) && (comm_timer>0));
	ethernet_comm_break(false,comm__break,0xffff);
	if((comm_timer==0) || (rx_ack==2))
	{
		GSM_rx_Buffer_reset();

		user_debug_1("AT_SOCKET_CLOSE Timer overflow\r\n");
		return 1;

	}

	GSM_rx_Buffer_reset();
	user_debug_1("Socket Closed\r\n");
	delay_s(2);
	return 0;

	
	
	
}

uint8_t get_network_time(void)
{
	uint8_t *ret = NULL;
	uint8_t *ret1 = NULL;
	uint8_t copy_len;

	ether_syncretry =20;
	user_debug_1("AT+QLTS=1\r\n");
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write("AT+QLTS=1\r",strlen("AT+QLTS=1\r"));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug("Not QLTS: %s\r\n",rx_buffer);
		return 1;
	}
	//	user_debug_1("AT_SIM_CCID\r\n");
	user_debug("QLTS: %s\r\n",rx_buffer);
	ret = (uint8_t *)strstr(rx_buffer,"+QLTS: ");
	if(ret)
	{
		ret += 8;
		ret1 = (uint8_t *)strstr(ret,"/");
		if(ret1)
		{
			copy_len=ret1-ret;
			date_tmp.year = atodec(ret,copy_len)+2000;
			ret = ret1 +1;
			ret1 = (uint8_t *)strstr(ret,"/");
			if(ret1)
			{
				copy_len=ret1-ret;
				date_tmp.month = atodec(ret,copy_len)-1;
				ret = ret1 +1;
				ret1 = (uint8_t *)strstr(ret,",");
				if(ret1)
				{
					copy_len=ret1-ret;
					date_tmp.date = atodec(ret,copy_len)-1;
					ret = ret1 +1;
					ret1 = (uint8_t *)strstr(ret,":");
					if(ret1)
					{
						copy_len=ret1-ret;
						date_tmp.hour = atodec(ret,copy_len);
						ret = ret1 +1;
						ret1 = (uint8_t *)strstr(ret,":");
						if(ret1)
						{
							copy_len=ret1-ret;
							date_tmp.minute = atodec(ret,copy_len);
							ret = ret1 +1;
							ret1 = (uint8_t *)strstr(ret,"+");
							if(ret1)
							{
								copy_len=ret1-ret;
								date_tmp.second = atodec(ret,copy_len);
								ds1343_tick = calendar_date_to_timestamp(&date_tmp);
							}
						}
					}
				}
			}
		}
	}
	GSM_rx_Buffer_reset();
	return 0;
}



uint8_t get_ntp_time(void)
{
	uint8_t num=1;
	uint8_t ntp_len;
//	uint8_t ntp_ip[2][4] = {{125,62,193,121},{120,88,46,10}};
	uint8_t ntp_ip[4][20] = {"1.asia.pool.ntp.org","3.in.pool.ntp.org","2.in.pool.ntp.org","1.in.pool.ntp.org"};
	int8_t ntp_retry_count=4;

	gsm_dns_set(1);
	delay_s(2);
	ntp_retry:
	GSM_rx_Buffer_reset();
	ntp_len=0;
	if(!GSM_Socket_Open("UDP",ntp_ip[ntp_retry_count-1],123,1))
	{
		memset(ethernet_buffer,0,BUFFER_SIZE);
		ntp_len = ntp_request(ethernet_buffer);
		GSM_Send_data_to_Server(ethernet_buffer,ntp_len);
		delay_s(5);
		if(rx_wr_i<40)
		{
			GSM_Socket_Close();
			ntp_retry_count=ntp_retry_count-1;
			if(ntp_retry_count>0)
			{
				goto ntp_retry;
			}
			return 1;
		}
		user_debug_1("Rx buffer fill %d\r\n",rx_wr_i);
		uint8_t ntp_dp= rx_wr_i-8;
		user_debug_1("NTP data %x %x",rx_buffer[ntp_dp],rx_buffer[ntp_dp+1]);
		user_debug_1(" %x %x\r\n",rx_buffer[ntp_dp+2],rx_buffer[ntp_dp+3]);
		ds1343_tick=((uint32_t)rx_buffer[ntp_dp]<<24)|((uint32_t)rx_buffer[ntp_dp+1]<<16)|((uint32_t)rx_buffer[ntp_dp+2]<<8)|((uint32_t)rx_buffer[ntp_dp+3]);
		ds1343_tick=ds1343_tick-0x83aa7e80;
		GSM_Socket_Close();
		return 0;
	}
	else
	{
		ntp_retry_count=ntp_retry_count-1;
		if(ntp_retry_count>0)
		{
			delay_s(2);
			goto ntp_retry;
		}
		return 1;
	}
}

uint8_t gsm_dns_set(uint8_t dns_flg)
{
	memset(searchForgsm_1,0,20);
	memcpy(searchForgsm_1,"\r\nALREADYCONNECT\r\n",18);
	memset(searchForgsm,0,20);
	memcpy(searchForgsm,"\r\nOK\r\n",6);
	GSM_rx_Buffer_reset();
	ether_syncretry =1;
	ethernet_break(true,comm_syn_break,0x7fff);
	memset(ethernet_buffer,0,BUFFER_SIZE);
	sprintf(ethernet_buffer,"AT+QIDNSIP=%d\r\n",dns_flg);
	gsm_user_write(ethernet_buffer,strlen(ethernet_buffer));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not DOMAIN NAME\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		GSM_rx_Buffer_reset();
		return 1;
	}
	user_debug_1("SET DOMAIN NAME\r\n");
	user_debug_1("%s",rx_buffer);
	GSM_rx_Buffer_reset();
	return 0;
}

uint16_t ntp_request(uint8_t *ntp_buf)
{
	uint16_t udp_len=0;
	
	while(udp_len<48){
		ntp_buf[udp_len]=0;
		udp_len++;
	}
	ntp_buf[0]=0xE3;
	ntp_buf[1]=0;
	ntp_buf[2]=4;
	ntp_buf[3]=0xFA;
	ntp_buf[4]=0;
	ntp_buf[5]=1;
	ntp_buf[6]=0;
	ntp_buf[7]=0;
	ntp_buf[8]=0;
	ntp_buf[9]=1;
	
	return udp_len;
}


uint8_t gsm_call(char *number)
{
	uint8_t call_len=0;
	comm_timer =23;
	GSM_rx_Buffer_reset();
	memset(ethernet_buffer,0,BUFFER_SIZE);
	call_len=sprintf(ethernet_buffer,"ATD+91");
	memcpy(&ethernet_buffer[call_len],number,10);
	call_len =call_len+10;
	call_len=call_len + sprintf(&ethernet_buffer[call_len],";\r\n");
	user_debug_1("Calling Number %s",ethernet_buffer);
	ethernet_comm_break(true,comm__break,0xffff);
	gsm_user_write(ethernet_buffer,call_len);
	while ((!GSM_check_acknowledge()) && (comm_timer>0));
	ethernet_comm_break(false,comm__break,0xffff);
	if((comm_timer==0))
	{
		user_debug_1("Call Not Success\r\n");
		user_debug_1("%s\r\n",rx_buffer);
		comm_timer =2;
		GSM_rx_Buffer_reset();
		ethernet_comm_break(true,comm__break,0xffff);
		gsm_user_write("ATH0\r\n",strlen("ATH0\r\n"));
		while ((!GSM_check_acknowledge()) && (comm_timer>0));
		ethernet_comm_break(false,comm__break,0xffff);
		return 1;
	}

	if(strstr(rx_buffer,"NO"))
	{
		comm_timer =2;
		GSM_rx_Buffer_reset();
		ethernet_comm_break(true,comm__break,0xffff);
		gsm_user_write("ATH0\r\n",strlen("ATH0\r\n"));
		while ((!GSM_check_acknowledge()) && (comm_timer>0));
		ethernet_comm_break(false,comm__break,0xffff);
		return 1;
	}
	if(strstr(rx_buffer,"BUSY"))
	{
		comm_timer =2;
		GSM_rx_Buffer_reset();
		ethernet_comm_break(true,comm__break,0xffff);
		gsm_user_write("ATH0\r\n",strlen("ATH0\r\n"));
		while ((!GSM_check_acknowledge()) && (comm_timer>0));
		ethernet_comm_break(false,comm__break,0xffff);
		return 1;
	}
	user_debug_1("%s\r\n",rx_buffer);
	user_debug_1("Call Successful");
	comm_timer =2;
	GSM_rx_Buffer_reset();
	ethernet_comm_break(true,comm__break,0xffff);
	gsm_user_write("ATH0\r\n",strlen("ATH0\r\n"));
	while ((!GSM_check_acknowledge()) && (comm_timer>0));
	ethernet_comm_break(false,comm__break,0xffff);
	return 0;

	
	
}


uint8_t GSM_Rss(void)
{
	uint8_t *ret;
	uint8_t strength=0;
	ether_syncretry =2;
	GSM_rx_Buffer_reset();
	memset(searchForgsm,0,20);
	memcpy(searchForgsm,"\r\nOK\r\n",6);
	ethernet_break(true,comm_syn_break,0xffff);
	gsm_user_write(AT_SIGNAL_QUALITY,strlen(AT_SIGNAL_QUALITY));
	while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
	ethernet_break(false,comm_syn_break,0x1388);
	if((ether_syncretry==0) || (rx_ack==2))
	{
		user_debug_1("Not SIGNAL_QUALITY %d \r\n",rx_ack);
		user_debug_1("%s\r\n",rx_buffer);
		GSM_rx_Buffer_reset();
	}
	user_debug_1("%s\r\n",rx_buffer);
	if(strstr(rx_buffer,"+CSQ:"))
	{
		ret = (uint8_t *)strstr(rx_buffer,"+CSQ:")+5;
		while(*ret!=',')
		{
			if((*ret>0x2f) && (*ret<0x3a))
			{
				strength = (*ret&0x0f)+(strength*10);
			}
			ret++;
		}
		user_debug_1("AT_SIGNAL_QUALITY %d\r\n",strength);
		return strength;		
	}
	else if(strstr(rx_buffer,"AT+CSQ"))
	{
		ether_syncretry =1;
		ethernet_break(true,comm_syn_break,0x7fff);
		gsm_user_write(AT_ECHO_OFF,strlen(AT_ECHO_OFF));
		while ((!GSM_check_acknowledge()) && (ether_syncretry>0));
		ethernet_break(false,comm_syn_break,0x1388);
		if((ether_syncretry==0) || (rx_ack==2))
		{
			user_debug_1("ECHO NOT OFF\r\n");
			return 1;
		}
		user_debug_1("%s",rx_buffer);
		GSM_rx_Buffer_reset();
	}
	GSM_rx_Buffer_reset();
	return 1;
}

uint8_t get_default_apn(uint8_t *buf,uint8_t *sp)
{
	user_debug_1("Service Provider %s\r\n",sp);
	if(strstr(sp,TATA_DOCOMO))
	{
		strcpy(gsm_tmp.gsm_apn,TATADOCOMO_APN);
		return sprintf(buf,"\"%s\"",TATADOCOMO_APN);		
	}
	else if(strstr(sp,RELIANCE))
	{
		strcpy(gsm_tmp.gsm_apn,RELIANCE_APN);
		return sprintf(buf,"\"%s\"",RELIANCE_APN);
	}
	else if(strstr(sp,AIRTEL))
	{
		strcpy(gsm_tmp.gsm_apn,AIRTEL_APN);
		return sprintf(buf,"\"%s\"",AIRTEL_APN);
	}
	else if(strstr(sp,VODAFONE))
	{
		strcpy(gsm_tmp.gsm_apn,VODAFONE_APN);
		return sprintf(buf,"\"%s\"",VODAFONE_APN);
	}
	else if(strstr(sp,AIRCEL))
	{
		strcpy(gsm_tmp.gsm_apn,AIRCEL_APN);
		return sprintf(buf,"\"%s\"",AIRCEL_APN);
	}
	else if(strstr(sp,BSNL))
	{
		strcpy(gsm_tmp.gsm_apn,BSNL_APN);
		return sprintf(buf,"\"%s\"",BSNL_APN);
	}
	else if(strstr(sp,LOOP))
	{
		strcpy(gsm_tmp.gsm_apn,LOOP_APN);
		return sprintf(buf,"\"%s\"",LOOP_APN);
	}
	else if(strstr(sp,UNINOR))
	{
		strcpy(gsm_tmp.gsm_apn,UNINOR_APN);
		return sprintf(buf,"\"%s\"",UNINOR_APN);
	}
	else if(strstr(sp,IDEA))
	{
		strcpy(gsm_tmp.gsm_apn,IDEA_APN);
		return sprintf(buf,"\"%s\"",IDEA_APN);
	}
	else if(strstr(sp,MTNL))
	{
		strcpy(gsm_tmp.gsm_apn,MTNL_APN);
		return sprintf(buf,"\"%s\"",MTNL_APN);
	}
	else if(strstr(sp,RELIANCE_JIO))
	{
		strcpy(gsm_tmp.gsm_apn,RELIANCE_JIO_APN);
		return sprintf(buf,"\"%s\"",RELIANCE_JIO_APN);
	}
	else if(strstr(sp,MOBILE012))
	{
		strcpy(gsm_tmp.gsm_apn,MOBILE012_APN);
		return sprintf(buf,"\"%s\"",MOBILE012_APN);
	}
	else if(strstr(sp,ORANGE))
	{
		strcpy(gsm_tmp.gsm_apn,ORANGE_APN);
		strcpy(gsm_obj.gsm_username,"orange");
		strcpy(gsm_obj.gsm_pass,"orange");
		return sprintf(buf,"\"%s\"",ORANGE_APN);
	}
	else if(strstr(sp,GOLAN))
	{
		strcpy(gsm_tmp.gsm_apn,GOLAN_APN);
		return sprintf(buf,"\"%s\"",GOLAN_APN);
	}
	else if(strstr(sp,HOME_CELL))
	{
		strcpy(gsm_tmp.gsm_apn,HOME_CELL_APN);
		return sprintf(buf,"\"%s\"",HOME_CELL_APN);
	}
	else if(strstr(sp,CELLCOM))
	{
		strcpy(gsm_tmp.gsm_apn,CELLCOM_APN);
		return sprintf(buf,"\"%s\"",CELLCOM_APN);
	}
	else if(strstr(sp,PELEPHONE))
	{
		strcpy(gsm_tmp.gsm_apn,PELEPHONE_APN);
		strcpy(gsm_obj.gsm_username,"pcl@3g");
		strcpy(gsm_obj.gsm_pass,"pcl");
		return sprintf(buf,"\"%s\"",PELEPHONE_APN);
	}
	else if(strstr(sp,HOTMOBILE))
	{
		strcpy(gsm_tmp.gsm_apn,HOTMOBILE_APN);
		return sprintf(buf,"\"%s\"",HOTMOBILE_APN);
	}
	else if(strstr(sp,RAMI_LEVI))
	{
		strcpy(gsm_tmp.gsm_apn,RAMI_LEVI_APN);
		strcpy(gsm_obj.gsm_username,"rl@3g");
		strcpy(gsm_obj.gsm_pass,"rl");
		return sprintf(buf,"\"%s\"",RAMI_LEVI_APN);
	}
	else if(strstr(sp,YOUPHONE))
	{
		strcpy(gsm_tmp.gsm_apn,YOUPHONE_APN);
		return sprintf(buf,"\"%s\"",YOUPHONE_APN);
	}
	
}
