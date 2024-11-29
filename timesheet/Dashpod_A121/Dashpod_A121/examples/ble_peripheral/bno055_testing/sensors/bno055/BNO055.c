#include "BNO055.h"


/* @Brief This function is about setting up the bno055 sensor 
*
*
*/

uint8_t sys, accel, gyro, mag;

void setup_bno055(const nrf_drv_twi_t* m_twi,uint8_t addr)
{

    if(!is_twi_initialized())
    {
        twi_init();
        printf("TWI Initialized\n");
    
    }
    

    uint8_t data_reset[2] = {BNO055_SYS_TRIGGER_ADDR,0x20};
    uint8_t data_pwr_mode[2] = {BNO055_PWR_MODE_ADDR,0x00};
    uint8_t data_page[2]={0x07,0x00};
    uint8_t data_opr_mode[2] = {BNO055_OPR_MODE_ADDR,OPERATION_MODE_NDOF_OR_IMU};

    ret_code_t err_code;

    //here resetting the sensor
    err_code = (nrf_drv_twi_tx(m_twi, addr, data_reset, sizeof(data_reset), false));
    APP_ERROR_CHECK(err_code);
    nrf_delay_ms(1000);


    //here setting into power mode 
    err_code = (nrf_drv_twi_tx(m_twi, addr, data_pwr_mode, sizeof(data_pwr_mode), false));  
    nrf_delay_ms(1000); 
    APP_ERROR_CHECK(err_code);
   
    
     //if (err_code == NRF_SUCCESS) {
     // printf("succesfully configured power mode\n");
     //}
     //else
     // printf("failed power mode configuration\n");
    


    //here  setting into page 0
    err_code = (nrf_drv_twi_tx(m_twi, addr, data_page, sizeof(data_page), false));   
    APP_ERROR_CHECK(err_code);
    nrf_delay_ms(10);
    
     //if (err_code == NRF_SUCCESS) {
     // printf("succesfully configured page 0\n");
     //}
     //else
     // printf("failed page 0 configuration\n");
    


    //here setting into operation mode NDOF
    err_code = (nrf_drv_twi_tx(m_twi, addr, data_opr_mode, sizeof(data_opr_mode), false));   
    APP_ERROR_CHECK(err_code);
    nrf_delay_ms(20);
    
     //if (err_code == NRF_SUCCESS) {
     // printf("succesfully configured NDOF mode\n");
     //}
     //else
     // printf("failed NDOF configuration\n");
    printf("succesfully configured BNO055\n");
    
    
}

/* @Brief This function is about finding slave address of BNO055
*
*
*/


uint8_t bno_i2c_scanner(const nrf_drv_twi_t* m_twi,uint8_t addr)
  {

    bool detected_device = false;
      
    if(!is_twi_initialized()) 
        {
            twi_init();
            printf("TWI initialized\n");
        }

    uint8_t data = 0xff;

    printf("Twi scanning started\n");
    for (; addr <= TWI_ADDRESSES; addr++)
    {
        ret_code_t err_code = nrf_drv_twi_tx(m_twi,addr,&data,1,false);
        nrf_delay_ms(50);
        if (err_code == NRF_SUCCESS)
        {
            detected_device = true;
            printf("\nTWI device detected at addr 0x%x\n", addr);
             
        }

    }
    return detected_device;

  }

  

/* @Brief This is a function which displays sensors calibration values 
*
*/
void displayCalStatus(const nrf_drv_twi_t* m_twi,uint8_t addr) {

    uint8_t value, reg = BNO055_CALIB_STAT_ADDR;
    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(m_twi, addr, &reg, 1, true);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_twi_rx(m_twi, addr, &value, 1);
    APP_ERROR_CHECK(err_code);

     sys = (value >> 6) & 0x03;
     gyro = (value >> 4) & 0x03;
     accel = (value >> 2) & 0x03;
     mag = value & 0x03;

    printf("\nSys: %d\t",sys);
    printf(" Gyro: %d\t",gyro);
    printf(" Accel: %d\t",accel);
    printf(" Mag: %d\n",mag);
    
    uint8_t data[4];

    data[0] = sys;
    data[1] = gyro;
    data[2] = accel;
    data[3] = mag;
    

}


