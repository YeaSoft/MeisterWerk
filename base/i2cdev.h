
#ifndef i2cdev_h
#define i2cdev_h

#include <ESP8266WiFi.h>

// dependencies
#include "../core/entity.h"

namespace meisterwerk {
    namespace base {
        class i2cdev : public meisterwerk::core::entity {
            public:
            String i2ctype;

            i2cdev( String name, String i2cType )
                : meisterwerk::core::entity( name ), i2ctype{i2ctype} {
                DBG("INIT i2cdev");
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onSetup() override {
                DBG("i2cdev pub/sub in setup");
                subscribe( "i2cbus/online" );
                publish("i2cbus/enum","");
            }

            void i2cSetup(String json) {
                DBG("Received i2c-info: "+json);
            }

            int l=0;
            virtual void onLoop( unsigned long ticker ) override {
                if (l==0) {
                    l=1;
                    DBG("first loop call for "+i2ctype);
                }
            }

            virtual void onReceiveMessage( String topic, const char *pBuf,
                                           unsigned int len ) override {
                if ( topic == "i2cbus/online" ) {
                    i2cSetup(String(pBuf));
                }
            }
        };
    } // namespace base
} // namespace meisterwerk

#endif
