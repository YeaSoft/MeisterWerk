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
                static const unsigned char REACTION = 1;
                static const unsigned char NOTIFIED = 2;
                static const unsigned char GETTABLE = 4;
                static const unsigned char SETTABLE = 8;
                String                     cwname;
                unsigned char              cwtype;
                controlword() {
                    cwtype = 0;
                }
                controlword( const char *cwname, unsigned char cwtype ) : cwname{cwname}, cwtype{cwtype} {
                }

                bool subscribe( jentity *pe ) const {
                    if ( cwtype & controlword::REACTION ) {
                        pe->subscribe( pe->entName + "/" + cwname );
                    }
                    if ( cwtype & controlword::GETTABLE ) {
                        pe->subscribe( pe->entName + "/" + cwname + "/get" );
                    }
                    if ( cwtype & controlword::SETTABLE ) {
                        pe->subscribe( pe->entName + "/" + cwname + "/set" );
                    }
                }
            };

            array<controlword> wordList;

            public:
            // members
            bool bValidTime = false;

            public:
            jentity( String name, unsigned long minMicroSecs, T_PRIO priority = PRIORITY_NORMAL,
                     unsigned int wordListSize = 16 )
                : entity( name, minMicroSecs, priority ), wordList( wordListSize ) {
                subscribe( "info" );
                subscribe( "mastertime/time/set" );
                registerWord( "info", controlword::GETTABLE );
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

            void Reaction( const char *wordName ) {
                registerWord( wordName, controlword::REACTION );
            }

            void Notifies( const char *wordName ) {
                registerWord( wordName, controlword::NOTIFIED );
            }

            void SensorValue( const char *wordName ) {
                registerWord( wordName, controlword::NOTIFIED | controlword::GETTABLE );
            }

            void ReadOnly( const char *wordName ) {
                registerWord( wordName, controlword::GETTABLE );
            }

            void Setting( const char *wordName ) {
                registerWord( wordName, controlword::GETTABLE | controlword::SETTABLE );
            }

            void SettableState( const char *wordName ) {
                registerWord( wordName, controlword::NOTIFIED | controlword::GETTABLE | controlword::SETTABLE );
            }

            void registerWord( const char *wordName, unsigned char wordType ) {
                controlword cw( wordName, wordType );
                wordList.add( cw );

                if ( wordType & controlword::REACTION ) {
                    subscribe( entName + "/" + wordName );
                }
                if ( wordType & controlword::GETTABLE ) {
                    subscribe( entName + "/" + wordName + "/get" );
                }
                if ( wordType & controlword::SETTABLE ) {
                    subscribe( entName + "/" + wordName + "/set" );
                }
            }

            void prepareData( JsonObject &data, const char *correlation = nullptr ) const {
                data["name"]  = entName;
                data["timer"] = millis();
                if ( bValidTime ) {
                    data["time"] = util::msgtime::ISOnowMillis();
                }
                if ( correlation ) {
                    data["correlation"] = correlation;
                }
            }

            void prepareInfo( JsonObject &data ) const {
                JsonArray &reaction = data.createNestedArray( String( "reaction" ) );
                JsonArray &notified = data.createNestedArray( String( "notified" ) );
                JsonArray &gettable = data.createNestedArray( String( "gettable" ) );
                JsonArray &settable = data.createNestedArray( String( "settable" ) );

                for ( unsigned int i = 0; i < wordList.length(); i++ ) {
                    if ( wordList[i].cwtype & controlword::REACTION ) {
                        reaction.add( wordList[i].cwname );
                    }
                    if ( wordList[i].cwtype & controlword::NOTIFIED ) {
                        notified.add( wordList[i].cwname );
                    }
                    if ( wordList[i].cwtype & controlword::GETTABLE ) {
                        gettable.add( wordList[i].cwname );
                    }
                    if ( wordList[i].cwtype & controlword::SETTABLE ) {
                        settable.add( wordList[i].cwname );
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

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                DynamicJsonBuffer reqBuffer( 256 );
                DynamicJsonBuffer resBuffer( 256 );
                JsonObject &      req = reqBuffer.parseObject( msg );
                JsonObject &      res = resBuffer.createObject();
                Topic             tpc = topic;

                prepareData( res, req["correlation"] );

                if ( tpc == "mastertime/time/set" ) {
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

            virtual void onReaction( String cmd, JsonObject &params, JsonObject &data ) {
            }
            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) {
            }
            virtual void onSetValue( String value, JsonObject &params, JsonObject &data ) {
            }
            virtual void onMessage( Topic topic, JsonObject &params, JsonObject &data ) {
            }
        };

    } // namespace core
} // namespace meisterwerk
