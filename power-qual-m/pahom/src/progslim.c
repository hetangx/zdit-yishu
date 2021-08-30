#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mqttfunc.h"
#include "dataproc.h"

config fg;
char* topics[7];

void MakeTopic(char* a[], char* gate, int num);

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        printf("Usage: %s tcp://hostname:port username password gateway\n", argv[0]);
        return 0;
    }
    fg.hp = argv[1];

    fg.user = argv[2];
    fg.pw = argv[3];
    // fg.user = "yishu";
    // fg.pw = "123456";
    MakeTopic(topics, argv[4], 7);

    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    if ((rc = MQTTAsync_create(&client, fg.hp, "Power-Quality", MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);

        exit(EXIT_FAILURE);
    }

    if ((rc = MQTTAsync_setCallbacks(client, client, connlost, msgarrvd, NULL)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        MQTTClient_destroy(client);
        exit(EXIT_FAILURE);
    }

    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;
    conn_opts.username = fg.user;
    conn_opts.password = fg.pw;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    conn_opts.automaticReconnect = true;
    conn_opts.minRetryInterval = 2;
    conn_opts.maxRetryInterval = 6;


    disc_opts.onSuccess = onDisconnect;
    disc_opts.onFailure = onDisconnectFailure;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        MQTTClient_destroy(client);
        exit(EXIT_FAILURE);
    }


    while (!subscribed && !finished)
        usleep(100);

    if (finished)
        return rc;

    printf("Enter Q to Quit this program...\n");
    int ch;
    do
    {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');


    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start disconnect, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        exit(EXIT_FAILURE);
    }

    while (!disc_finished)
    {
        usleep(10000L);
    }

    return rc;
}

void MakeTopic(char* a[], char* gate, int num)
{
    char* front = "plat/gateway/";
    char* mid = "/pkg/";
    char* back = "/up";
    int length = strlen(front) + strlen(gate) + strlen(mid) + strlen(back) + 2;
    char* make = (char*)malloc(sizeof(char) * length);
    if (make == NULL)
    {
        return;
    }

    for (int i = 0; i < num; i++)
    {
        sprintf(make, "%s%s%s%d%s", front, gate, mid, i + 1, back);
        a[i] = (char*)malloc(strlen(make) + 1);
        if (a[0])
        {
            memset(a[i], 0, strlen(make) + 1);
            sprintf(a[i], "%s", make);
        }
        memset(make, 0, length);
    }
}