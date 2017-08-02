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
                static const unsigned char REACT   = 1;
                static const unsigned char NOTIFY  = 2;
                static const unsigned char READ    = 4;
                static const unsigned char WRITE   = 8;
                static const unsigned char GENERIC = 64;
                String                     name;
                unsigned char              type;
                bool                       generic;
                controlword() {
                    type = 0;
                }
                controlword( const char *name, unsigned char type ) : name{name}, type{type} {
                }

                bool shouldReact( String &entName, Topic &topic ) const {
                    return shouldDo( entName, topic, REACT, "" );
                }

                bool shouldRead( String &entName, Topic &topic ) const {
                    return shouldDo( entName, topic, READ, "/get" );
                }

                bool shouldWrite( String &entName, Topic &topic ) const {
                    return shouldDo( entName, topic, READ, "/get" );
                }

                bool shouldDo( String &entName, Topic &topic, unsigned char flag, const char *suffix ) const {
                    if ( type & flag ) {
                        if ( topic == entName + "/" + name + suffix ) {
                            return true;
                        }
                        if ( type & GENERIC ) {
                            return topic == "/" + name + suffix;
                        }
                    }
                    return false;
                }
            };

            protected:
            // members
            array<controlword> wordList;
            bool               bValidTime  = false;
            bool               bStrictMode = false;

            public:
            // Constructor for non i2c entities
            jentity( String name, unsigned long minMicroSecs, T_PRIO priority = PRIORITY_NORMAL,
                     unsigned int wordListSize = 16 )
                : entity( name, minMicroSecs, priority ), wordList( wordListSize ) {
                Reading( "info", true );
                subscribe( "mastertime/time/set" );
            }

            // entity overridables

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                if ( !strcmp( topic, "mastertime/time/set" ) ) {
                    bValidTime = true;
                    return;
                }

                DynamicJsonBuffer reqBuffer( 256 );
                DynamicJsonBuffer resBuffer( 256 );
                JsonObject &      req = reqBuffer.parseObject( msg );
                JsonObject &      res = resBuffer.createObject();
                Topic             tpc = topic;

                prepareData( res );

                for ( unsigned int i = 0; i < wordList.length(); i++ ) {
                    if ( wordList[i].shouldReact( entName, tpc ) ) {
                        onReaction( wordList[i].name, req, res );
                        return;
                    } else if ( wordList[i].shouldRead( entName, tpc ) ) {
                        onGetValue( wordList[i].name, req, res );
                        return;
                    } else if ( wordList[i].shouldWrite( entName, tpc ) ) {
                        onSetValue( wordList[i].name, req, res );
                        return;
                    }
                }

                // was not processed by registered control words
                if ( bStrictMode ) {
                    onMessage( topic, req, res );
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
            virtual void onReaction( String cmd, JsonObject &params, JsonObject &data ) {
            }
            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) {
            }
            virtual void onSetValue( String value, JsonObject &params, JsonObject &data ) {
            }
            virtual void onMessage( Topic topic, JsonObject &params, JsonObject &data ) {
            }

            // jentity methods
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

            void Reaction( const char *wordName, bool generic = false ) {
                registerWord( wordName, controlword::REACT | ( generic ? controlword::GENERIC : 0 ) );
            }

            void Notifies( const char *wordName, bool generic = false ) {
                registerWord( wordName, controlword::NOTIFY | ( generic ? controlword::GENERIC : 0 ) );
            }

            void SensorValue( const char *wordName, bool generic = false ) {
                registerWord( wordName,
                              controlword::NOTIFY | controlword::READ | ( generic ? controlword::GENERIC : 0 ) );
            }

            void Reading( const char *wordName, bool generic = false ) {
                registerWord( wordName, controlword::READ | ( generic ? controlword::GENERIC : 0 ) );
            }

            void Setting( const char *wordName, bool generic = false ) {
                registerWord( wordName,
                              controlword::READ | controlword::WRITE | ( generic ? controlword::GENERIC : 0 ) );
            }

            void SettableState( const char *wordName, bool generic = false ) {
                registerWord( wordName,
                              controlword::NOTIFY | controlword::READ | controlword::WRITE |
                                  ( generic ? controlword::GENERIC : 0 ) );
            }

            void registerWord( const char *wordName, unsigned char wordType ) {
                controlword cw( wordName, wordType );
                wordList.add( cw );

                if ( wordType & controlword::REACT ) {
                    subscribe( entName + "/" + wordName );
                    if ( wordType & controlword::GENERIC ) {
                        subscribe( wordName );
                    }
                }
                if ( wordType & controlword::READ ) {
                    subscribe( entName + "/" + wordName + "/get" );
                    if ( wordType & controlword::GENERIC ) {
                        subscribe( String( wordName ) + "/get" );
                    }
                }
                if ( wordType & controlword::WRITE ) {
                    subscribe( entName + "/" + wordName + "/set" );
                    if ( wordType & controlword::GENERIC ) {
                        subscribe( String( wordName ) + "/set" );
                    }
                }
            }

            void prepareData( JsonObject &data ) const {
                data["name"]  = entName;
                data["timer"] = millis();
            }

            void prepareTimestamp( JsonObject &data, const char *valueName = "time",
                                   String ts = util::msgtime::ISOnowMillis() ) const {
                if ( bValidTime ) {
                    data[valueName] = ts;
                }
            }

            void prepareInfo( JsonObject &data ) const {
                JsonArray &react  = data.createNestedArray( "react" );
                JsonArray &notify = data.createNestedArray( "notify" );
                JsonArray &read   = data.createNestedArray( "read" );
                JsonArray &write  = data.createNestedArray( "write" );

                for ( unsigned int i = 0; i < wordList.length(); i++ ) {
                    if ( wordList[i].type & controlword::REACT ) {
                        react.add( wordList[i].name );
                    }
                    if ( wordList[i].type & controlword::NOTIFY ) {
                        notify.add( wordList[i].name );
                    }
                    if ( wordList[i].type & controlword::READ ) {
                        read.add( wordList[i].name );
                    }
                    if ( wordList[i].type & controlword::WRITE ) {
                        write.add( wordList[i].name );
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
        };

    } // namespace core
} // namespace meisterwerk
