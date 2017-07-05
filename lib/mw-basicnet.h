/*
ESP Wlan & web configurator
*/

#define EE_SIZE 512         // Size of eeprom
#define EE_SSID_SIZE 32     // Max SSID size
#define EE_PWD_SIZE 32      // Max password size
#define EE_HOST_SIZE 32     // Max hostname size
#define EE_MQSERVER_SIZE 32 // Max server name of MQTT server
#define EE_SIG 0x7812       // Signature of a valid EEPROM content

// WLAN configuration states
#define ST_UNDEFINED 0       // Nothing
#define ST_NOTCONFIGURED 1   // No valid WLAN configuration, starting AP
#define ST_WAITFORCONFIG 2   // AP is started, waiting for User to enter data
#define ST_INITCONFIGURED 3  // User entered connection information, closing AP
#define ST_CONFIGURED 4      // Trying to connect to external AP
#define ST_CONNECTED 5       // Connected to AP
#define ST_STARTUPOK 6       // Cleanup
#define ST_NORMALOPERATION 7 // Normal operation, connected to external AP

typedef struct t_eeprom
{
    unsigned short int sig;  // Only, if sig is equal to EE_SIG, content of eeprom is considered valid
    unsigned short int uuid; // current micro-UUID
    char SSID[EE_SSID_SIZE]; // Network SSID
    char password[EE_PWD_SIZE];
    char localhostname[EE_HOST_SIZE];
    char mqttserver[EE_MQSERVER_SIZE];
    char _fill[EE_SIZE - EE_SSID_SIZE - EE_HOST_SIZE - EE_PWD_SIZE - EE_MQSERVER_SIZE - 2 * sizeof(short int)];
} T_EEPROM;

ESP8266WebServer mw_webserver(80);
const byte DNS_PORT = 53;
DNSServer mw_CaptiveDnsServer;

typedef struct t_mwbn
{
    String body;
#include "styles.cssh" // defines var 'stylesheet' that contains styles

    String header;
    String ipStr;
    int statusCode;
    String appName;

    T_EEPROM eepr;
    unsigned int uuid; // Micro UUID
    String uuidstr;
    String localhostname;
    // WLAN configuration states ST_*
    unsigned int state;
} T_MBWN;

T_MBWN tmwbn;

class MW_BasicNet
{
  private:
    bool debugMsg = false;

    static void initEEPROM(T_EEPROM *pep)
    {
        memset((void *)pep, 0, EE_SIZE);
        pep->sig = EE_SIG;
        pep->uuid = tmwbn.uuid;
    }

    void readEEPROM(T_EEPROM *pep)
    {
        unsigned char *buf = (unsigned char *)pep;
        for (int i = 0; i < EE_SIZE; i++)
        {
            buf[i] = EEPROM.read(i);
        }
    }

    static void writeEEPROM(T_EEPROM *pep)
    {
        unsigned char *buf = (unsigned char *)pep;
        for (int i = 0; i < EE_SIZE; i++)
        {
            EEPROM.write(i, buf[i]);
        }
        EEPROM.commit();
    }

    unsigned int genUUID()
    {
        unsigned int uuid = random(65536);
        return uuid;
    }

    String UUIDtoString(unsigned int uuid)
    {
        char uuidstr[16];
        itoa(uuid, uuidstr, 16);
        return String(uuidstr);
    }

    String defaultHostname()
    {
        return tmwbn.appName + "x" + UUIDtoString(tmwbn.uuid);
    }

    static void createBody()
    {
        tmwbn.body = "<!DOCTYPE HTML>\r\n<html>";
        tmwbn.body += tmwbn.stylesheet;
        tmwbn.body += "<div style=\"background-color: #e2e2f8\"><h3>" + tmwbn.header + "</h3>";
        tmwbn.body += tmwbn.appName + "-node at " + tmwbn.ipStr + ", " + tmwbn.localhostname + ", UUID=" + tmwbn.uuidstr + "</div><p></p>";
        tmwbn.body += "<div><form method='get' action='save'>"
                      "<label>SSID:     </label><input name='ssid' type='text' value='" +
                      String(tmwbn.eepr.SSID) + "' length=31><br>"
                                                "<label>Password: </label><input name='pass' type='password' value='" +
                      String(tmwbn.eepr.password) + "' length=31><br>"
                                                    "<label>Hostname: </label><input name='host' type='text' value='" +
                      String(tmwbn.eepr.localhostname) + "' length=31><br>"
                                                         "<label>MQTT Server: </label><input name='mqserver' type='text' value='" +
                      String(tmwbn.eepr.mqttserver) + "' length=31><br>"
                                                      "<input type='submit' value='Save'></form></div><p></p>";

        tmwbn.body += "<div><form method='get' action='factory'>"
                      "<label>Factory reset:</label><br><input type='submit' value='Reset'></form></div>";
        tmwbn.body += "</html>";
    }

