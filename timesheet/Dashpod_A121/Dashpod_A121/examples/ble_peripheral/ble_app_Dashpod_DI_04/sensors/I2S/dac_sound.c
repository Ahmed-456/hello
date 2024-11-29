#include "dac_sound.h"

uint16_t sine_wave[BUFFER_SIZE];
static volatile bool m_i2s_done = false;
volatile bool is_beeping = false;

volatile bool i2sCmp = false ;

static uint32_t       * volatile mp_block_to_fill  = NULL;
static uint32_t const * volatile mp_block_to_check = NULL;

static float SAMPLE_RATE = 54100 ;   // Sample rate in Hz    //basic 48000
static float FREQUENCY   = 20000;     // Frequency of the sine wave in Hz
static float AMPLITUDE   = 15276 ;   // Amplitude of the sine wave (max value for 16-bit audio)
static nrf_drv_i2s_buffers_t gInitial_buffers;

#define BLAZE_BUFFER 1024

void data_handler(nrf_drv_i2s_buffers_t const *p_released, uint32_t status) 
{
    #ifdef DEBUG
    //printf("Data handler status = %d\n", status);
    #endif
    
    ASSERT(p_released);

    if (!(status & NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED)) {
        return;
    }


    if (!p_released->p_rx_buffer) {
        // Provide next buffer for TX
        nrf_drv_i2s_buffers_t const next_buffers = {
            .p_rx_buffer = NULL,
            .p_tx_buffer = m_buffer_tx[1],
        };
        APP_ERROR_CHECK(nrf_drv_i2s_next_buffers_set(&next_buffers));

        mp_block_to_fill = m_buffer_tx[1];
    } else {
        mp_block_to_check = p_released->p_rx_buffer;

        // Refill TX buffer and continue
        APP_ERROR_CHECK(nrf_drv_i2s_next_buffers_set(p_released));
        mp_block_to_fill = (uint32_t *)p_released->p_tx_buffer;
    }
    
    i2sCmp = true;
}


// Generate sine wave data
void generate_sine_wave(uint16_t *buffer, size_t buffer_size, float frequency, float sample_rate, float amplitude) 
{
  for (size_t i = 0; i < buffer_size; i += 2) 
  {
      // Generate sine wave sample
      float phase = (2 * M_PI * frequency * (i / 2)) / sample_rate;  // i/2 for the correct phase
      int16_t sample_value = (int16_t)((amplitude * sinf(phase)) + amplitude); // Calculate sample value

      // Fill left and right channels
      buffer[i] = (uint16_t)sample_value;       // Left channel
      buffer[i + 1] = (uint16_t)sample_value;   // Right channel
  }

}


void i2s_init() 
{
    ret_code_t err_code;
    nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
    
    config.sck_pin = NRF_GPIO_PIN_MAP(0,15);
    config.lrck_pin = NRF_GPIO_PIN_MAP(0,3);
    config.sdout_pin = NRF_GPIO_PIN_MAP(0, 2);
    config.mck_pin = NRFX_I2S_PIN_NOT_USED;

    nrf_gpio_cfg_output(SD_MODE);

    config.mode = NRF_I2S_MODE_MASTER;
    config.format = NRF_I2S_FORMAT_I2S;
    config.alignment = NRF_I2S_ALIGN_LEFT;
    config.sample_width = NRF_I2S_SWIDTH_16BIT;
    config.channels = NRF_I2S_CHANNELS_LEFT;
    config.mck_setup = NRF_I2S_MCK_32MDIV23;
    config.ratio = NRF_I2S_RATIO_64X;

    err_code = nrf_drv_i2s_init(&config, data_handler);
    APP_ERROR_CHECK(err_code);

    if (err_code != NRFX_SUCCESS) {
        printf("I2S initialization error: %d\n", err_code);
    }
    
    #ifdef DEBUG
    printf("I2S intialization succcesfully\n");
    #endif
}



void beep(uint32_t sec,uint8_t choice)
{
    nrf_gpio_pin_set(SD_MODE);
    sample_freq(choice);
    generate_sine_wave(sine_wave, BUFFER_SIZE, FREQUENCY, SAMPLE_RATE, AMPLITUDE);
    
    nrf_drv_i2s_buffers_t initial_buffers = 
    {
        .p_tx_buffer = (uint32_t *)sine_wave,
        .p_rx_buffer = NULL,
    };

    m_i2s_done = false;

    nrfx_err_t err_code = nrf_drv_i2s_start(&initial_buffers, BUFFER_SIZE, 0);
    
    if (err_code != NRFX_SUCCESS) {
        printf("I2S start error: %d\n", err_code);
        return;
    }

    // Continuous audio playback loop
    while (sec--) 
    {
        while (!i2sCmp);  // Wait until the current buffer completes
        i2sCmp = false;   // Reset the flag for the next buffer
    }

    nrfx_i2s_stop();
    nrf_gpio_pin_clear(SD_MODE);
}


