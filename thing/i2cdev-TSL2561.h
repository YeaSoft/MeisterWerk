// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"
#include "../util/sensorprocessor.h"

// hardware dependencies
#include <TSL2561.h>

namespace meisterwerk {
    namespace thing {
        class i2cdev_TSL2561 : public meisterwerk::base::i2cdev {
            public:
            bool                               pollSensor = false;
            meisterwerk::util::sensorprocessor luminosityProcessor;
            String                             json;
            bool                               luminosityValid;
            double                             luminosityLast;
            String                             luminosityTime;
            bool                               bOptionWaitForValidTime = true;
            bool                               bTimeValid              = false;
            TSL2561 *                          ptsl;

            i2cdev_TSL2561( String name, uint8_t address )
                : meisterwerk::base::i2cdev( name, "TSL2561", address ), luminosityProcessor( 5, 900, 1.0 ) {
                // send luminosity updates, if luminosity changes for 1.0lux over an average of 5
                // measurements, but at least every 15min (900sec)
                luminosityValid = false;
            }
            ~i2cdev_TSL2561() {
                if ( pollSensor ) {
                    pollSensor = false;
                    delete ptsl;
                }
            }

            virtual void setup() override {
                i2cdev::setup();
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                DBG( "Instantiating TSL2561 device at address 0x" + meisterwerk::util::hexByte( address ) );
                // The address will be different depending on whether you let
                // the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
                // use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
                ptsl = new TSL2561( address );
                if ( !ptsl->begin() ) {
                    DBG( "TSL2561 initialization failure." );
                } else {
                    // You can change the gain on the fly, to adapt to brighter/dimmer light situations
                    ptsl->setGain( TSL2561_GAIN_0X ); // set no gain (for bright situtations)
                    // tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)

                    // Changing the integration time gives you a longer time over which to sense light
                    // longer timelines are slower, but are good in very low light situtations!
                    ptsl->setTiming( TSL2561_INTEGRATIONTIME_13MS ); // shortest integration time (bright light)
                    // tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
                    // tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)

                    pollSensor = true;
                    subscribe( entName + "/luminosity/get" );
                }
            }

            void publishLuminosity() {
                if ( luminosityValid ) {
                    json = "{\"time\":\"" + luminosityTime + "\",\"luminosity\":" + String( luminosityLast ) + "}";
                    publish( entName + "/luminosity", json );
                    log( T_LOGLEVEL::INFO, "Luminosity: " + String( luminosityLast ) );
                } else {
                    DBG( "No valid luminosity measurement for pub" );
                }
            }
            virtual void loop() override {
                if ( pollSensor ) {
                    if ( timeStatus() != timeNotSet )
                        bTimeValid = true;
                    if ( bOptionWaitForValidTime && !bTimeValid )
                        return;

                    uint32_t lum = ptsl->getFullLuminosity();
                    uint16_t ir, full;
                    ir                = lum >> 16;
                    full              = lum & 0xFFFF;
                    double luminosity = (double)ptsl->calculateLux( full, ir );

                    if ( luminosityProcessor.filter( &luminosity ) ) {
                        luminosityLast  = luminosity;
                        luminosityValid = true;
                        luminosityTime  = util::msgtime::time_t2ISO( now() );
                        publishLuminosity();
                    }
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                meisterwerk::base::i2cdev::receive( origin, ctopic, msg );
                String topic( ctopic );
                if ( topic == "mastertime/time/set" )
                    bTimeValid = true;
                if ( topic == entName + "/luminosity/get" || topic == "*/luminosity/get" ) {
                    publishLuminosity();
                }
            }
        };
    } // namespace thing
} // namespace meisterwerk
