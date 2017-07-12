
#ifndef i2cdev_BMP085_h
#define i2cdev_BMP085_h

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"
#include "../base/i2cdev.h"
#include "../util/hextools.h"

#include <Adafruit_BMP085.h>

namespace meisterwerk {
    namespace base {
        class i2cdev_BMP085 : public meisterwerk::base::i2cdev {
            public:
            Adafruit_BMP085 *pbmp;
            bool pollSensor=false;
            SensorProcessor tempProcessor, pressProcessor;
            String json;

            i2cdev_BMP085( String name)
                : meisterwerk::base::i2cdev( name, "BMP085" ) {
            }
            ~i2cdev_BMP085() {
                if (pollSensor) {
                    pollSensor=false;
                    delete pbmp;
                }
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 5000000 );
            }

            virtual void instantiate(String i2ctype, uint8_t address) override {
                DBG("Instantiating BMP085 device at address 0x"+hexByte(address));
                pbmp = new Adafruit_BMP085();
                if (!pbmp->begin()) {
                    DBG("BMP085 initialization failure.");
                } else {
                    pollSensor=true;
                }
            }

            virtual void onLoop( unsigned long ticker ) override {
                if (pollSensor) {
                    float temperature=pbmp->readTemperature();
                    if (tempProcessor.filter(&temperature)) {
                        json = "{\"temperature\":" + String( temperature ) + "}";
                        DBG( "jsonstate i2c bmp085:" + json );
                        publish( entName+"/temperature", json );
                    }
                    float pressure=pbmp->readPressure()/100.0;
                    if (pressProcessor.filter(&pressure)) {
                        json = "{\"pressure\":" +String(pressure)+"}";
                        DBG( "jsonstate i2c bmp085:" + json );
                        publish( entName+"/pressure", json ); 
                    }
                }
            }

        };
    } // namespace base
} // namespace meisterwerk

#endif
