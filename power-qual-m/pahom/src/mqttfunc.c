#include "mqttfunc.h"
#include "dataproc.h"

int disc_finished = 0;
int subscribed = 0;
int finished = 0;
extern config fg;
extern char* topics[7];

void onConnect(void* context, MQTTAsync_successData* response)
{
    char* hostport = response->alt.connect.serverURI;
    int qoss[7] = {1,1,1,1,1,1,1};

    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    printf("Successful connection to %s\n", hostport);
    printf("Subscribing to topic: %s\n%s\n%s\n%s\n%s\n%s\n%s\n", topics[0], topics[1], topics[2], topics[3], topics[4], topics[5], topics[6]);
    printf("For client %s using QoS%d\n\n"
        "Press Q or q<Enter> to quit\n\n", CLIENTID1, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;
 
    if ((rc = MQTTAsync_subscribeMany(client, 7, topics, qoss, &opts) != MQTTASYNC_SUCCESS))
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }    
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
    printf("Subscribe succeeded\n");
    subscribed = 1;
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response->code);
    finished = 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
    printf("Successful disconnection\n");
    disc_finished = 1;
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Disconnect failed, rc %d\n", response->code);
    disc_finished = 1;
}

void onSend(void* context, MQTTAsync_successData* response)
{


    printf("Message with token value %d delivery confirmed\n", response->token);


}

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Message send failed token %d error code %d\n", response->token, response->code);

}


void connlost(void* context, char* cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost\n");
    if (cause)
        printf("     cause: %s\n", cause);

    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

void CatPubtopic(char** p, char* maintopic)
{
    int lenp = (int)(strlen(maintopic) + 5);
    char* p1 = strstr(maintopic, "/up");
    if (p1)
    {
        // sprintf(*p, "%s%s", maintopic, "/return");
        strncpy(*p,  maintopic, strlen(maintopic) - strlen(p1));
        strcat(*p,  "/return");
    }
}


int msgarrvd(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
    if (message->payloadlen < 0)
    {
        printf("Message empty, please contact the administrator.\nLast Message didn't send because of some bugs.\n");
        return -1;
    }

    char* pubmsgpayload = (char*)malloc(JSONLENGTH);
    if (pubmsgpayload == NULL)
    {
        printf("malloc pubmsg memory error, pls restart program...\n");
        return -1;
    }
    memset(pubmsgpayload, 0, JSONLENGTH);
    MessageSwitch(topicName, message->payload, &pubmsgpayload);


    //PUBLISH
    char* returnTopic = (char*)malloc((size_t)topicLen + 5);
    if (returnTopic == NULL)
    {
        printf("malloc returnTopic memory error, pls restart program...\n");
        return -1;
    }
    memset(returnTopic, 0, (size_t)topicLen + 5);

    CatPubtopic(&returnTopic, topicName);

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);

    MQTTAsync client = (MQTTAsync)context;
    pub(pubmsgpayload, returnTopic);

    // free(pubmsgpayload);
    // free(returnTopic);

    //FREE  MESSAGE MEMORY
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
}



int pub(char* payload, char* topic)
{

    char* ADDRESS = fg.hp;
    char* CLIENTID = "qanpub-er";
    char* TOPIC = topic;
    char* PAYLOAD = payload;


    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = fg.user;
    conn_opts.password = fg.pw;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = (int)strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to publish message, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for up to %d seconds for publication of %s\n"
        "on topic %s for client with ClientID: %s\n",
        (int)(TIMEOUT / 1000), PAYLOAD, TOPIC, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
        printf("Failed to disconnect, return code %d\n", rc);
    MQTTClient_destroy(&client);

    if (payload)
        free(payload);
    if (topic)
        free(topic);
    return rc;
}