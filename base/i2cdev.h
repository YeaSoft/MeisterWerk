
#ifndef i2cdev_h
#define i2cdev_h

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace base {

        class SensorProcessor {
            public:
            int noVals=0;
            int smoothIntervall;
            int pollTimeSec;
            float sum=0.0;
            float eps;
            bool first=true;
            float lastVal=-99999.0;
            unsigned long last;
        
            // average of smoothIntervall measurements
            // update sensor value, if newvalue differs by at least eps, or if pollTimeSec has elapsed.
            SensorProcessor(int smoothIntervall=5, int pollTimeSec=60, float eps=0.1): smoothIntervall{smoothIntervall}, 
                                                                                                pollTimeSec{pollTimeSec}, eps{eps} {
                last=millis();
            }

            // changes the value into a smoothed version
            // returns true, if sensor-value is a valid update
            // an update is valid, if the new value differs by at least eps from last last value,
            // or, if pollTimeSec secs have elapsed.
            bool filter(float *pvalue) {
                float newVal=(lastVal*noVals+(*pvalue))/(noVals+1);
                lastVal=newVal;
                if (noVals<smoothIntervall) ++noVals;
                float delta=(*pvalue)-newVal;
                if (delta<0.0) delta= (-1.0)*delta;
                if (delta>eps || first) {
                    first=false;
                    *pvalue=newVal;
                    last=millis();
                    return true;
                } else {
                    if (pollTimeSec!=0) {
                        if (millis()-last > pollTimeSec*1000L) {
                            *pvalue=newVal;
                            last=millis();
                            return true;
                        }
                    }
                }
                return false;
            }
        };

        class i2cdev : public meisterwerk::core::entity {
            public:
            String i2ctype;

            i2cdev( String name, String i2cType )
                : meisterwerk::core::entity( name ), i2ctype{i2cType} {
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onSetup() override {
                DBG("i2cdev pub/sub in setup");
                subscribe( "i2cbus/online" );
                publish("i2cbus/enum","");
            }

            virtual void instantiate(String i2ctype, uint8_t address) {
                DBG("Your code should override this function and instantiate a "+i2ctype+" device at address 0x"+hexByte(address));
            }

            void i2cSetup(String json) {
                bool ok=false;
                DBG("Received i2c-info: "+json);
                DynamicJsonBuffer  jsonBuffer(200);
                JsonObject& root = jsonBuffer.parseObject(json);
                if (!root.success()) {
                    DBG("Invalid JSON received!");
                    return;
                }
                JsonArray& devs=root["i2cdevs"];
                JsonArray& ports=root["portlist"];
                for (int i=0; i<devs.size(); i++) {
                    String dev=devs[i];
                    uint8_t address=(uint8_t)ports[i];
                    DBG(i2ctype+"<->"+dev);
                    if (dev==i2ctype) {
                        instantiate(i2ctype, address);
                    }
                }

            }

            virtual void onLoop( unsigned long ticker ) override {
                
            }
/*
            virtual void onReceiveMessage( String topic, const char *pBuf,
                                           unsigned int len ) override {
                if ( topic == "i2cbus/online" ) {
                    i2cSetup(String(pBuf));
                }
            }
*/
            virtual void onReceive( String topic, String msg ) override {
                if ( topic == "i2cbus/online" ) {
                    i2cSetup(msg);
                }
            }
        };
    } // namespace base
} // namespace meisterwerk

#endif
