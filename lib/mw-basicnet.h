/*
ESP Wlan & web configurator
*/

#define EE_SIZE 512     // Size of eeprom
#define EE_SSID_SIZE 32 // Max SSID size
#define EE_PWD_SIZE 32  // Max password size
#define EE_HOST_SIZE 32 // Max hostname size
#define EE_SIG 0x7812   // Signature of a valid EEPROM content

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
    char _fill[EE_SIZE - EE_SSID_SIZE - EE_HOST_SIZE - EE_PWD_SIZE - 2 * sizeof(short int)];
} T_EEPROM;

ESP8266WebServer webserver(80);

    String stylesheet = R"MWx(<style>
        input[type=text], select {
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 18px;
        }

        input[type=password], select {
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 18px;
        }

        input[type=submit] {
            width: 100%;
            background-color: #7C7CE0;
            color: white;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 18px;
        }

        input[type=submit]:hover {
            background-color: #4549a0;
        }

        div {
            width: 640px;
            border-radius: 5px;
            background-color: #f2f2f2;
            padding: 20px;
            font-family: Verdana, Arial, Sans-serif;
        }</style>)MWx";
String body;
String header;
String ipStr;
int statusCode;
String appName = "";

T_EEPROM eepr;
unsigned int uuid = 0; // Micro UUID
String uuidstr="";
String localhostname;
// WLAN configuration states ST_*
unsigned int state = ST_UNDEFINED;


class MW_BasicNet
{
  public:
    bool debugMsg = false;

    MW_BasicNet(String applicationName, bool bDebug = false)
    {
        appName = applicationName;
        debugMsg = bDebug;
    }

