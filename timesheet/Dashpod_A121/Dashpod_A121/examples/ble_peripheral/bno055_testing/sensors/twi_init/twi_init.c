
#include "twi_init.h"


const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


uint8_t twi_initialized = 0;



/**@brief Function for checking twi is initialized or not
*
*
*/
uint8_t is_twi_initialized(void) 
{
    return twi_initialized;
}


/**@brief Function for initialization of twi 
*
*
*/
void twi_init(void) 
{
    
      ret_code_t err_code;

      const nrf_drv_twi_config_t twi_config = {
         .scl                = NRF_GPIO_PIN_MAP(1,9),//33
         .sda                = NRF_GPIO_PIN_MAP(0,11),//11
         .frequency          = NRF_DRV_TWI_FREQ_400K,
         .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
         .clear_bus_init     = true
        };

      err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
      //APP_ERROR_CHECK(err_code);

      nrf_drv_twi_enable(&m_twi);

      twi_initialized = 1;
}