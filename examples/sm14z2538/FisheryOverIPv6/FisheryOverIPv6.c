#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "dev/leds.h"
#include "dev/ds18b20-arch.h"//18b20头文件
#include "dev/adc.h"//PH头文件

#include "aes.h"
#include "ip64-addr.h"
#include "apps/mdns/mdns.h"
#include "apps/simple-rpl/simple-rpl.h"
#include "tcp-socket.h"
#include "udp-socket.h"
//#include "websocket.h"
#include "http-socket/http-socket.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dev/DHT11-arch.h"
#include "dev/oled.h"
#define PORT 13140

static struct udp_socket s;
static struct uip_ds6_notification n;

#define SEND_INTERVAL		(2 * CLOCK_SECOND)//每2秒发送
static struct etimer periodic_timer;// send_timer;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_example_process, "Link local unicast example process");
AUTOSTART_PROCESSES(&unicast_example_process);


/*--------------------路由发现后的回调函数--------------------------------------*/
static void
route_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr,
               int numroutes)
{
  if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    leds_off(LEDS_ALL);
    printf("Got a RPL route\n");
  }
}

/*---------------------接收UDP Server回传消息----------------------------------*/
static void
receiver_callback(struct udp_socket *c,
         void *ptr,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received on port %d from port %d with length %d, '%s'\n",
         receiver_port, sender_port, datalen, data);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_example_process, ev, data)
{
  uip_ip6addr_t ip6addr;
  uip_ip4addr_t ip4addr;

  PROCESS_BEGIN();   //函数开始

  leds_on(LEDS_ALL);//点亮全部LED

  uip_ds6_notification_add(&n, route_callback);

  udp_socket_register(&s, NULL, receiver_callback);

  /* Bind UDP socket to local port */
  udp_socket_bind(&s, PORT);

  /* Connect UDP socket to remote port */
  udp_socket_connect(&s, NULL, PORT);

  //etimer_set(&et, CLOCK_SECOND*2); 

  while(DS18B20_Init());//18b20初始化

  while(1) {
    unsigned int temperature;
    temperature=DS18B20_Get_Temp();	
    printf("temperature:%d\n",temperature);;
    /* Set up two timers, one for keeping track of the send interval,
       which is periodic, and one for setting up a randomized send time
       within that interval. */
    etimer_set(&periodic_timer, SEND_INTERVAL);
    //etimer_set(&send_timer, (random_rand() % SEND_INTERVAL));

    PROCESS_WAIT_UNTIL(etimer_expired(&periodic_timer));
    unsigned char  temp[7]="Tem:", humidity[7]="Hun:";
    static unsigned char ch[8];//18b20
    unsigned char ds18b20[12]="DS :";//18b20 sum=4+7+1=12
    unsigned int i;
    unsigned int ph;//PH
    unsigned int ph_save;//PH
    unsigned char PH_string[9]="PH :";

    ph=adc_get(SOC_ADC_ADCCON_CH_AIN6, SOC_ADC_ADCCON_REF_AVDD5, SOC_ADC_ADCCON_DIV_512)>>4; //获取PH
    ph_save=ph;

    if(ph<=9999){
        if(ph>=1000){
            PH_string[4]=ph/1000+'0';
	    ph%=1000;
	}
	else 
	    PH_string[4]='0';
	if(ph>=100){
	    PH_string[5]=ph/100+'0';
	    ph%=100;
	}
	else 
	    PH_string[5]='0';
	if(ph>=10){
	    PH_string[6]=ph/10+'0';
	    ph%=10;
	}
	else 
	    PH_string[6]='0';
	    PH_string[7]=ph+'0';
	}
    else{
        PH_string[4]='-';
        PH_string[5]='-';
	PH_string[6]='-';
	PH_string[7]='-';
    }
		
    PH_string[8]='\0';

    uip_ipaddr(&ip4addr, 192,168,3,38);//uip_ipaddr(&ip4addr, 192,168,18,86);

    ip64_addr_4to6(&ip4addr, &ip6addr);

    ds18b20_read(ch);//18b20
    for(i=0;i<7;i++)
	ds18b20[4+i]=ch[i];
    ds18b20[11]='\0';

    DHT11_Inint();
        //temp="Tem:";
        temp[4] = Tem_dec+0x30;
	temp[5] = Tem_uni+0x30;
	temp[6] = '\0';
        //humidity="Hum:";
	humidity[4] = Hum_dec+0x30;
	humidity[5] = Hum_uni+0x30;
	humidity[6] = '\0';


    if((Tem_data_H!=0)&&(RH_data_H!=0))
    {
        printf("Tem: %d \r\n",Tem_data_H);
        printf("Hum: %d \r\n",RH_data_H);
        printf("DS : %s \r\n",(char *)ch);//18b20
      	printf("PH : %d\n", ph_save);//PH
      	OLED_Display_On1();
        OLED_P8x16Str(20,0,"DaLian");
        OLED_P8x16Str(20,2,"Sanmu-Link");
        OLED_P8x16Str(20,4,temp);
        OLED_P8x16Str(20,6,humidity);
    }

    //printf("Sending unicast %d\n",i);
    //i++;
    udp_socket_sendto(&s,
                      temp, 7,
                      &ip6addr, PORT);
    udp_socket_sendto(&s,
                      humidity, 7,
                      &ip6addr, PORT);

    udp_socket_sendto(&s,
                      ds18b20, 12,
                      &ip6addr, PORT);//18b20 output

    udp_socket_sendto(&s,
                      PH_string, 9,
                      &ip6addr, PORT);//ph output

    //PROCESS_WAIT_UNTIL(etimer_expired(&periodic_timer));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
