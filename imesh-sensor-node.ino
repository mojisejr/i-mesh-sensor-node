#include "painlessMesh.h"
#include "SHT21.h"
#include "wire.h"

#define   MESH_PREFIX     "chamber1"
#define   MESH_PASSWORD   "chamber123"
#define   MESH_PORT       5555


//Senser params
SHT21 sht21;
Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;

// Prototype
void receivedCallback( uint32_t from, String &msg );

size_t ServerId = 0;

// Send message to the Server every 10 seconds 
Task sendSensorDataTask(10000, TASK_FOREVER, []() { //dataSendEvery 10 sec
    DynamicJsonBuffer jsonBuffer;
    JsonObject& msg = jsonBuffer.createObject();
    msg["topic"] = "sensor1";
    msg["temp"] = String(sht21.getTemperature());
    msg["humid"] = String(sht21.getHumidity());

    String str;
    msg.printTo(str);
    if (ServerId == 0) // If we don't know the Server yet
        mesh.sendBroadcast(str);
    else
        mesh.sendSingle(ServerId, str); //Send sensor data directly to Server Node

    // log to serial
    msg.printTo(Serial);
    Serial.printf("\n");
});

void setup() {
  Serial.begin(115200);
  sht21.begin();
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION ); 

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6 );
  mesh.onReceive(&receivedCallback);

  // Add the task to the your scheduler
  userScheduler.addTask(sendSensorDataTask); 
  sendSensorDataTask.enable(); //enable this task
}

void loop() {
    userScheduler.execute();
    mesh.update();
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("SENSOR1: Received from %u msg=%s\n", from, msg.c_str());

  // Saving logServer
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  if (root.containsKey("topic")) {
      if (String("Server").equals(root["topic"].as<String>())) {
          // check for on: true or false
          ServerId = root["nodeId"];
          Serial.printf("logServer detected!!!\n");
      }
      Serial.printf("Handled from %u msg=%s\n", from, msg.c_str());
  }
}