    void runWebServer()
    {
        mw_webserver.on("/", []() {
            createBody();
            mw_webserver.send(200, "text/html", tmwbn.body);
        });
        mw_webserver.on("/save", []() {
            initEEPROM(&tmwbn.eepr);
            String ssid = mw_webserver.arg("ssid");
            String pwd = mw_webserver.arg("pass");
            String host = mw_webserver.arg("host");
            String mqsrv = mw_webserver.arg("mqserver");
            strncpy(tmwbn.eepr.SSID, ssid.c_str(), EE_SSID_SIZE - 1);
            strncpy(tmwbn.eepr.password, pwd.c_str(), EE_PWD_SIZE - 1);
            strncpy(tmwbn.eepr.localhostname, host.c_str(), EE_HOST_SIZE - 1);
            strncpy(tmwbn.eepr.mqttserver, mqsrv.c_str(), EE_MQSERVER_SIZE - 1);
            tmwbn.localhostname = String(tmwbn.eepr.localhostname);
            writeEEPROM(&tmwbn.eepr);
            tmwbn.body = "{\"Success\":\"saved " + ssid + " to eeprom.\"}";
            tmwbn.statusCode = 200;
            mw_webserver.send(tmwbn.statusCode, "application/json", tmwbn.body);
            tmwbn.state = ST_INITCONFIGURED;
        });
        mw_webserver.on("/factory", []() {
            memset((unsigned char *)&tmwbn.eepr, 0, sizeof(T_EEPROM));
            writeEEPROM(&tmwbn.eepr);
            tmwbn.body = "{\"Success\":\"erased eeprom.\"}";
            tmwbn.statusCode = 200;
            mw_webserver.send(tmwbn.statusCode, "application/json", tmwbn.body);
            tmwbn.state = ST_NOTCONFIGURED;
        });
        mw_webserver.onNotFound([]() {
            createBody();
            mw_webserver.send(200, "text/html", tmwbn.body);
        });

        mw_webserver.begin();
    }

    // Web server that runs during initial configuration, AP is ESP-module.
    void initialConfigServer()
    {
        IPAddress ip = WiFi.softAPIP();
        tmwbn.ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' +
                      String(ip[2]) + '.' + String(ip[3]);
        tmwbn.header = "Access point mode";
        runWebServer();
    }

    // Web server that runs if connected to external access point (normal operation)
    void startConfigServer()
    {
        IPAddress ip = WiFi.localIP();
        tmwbn.ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' +
                      String(ip[2]) + '.' + String(ip[3]);
        tmwbn.header = "Client mode";
        Serial.println(tmwbn.ipStr);
        runWebServer();
    }

    // create local AP for initial config
    bool createAP(String name, String password = "")
    {
        IPAddress apIP(10, 1, 1, 1); // Private network address: local & gateway
        IPAddress apNet(255, 255, 255, 0);
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        //WiFi.mode(WIFI_AP_STA);
        WiFi.softAPConfig(apIP, apIP, apNet);
        if (password == "")
            WiFi.softAP(name.c_str());
        else
            WiFi.softAP(name.c_str(), password.c_str());
        WiFi.hostname(tmwbn.localhostname.c_str());

        /* Setup Captive DNS server redirecting all the domains to the apIP */
        delay(500);
        mw_CaptiveDnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        mw_CaptiveDnsServer.start(DNS_PORT, "*", apIP);

        tmwbn.state = ST_WAITFORCONFIG;
        initialConfigServer();
    }

