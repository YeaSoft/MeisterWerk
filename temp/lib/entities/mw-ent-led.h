
#ifndef _MW_ENT_LED_H
#define _MW_ENT_LED_H

// states for buttons and leds
#define MW_STATE_OFF 0
#define MW_STATE_ON 1
#define MW_STATE_UNDEFINED 2

//--- LED task new style ------------------------------------------------------
#define LED_MODE_STATIC 0
#define LED_MODE_BLINK 1
class MW_Led : public MW_Entity {
    private:
    unsigned int ledPort;
    bool         bVerbose = false;

    unsigned int  ledMode             = LED_MODE_STATIC;
    unsigned int  ledState            = MW_STATE_OFF; // MW_STATE_*
    unsigned int  ledBlinkIntervallMs = 0;
    unsigned long ledLastChange       = 0;

    public:
    MW_Led( String eName, unsigned int port, bool verbose = false,
            unsigned long minMicroSecs = 50000L, unsigned int priority = MW_PRIORITY_NORMAL ) {
        entName  = eName;
        ledPort  = port;
        bVerbose = verbose;
        pinMode( ledPort, OUTPUT );

        Serial.println( "Registering LED " + entName );
        registerEntity( entName, this, &MW_Entity::loop, &MW_Entity::receiveMessage, minMicroSecs,
                        priority );
        subscribe( entName + "/setstate" );
        subscribe( entName + "/setmode" );
        subscribe( entName + "/setblinkMs" );
    }

    virtual void loop( unsigned long ticker ) override {
        if ( ledMode == LED_MODE_BLINK ) {
            unsigned long millis = ( ticker - ledLastChange ) / 1000L;
            if ( millis >= ledBlinkIntervallMs ) {
                ledLastChange = ticker;
                if ( ledState == MW_STATE_OFF ) {
                    setState( MW_STATE_ON );
                } else {
                    setState( MW_STATE_OFF );
                }
            }
        }
    }

    virtual void receiveMessage( String topic, char *pBuf, unsigned int len ) override {
        Serial.println( "msg recv! " + topic );
        if ( pBuf != nullptr )
            free( pBuf );
        return;
    }

    void setMode( unsigned int mode ) { // LED_MODE_{STATIC|BLINK}
        ledMode = mode;
    }
    void setState( unsigned int state ) {
        ledState = state;
        if ( ledState == MW_STATE_OFF ) {
            digitalWrite( ledPort, HIGH ); // Turn the LED off
            if ( bVerbose ) {
                publish( entName + "/hasstate", R"<>({"state":"off"})<>" );
            }
        } else {
            digitalWrite( ledPort, LOW ); // Turn the LED on
            if ( bVerbose ) {
                publish( entName + "/hasstate", R"<>({"state":"on"})<>" );
            }
        }
    }
    void setBlinkIntervallMs( unsigned int intervall ) {
        ledBlinkIntervallMs = intervall;
    }
};

#endif
