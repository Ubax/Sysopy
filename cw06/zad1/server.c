#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <string.h>

#include "communication.h"

int running = 1;
int clients[MAX_NUMBER_OF_CLIENTS];

int processResponse(struct MESSAGE *msg);

int main() {
    for (int i = 0; i < MAX_NUMBER_OF_CLIENTS; i++)clients[i] = -1;
    int queueId = msgget(getServerQueueKey(), IPC_CREAT | IPC_EXCL | 0666);
    if (queueId == -1) ERROR_EXIT("Creating queue");
    struct MESSAGE msgbuf;
    while (running) {
        if (msgrcv(queueId, &msgbuf, MSGSZ, -(MAX_COMMAND_ID + 1), 0) == -1) ERROR_EXIT("Receiving");
        processResponse(&msgbuf);
    }
    if (msgctl(queueId, IPC_RMID, NULL) == -1) ERROR_EXIT("Deleting queue");
    return 0;
}

int send(int clientId, enum COMMAND type, char text[MESSAGE_SIZE]) {
    struct MESSAGE msg;
    msg.mType = type;
    strcpy(msg.message, text);
    msg.senderId = -1;
    if (clients[clientId] == -1 || clientId >= MAX_NUMBER_OF_CLIENTS || clientId < 0) {
        printf("WRONG CLIENT ID\n");
        return 1;
    }

    if (msgsnd(clients[clientId], &msg, MSGSZ, IPC_NOWAIT)) ERROR_EXIT("Sending");
    return 0;
}

void do_echo(int clientId, char msg[MESSAGE_SIZE]) {
    char response[MESSAGE_SIZE];
    char date[64];
    FILE *f = popen("date", "r");
    fread(date, sizeof(char), 64, f);
    pclose(f);
    strcpy(response, msg);
    strcat(response, "\t");
    strcat(response, date);
    send(clientId, ECHO, response);
    printf("%s", date);
}

void do_stop() {
    running=0;
}

void do_init(char message[MESSAGE_SIZE]) {
    int clientId = 0;
    for (; clientId < MAX_NUMBER_OF_CLIENTS; clientId++) {
        if (clients[clientId] == -1)break;
    }
    if (clientId >= MAX_NUMBER_OF_CLIENTS) {
        printf("Too many clients\n");
        return;
    }
    int clientQueueId;
    sscanf(message, "%i", &clientQueueId);
    clients[clientId]=clientQueueId;

    char text[MESSAGE_SIZE];
    sprintf(text, "%i", clientId);
    send(clientId, INIT, text);
}

void do_list(int clientId) {
    char response[MESSAGE_SIZE], buf[MESSAGE_SIZE];
    strcpy(response, "");
    for(int i=0;i<MAX_NUMBER_OF_CLIENTS;i++){
        if(clients[i]>=0){
            sprintf(buf, "Id: %i\tQueueID: %i\n", i, clients[i]);
            strcat(response, buf);
        }
    }
    send(clientId, LIST, response);
}

int processResponse(struct MESSAGE *msg) {
    switch(msg->mType){
        case STOP:
            do_stop();
        break;
        case ECHO:
            do_echo(msg->senderId, msg->message);
        break;
        case INIT:
            do_init(msg->message);
        break;
        case LIST:
            do_list(msg->senderId);
            break;
    }
    printf("%s\n", msg->message);
    return 0;
}