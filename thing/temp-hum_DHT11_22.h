// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies

//#include <Adafruit_Sensor.h>
#include <DHT.h>
//#include <DHT_U.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"
#include "../util/metronome.h"
#include "../util/msgtime.h"
#include "../util/sensorprocessor.h"

namespace meisterwerk {
    namespace thing {
        class dht : public meisterwerk::core::entity {
            public:
            bool    pollSensor = false;
            uint8_t dhtPin; // def = GPIO9, note: D8 needs to be low on boot, don't use if you can avoid.
            // DHT_Unified *                 pdht;
            DHT *                 pdht;
            String                dhtSType;
            uint8_t               dhtType;
            util::sensorprocessor tempProcessor, humProcessor;
            String                json;
            bool                  tempvalid;
            double                templast;
            String                temptime;
            bool                  humvalid;
            double                humlast;
            String                humtime;
            bool                  bOptionWaitForValidTime = true;
            bool                  bTimeValid              = false;
            bool                  bStarting               = false;
            unsigned long         startTime               = 0L;

            dht( String name, String type, uint8_t pin )
                : meisterwerk::core::entity( name, 2000000 ), tempProcessor( 5, 900, 0.1 ),
                  humProcessor( 5, 900, 1.0 ), dhtSType{type}, dhtPin{pin} {
            }
            ~dht() {
                if ( pollSensor ) {
                    pollSensor = false;
                }
            }

            virtual void setup() override {
                // 5sec sensor checks
                DBG( "Configuring DHT-Sensor on pin: " + String( dhtPin ) );

                if ( dhtSType == "DHT11" )
                    dhtType = 11;
                else if ( dhtSType == "DHT22" )
                    dhtType = 22;
                else if ( dhtSType == "DHT21" )
                    dhtType = 21;
                else if ( dhtSType == "AM2301" )
                    dhtType = 21;
                // pdht = new DHT_Unified(dhtPin, dhtType);
                pdht = new DHT( dhtPin, dhtType );

                pdht->begin();
                DBG( "DHT Sensor initialized." );
                bStarting = true;
                startTime = millis();
            }

            void publishTemp() {
                if ( tempvalid ) {
                    json = "{\"time\":\"" + temptime + "\",\"temperature\":" + String( templast ) +
                           ",\"sensortype\":" + dhtSType + ",\"id\":" + entName + "}";
                    publish( entName + "/temperature", json );
                } else {
                    DBG( "No valid temp measurement for pub" );
                }
            }
            void publishHumidity() {
                if ( humvalid ) {
                    json = "{\"time\":\"" + humtime + "\",\"humidity\":" + String( humlast ) +
                           ",\"sensortype\":" + dhtSType + ",\"id\":" + entName + "}";
                    publish( entName + "/humidity", json );
                } else {
                    DBG( "No valid humidity measurement for pub" );
                }
            }

            virtual void loop() override {
                if ( pollSensor || bStarting ) {
                    if ( bStarting ) {
                        if ( util::timebudget::delta( startTime, millis() ) > 2000 ) {
                            bStarting = false;
                            if ( isnan( pdht->readTemperature() ) ) {
                                DBG( "DHT temperature/humidity sensor, initialization failure." );
                                pollSensor = false;
                            } else {
                                pollSensor = true;
                                subscribe( entName + "/temperature/get" );
                                subscribe( entName + "/humidity/get" );
                                subscribe( "temperature/get" );
                                subscribe( "humidity/get" );
                                subscribe( "mastertime/time/set" );
                                DBG( "DHT sensor initialized." );
                            }
                        }
                    } else {
                        if ( timeStatus() != timeNotSet )
                            bTimeValid = true;
                        if ( bOptionWaitForValidTime && !bTimeValid )
                            return;
                        double temperature = pdht->readTemperature();
                        if ( isnan( temperature ) ) {
                            DBG( "Sensor failure -- cannot read DHT temperature" );
                        } else {
                            if ( tempProcessor.filter( &temperature ) ) {
                                templast  = temperature;
                                tempvalid = true;
                                temptime  = util::msgtime::time_t2ISO( now() );
                                publishTemp();
                            }
                        }
                        double humidity = pdht->readHumidity();
                        if ( isnan( humidity ) ) {
                            DBG( "Sensor failure -- cannot read DHT humidity" );
                        } else {
                            if ( humProcessor.filter( &humidity ) ) {
                                humlast  = humidity;
                                humvalid = true;
                                humtime  = util::msgtime::time_t2ISO( now() );
                                publishHumidity();
                            }
                        }
                    }
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                String topic( ctopic );
                if ( topic == "mastertime/time/set" )
                    bTimeValid = true;
                if ( topic == entName + "/temperature/get" || topic == "temperature/get" ) {
                    publishTemp();
                }
                if ( topic == entName + "/humidity/get" || topic == "humidity/get" ) {
                    publishHumidity();
                }
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
