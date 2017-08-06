// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <TSL2561.h>

// dependencies
#include "../core/jentity.h"
#include "../util/sensorvalue.h"
#include "../util/stopwatch.h"

namespace meisterwerk {
    namespace thing {
        class tsl2561 : public core::jentity {
            protected:
            uint8_t           tslAddress;
            const char *      tslType = "TSL2561";
            TSL2561 *         ptsl    = nullptr;
            util::sensorvalue luminosity;

            public:
            tsl2561( String name, uint8_t address )
                : core::jentity( name, 200000, core::PRIORITY_NORMAL, 4 ),
                  luminosity( "luminosity", 0, 5, 900, 5.0 ), tslAddress{address} {
                // original: 50ms, 20 samples
                // now: 200ms, 5 samples
            }

            virtual void setup() override {
                DBGF( "Configuring environmental sensor TSL2561 on address 0x%02x\n", tslAddress );
                // The address will be different depending on whether you let
                // the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
                // use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
                if ( nullptr != ( ptsl = new TSL2561( tslAddress ) ) ) {
                    // You can change the gain on the fly, to adapt to brighter/dimmer light situations
                    ptsl->setGain( TSL2561_GAIN_0X ); // set no gain (for bright situtations)
                    // tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)

                    // Changing the integration time gives you a longer time over which to sense light
                    // longer timelines are slower, but are good in very low light situtations!
                    ptsl->setTiming( TSL2561_INTEGRATIONTIME_13MS ); // shortest integration time (bright light)
                    // tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
                    // tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)

                    SensorValue( "luminosity", true );
                } else {
                    DBG( "TSL2561 initialization failure." );
                }
            }

            virtual void loop() override {
                updateSensorValue( luminosity, readLuminosity(), tslType );
            }

            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "info" ) {
                    data["type"] = "environment";
                    luminosity.prepare( data, tslType, false );
                    notify( value, data );
                } else if ( value == "luminosity" ) {
                    if ( luminosity.prepare( data, tslType ) ) {
                        notify( value, data );
                    }
                }
            }

            double readLuminosity() {
                if ( ptsl ) {
                    uint32_t lum  = ptsl->getFullLuminosity();
                    uint16_t ir   = lum >> 16;
                    uint16_t full = lum & 0xFFFF;
                    return (double)ptsl->calculateLux( full, ir );
                } else {
                    return NAN;
                }
            }
        };
    } // namespace thing
} // namespace meisterwerk
