// jentity.h - A special entity class
//
// The jentity class is a special entity class with
// a specific semantic. It supports the concept
// of named values that will be spontaneously
// published when needed (e.g. on change), explicitely
// queried and optionally set from outside.
// In addition the class implements the pseudo
// value "info" that contains all information about
// the entity.
// Any message payload of this special entity class
// is strictly formatted as json

#pragma once

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../util/hextools.h"
#include "../util/msgtime.h"
#include "array.h"
#include "entity.h"

namespace meisterwerk {
    namespace core {

        class jentity : public entity {
            protected:
            // type definitions
            class controlword {
                public:
                static const unsigned char REACT  = 1;
                static const unsigned char NOTIFY = 2;
                static const unsigned char READ   = 4;
                static const unsigned char WRITE  = 8;
                String                     cwname;
                unsigned char              cwtype;
                controlword() {
                    cwtype = 0;
                }
                controlword( const char *cwname, unsigned char cwtype ) : cwname{cwname}, cwtype{cwtype} {
                }
            };

            protected:
            // members
            array<controlword> wordList;
            String             i2ctype;
            uint8_t            address;
            bool               isInstantiated = false;
            bool               bValidTime     = false;

            public:
            // Constructor for non i2c entities
            jentity( String name, unsigned long minMicroSecs, T_PRIO priority = PRIORITY_NORMAL,
                     unsigned int wordListSize = 16 )
                : entity( name, minMicroSecs, priority ), wordList( wordListSize ) {
                commonInit();
            }

            // Constructor for i2c entities
            jentity( String name, String i2cType, uint8_t address, unsigned int wordListSize = 16 )
                : meisterwerk::core::entity( name, 50000 ),
                  wordList( wordListSize ), i2ctype{i2cType}, address{address} {
                // 50 ms bus poll interval
                DBG( "i2ctype:" + i2ctype );
                commonInit();
            }

            // entity overridables
            virtual void setup() override {
                if ( i2ctype.length() > 0 ) {
                    // i2c entity
                    subscribe( "i2cbus/devices" );
                    entity::publish( "i2cbus/devices/get" );
                }
            }

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                DynamicJsonBuffer reqBuffer( 256 );
                DynamicJsonBuffer resBuffer( 256 );
                JsonObject &      req = reqBuffer.parseObject( msg );
                JsonObject &      res = resBuffer.createObject();
                Topic             tpc = topic;

                prepareData( res );

                if ( tpc == "i2cbus/devices" && i2ctype.length() > 0 ) {
                    if ( !isInstantiated ) {
                        i2cSetup( req, res );
                    }
                } else if ( tpc == "mastertime/time/set" ) {
                    bValidTime = true;
                } else if ( tpc == "info" || tpc == entName + "/info/get" ) {
                    prepareInfo( res );
                    onGetValue( "info", req, res );
                } else if ( tpc.match( entName + "/+" ) ) {
                    onReaction( tpc.gettail(), req, res );
                } else if ( tpc.match( entName + "/+/get" ) ) {
                    onGetValue( tpc.gettail().getfirst(), req, res );
                } else if ( tpc.match( entName + "/+/set" ) ) {
                    onSetValue( tpc.gettail().getfirst(), req, res );
                } else {
                    onMessage( topic, req, res );
                }
            }

            // jentity overridables
            virtual void onInstantiate( String i2ctype, uint8_t address ) {
                DBG( "Your code should override this function and instantiate a " + i2ctype + " device at address 0x" +
                     meisterwerk::util::hexByte( address ) );
            }
            virtual void onReaction( String cmd, JsonObject &params, JsonObject &data ) {
            }
            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) {
            }
            virtual void onSetValue( String value, JsonObject &params, JsonObject &data ) {
            }
            virtual void onMessage( Topic topic, JsonObject &params, JsonObject &data ) {
            }

            bool notify( const char *name, JsonObject &json ) const {
                String buffer;
                json.printTo( buffer );
                return entity::publish( entName + "/" + name, buffer );
            }

