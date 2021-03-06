
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
    TSL_2561,
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
    LPC810DCF77,
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
    {Sensor, TSL_2561, "TSL2561", "Luminosity sensor", false, {0x29, 0x39, 0x49, 0}},
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
    {RTC, LPC810DCF77, "LPC810DCF77", "MicroWerk DCF77-Cortex-M0", false, {0x6e, 0, 0, 0}},
    // https://www.elektormagazine.com/labs/lpc810-as-dcf77-decoder-and-i2c-slave
    //{Sensor, MS5607, "MS5607", "Altimeter", false, {0x76, 0x77}},
    // https://www.parallax.com/product/29124
    //{Sensor, MS5611, "MS5611", "Barometric pressure altitude sensor", false, {0x76, 0x77}},
    // http://www.amsys.info/products/ms5611.htm
    //{Sensor, LSM9DS0_Accel_Mag, "LSM9DS0_Accel_Mag", "", false, {0x1D}},
    //{Sensor, LSM9DS0_Gyro, "LSM9DS0_Gyro", "", false, {0x6B}},
    //{Sensor, LSM303_Accel, "LSM303_Accel", "", false, {0x19}},
    //{Sensor, LSM303_Mag, "LSM303_Mag", "", false, {0x1E}},
    {RTC, DS1307_3231, "DS1307_3231", "Real time clock", false, {0x68, 0, 0, 0}},
    {EEPROM, DS1307_EEPROM, "DS1307_EEPORM", "RTC EEPROM", false, {0x50, 0x57, 0, 0}},
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
            bool            bSetup;
            bool            bEnum;
            bool            bInternalError;
            uint8_t         sdaport, sclport;
            unsigned int    nDevices;
            String          i2cjson;
            String          i2cjsonOld;
            unsigned int    nDevicesOld;
            time_t          lastScanTime    = 0;
            unsigned long   i2cCacheTimeout = 15; // 15 sec, scan requests repeated within 15sec are answered by cache.
            util::metronome i2cWatchdog;

            i2cbus( String name, uint8_t sdaport, uint8_t sclport )
                : meisterwerk::core::entity( name, 50000 ), i2cWatchdog( 600000 ), sdaport{sdaport}, sclport{sclport} {
                bSetup         = false;
                bEnum          = false;
                bInternalError = false;
            }

            virtual void setup() override {
                Wire.begin( sdaport, sclport ); // SDA, SCL;
                bSetup = true;
                bEnum  = false;
                subscribe( "i2cbus/devices/get" );
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

            int  hwErrs       = 0;
            bool bHWErrDetect = false;
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
                    ++hwErrs;
                    if ( !bHWErrDetect ) {
                        bHWErrDetect = true;
                    }
                }
                return bDevFound;
            }

            unsigned int i2cScan( bool publishResult = true ) {
                byte address;
                int  niDevs, i2cid;

                if ( !bSetup ) {
                    DBG( "i2cbus not initialized!" );
                    if ( publishResult )
                        publish( "i2cbus/devices", "{\"devices\":[]}" );
                    bInternalError = true;
                    return 0;
                }
                if ( bEnum && lastScanTime - now() < i2cCacheTimeout ) {
                    DBG( "Cached result for i2c scan." );
                    if ( publishResult )
                        publish( "i2cbus/devices", i2cjson );
                    return 0;
                }
                lastScanTime = now();

                hwErrs       = 0;
                bHWErrDetect = false;

                DBG( "Scanning I2C-Bus, SDA=" + String( sdaport ) + ", SCL=" + String( sclport ) );
                nDevices       = 0;
                niDevs         = 0;
                String devlist = "";
                for ( uint8_t address = 1; address < 127; address++ ) {
                    if ( check( address ) ) {
                        String port = String( address );
                        i2cid       = identify( address );
                        if ( i2cid != -1 ) {
                            String dev = i2cProps[i2cid].name;
                            niDevs++;
                            if ( niDevs > 1 ) {
                                devlist += ",";
                            }
                            devlist += "{\"" + dev + "\": \"" + port + "\"}";
                        }
                    }
                }
                if ( hwErrs > 0 ) {
                    String errmsg = "I2C-bus hardware problem: " + String( hwErrs ) +
                                    " errors during scan. Try power power-cycling device.";
                    DBG( errmsg );
                    log( T_LOGLEVEL::ERR, errmsg );
                }
                if ( niDevs == 0 ) {
                    DBG( "No I2C devices found" );
                    if ( publishResult )
                        publish( "i2cbus/devices", "{\"devices\":[]}" );
                } else {
                    i2cjson = "{\"devices\":[" + devlist + "]}";
                    DBG( "jsonstate i2c:" + i2cjson );
                    if ( publishResult )
                        publish( "i2cbus/devices", i2cjson );
                }
                if ( bEnum ) {
                    if ( nDevices != nDevicesOld || i2cjson != i2cjsonOld ) {
                        DBG( "I2C bus state changed! " + String( nDevices ) +
                             " connected. (Before: " + String( nDevicesOld ) + " devices." );
                    }
                }
                log( T_LOGLEVEL::INFO, "I2Cbus enumeration: " + i2cjson );
                nDevicesOld = nDevices;
                i2cjsonOld  = i2cjson;
                bEnum       = true;
                return nDevices;
            }

            virtual void loop() override {
                if ( bInternalError )
                    return;
                if ( !bEnum ) {
                    i2cScan();
                    bEnum = true;
                }
                if ( i2cWatchdog.beat() > 0 ) {
                    int    oldDevs = nDevices;
                    String oldJson = i2cjson;
                    if ( i2cScan( false ) != oldDevs || i2cjson != oldJson ) {
                        log( T_LOGLEVEL::ERR, "I2C-bus change from: " + oldJson + " to: " + i2cjson );
                    }
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                String topic( ctopic );
                if ( topic == "i2cbus/devices/get" ) {
                    i2cScan();
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
