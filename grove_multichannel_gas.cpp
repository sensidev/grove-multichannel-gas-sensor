#include "grove_multichannel_gas.h"

GroveMultichannelGasSensor::GroveMultichannelGasSensor(I2C &i2c) {
    _i2c = &i2c;
    i2cAddress = DEFAULT_I2C_ADDR;
}

/*********************************************************************************************************
** Function name:           begin
** Descriptions:            initialize I2C
*********************************************************************************************************/
void GroveMultichannelGasSensor::begin(int address)
{
    __version = 1;          // version 1/2

    i2cAddress = address;
    __version = getVersion();
}

unsigned char GroveMultichannelGasSensor::getVersion()
{
    if(get_addr_dta(CMD_READ_EEPROM, ADDR_IS_SET) == 1126)        // get version
    {
        __version = 2;
        debug_if(DEBUG_GROVE, "version = 2\r\n");
        return 2;
    }

    __version = 1;
    debug_if(DEBUG_GROVE, "version = 1\r\n");
    return 1;
}

void GroveMultichannelGasSensor::begin()
{
    begin(DEFAULT_I2C_ADDR);
}

/*********************************************************************************************************
** Function name:           sendI2C
** Descriptions:            send one byte to I2C Wire
*********************************************************************************************************/
void GroveMultichannelGasSensor::sendI2C(char dta)
{
    char i2c_data_write[1];

    i2c_data_write[0] = dta;
    _i2c->write((int) i2cAddress, i2c_data_write, 1);
}


unsigned int GroveMultichannelGasSensor::get_addr_dta(char addr_reg)
{
    unsigned int dta = 0;
    char i2c_data_write[1];
    char i2c_data_read[2];

    i2c_data_write[0] = addr_reg;
    _i2c->write((int) i2cAddress, i2c_data_write, 1);
    wait_ms(2);
    debug_if(DEBUG_GROVE_I2C, "I2C: Write %x\r\n", i2c_data_write[0]);

    _i2c->read((int) i2cAddress, i2c_data_read, 2);
    wait_ms(2);
    debug_if(DEBUG_GROVE_I2C, "I2C: Read %x %x\r\n", i2c_data_read[0], i2c_data_read[1]);

    dta = i2c_data_read[0];
    dta <<= 8;
    dta += i2c_data_read[1];

    switch(addr_reg)
    {
        case CH_VALUE_NH3:

        if(dta > 0)
        {
            adcValueR0_NH3_Buf = dta;
        }
        else
        {
            dta = adcValueR0_NH3_Buf;
        }

        break;

        case CH_VALUE_CO:

        if(dta > 0)
        {
            adcValueR0_CO_Buf = dta;
        }
        else
        {
            dta = adcValueR0_CO_Buf;
        }

        break;

        case CH_VALUE_NO2:

        if(dta > 0)
        {
            adcValueR0_NO2_Buf = dta;
        }
        else
        {
            dta = adcValueR0_NO2_Buf;
        }

        break;

        default:;
    }
    return dta;
}

unsigned int GroveMultichannelGasSensor::get_addr_dta(char addr_reg, char __dta)
{
    char i2c_data_write[2];
    char i2c_data_read[2];
    unsigned int dta = 0;

    i2c_data_write[0] = addr_reg;
    i2c_data_write[1] = __dta;

    _i2c->write((int) i2cAddress, i2c_data_write, 2);
    wait_ms(2);
    debug_if(DEBUG_GROVE_I2C, "I2C: Write 0x%2.2X 0x%2.2X\r\n", i2c_data_write[0], i2c_data_write[1]);

    _i2c->read((int) i2cAddress, i2c_data_read, 2);
    wait_ms(2);
    debug_if(DEBUG_GROVE_I2C, "I2C: Read 0x%2.2X 0x%2.2x\r\n", i2c_data_read[0], i2c_data_read[1]);

    dta = i2c_data_read[0];
    dta <<= 8;
    dta += i2c_data_read[1];

    return dta;
}

void GroveMultichannelGasSensor::write_i2c(unsigned char addr, const char *dta, int dta_len)
{
    _i2c->write((int) addr, dta, dta_len);
}


