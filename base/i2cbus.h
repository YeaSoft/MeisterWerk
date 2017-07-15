
// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>
#include <Wire.h>

// dependencies
#include "../core/entity.h"
#include "../util/hextools.h"

// This is based on: http://www.ladyada.net/library/i2caddr.html
// and web dom's sensorclock research

// I2C addresses used:
// Luminosity (TSL2561)    0x39
// OLED Display            0x3c
// RTC Eeprom              0x50
// RTC                     0x68
// SevenSegment (Adafruit) 0x70
// Barometer (BMP085):     0x77

enum I2CDevType {
    OLED,
    LED,
    LCD,
    NFC,
    Sensor,
    RTC,
    IO,
    PWM,
    DAC,
    Radio,
    Touch,
    EEPROM,
};

enum I2CDev {
    SSD1306,
    PN532,
    TSL2561,
    BMP085,
    ADXL345,
    HMC5883L,
    BMA180,
    MMA7455L,
    VCNL4000,
    ITG3200,
    MS5607,
    MS5611,
    LSM9DS0_Accel_Mag,
    LSM9DS0_Gyro,
    LSM303_Accel,
    LSM303_Mag,
    DS1307_3231,
    DS1307_EEPROM,
    DS3231,
    MCP23008,
    MCP23017,
    PCA9685,
    MCP4725A0,
    MCP4725A1,
    MCP4725A2,
    MCP4725A3,
    TEA5767,
    Si4713,
    FT6206,
    STMPE610,
    LED7_14_SEG,
    LCD_2_4_16_20
};

#define MAX_I2C_ADDRESS_VAR 4
typedef struct t_i2c_properties {
    I2CDevType    id;
    I2CDev        type;
    String        name;
    String        description;
    bool          supported;
    unsigned char addresses[MAX_I2C_ADDRESS_VAR];
} T_I2C_PROPERTIES;

const T_I2C_PROPERTIES i2cProps[] = {
    {OLED, SSD1306, "SSD1306", "OLED-display (128x64)", false, {0x3C, 0x3D, 0, 0}},
    // http://www.solomon-systech.com/en/product/display-ic/oled-driver-controller/ssd1306/
    {LED, LED7_14_SEG, "LED7_14_SEG", "4x 7/14 Segment LED Display", false, {0x70, 0x71}},
    // I2C backback for LCD: 0x71: bridge on A0.
    {LCD, LCD_2_4_16_20, "LCD_2_4_16_20", "2x16 or 4x20 LCD Display", false, {0x25, 0x26, 0x27}},
    // I2C backpack for LCD: 0x26: solder bridge on A0, 0x25: on A1.
    // https://learn.adafruit.com/adafruit-led-backpack/0-54-alphanumeric
    // {NFC, PN532, "PN532", "RFID controller", false, {0x48}},
    // https://www.adafruit.com/product/364
    {Sensor, TSL2561, "TSL2561", "Luminosity sensor", false, {0x29, 0x39, 0x49, 0}},
    // https://www.adafruit.com/product/439
    {Sensor, BMP085, "BMP085", "Pressure, temperature, altitude sensor", false, {0x77, 0, 0, 0}},
    // https://www.adafruit.com/product/391
    // {Sensor, ADXL345, "ADXL345", "Accelerometer", false, {0x1D, 0x53}},
    // http://www.analog.com/en/products/mems/accelerometers/adxl345.html
    {Sensor, HMC5883L, "HMC5883L", "Magnetic compass", false, {0x1E, 0, 0, 0}}, // Also GY271
    // https://aerospace.honeywell.com/en/products/navigation-and-sensors/magnetic-sensors-and-transducers
    // {Sensor, BMA180, "BMA180", "3 axis accelerometer", false, {0x77}},
    // http://www.geeetech.com/wiki/index.php/BMA180_Triple_Axis_Accelerometer_Breakout
    {Sensor, VCNL4000, "VCNL4000", "Proximity light sensor", false, {0x13, 0, 0, 0}},
    // https://www.adafruit.com/product/466
    //{Sensor, ITG3200, "ITG3200", "3 axis accelerometer", false, {0x68, 0x69}},
    // https://www.invensense.com/products/motion-tracking/3-axis/itg-3200/
    //{Sensor, MS5607, "MS5607", "Altimeter", false, {0x76, 0x77}},
    // https://www.parallax.com/product/29124
    //{Sensor, MS5611, "MS5611", "Barometric pressure altitude sensor", false, {0x76, 0x77}},
    // http://www.amsys.info/products/ms5611.htm
    //{Sensor, LSM9DS0_Accel_Mag, "LSM9DS0_Accel_Mag", "", false, {0x1D}},
    //{Sensor, LSM9DS0_Gyro, "LSM9DS0_Gyro", "", false, {0x6B}},
    //{Sensor, LSM303_Accel, "LSM303_Accel", "", false, {0x19}},
    //{Sensor, LSM303_Mag, "LSM303_Mag", "", false, {0x1E}},
    {RTC, DS1307_3231, "DS1307_3231", "Real time clock", false, {0x68, 0, 0, 0}},
    {EEPROM, DS1307_EEPROM, "DS1307_EEPORM", "RTC EEPROM", false, {0x50, 0, 0, 0}},
    //{IO, MCP23008, "MCP23008", "", false, {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27}}, //
    // used for LCD displays {IO, MCP23017, "MCP23017", "", false, {0x20, 0x21, 0x22, 0x23, 0x24,
    // 0x25, 0x26, 0x27}}, {PWM, PCA9685, "PCA9685", "", false, {0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
    // 0x46, 0x47,
    //                                      0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F}},
    //{DAC, MCP4725A0, "MCP4725A0", "", false, {0x60, 0x61}},
    //{DAC, MCP4725A1, "MCP4725A1", "", false, {0x62, 0x63}},
    //{DAC, MCP4725A2, "MCP4725A2", "", false, {0x64, 0x65}},
    //{DAC, MCP4725A3, "MCP4725A3", "", false, {0x66, 0x67}},
    //{Radio, TEA5767, "TEA5767", "", false, {0x60}},
    //{Radio, Si4713, "Si4713", "", false, {0x11, 0x63}},
    //{Touch, FT6206, "FT6206", "", false, {0x38}},
    //{Touch, STMPE610, "STMPE610", "", false, {0x41}}
};

