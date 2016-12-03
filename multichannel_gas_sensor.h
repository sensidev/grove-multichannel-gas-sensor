#ifndef __MUTICHANNEL_GAS_SENSOR_H__
#define __MUTICHANNEL_GAS_SENSOR_H__

#include "mbed.h"

#define DEFAULT_I2C_ADDR    0x04 << 1       // shift 1 bit, mbed uses 8bits, arduino 7bits.

#define ADDR_IS_SET             0           // if this is the first time to run, if 1126, set
#define ADDR_FACTORY_ADC_NH3    2
#define ADDR_FACTORY_ADC_CO     4
#define ADDR_FACTORY_ADC_NO2    6

#define ADDR_USER_ADC_HN3       8
#define ADDR_USER_ADC_CO        10
#define ADDR_USER_ADC_NO2       12
#define ADDR_IF_CALI            14          // IF USER HAD CALI

#define ADDR_I2C_ADDRESS        20

#define CH_VALUE_NH3            1
#define CH_VALUE_CO             2
#define CH_VALUE_NO2            3

#define CMD_ADC_RES0            1           // NH3
#define CMD_ADC_RES1            2           // CO
#define CMD_ADC_RES2            3           // NO2
#define CMD_ADC_RESALL          4           // ALL CHANNEL
#define CMD_CHANGE_I2C          5           // CHANGE I2C
#define CMD_READ_EEPROM         6           // READ EEPROM VALUE, RETURN UNSIGNED INT
#define CMD_SET_R0_ADC          7           // SET R0 ADC VALUE
#define CMD_GET_R0_ADC          8           // GET R0 ADC VALUE
#define CMD_GET_R0_ADC_FACTORY  9           // GET FACTORY R0 ADC VALUE
#define CMD_CONTROL_LED         10
#define CMD_CONTROL_PWR         11

enum{CO, NO2, NH3, C3H8, C4H10, CH4, H2, C2H5OH};

class GroveMutichannelGasSensor{

private:
    I2C _i2c;

    int __version = 0;
    char dta_test[20];

    unsigned int readChAdcValue(int ch);
    unsigned int adcValueR0_NH3_Buf;
    unsigned int adcValueR0_CO_Buf;
    unsigned int adcValueR0_NO2_Buf;

public:
    GroveMutichannelGasSensor(I2C &i2c);

    uint8_t i2cAddress;     //I2C address of this MCU
    uint16_t res0[3];       //sensors res0
    uint16_t res[3];        //sensors res

    inline unsigned int get_addr_dta(char addr_reg);
    inline unsigned int get_addr_dta(char addr_reg, char __dta);
    inline void write_i2c(unsigned char addr, const char *dta, int dta_len);

    void sendI2C(char dta);
    int16_t readData(uint8_t cmd);
    int16_t readR0(void);
    int16_t readR(void);
    float calcGas(int gas);

public:

    void begin(int address);
    void begin();
    void changeI2cAddr(uint8_t newAddr);
    void powerOn(void);
    void powerOff(void);

    //get gas concentration, unit: ppm
    float measure_CO(){return calcGas(CO);}
    float measure_NO2(){return calcGas(NO2);}
    float measure_NH3(){return calcGas(NH3);}
    float measure_C3H8(){return calcGas(C3H8);}
    float measure_C4H10(){return calcGas(C4H10);}
    float measure_CH4(){return calcGas(CH4);}
    float measure_H2(){return calcGas(H2);}
    float measure_C2H5OH(){return calcGas(C2H5OH);}

    float getR0(unsigned char ch);      // 0:CH3, 1:CO, 2:NO2
    float getRs(unsigned char ch);      // 0:CH3, 1:CO, 2:NO2

public:

    void ledOn()
    {
        dta_test[0] = CMD_CONTROL_LED;
        dta_test[1] = 1;
        write_i2c(i2cAddress, dta_test, 2);
    }

    void ledOff()
    {
        dta_test[0] = CMD_CONTROL_LED;
        dta_test[1] = 0;
        write_i2c(i2cAddress, dta_test, 2);
    }

    void display_eeprom();
    void change_i2c_address(unsigned char addr);
    unsigned char getVersion();
};

extern Serial pc;

#endif

/*********************************************************************************************************
END FILE
*********************************************************************************************************/