/*********************************************************************************************************
** Function name:           readData
** Descriptions:            read 4 bytes from I2C slave
*********************************************************************************************************/
int16_t GroveMultichannelGasSensor::readData(uint8_t cmd)
{
    char i2c_data_read[4];
    uint8_t checksum = 0;
    int16_t rtnData = 0;

    //send command
    sendI2C(cmd);
    //wait for a while
    wait_ms(2);
    //get response

    _i2c->read((int) i2cAddress, i2c_data_read, 4);

    checksum = (uint8_t)(i2c_data_read[0] + i2c_data_read[1] + i2c_data_read[2]);
    if(checksum != i2c_data_read[3])
    return -4;//checksum wrong
    rtnData = ((i2c_data_read[1] << 8) + i2c_data_read[2]);

    return rtnData;//successful
}

/*********************************************************************************************************
** Function name:           readR0
** Descriptions:            read R0 stored in slave MCU
*********************************************************************************************************/
int16_t GroveMultichannelGasSensor::readR0(void)
{
    int16_t rtnData = 0;

    rtnData = readData(0x11);

    if(rtnData > 0)
    res0[0] = rtnData;
    else
    return rtnData;         //unsuccessful

    rtnData = readData(0x12);
    if(rtnData > 0)
    res0[1] = rtnData;
    else
    return rtnData;         //unsuccessful

    rtnData = readData(0x13);
    if(rtnData > 0)
    res0[2] = rtnData;
    else
    return rtnData;         //unsuccessful

    return 1;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            read resistance value of each channel from slave MCU
*********************************************************************************************************/
int16_t GroveMultichannelGasSensor::readR(void)
{
    int16_t rtnData = 0;

    rtnData = readData(0x01);
    if(rtnData >= 0)
    res[0] = rtnData;
    else
    return rtnData;//unsuccessful

    rtnData = readData(0x02);
    if(rtnData >= 0)
    res[1] = rtnData;
    else
    return rtnData;//unsuccessful

    rtnData = readData(0x03);
    if(rtnData >= 0)
    res[2] = rtnData;
    else
    return rtnData;//unsuccessful

    return 0;//successful
}

/*********************************************************************************************************
** Function name:           readR
** Descriptions:            calculate gas concentration of each channel from slave MCU
** Parameters:
gas - gas type
** Returns:
float value - concentration of the gas
*********************************************************************************************************/
float GroveMultichannelGasSensor::calcGas(int gas)
{

    float ratio0, ratio1, ratio2;
    if(1 == __version)
    {
        if(readR() < 0)
        return -2.0f;

        ratio0 = (float)res[0] / res0[0];
        ratio1 = (float)res[1] / res0[1];
        ratio2 = (float)res[2] / res0[2];
    }
    else if(2 == __version)
    {
        // how to calc ratio/123
        ledOn();
        int A0_0 = get_addr_dta(6, ADDR_USER_ADC_HN3);
        int A0_1 = get_addr_dta(6, ADDR_USER_ADC_CO);
        int A0_2 = get_addr_dta(6, ADDR_USER_ADC_NO2);

        int An_0 = get_addr_dta(CH_VALUE_NH3);
        int An_1 = get_addr_dta(CH_VALUE_CO);
        int An_2 = get_addr_dta(CH_VALUE_NO2);

        ratio0 = (float)An_0/(float)A0_0*(1023.0-A0_0)/(1023.0-An_0);
        ratio1 = (float)An_1/(float)A0_1*(1023.0-A0_1)/(1023.0-An_1);
        ratio2 = (float)An_2/(float)A0_2*(1023.0-A0_2)/(1023.0-An_2);

    }

    float c = 0;

    switch(gas)
    {
        case CO:
        {
            c = pow(ratio1, -1.179)*4.385;  //mod by jack
            break;
        }
        case NO2:
        {
            c = pow(ratio2, 1.007)/6.855;  //mod by jack
            break;
        }
        case NH3:
        {
            c = pow(ratio0, -1.67)/1.47;  //modi by jack
            break;
        }
        case C3H8:  //add by jack
        {
            c = pow(ratio0, -2.518)*570.164;
            break;
        }
        case C4H10:  //add by jack
        {
            c = pow(ratio0, -2.138)*398.107;
            break;
        }
        case CH4:  //add by jack
        {
            c = pow(ratio1, -4.363)*630.957;
            break;
        }
        case H2:  //add by jack
        {
            c = pow(ratio1, -1.8)*0.73;
            break;
        }
        case C2H5OH:  //add by jack
        {
            c = pow(ratio1, -1.552)*1.622;
            break;
        }
        default:
        break;
    }

    if(2==__version)ledOff();
    return isnan(c)?-3:c;
}

/*********************************************************************************************************
** Function name:           changeI2cAddr
** Descriptions:            change I2C address of the slave MCU, and this address will be stored in EEPROM of slave MCU
*********************************************************************************************************/
void GroveMultichannelGasSensor::changeI2cAddr(uint8_t newAddr)
{
    char i2c_data_write[2];

    i2c_data_write[0] = 0x23;
    i2c_data_write[1] = (char) newAddr;

    _i2c->write((int) i2cAddress, i2c_data_write, 2);
    i2cAddress = newAddr;
}

/*********************************************************************************************************
** Function name:           powerOn
** Descriptions:            power on sensor heater
*********************************************************************************************************/
void GroveMultichannelGasSensor::powerOn(void)
{
    if(__version == 1)
    {
        sendI2C(0x21);
    }
    else if(__version == 2)
    {
        dta_test[0] = 11;
        dta_test[1] = 1;
        write_i2c(i2cAddress, dta_test, 2);
    }
}

/*********************************************************************************************************
** Function name:           powerOff
** Descriptions:            power off sensor heater
*********************************************************************************************************/
void GroveMultichannelGasSensor::powerOff(void)
{
    if(__version == 1)
    {
        sendI2C(0x20);
    }
    else if(__version == 2)
    {
        dta_test[0] = 11;
        dta_test[1] = 0;
        write_i2c(i2cAddress, dta_test, 2);
    }
}

void GroveMultichannelGasSensor::display_eeprom()
{
    if(__version == 1)
    {
        debug_if(DEBUG_GROVE, "ERROR: display_eeprom() is NOT support by V1 firmware.\r\n");
        return ;
    }

    debug_if(DEBUG_GROVE, "ADDR_IS_SET = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_IS_SET));
    debug_if(DEBUG_GROVE, "ADDR_FACTORY_ADC_NH3 = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_NH3));
    debug_if(DEBUG_GROVE, "ADDR_FACTORY_ADC_CO = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_CO));
    debug_if(DEBUG_GROVE, "ADDR_FACTORY_ADC_NO2 = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_FACTORY_ADC_NO2));
    debug_if(DEBUG_GROVE, "ADDR_USER_ADC_HN3 = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_HN3));
    debug_if(DEBUG_GROVE, "ADDR_USER_ADC_CO = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_CO));
    debug_if(DEBUG_GROVE, "ADDR_USER_ADC_NO2 = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_NO2));
    debug_if(DEBUG_GROVE, "ADDR_I2C_ADDRESS = %d\r\n", get_addr_dta(CMD_READ_EEPROM, ADDR_I2C_ADDRESS));
}

float GroveMultichannelGasSensor::getR0(unsigned char ch)         // 0:CH3, 1:CO, 2:NO2
{
    if(__version == 1)
    {
        debug_if(DEBUG_GROVE, "ERROR: getR0() is NOT support by V1 firmware.\r\n");
        return -1;
    }

    int a = 0;
    switch(ch)
    {
        case 0:         // CH3
        a = get_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_HN3);
        debug_if(DEBUG_GROVE, "a_ch3 = %d\r\n", a);
        break;

        case 1:         // CO
        a = get_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_CO);
        debug_if(DEBUG_GROVE, "a_co = %d\r\n", a);
        break;

        case 2:         // NO2
        a = get_addr_dta(CMD_READ_EEPROM, ADDR_USER_ADC_NO2);
        debug_if(DEBUG_GROVE, "a_no2 = %d\r\n", a);
        break;

        default:;
    }

    float r = 56.0*(float)a/(1023.0-(float)a);
    return r;
}

float GroveMultichannelGasSensor::getRs(unsigned char ch)         // 0:CH3, 1:CO, 2:NO2
{

    if(__version == 1)
    {
        debug_if(DEBUG_GROVE, "ERROR: getRs() is NOT support by V1 firmware.\r\n");
        return -1;
    }

    int a = 0;
    switch(ch)
    {
        case 0:         // NH3
        a = get_addr_dta(1);
        break;

        case 1:         // CO
        a = get_addr_dta(2);
        break;

        case 2:         // NO2
        a = get_addr_dta(3);
        break;

        default:;
    }

    float r = 56.0*(float)a/(1023.0-(float)a);
    return r;
}

void GroveMultichannelGasSensor::change_i2c_address(unsigned char addr)
{
    dta_test[0] = CMD_CHANGE_I2C;
    dta_test[1] = addr;
    write_i2c(i2cAddress, dta_test, 2);

    debug_if(DEBUG_GROVE, "FUNCTION: CHANGE I2C ADDRESS: 0x%x > 0x%x\r\n", i2cAddress, addr);

    i2cAddress = addr;
}

/*********************************************************************************************************
END FILE
*********************************************************************************************************/