void read_euler(const nrf_drv_twi_t* m_twi, uint8_t addr)
 {

    if((accel==3) && (mag == 3) && (gyro == 3)){
            uint8_t euler_buffer[6] = {0};
            uint8_t reg = 0x1A;

            ret_code_t err_code;
            int16_t x,y,z;

            int i;
            for (i=0;i<sizeof(euler_buffer);i++,reg++){
        // this loop runs for 6 axis including positive and negative.
                  err_code = nrf_drv_twi_tx(m_twi,addr,&reg,1,true);
                  APP_ERROR_CHECK(err_code);

                  err_code = nrf_drv_twi_rx(m_twi, addr, &euler_buffer[i] , sizeof(euler_buffer[i]));
                  APP_ERROR_CHECK(err_code);
            }

            x = ((int16_t)euler_buffer[0]) | (((int16_t)euler_buffer[1]) << 8);
            y = ((int16_t)euler_buffer[2]) | (((int16_t)euler_buffer[3]) << 8);
            z = ((int16_t)euler_buffer[4]) | (((int16_t)euler_buffer[5]) << 8);

            printf("\n\nEULER ANGLES\n");
            printf("RAW_X: %d\tRAW_Y: %d\tRAW_Z: %d\n", x, y, z);
            printf("X: %-10.2fY: %-10.2fZ: %-10.2f\n", x / 16.0, y / 16.0, z / 16.0);
    
            float data[3];
            data[0] = x/16.0;
            data[1] = y/16.0;
            data[2] = z/16.0;
    
         
    }
 }


 void read_quaternion(const nrf_drv_twi_t* m_twi, uint8_t addr)  {
        if((accel==3) && (mag == 3) && (gyro == 3)){
            uint8_t quater_buffer[8] = {0};
            uint8_t reg = 0x20;

            ret_code_t err_code;
            int16_t qw,qx,qy,qz;

            for(int i =0; i<sizeof(quater_buffer);i++){
               err_code = nrf_drv_twi_tx(m_twi,addr,&reg,1,true);
               APP_ERROR_CHECK(err_code);

               err_code = nrf_drv_twi_rx(m_twi, addr, &quater_buffer[i] , sizeof(quater_buffer[i]));
               APP_ERROR_CHECK(err_code);
               reg++;

            }

            qw = ((int16_t)quater_buffer[0]) | (((int16_t)quater_buffer[1]) << 8);
            qx = ((int16_t)quater_buffer[2]) | (((int16_t)quater_buffer[3]) << 8);
            qy = ((int16_t)quater_buffer[4]) | (((int16_t)quater_buffer[5]) << 8);
            qz = ((int16_t)quater_buffer[6]) | (((int16_t)quater_buffer[7]) << 8);

            printf("\n\nRAW QUTERNION VALUES\n");
            printf("QWR: %d\tQXR: %d\tQYR: %d\tQZR: %d\n",qw, qx, qy, qz);

            float data[4];

            data[0] = qw / 16384.0;
            data[1] = qx / 16384.0;
            data[2] = qy / 16384.0;
            data[3] = qz / 16384.0;

            printf(" Scaled values : QW: %-10.4f\tQX: %-10.4f\tQY: %-10.4f\tQZ: %-10.4f\n",data[0],data[1],data[2],data[3]);
            quaternion_to_angles(data[0],data[1],data[2],data[3]);
          
    }

 }


 /* @Brief This function is about converting quaternion values into angles 
 *
 *
 */
 void quaternion_to_angles(float qw, float qx, float qy, float qz)  {
   
   float x,y,z;
    // Roll (x-axis rotation) in radians
    x = atan2(2.0 * (qw * qx + qy * qz), 1.0 - 2.0 * (qx * qx + qy * qy));
    
    // Pitch (y-axis rotation) in radians
    y = asin(2.0 * (qw * qy - qz * qx));

    // Yaw (z-axis rotation) in radians
    z = atan2(2.0 * (qw * qz + qx * qy), 1.0 - 2.0 * (qy * qy + qz * qz));

    printf("\nThe converted angles from quaternion\n");

    // printing the values directly in degrees since the bno055 calculates the euler values in degrees by default when in NDOF mode.

    //converting radians to degree
    printf("X = %-10.2fY = %-10.2fZ = %-10.2f", x*180/M_PI, y*180/M_PI, z*180/M_PI);
    
   // printf("X = %-10.2fY = %-10.2fZ = %-10.2f", x, y, z);
 }

