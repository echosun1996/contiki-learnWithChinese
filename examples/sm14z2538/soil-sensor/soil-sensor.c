#include "contiki.h"
#include "dev/adc.h"
#include <stdio.h> /* For printf() */

/*---------------------------------------------------------------------------*/
PROCESS(soil_process, "soil-sensor");
AUTOSTART_PROCESSES(&soil_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(soil_process, ev, data)
{
  PROCESS_BEGIN();

  unsigned int value;
  static struct etimer et;
  etimer_set(&et, CLOCK_SECOND);

  printf("ADC soil sens\n");
    
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    value=adc_get(SOC_ADC_ADCCON_CH_AIN6, SOC_ADC_ADCCON_REF_AVDD5, SOC_ADC_ADCCON_DIV_512)>>4;                      
    // Print the result on UART
    printf("ADC readout: %d\n", value);
        
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