    static void initEEPROM(T_EEPROM *pep)
    {
        memset((void *)pep, 0, EE_SIZE);
        pep->sig = EE_SIG;
        pep->uuid = uuid;
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

    void begin()
    {
        EEPROM.begin(512);
        state = ST_CONFIGURED;
        readEEPROM(&eepr);
        if (eepr.sig != EE_SIG)
        {
            uuid = genUUID();
            if (debugMsg)
                Serial.println("New UUID generated.");
            initEEPROM(&eepr);
            localhostname = defaultHostname();
            strncpy(eepr.localhostname, localhostname.c_str(), EE_HOST_SIZE - 1);
            writeEEPROM(&eepr);
        }
        else
        {
            uuid = eepr.uuid;
            localhostname = String(eepr.localhostname);
        }
        uuidstr=UUIDtoString(uuid);
        if (debugMsg)
            Serial.println("Unique name: " + localhostname);
        if (eepr.SSID[0] == 0)
        {
            if (debugMsg)
                Serial.println("State: ST_NOTCONFIGURED");
            state = ST_NOTCONFIGURED;
        }
        else
        {
            if (debugMsg)
                Serial.println("State: ST_CONFIGURED");
            state = ST_CONFIGURED;
        }
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
        return appName + "x" + UUIDtoString(uuid);
    }

    void runWebServer()
    {
        webserver.on("/", []() {
            body = "<!DOCTYPE HTML>\r\n<html>";
            body += stylesheet;
            body += "<div style=\"background-color: #e2e2f8\"><h3>" + header + "</h3>";
            body += appName + "-node at " + ipStr + ", " + localhostname + ", UUID=" + uuidstr + "</div><p></p>";
            body += "<div><form method='get' action='save'>"
                    "<label>SSID:     </label><input name='ssid' type='text' value='" +
                    String(eepr.SSID) + "' length=31><br>"
                                        "<label>Password: </label><input name='pass' type='password' value='" +
                    String(eepr.password) + "' length=31><br>"
                                            "<label>Hostname: </label><input name='host' type='text' value='" +
                    String(eepr.localhostname) + "' length=31><br>"
                                                 "<input type='submit' value='Save'></form></div><p></p>";
            body += "<div><form method='get' action='factory'>"
                    "<label>Factory reset:</label><br><input type='submit' value='Reset'></form></div>";
            body += "</html>";
            webserver.send(200, "text/html", body);
        });
        webserver.on("/save", []() {
            initEEPROM(&eepr);
            String ssid = webserver.arg("ssid");
            String pwd = webserver.arg("pass");
            String host = webserver.arg("host");
            strncpy(eepr.SSID, ssid.c_str(), EE_SSID_SIZE - 1);
            strncpy(eepr.password, pwd.c_str(), EE_PWD_SIZE - 1);
            strncpy(eepr.localhostname, host.c_str(), EE_HOST_SIZE - 1);
            localhostname = String(eepr.localhostname);
            writeEEPROM(&eepr);
            body = "{\"Success\":\"saved " + ssid + " to eeprom.\"}";
            statusCode = 200;
            webserver.send(statusCode, "application/json", body);
            state = ST_INITCONFIGURED;
        });
        webserver.on("/factory", []() {
            memset((unsigned char *)&eepr, 0, sizeof(T_EEPROM));
            writeEEPROM(&eepr);
            body = "{\"Success\":\"erased eeprom.\"}";
            statusCode = 200;
            webserver.send(statusCode, "application/json", body);
            state = ST_NOTCONFIGURED;
        });

        webserver.begin();
    }

    // Web server that runs during initial configuration, AP is ESP-module.
    void initialConfigServer()
    {
        IPAddress ip = WiFi.softAPIP();
        ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' +
                String(ip[2]) + '.' + String(ip[3]);
        header = "Access point mode";
        runWebServer();
    }

    // Web server that runs if connected to external access point (normal operation)
    void startConfigServer()
    {
        IPAddress ip = WiFi.localIP();
        ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' +
                String(ip[2]) + '.' + String(ip[3]);
        header = "Client mode";
        runWebServer();
    }

    // create local AP for initial config
    bool createAP(String name, String password = "")
    {
        IPAddress apIP(10, 1, 1, 1); // Private network address: local & gateway
        IPAddress apNet(255, 255, 255, 0);
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAPConfig(apIP, apIP, apNet);
        if (password == "")
            WiFi.softAP(name.c_str());
        else
            WiFi.softAP(name.c_str(), password.c_str());
        WiFi.hostname(localhostname.c_str());
        state = ST_WAITFORCONFIG;
        initialConfigServer();
    }

    unsigned int connectTimeout = 30; // Number seconds, a connect to configured access point is tried,
                                      // before a local access point is spawned.
    // connect to external AP, create a local AP if cnnection fails for connectTimeout secs.
    bool connectToAp()
    {
        WiFi.mode(WIFI_STA);
        if (debugMsg)
            Serial.println("Connecting to " + String(eepr.SSID) + ", " + String(eepr.password));
        WiFi.begin(eepr.SSID, eepr.password);
        WiFi.hostname(localhostname.c_str());
        digitalWrite(LED_BUILTIN, LOW); // LED on during connect-attempt
        for (int t = 0; t < connectTimeout * 10; t++)
        { // connectTimeout sec. timeout.
            if (WiFi.status() == WL_CONNECTED)
            {
                state = ST_CONNECTED;
                if (debugMsg)
                    Serial.println("Connection successful!");
                return true;
            }
            delay(100);
        }
        WiFi.disconnect();
        if (debugMsg)
            Serial.println("Connection failed!");
        state = ST_NOTCONFIGURED; // This needs to be removed (or adapted) for Leo-Use-Case   XXX-LEO
                                  // Setting state to NOTCONFIGURED causes spawning of local access point.
                                  // In order to retry connection with existing configuration, simply set to ST_CONFIGURED.
        return false;
    }

    bool handleCom()
    {
        bool ret=false;
        switch (state)
        {
        case ST_NOTCONFIGURED:
            Serial.println("Unconfigured server, creating AP");
            localhostname = defaultHostname();
            createAP(localhostname); // use localhostname as network-name on initial setup.
            break;
        case ST_WAITFORCONFIG:
            break;
        case ST_INITCONFIGURED:
            Serial.println("Received connection information");
            webserver.stop();
            WiFi.disconnect();
            state = ST_CONFIGURED;
            break;
        case ST_CONFIGURED:
            Serial.println("Configured, trying connect to AP");
            connectToAp();
            break;
        case ST_CONNECTED:
            Serial.println("Connected to AP");
            startConfigServer();
            state = ST_STARTUPOK;
            break;
        case ST_STARTUPOK:
            Serial.println("Ready for things to be done.");
            state = ST_NORMALOPERATION;
            ret=true;
            break;
        case ST_NORMALOPERATION:
            break;
        }

        webserver.handleClient(); // handle web requests
        return ret;
    }
};