// this function prints the acceleration values and need to be scaled to obtain the user readable values.
 void acceleration_values(const nrf_drv_twi_t* m_twi, uint8_t addr){
    
    uint8_t accel_buff[6] ={0};
    uint8_t reg = 0x08;

    ret_code_t err_code;

    int16_t x,y,z;                          // 9.93881 Threshold

    int i;

    for(i=0;i<sizeof(accel_buff);i++,reg++){
        
        err_code = nrf_drv_twi_tx(m_twi, addr, &reg, 1, true);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_drv_twi_rx(m_twi, addr, &accel_buff[i], sizeof(accel_buff[i]));
        APP_ERROR_CHECK(err_code);
    
    }

    x = (((int16_t)accel_buff[0]) | (((int16_t)accel_buff[1]) << 8));
    y = (((int16_t)accel_buff[2]) | (((int16_t)accel_buff[3]) << 8));
    z = (((int16_t)accel_buff[4]) | (((int16_t)accel_buff[5]) << 8));

    printf("\nAcceleration values: ");

    float data[3];

    data[0] = x/100.0;
    data[1] = y/100.0;
    data[2] = z/100.0;

    printf("A_X = %.2f, A_Y = %.2f, A_Z = %.2f\n",data[0],data[1],data[2]);
 }


 void mag_values(const nrf_drv_twi_t* m_twi, uint8_t addr){
    
    uint8_t mag_buffer[6] = {0};
    uint8_t reg = 0x0E;

    ret_code_t err_code;
    
    int16_t x,y,z;
    int i;

    for(i=0;i<sizeof(mag_buffer);i++,reg++){
        
        err_code = nrf_drv_twi_tx(m_twi, addr, &reg, 1, true);
        APP_ERROR_CHECK(err_code);

        err_code = nrf_drv_twi_rx(m_twi, addr, &mag_buffer[i], sizeof(mag_buffer[i]));
        APP_ERROR_CHECK(err_code);
    }

    x = (((int16_t)mag_buffer[0]) | (((int16_t)mag_buffer[1]) << 8));
    y = (((int16_t)mag_buffer[2]) | (((int16_t)mag_buffer[3]) << 8));
    z = (((int16_t)mag_buffer[4]) | (((int16_t)mag_buffer[5]) << 8));

    printf("\nMagnetometer Values: ");
    float data[3];
    data[0] = x;
    data[1] = y;
    data[2] = z;

    printf("M_X = %.2f, M_Y = %.2f, M_Z = %.2f \n",data[0],data[1],data[2]);

 }

  void gyro_values(const nrf_drv_twi_t* m_twi, uint8_t addr){
    
    uint8_t gyro_buffer[6] = {0};
    uint8_t reg = 0x14;

    ret_code_t err_code;
    
    int16_t x,y,z;
    int i;

    for(i=0;i<sizeof(gyro_buffer);i++,reg++){
        
        err_code = nrf_drv_twi_tx(m_twi, addr, &reg, 1, true);
        APP_ERROR_CHECK(err_code);

        err_code = nrf_drv_twi_rx(m_twi, addr, &gyro_buffer[i], sizeof(gyro_buffer[i]));
        APP_ERROR_CHECK(err_code);
    }

    x = (((int16_t)gyro_buffer[0]) | (((int16_t)gyro_buffer[1]) << 8));
    y = (((int16_t)gyro_buffer[2]) | (((int16_t)gyro_buffer[3]) << 8));
    z = (((int16_t)gyro_buffer[4]) | (((int16_t)gyro_buffer[5]) << 8));

    printf("\n Gyroscope Values: ");
    float data[3];
    data[0] = x;
    data[1] = y;
    data[2] = z;

    printf("G_X = %.2f, G_Y = %.2f, G_Z = %.2f \n",data[0],data[1],data[2]);

 }

 
/*
 void step_length(float x, float y, float z){
      
      // Variables to store velocity and step length
      float velocity_x = 0, velocity_y = 0, velocity_z = 0;
      float step_length = 0;
      float delta_t = 0.01; // Assume 100Hz sampling rate

    
    // Calculate acceleration magnitude
    float magnitude = sqrt(x * x + y * y + z * z);




    // Step detection logic based on threshold
    if (magnitude > threshold) {
        // Integrate acceleration to calculate velocity
        velocity_x = velocity_x + (x * delta_t);
        velocity_y = velocity_y + (y * delta_t);
        velocity_z = velocity_z + (z * delta_t);

        // Integrate velocity to calculate step length
        float step_length_x = velocity_x * delta_t;
        float step_length_y = velocity_y * delta_t;
        float step_length_z = velocity_z * delta_t;

        // Calculate total step length
        step_length = sqrt(step_length_x * step_length_x + step_length_y * step_length_y + step_length_z * step_length_z);

        // Reset velocities after detecting a step
        velocity_x = velocity_y = velocity_z = 0;

        // Print or store the calculated step length
        printf("Step length: %f meters\n", step_length);
    }

 
 
 }

 */

