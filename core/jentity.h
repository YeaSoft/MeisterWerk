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
#include <map>

// dependencies
#include "../util/msgtime.h"
#include "entity.h"

namespace meisterwerk {
    namespace core {

        class jentity : public entity {
            public:
            // type definitions
            enum T_VALUETYPE { MW_COMMAND, MW_READ, MW_WRITE, MW_READWRITE };

            // members
            std::map<String, T_VALUETYPE> valueList;

            public:
            jentity( String name, unsigned long minMicroSecs, T_PRIO priority = PRIORITY_NORMAL )
                : entity( name, minMicroSecs, priority ) {
                subscribe( "info" );
                registerValue( "info", MW_READ );
            }

            void registerValue( const char *valueName, T_VALUETYPE valueType ) {
                valueList[valueName] = valueType;

                if ( valueType == MW_READ || valueType == MW_READWRITE ) {
                    subscribe( entName + "/" + valueName + "/get" );
                }
                if ( valueType == MW_WRITE || valueType == MW_READWRITE ) {
                    subscribe( entName + "/" + valueName + "/set" );
                }
                if ( valueType == MW_COMMAND ) {
                    subscribe( entName + "/" + valueName );
                }
            }

            void prepareData( JsonObject &data, const char *correlation = nullptr ) const {
                data["timer"] = millis();
                data["date"]  = util::msgtime::ISOnowMillis();
                data["name"]  = entName;
                if ( correlation ) {
                    data["correlation"] = correlation;
                }
            }

            bool publish( const char *topic, JsonObject &json ) const {
                String buffer;
                json.printTo( buffer );
                return entity::publish( topic, buffer.c_str() );
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

            virtual void setup() override {
                entity::setup();
            }

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                DynamicJsonBuffer reqBuffer( 256 );
                DynamicJsonBuffer resBuffer( 256 );
                JsonObject &      req = reqBuffer.parseObject( msg );
                JsonObject &      res = resBuffer.createObject();
                Topic             tpc = topic;
                /*
                Topic             tp1 = tpc.getfirst();
                Topic             tp2 = tpc.gettail().getfirst();
                Topic             tp3 = tp2.getnext();

                prepareData( res, req["correlation"] );

                if ( tpc == "info" || isOwnTopic( tpc, "info/get" ) ) {
                    if ( onGetInfo( res ) ) {
                        publish( ownTopic( "info" ).c_str(), res );
                    }
                    return;
                }
                */
                // onReceive( origin, topic, req, res );
            }

            // ABSTRACT METHODS - Must be implemented in derived classes
            virtual bool onGetInfo( JsonObject &info ) = 0;

            virtual bool onCommand( Topic &cmd, JsonObject &params, JsonObject & ) = 0;
        };

    } // namespace core
} // namespace meisterwerk
