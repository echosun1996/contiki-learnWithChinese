#include "contiki.h"
#include "dev/adc.h"
#include <stdio.h>/* For printf() */

/*---------------------------------------------------------------------------*/
PROCESS(soil_process, "soil-sensor");
AUTOSTART_PROCESSES(&soil_process);
/*---------------------------------------------------------------------------*/
unsigned int GotVoltage(unsigned int read){
  return 0.0016*read - 0.277;
}
PROCESS_THREAD(soil_process, ev, data)
{
  PROCESS_BEGIN();

  static unsigned int value;
  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND);

  printf("ADC soil sens\n");
    
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    value=GotVoltage(adc_get(SOC_ADC_ADCCON_CH_AIN6, SOC_ADC_ADCCON_REF_AVDD5, SOC_ADC_ADCCON_DIV_512)>>4);                      
    
    // Print the result on UART
    printf("Voltage: %d V. \n", value);
        
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
