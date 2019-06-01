#include "contiki.h"
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/ph.h"

#include <stdint.h>

#define ADC_ALS_PWR_PORT_BASE    GPIO_PORT_TO_BASE(ADC_ALS_PWR_PORT)
#define ADC_ALS_PWR_PIN_MASK     GPIO_PIN_MASK(ADC_ALS_PWR_PIN)
#define ADC_ALS_OUT_PIN_MASK     GPIO_PIN_MASK(ADC_ALS_OUT_PIN)
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  uint8_t channel = SOC_ADC_ADCCON_CH_AIN0 + ADC_ALS_OUT_PIN;
  int16_t res;

  GPIO_SET_PIN(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
  clock_delay_usec(2000);

  res = adc_get(channel, SOC_ADC_ADCCON_REF_INT, SOC_ADC_ADCCON_DIV_512);

  GPIO_CLR_PIN(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);

  return res;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    GPIO_SOFTWARE_CONTROL(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    GPIO_SET_OUTPUT(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    GPIO_CLR_PIN(ADC_ALS_PWR_PORT_BASE, ADC_ALS_PWR_PIN_MASK);
    ioc_set_over(ADC_ALS_PWR_PORT, ADC_ALS_PWR_PIN, IOC_OVERRIDE_DIS);

    GPIO_SOFTWARE_CONTROL(GPIO_A_BASE, ADC_ALS_OUT_PIN_MASK);
    GPIO_SET_INPUT(GPIO_A_BASE, ADC_ALS_OUT_PIN_MASK);
    ioc_set_over(GPIO_A_NUM, ADC_ALS_OUT_PIN, IOC_OVERRIDE_ANA);

    break;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(ph_sensor, PH_SENSOR, value, configure, status);

/** @} */
