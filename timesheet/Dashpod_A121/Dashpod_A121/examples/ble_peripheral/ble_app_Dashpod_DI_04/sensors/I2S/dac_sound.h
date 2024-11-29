#include <stdio.h>
#include "nrf_drv_i2s.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#include "math.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define I2S_DATA_BLOCK_WORDS    1024
static uint32_t m_buffer_rx[2][I2S_DATA_BLOCK_WORDS];
static uint32_t m_buffer_tx[2][I2S_DATA_BLOCK_WORDS];

// Definitions and configurations

#define EEPROM_I2C_ADDRESS  0x50        // EEPROM I2C address

//float SAMPLE_RATE = 54100 ;   // Sample rate in Hz
//float FREQUENCY   = 20000;     // Frequency of the sine wave in Hz
//float AMPLITUDE   = 15276 ;   // Amplitude of the sine wave (max value for 16-bit audio)

#define BUFFER_SIZE  2048   // Buffer size for I2S

#define SD_MODE  NRF_GPIO_PIN_MAP(0,17)

void data_handler(nrf_drv_i2s_buffers_t const * p_released, uint32_t status);
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) ;
void generate_sine_wave(uint16_t *buffer, size_t buffer_size, float frequency, float sample_rate, float amplitude);
void i2s_init();
void play_audio(uint32_t loop) ;
void beep(uint32_t sec,uint8_t choice);
void sample_freq(uint8_t choice);
void tone(void);
void beep_for_charge(uint32_t sec,uint8_t choice);
void start_beep(uint32_t sec, uint8_t choice);
void stop_beep(); 
void cont_beep_init();
void beep_start();
void beep_for_func(uint32_t sec,uint8_t choice);

