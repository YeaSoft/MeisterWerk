/* MQTT support stub */

WiFiClient mw_mq_wf_client;
PubSubClient mw_mq_client(mw_mq_wf_client);

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

class MW_MQTT
{
  private:
    String mq_server;
    bool mqState;

    void reconnect() // XXX this hammers on failure -- needs timed block
    {
        // Loop until we're reconnected
        if (!mw_mq_client.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Create a random client ID
            String clientId = "ESP8266Client-";
            clientId += String(random(0xffff), HEX);
            // Attempt to connect
            if (mw_mq_client.connect(clientId.c_str()))
            {
                Serial.println("connected");
                // Once connected, publish an announcement...
                mw_mq_client.publish("outTopic", "hello world");
                // ... and resubscribe
                mw_mq_client.subscribe("inTopic");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(mw_mq_client.state());
            }
        }
    }

  public:
    MW_MQTT(String mqtt_server)
    {
        mq_server = mqtt_server;
        mqState = false;
    }
    bool mqStarted()
    {
        return mqState;
    }
    void begin()
    {
        char mqS[128];
        strncpy(mqS, mq_server.c_str(), 127);
        mw_mq_client.setServer(mq_server.c_str(), 1883);
        mw_mq_client.setCallback(callback);
        mqState = true;
    }
    void publish(String topic, String msg)
    {
        mw_mq_client.publish(topic.c_str(), msg.c_str());
    }
    void handleMQTT()
    {
        if (!mw_mq_client.connected())
        {
            reconnect();
        }
        mw_mq_client.loop();
    }
};