void start_beep(uint32_t sec,uint8_t choice)
{
    if (is_beeping) 
    {
      memcpy(m_buffer_tx[1], sine_wave, BLAZE_BUFFER);
      return; // Avoid re-triggering if already active
    }

    is_beeping = true;
    nrf_gpio_pin_set(SD_MODE);
    sample_freq(choice);
    generate_sine_wave(sine_wave, BUFFER_SIZE, FREQUENCY, SAMPLE_RATE, AMPLITUDE);

    nrf_drv_i2s_buffers_t initial_buffers = {
        .p_tx_buffer = (uint32_t *)sine_wave,
        .p_rx_buffer = NULL,
    };

    //gInitial_buffers.p_tx_buffer = (uint32_t *)sine_wave;
    //gInitial_buffers.p_rx_buffer = NULL;

    nrfx_err_t err_code = nrf_drv_i2s_start(&initial_buffers,BLAZE_BUFFER,0);
    if (err_code != NRFX_SUCCESS) {
        printf("I2S start error: %d\n", err_code);
        is_beeping = false;
    }
    while (sec--) 
    {
        //printf("\nsec is %d\n",sec);
        while (!i2sCmp);  // Wait until the current buffer completes
        i2sCmp = false;   // Reset the flag for the next buffer
    }
    

}

void stop_beep()
{
    nrfx_i2s_stop();
    nrf_gpio_pin_clear(SD_MODE);
    is_beeping = false;
}


void beep_for_charge(uint32_t sec,uint8_t choice)
{
    nrf_gpio_pin_set(SD_MODE);
    sample_freq(choice);
    generate_sine_wave(sine_wave, BUFFER_SIZE, FREQUENCY, SAMPLE_RATE, AMPLITUDE);
    
    nrf_drv_i2s_buffers_t initial_buffers = {
        .p_tx_buffer = (uint32_t *)sine_wave,
        .p_rx_buffer = NULL,
    };

    m_i2s_done = false;

    nrfx_err_t err_code = nrf_drv_i2s_start(&initial_buffers, BUFFER_SIZE, 0);
    
    if (err_code != NRFX_SUCCESS) {
        printf("I2S start error: %d\n", err_code);
        return;
    }

     //Continuous audio playback loop
    //uint32_t a = 15000;
    //while (a--) 
    //{
    //    printf(" ");
       
    //}

    while(sec--)
    {
         nrf_delay_ms(100);
    }

    nrfx_i2s_stop();
    nrf_gpio_pin_clear(SD_MODE);
}

//void beep_for_func(uint32_t sec,uint8_t choice)
//{
//    nrf_gpio_pin_set(SD_MODE);
//    sample_freq(choice);
//    generate_sine_wave(sine_wave, BUFFER_SIZE, FREQUENCY, SAMPLE_RATE, AMPLITUDE);
    
//    nrf_drv_i2s_buffers_t initial_buffers = {
//        .p_tx_buffer = (uint32_t *)sine_wave,
//        .p_rx_buffer = NULL,
//    };

//    m_i2s_done = false;

//    nrfx_err_t err_code = nrf_drv_i2s_start(&initial_buffers, BUFFER_SIZE, 0);
    
//    if (err_code != NRFX_SUCCESS) {
//        printf("I2S start error: %d\n", err_code);
//        return;
//    }

//    while(sec--)
//    {
//         nrf_delay_ms(10);
//    }

//    nrfx_i2s_stop();
//    nrf_gpio_pin_clear(SD_MODE);
//}

//void sample_freq(uint8_t choice)
//{
//        switch(choice)
//        {
//            case 1:
//                    FREQUENCY   = 20000;
//                    SAMPLE_RATE = 54100;
//                    AMPLITUDE   = 15276;
//                    break;

//            case 2:
//                    FREQUENCY   = 25000;
//                    SAMPLE_RATE = 70000;
//                    AMPLITUDE   = 15276;
//                    break;

//            case 3: 
//                    FREQUENCY   = 17000;
//                    SAMPLE_RATE = 50000;
//                    AMPLITUDE   = 15000;
//                    break;

//            case 4: 
//                    FREQUENCY   =  15000;
//                    SAMPLE_RATE =  60000;
//                    AMPLITUDE   =  20000;
//                    break;
                   
//        }
//}

void tone()
{
      beep(2,1);
      beep(2,2);
      beep(2,4);
      beep(2,1);
      beep(2,2);
      beep(2,4);
      beep(2,1);
      beep(2,2);
      beep(2,3);
      beep(2,4);
      beep(2,5);       
}

void sample_freq(uint8_t choice)
{
    switch(choice)
    {
        case 0:
              FREQUENCY   = 20000;
              SAMPLE_RATE = 54100;
              AMPLITUDE   = 15276;
              break;
        case 1:
              FREQUENCY   = 330;    // "E" note in Hz
              SAMPLE_RATE = 14000;  // Sample rate to accurately produce 330 Hz tone
              AMPLITUDE   = 20000;  // Volume level
              break;

        case 2:
              FREQUENCY   = 415;    // "G#" note in Hz
              SAMPLE_RATE = 14000;  // Sample rate for this range
              AMPLITUDE   = 20000;
              break;

        case 3: 
              FREQUENCY   = 494;    // "B" note in Hz
              SAMPLE_RATE = 14000;
              AMPLITUDE   = 20000;
              break;

        case 4: 
              FREQUENCY   = 554;    // "C#" note in Hz
              SAMPLE_RATE = 14000;
              AMPLITUDE   = 20000;
              break;
        case 5: 
              FREQUENCY   = 440;    // "A" note in Hz
              SAMPLE_RATE = 14000;
              AMPLITUDE   = 20000;
              break;
    }
}