            bool notify( const String &name, JsonObject &json ) const {
                String buffer;
                json.printTo( buffer );
                return entity::publish( entName + "/" + name, buffer );
            }

            bool publish( const char *topic, JsonObject &json ) const {
                String buffer;
                json.printTo( buffer );
                return entity::publish( topic, buffer.c_str() );
            }

            bool publish( const String &topic, JsonObject &json ) const {
                String buffer;
                json.printTo( buffer );
                return entity::publish( topic.c_str(), buffer.c_str() );
            }

            void Reaction( const char *wordName, bool bGeneric = false ) {
                registerWord( wordName, controlword::REACT );
            }

            void Notifies( const char *wordName ) {
                registerWord( wordName, controlword::NOTIFY );
            }

            void SensorValue( const char *wordName ) {
                registerWord( wordName, controlword::NOTIFY | controlword::READ );
            }

            void ReadOnly( const char *wordName ) {
                registerWord( wordName, controlword::READ );
            }

            void Setting( const char *wordName ) {
                registerWord( wordName, controlword::READ | controlword::WRITE );
            }

            void SettableState( const char *wordName ) {
                registerWord( wordName, controlword::NOTIFY | controlword::READ | controlword::WRITE );
            }

            void registerWord( const char *wordName, unsigned char wordType ) {
                controlword cw( wordName, wordType );
                wordList.add( cw );

                if ( wordType & controlword::REACT ) {
                    subscribe( entName + "/" + wordName );
                }
                if ( wordType & controlword::READ ) {
                    subscribe( entName + "/" + wordName + "/get" );
                }
                if ( wordType & controlword::WRITE ) {
                    subscribe( entName + "/" + wordName + "/set" );
                }
            }

            void prepareData( JsonObject &data ) const {
                data["name"]  = entName;
                data["timer"] = millis();
                if ( bValidTime ) {
                    data["time"] = util::msgtime::ISOnowMillis();
                }
            }

            void prepareInfo( JsonObject &data ) const {
                JsonArray &react  = data.createNestedArray( "react" );
                JsonArray &notify = data.createNestedArray( "notify" );
                JsonArray &read   = data.createNestedArray( "read" );
                JsonArray &write  = data.createNestedArray( "write" );

                for ( unsigned int i = 0; i < wordList.length(); i++ ) {
                    if ( wordList[i].cwtype & controlword::REACT ) {
                        react.add( wordList[i].cwname );
                    }
                    if ( wordList[i].cwtype & controlword::NOTIFY ) {
                        notify.add( wordList[i].cwname );
                    }
                    if ( wordList[i].cwtype & controlword::READ ) {
                        read.add( wordList[i].cwname );
                    }
                    if ( wordList[i].cwtype & controlword::WRITE ) {
                        write.add( wordList[i].cwname );
                    }
                }
            }

            String ownTopic( String &subtopic ) const {
                return entName + "/" + subtopic;
            }

            String ownTopic( const char *subtopic ) const {
                return entName + "/" + subtopic;
            }

            bool isOwnTopic( String &topic, String &subtopic ) const {
                return topic == entName + "/" + subtopic;
            }

            // internal helpers
            protected:
            void commonInit() {
                subscribe( "info" );
                subscribe( "mastertime/time/set" );
                registerWord( "info", controlword::READ );
            }

            void i2cSetup( JsonObject &params, JsonObject &data ) {
                if ( !params.success() ) {
                    DBG( "jentity::i2cSetup - Invalid JSON received" );
                    return;
                }
                JsonArray &devs = params["devices"];
                for ( int i = 0; i < devs.size(); i++ ) {
                    JsonObject &dev = devs[i];
                    for ( auto obj : dev ) {
                        DBG( String( obj.key ) + "<->" + i2ctype );
                        if ( i2ctype == obj.key && address == obj.value ) {
                            onInstantiate( i2ctype, address );
                            isInstantiated = true;
                            return;
                        }
                    }
                }
            }
        };

    } // namespace core
} // namespace meisterwerk
