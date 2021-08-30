#ifndef MQTTFUNC_H

#include "MQTTClient.h"
#include "MQTTAsync.h"
#include "zzzjson.h"


#define CLIENTID1 "Power-Quality-Analyzer"
#define USERNAME "yishu"
#define PASSWORD "123456"
#define QOS 1
#define TIMEOUT 10000L

extern int disc_finished;
extern int subscribed;
extern int finished;

void connlost(void* context, char* cause); 
int msgarrvd(void* context, char* topicName, int topicLen, MQTTAsync_message* message);
int pub(char* payload, char* topic);
void onConnect(void* context, MQTTAsync_successData* response);
void onSubscribe(void* context, MQTTAsync_successData* response);
void onSubscribeFailure(void* context, MQTTAsync_failureData* response);
void onConnectFailure(void* context, MQTTAsync_failureData* response);
void onDisconnectFailure(void* context, MQTTAsync_failureData* response);
void onDisconnect(void* context, MQTTAsync_successData* response);
void onSend(void* context, MQTTAsync_successData* response);
void onSendFailure(void* context, MQTTAsync_failureData* response);


void CatPubtopic(char** p, char* maintopic);


typedef struct CONFIG {
    char* hp;
    char* user;
    char* pw;
}config;
#endif // !MQTTFUNC_H