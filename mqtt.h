#ifndef mqtt_h
#define mqtt_h

#undef nullptr

#define USEJSON
#ifdef USEJSON
#include <ArduinoJson.h>
StaticJsonDocument<2048> payload;
StaticJsonDocument<2048> rootdoc;
// payload["sensor"] = "gps";
// payload["time"] = 1351824120;

  // const size_t CAPACITY = JSON_ARRAY_SIZE(1);
  // StaticJsonDocument<CAPACITY> docH;
  // JsonArray arrayH = docH.to<JsonArray>();
  DynamicJsonDocument pubjson(2048);
  JsonArray jsondata = pubjson.to<JsonArray>();
#endif

#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 2048

WiFiClient espClient;

PubSubClient client(espClient);

bool debug_mqtt = true;
String clientID = "";

void MQTTreconnect() {

  // Loop until we're reconnected
  // this is blocking
  while (!client.connected()) {
    Logger.println("[MQTT] Connecting...");
    // Attempt to connect
    if (client.connect(clientID.c_str())) {
      Logger.println("[MQTT] Connected");
      // Once connected, publish an announcement...
      client.publish("TESTOUT", "hello world");
      // ... and resubscribe
      client.subscribe("TESTIN");
    } else {
      Logger.print("[ERROR] [MQTT] failed, rc=");
      Logger.print(client.state());
      Logger.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      // delay(5000);
    }
    delay(100);
  }
}


void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  Logger.print("[MQTT] IN Message arrived [");
  Logger.print(topic);
  Logger.print("] ");
  Logger.print("payload: ");
  for (int i = 0; i < length; i++) {
    Logger.print((char)payload[i]);
  }
  Logger.println("");
  if ((char)payload[0] == '1') {
    Logger.println("[MQTT] payload: TRIGGERED");
  }
}

long lastReconnectAttempt = 0;
uint16_t mqttretry = 5000;

boolean mqttReconnect() {
  WiFi.reconnect();
  return WiFi.status() == 'WL_CONNECTED';
}

void mqtt_checkconn(){
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > mqttretry) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (mqttReconnect()) {
        lastReconnectAttempt = 0;
        client.connect(clientID.c_str());
      }
    }
  } else {
    // Client connected
    client.loop();
  }  
}

void process_MQTT_nb(){
  if (!client.connected()) {
    mqtt_checkconn();
  }
  client.loop(); // will wait loop reconnect to mqtt
}

void process_MQTT(){
  MQTTreconnect();
  client.loop(); // will wait loop reconnect to mqtt
}

void init_MQTT(){
  client.setServer(mqtt_server_ip, mqtt_server_port);
  client.setCallback(MQTTcallback);
  if (client.connect(clientID.c_str())) Logger.println("[MQTT] connected");
  client.setBufferSize(2048);
  client.subscribe("downstream");
  process_MQTT();
  // jsondata = pubjson.to<JsonArray>();
  // jsondata = pubjson.createNestedArray();
}

void init_MQTT(String clientid){
  clientID = clientid; // global
  init_MQTT();
}

#ifdef USEJSON

void MQTT_pub(String topic, String sensor, String value){
    Logger.print("[MQTT] Publish: ");
    Logger.print(sensor);
    Logger.print(" ");
    Logger.println(value);
    if(value == "") {
      Logger.println("[ERROR] MQTT value is empty");
      return;
    }

    // JsonArray data = payload.createNestedArray(topic);
    payload["topic"] = topic;
    payload["clientid"] = clientID;
    payload["type"] = sensor;
    payload["value"] = value.toFloat();
    // payload["unit"] = "";
    // if(topic == "device") 
    jsondata.add(payload);

    // serializeJson(jsondata, Serial);
    // if(debug_mqtt) serializeJson(payload, Serial);
    // serializeJsonPretty(payload, Serial);
    
    // String output;
    // serializeJson(payload,output);
    // client.publish(topic.c_str(),("["+output+"\n]").c_str()); // must be object?
    // close ?
}

void MQTT_pub(String topic, String sensor, String value, bool json){
    Logger.print("[MQTT] Publish: ");
    Logger.print(sensor);
    Logger.print(" ");
    Logger.println(value);
    if(value == "") {
      Logger.println("[ERROR] MQTT value is empty");
      return;
    }
    JsonArray data = payload.createNestedArray(topic);
    payload["topic"] = data;
    payload["clientid"] = clientID;
    payload["type"] = sensor;
    payload["value"] = value.toFloat();
    // payload["unit"] = "";
    if(debug_mqtt) serializeJson(payload, Serial);
    // serializeJsonPretty(payload, Serial);
    
    String output;
    serializeJson(payload,output);
    
    client.publish(topic.c_str(),("["+output+"\n]").c_str()); // must be object?
    // close ?
    // mqttClient.beginMessage(topic);
    // serializeJson(doc, mqttClient);
    // mqttClient.endMessage();
    // client.print
    // client.endPublish();
    //     client.beginPublish("greenBottles/lyrics", msgLen, false);

    // const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3);
    // DynamicJsonBuffer jsonBuffer(capacity);

    // JsonObject& root = jsonBuffer.createObject();
    // root["sensor"] = "gps";
    // root["time"] = 1351824120;

    // JsonArray& data = root.createNestedArray("data");
    // data.add(48.75608);
    // data.add(2.302038);

    // root.printTo(Serial);
}

void MQTT_pub_send(String topic){
  Logger.println("[MQTT] sending json");
  // serializeJson(jsondata, Logger);
  // rootdoc.createNestedObject();
  // rootdoc.add(pubjson);
  // String output;
  // serializeJson(pubjson,output);
  // Serial.println(output);
  // serializeJson(rootdoc,output);
  // Serial.println(output);
  // Serial.flush();

  char message[2048];
  serializeJson(pubjson, message);
  Serial.println((String)message);
  client.publish(topic.c_str(),message);
  delay(500);
  // pubjson.clear();
  rootdoc.clear();
  // jsondata.clear();
  // rootdoc.createNestedObject(jsondata);
  jsondata = pubjson.to<JsonArray>();
  // payload.clear();
  // pubjson.garbageCollect();
}

#else
void MQTT_pub(String topic, String msg){
    Logger.print("[MQTT] Publish message: ");
    Logger.print("topic: ");
    Logger.print(topic);
    Logger.print(" mesg: ");
    Logger.println(msg);
    client.publish(topic.c_str(), msg.c_str());
}

void MQTT_pub(String topic, String sensor, String value){
    Logger.print("[MQTT] Publish message: ");
    Logger.print("topic: ");
    Logger.print(topic+"/"+clientID+"/"+sensor);
    Logger.print("\t\tvalue: ");
    Logger.println(value);
    if(value == "") {
      Logger.println("[ERROR] MQTT value is empty");
      return;
    }
    client.publish((topic+"/"+clientID+"/"+sensor).c_str(), value.c_str());
}
#endif

#endif