    unsigned int connectTimeout = 30; // Number seconds, a connect to configured access point is tried,
                                      // before a local access point is spawned.
    // connect to external AP, create a local AP if cnnection fails for connectTimeout secs.
    bool connectToAp()
    {
        WiFi.mode(WIFI_STA);
        if (debugMsg)
            Serial.println("Connecting to " + String(tmwbn.eepr.SSID) + ", " + String(tmwbn.eepr.password));
        WiFi.begin(tmwbn.eepr.SSID, tmwbn.eepr.password);
        WiFi.hostname(tmwbn.localhostname.c_str());
        digitalWrite(LED_BUILTIN, LOW); // LED on during connect-attempt
        for (int t = 0; t < connectTimeout * 10; t++)
        { // connectTimeout sec. timeout.
            if (WiFi.status() == WL_CONNECTED)
            {
                tmwbn.state = ST_CONNECTED;
                if (debugMsg)
                    Serial.println("Connection successful!");
                return true;
            }
            delay(100);
        }
        WiFi.disconnect();
        if (debugMsg)
            Serial.println("Connection failed!");
        tmwbn.state = ST_NOTCONFIGURED; // This needs to be removed (or adapted) for Leo-Use-Case   XXX-LEO
                                        // Setting state to NOTCONFIGURED causes spawning of local access point.
                                        // In order to retry connection with existing configuration, simply set to ST_CONFIGURED.
        return false;
    }

  public:
    MW_BasicNet(String applicationName, unsigned int connectionTimeout = 20, bool bDebug = false)
    {
        tmwbn.appName = applicationName;
        tmwbn.uuid = 0;
        tmwbn.uuidstr = "";
        debugMsg = bDebug;
        connectTimeout = connectionTimeout;
        tmwbn.state = ST_UNDEFINED;
    }

    T_EEPROM getEEPROM()
    {
        return tmwbn.eepr;
    }

    void enterAccessPointMode()
    {
        WiFi.mode(WIFI_OFF);
        delay(1000);
        tmwbn.state = ST_NOTCONFIGURED;
    }

    void clearEEPROM()
    {
        memset(&tmwbn.eepr, 0, EE_SIZE);
        writeEEPROM(&tmwbn.eepr);
    }

    void begin()
    {
        EEPROM.begin(512);
        tmwbn.state = ST_CONFIGURED;
        readEEPROM(&tmwbn.eepr);
        if (tmwbn.eepr.sig != EE_SIG)
        {
            tmwbn.uuid = genUUID();
            if (debugMsg)
                Serial.println("New UUID generated.");
            initEEPROM(&tmwbn.eepr);
            tmwbn.localhostname = defaultHostname();
            strncpy(tmwbn.eepr.localhostname, tmwbn.localhostname.c_str(), EE_HOST_SIZE - 1);
            writeEEPROM(&tmwbn.eepr);
        }
        else
        {
            tmwbn.uuid = tmwbn.eepr.uuid;
            tmwbn.localhostname = String(tmwbn.eepr.localhostname);
        }
        tmwbn.uuidstr = UUIDtoString(tmwbn.uuid);
        if (debugMsg)
            Serial.println("Unique name: " + tmwbn.localhostname);
        if (tmwbn.eepr.SSID[0] == 0)
        {
            if (debugMsg)
                Serial.println("State: ST_NOTCONFIGURED");
            tmwbn.state = ST_NOTCONFIGURED;
        }
        else
        {
            if (debugMsg)
                Serial.println("State: ST_CONFIGURED");
            tmwbn.state = ST_CONFIGURED;
        }
    }

    bool handleCom()
    {
        bool ret = false;
        switch (tmwbn.state)
        {
        case ST_NOTCONFIGURED:
            Serial.println("Unconfigured server, creating AP");
            tmwbn.localhostname = defaultHostname();
            createAP(tmwbn.localhostname); // use localhostname as network-name on initial setup.
            break;
        case ST_WAITFORCONFIG:
            //Captive DNS
            mw_CaptiveDnsServer.processNextRequest();
            break;
        case ST_INITCONFIGURED:
            Serial.println("Received connection information");
            mw_webserver.stop();
            mw_CaptiveDnsServer.stop();
            WiFi.disconnect();
            tmwbn.state = ST_CONFIGURED;
            break;
        case ST_CONFIGURED:
            Serial.println("Configured, trying connect to AP");
            connectToAp();
            break;
        case ST_CONNECTED:
            Serial.println("Connected to AP");
            startConfigServer();
            tmwbn.state = ST_STARTUPOK;
            break;
        case ST_STARTUPOK:
            Serial.println("Ready for things to be done.");
            tmwbn.state = ST_NORMALOPERATION;
            break;
        case ST_NORMALOPERATION:
            ret = true;
            break;
        }

        mw_webserver.handleClient(); // handle web requests

        return ret;
    }
};
