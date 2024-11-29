#include "nrfx.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"

//modified on 06-07-2024

#ifndef TWI_INIT_H
#define TWI_INIT_H

/* TWI instance ID. */                        
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif


#define MS_OR_US 0
#define DEL 75

#if MS_OR_US
#define delay  nrf_delay_ms
#elif !MS_OR_US
#define delay  nrf_delay_us
#endif

/* Number of possible TWI addresses. */
#define TWI_ADDRESSES     127
#endif

uint8_t is_twi_initialized(void);
void twi_init(void);

