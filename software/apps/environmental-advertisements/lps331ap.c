#include <stdint.h>
#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <math.h>
#include "lps331ap.h"
#include <stdio.h>
#include <string.h>
#include "time.h"


#define REF_P_XL 0x08
#define REF_P_L 0x09
#define REF_P_H 0x0A

#define RES_CONF 0x10

#define CTRL_REG1 0x20 
#define POWER_ON   0x80
#define POWER_OFF   0x00
#define BLOCK_UPDATE_ENABLE 0x04


#define CTRL_REG2 0x21 
#define ONE_SHOT_ENABLE 0x01
#define SW_RESET 0x04

#define CTRL_REG3 0x22 

#define INT_CFG_REG 0x23 
#define INT_SOURCE_REG 0x24 

#define THS_P_LOW_REG 0x25
#define THS_P_HIGH_REG 0x26

#define STATUS_REG 0x27 

#define PRESS_OUT_XL 0x28
#define PRESS_OUT_L 0x29 
#define PRESS_OUT_H 0x2A 

#define TEMP_OUT_L 0x2B
#define TEMP_OUT_H 0x2C 

#define AMP_CTRL 0x30 

// Read/Write def
#define TWI_READ                        0b1
#define TWI_WRITE                       0b0

#define SENSOR_ADDR	0x5C

#define AUTO_INCR	0x80

static nrf_drv_twi_t * m_instance;


// must only be called after lps331ap_on has been called
void lps331ap_readPressure(float *pres){

    uint8_t command = PRESS_OUT_XL | AUTO_INCR;
    
    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
           	&command,
            sizeof(command),
            true
    );


    uint8_t pressure_data[3];

    nrf_drv_twi_rx(
            m_instance, 
            SENSOR_ADDR,
            pressure_data,
            sizeof(pressure_data),
            false
    );

    float pressure =    (0x00FFFFFF & (((uint32_t)pressure_data[2] << 16) |
                        ((uint32_t) pressure_data[1] << 8) |
                        ((uint32_t) pressure_data[0]))) / 4096.0;

    *pres = pressure;

}

//does not return correct temperature
void lps331ap_readTemp (float *temp){


    uint8_t command = TEMP_OUT_L | AUTO_INCR;

    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            &command,
            sizeof(command),
            true
    );

    uint8_t temp_data[2];

    nrf_drv_twi_rx(
            m_instance, 
            SENSOR_ADDR,
            temp_data,
            sizeof(temp_data),
            false
    );


    float temperature = (((int16_t)( 0x0000FFFF & (  ( (uint32_t) temp_data[1] << 8) |  ((uint32_t) temp_data[0]) ) )) / 480.0 ) + 42.5;


    *temp = temperature;

}


//resets everything
void lps331ap_power_off(){

	
	uint8_t command[] = {
		CTRL_REG1, 
		POWER_OFF
	};

    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            command,
            sizeof(command),
            false
    );

}

void lps331ap_sw_reset(){

    uint8_t command[] = 
    {
        CTRL_REG2,
        SW_RESET

    };

    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            &command,
            sizeof(command),
            false
    );

}


void lps331ap_config(lps331ap_data_rate_t data_rate, lps331ap_p_res_t p_res, lps331ap_t_res_t t_res){


    lps331ap_config_data_rate(data_rate);

    lps331ap_config_res(p_res, t_res);

}


void lps331ap_config_data_rate(lps331ap_data_rate_t data_rate){

    uint8_t command[2] = 
    {
        CTRL_REG1,
        (data_rate << 4)
    };

    nrf_drv_twi_tx(
        m_instance, 
        SENSOR_ADDR,
        command,
        sizeof(command),
        false
    );


}

void lps331ap_init(nrf_drv_twi_t * p_instance){

    //lps331ap_sw_reset();

    m_instance = p_instance;


}

void lps331ap_read_controlreg1(uint8_t * data){

    uint8_t command = CTRL_REG1;

    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            &command,
            sizeof(command),
            true
    );

    nrf_drv_twi_rx(
            m_instance, 
            SENSOR_ADDR,
            data,
            sizeof(data),
            false
    );

}


void lps331ap_read_controlreg2(uint8_t * data){

    uint8_t command = CTRL_REG2;

    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            &command,
            sizeof(command),
            true
    );

    nrf_drv_twi_rx(
            m_instance, 
            SENSOR_ADDR,
            data,
            sizeof(data),
            false
    );

}

void lps331ap_read_status_reg(uint8_t * buf){

    uint8_t command[] = {STATUS_REG};

    nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            &command,
            sizeof(command),
            true
    );

    nrf_drv_twi_rx(
            m_instance, 
            SENSOR_ADDR,
            buf,
            sizeof(buf),
            false
    );

}

void lps331ap_config_res(lps331ap_p_res_t p_res, lps331ap_t_res_t t_res){

    uint8_t data = (p_res | (t_res << 4));

    uint8_t command[] = 
    {
        RES_CONF,
        data

    };

     nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            &command,
            sizeof(command),
            false
    );

}


// must only be called after lps331ap_init has been called
void lps331ap_power_on(){

    uint8_t command[1] = {CTRL_REG1};

    nrf_drv_twi_tx(
        m_instance, 
        SENSOR_ADDR,
        command,
        sizeof(command),
        true
    );

    uint8_t data[1] = {0x00};

    nrf_drv_twi_rx(
        m_instance, 
        SENSOR_ADDR,
        data,
        sizeof(data),
        false
    );


	uint8_t command2[] = {
		CTRL_REG1, 
		data[0] | POWER_ON | BLOCK_UPDATE_ENABLE
	};

    nrf_drv_twi_tx(
        m_instance, 
        SENSOR_ADDR,
        command2,
        sizeof(command2),
        false
    );

}

//need to test this
void lps331ap_one_shot_config(){


    uint8_t command[] = 
    {
        CTRL_REG1,
        BLOCK_UPDATE_ENABLE
    };

    nrf_drv_twi_tx(
        m_instance, 
        SENSOR_ADDR,
        command,
        sizeof(command),
        false
    ); 

}

void lps331ap_one_shot_enable(){

    uint8_t command2[] = 
    {
        CTRL_REG2,
        ONE_SHOT_ENABLE

    };

     nrf_drv_twi_tx(
            m_instance, 
            SENSOR_ADDR,
            command2,
            sizeof(command2),
            false
    );

}