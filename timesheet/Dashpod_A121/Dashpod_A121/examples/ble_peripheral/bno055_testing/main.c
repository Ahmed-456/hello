#include "BNO055.h"	

extern const nrf_drv_twi_t m_twi;

int main(void) 
{
    twi_init();
    nrf_delay_ms(500);

    bno_i2c_scanner(&m_twi, BNO055_ADDRESS);
    nrf_delay_ms(100);

    setup_bno055(&m_twi, BNO055_ADDRESS);
    nrf_delay_ms(500);
    
    

   while(1)
    {
      displayCalStatus(&m_twi, BNO055_ADDRESS);
      nrf_delay_ms(500);

      read_euler(&m_twi, BNO055_ADDRESS);
      nrf_delay_ms(500);

     //read_quaternion(&m_twi, BNO055_ADDRESS);
     //nrf_delay_ms(500);

      //acceleration_values(&m_twi, BNO055_ADDRESS);
     // nrf_delay_ms(500);

    }


}