namespace meisterwerk {
    namespace base {

        class i2cbus : public meisterwerk::core::entity {
            public:
            bool         bSetup;
            bool         bEnum;
            bool         bInternalError;
            uint8_t      sdaport, sclport;
            unsigned int nDevices;

            i2cbus( String name, uint8_t sdaport, uint8_t sclport )
                : meisterwerk::core::entity( name ), sdaport{sdaport}, sclport{sclport} {
                bSetup         = false;
                bEnum          = false;
                bInternalError = false;
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onRegister() override {
                Wire.begin( sdaport, sclport ); // SDA, SCL;
                bSetup = true;
                bEnum  = false;
                subscribe( "i2cbus/enum" );
            }

            int identify( uint8_t address ) {
                int numDevs = 0;
                int last    = -1;
                for ( int i = 0; i < sizeof( i2cProps ) / sizeof( T_I2C_PROPERTIES ); i++ ) {
                    for ( int j = 0; j < MAX_I2C_ADDRESS_VAR; j++ ) {
                        if ( i2cProps[i].addresses[j] == address ) {
                            ++numDevs;
                            if ( last != -1 ) {
                                DBG( "Ambiguous:" + String( last ) + "<->" + String( i ) );
                            }
                            last = i;
                        }
                    }
                }
                if ( numDevs == 1 ) {
                    DBG( i2cProps[last].name + ", " + i2cProps[last].description + " at port: 0x" +
                         meisterwerk::util::hexByte( address ) );
                } else if ( numDevs > 1 ) {
                    last = -1;
                    DBG( "WARNING: ambiguous addresses in I2C hardware detection for address 0x" +
                         meisterwerk::util::hexByte( address ) + ", cannot securely identify" );
                } else {
                    last = -1;
                    DBG( "WARNING: device at address 0x" + meisterwerk::util::hexByte( address ) +
                         " is of unknown type." );
                }
                return last;
            }

            bool check( uint8_t address ) {
                bool bDevFound = false;
                // The i2c_scanner uses the return value of
                // the Write.endTransmisstion to see if
                // a device did acknowledge to the address.
                // From: https://playground.arduino.cc/Main/I2cScanner
                Wire.beginTransmission( address );
                byte error = Wire.endTransmission();
                if ( error == 0 ) {
                    bDevFound = true;
                    // DBG( "I2C device found at address 0x" + meisterwerk::util::hexByte( address )
                    // );
                } else if ( error == 4 ) {
                    DBG( "Unknow error at address " + meisterwerk::util::hexByte( address ) );
                }
                return bDevFound;
            }

            unsigned int i2cScan() {
                byte   address;
                String portlist = "";
                String devlist  = "";
                int    niDevs, i2cid;

                if ( !bSetup ) {
                    DBG( "i2cbus not initialized!" );
                    publish( "i2cbus/offline", "" );
                    bInternalError = true;
                    return 0;
                }
                if ( bEnum ) {
                    DBG( "For now, mulitple I2C-bus enums are suppressed." );
                    return 0;
                }
                DBG( "Scanning I2C-Bus, SDA=" + String( sdaport ) + ", SCL=" + String( sclport ) );
                nDevices = 0;
                niDevs   = 0;
                for ( uint8_t address = 1; address < 127; address++ ) {
                    if ( check( address ) ) {
                        nDevices++;
                        if ( nDevices > 1 ) {
                            portlist += ",";
                        }
                        portlist += String( address );
                        i2cid = identify( address );
                        if ( i2cid != -1 ) {
                            niDevs++;
                            if ( niDevs > 1 ) {
                                devlist += ",";
                            }
                            devlist += "\"" + i2cProps[i2cid].name + "\"";
                        }
                    }
                }
                if ( nDevices == 0 ) {
                    DBG( "No I2C devices found" );
                    publish( "i2cbus/offline", "" );
                } else {
                    String json = "{\"devs\":" + String( nDevices ) + ",\"portlist\":[" + portlist +
                                  "],\"i2cdevs\":[" + devlist + "]}";
                    DBG( "jsonstate i2c:" + json );
                    publish( "i2cbus/online", json );
                }
                bEnum = true;
                return nDevices;
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( bInternalError )
                    return;
                if ( !bEnum ) {
                    i2cScan();
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == "i2cbus/enum" ) {
                    i2cScan();
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
