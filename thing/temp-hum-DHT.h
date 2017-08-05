// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <DHT.h>

// dependencies
#include "../core/jentity.h"
#include "../util/sensorvalue.h"
#include "../util/stopwatch.h"

namespace meisterwerk {
    namespace thing {
        class dht : public meisterwerk::core::jentity {
            protected:
            enum state { S_NONE, S_WARMING, S_INITIALIZING, S_ACTIVE };
            uint8_t           dhtPin; // def = GPIO9, note: D8 needs to be low on boot, don't use if you can avoid.
            DHT *             pdht     = nullptr;
            state             dhtState = S_NONE;
            String            dhtType;
            util::sensorvalue temperature, humidity;
            util::stopwatch   startTime;

            public:
            dht( String name, String type, uint8_t pin )
                : meisterwerk::core::jentity( name, 2000000 ), temperature( "temperature", 0, 5, 900, 0.1 ),
                  humidity( "humidity", 0, 5, 900, 1.0 ), dhtType{type}, dhtPin{pin} {
                // read cycle every 2 seconds
                pdht     = nullptr;
                dhtState = S_NONE;
            }

            virtual void setup() override {
                // 5sec sensor checks
                DBGF( "Configuring environmental sensor %s on pin: %d\n", dhtType.c_str(), (int)dhtPin );

                if ( dhtType == "DHT11" ) {
                    pdht = new DHT( dhtPin, 11 );
                } else if ( dhtType == "DHT21" || dhtType == "AM2301" ) {
                    pdht = new DHT( dhtPin, 21 );
                } else if ( dhtType == "DHT22" || dhtType == "AM2302" ) {
                    pdht = new DHT( dhtPin, 22 );
                }

                if ( pdht ) {
                    DBG( "DHT Sensor initialized." );
                    dhtState = S_WARMING;
                    pdht->begin();
                    startTime.start();
                    SensorValue( "humidity", true );
                    SensorValue( "temperature", true );
                }
            }

            virtual void loop() override {
                if ( dhtState == S_WARMING ) {
                    if ( startTime > 2000 ) {
                        dhtState = S_INITIALIZING;
                    }
                }
                if ( dhtState == S_INITIALIZING ) {
                    if ( isnan( pdht->readTemperature() ) ) {
                        DBG( dhtType + " temperature/humidity sensor, initialization failure." );
                    } else {
                        DBG( dhtType + " sensor initialized." );
                        dhtState = S_ACTIVE;
                    }
                }
                if ( dhtState == S_ACTIVE ) {
                    updateSensorValue( humidity, pdht->readTemperature(), dhtType.c_str() );
                    updateSensorValue( temperature, pdht->readTemperature(), dhtType.c_str() );
                }
            }

            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "info" ) {
                    data["type"]       = "environment";
                    data["sensortype"] = dhtType;
                    temperature.prepare( data, dhtType.c_str(), false );
                    humidity.prepare( data, dhtType.c_str(), false );
                    notify( value, data );
                } else if ( value == "humidity" ) {
                    if ( humidity.prepare( data, dhtType.c_str() ) ) {
                        notify( value, data );
                    }
                } else if ( value == "temperature" ) {
                    if ( temperature.prepare( data, dhtType.c_str() ) ) {
                        notify( value, data );
                    }
                }
            }
        };
    } // namespace thing
} // namespace meisterwerk
