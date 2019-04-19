#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>

#include "communication.h"

struct CLIENT {
    int queueId;
    int friends[MAX_NUMBER_OF_CLIENTS];
    int numberOfFriends;
    pid_t pid;
};

int running = 1;
struct CLIENT clients[MAX_NUMBER_OF_CLIENTS];

int processResponse(struct MESSAGE *msg);

int main() {
    for (int i = 0; i < MAX_NUMBER_OF_CLIENTS; i++) {
        clients[i].queueId = -1;
        clients[i].numberOfFriends = 0;
    }
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
    if (clients[clientId].queueId == -1 || clientId >= MAX_NUMBER_OF_CLIENTS || clientId < 0) {
        printf("WRONG CLIENT ID\n");
        return 1;
    }

    if (msgsnd(clients[clientId].queueId, &msg, MSGSZ, IPC_NOWAIT)) ERROR_EXIT("Sending");
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

void do_stop(int clientId) {
    clients[clientId].queueId=-1;
    for (int i = 0; i < MAX_NUMBER_OF_CLIENTS; i++) {
        if (clients[i].queueId >= 0) {
            send(i, STOP, "");
            kill(clients[i].pid, SIGRTMIN);
        }
    }
    running = 0;
}

void do_init(int clientPID, char message[MESSAGE_SIZE]) {
    int clientId = 0;
    for (; clientId < MAX_NUMBER_OF_CLIENTS; clientId++) {
        if (clients[clientId].queueId == -1)break;
    }
    if (clientId >= MAX_NUMBER_OF_CLIENTS) {
        printf("Too many clients\n");
        return;
    }
    int clientQueueId;
    sscanf(message, "%i", &clientQueueId);
    clients[clientId].queueId = clientQueueId;
    clients[clientId].pid = clientPID;

    char text[MESSAGE_SIZE];
    sprintf(text, "%i", clientId);
    send(clientId, INIT, text);
}

void do_list(int clientId) {
    char response[MESSAGE_SIZE], buf[MESSAGE_SIZE];
    strcpy(response, "");
    for (int i = 0; i < MAX_NUMBER_OF_CLIENTS; i++) {
        if (clients[i].queueId >= 0) {
            sprintf(buf, "Id: %i\tQueueID: %i\n", i, clients[i].queueId);
            strcat(response, buf);
        }
    }
    send(clientId, LIST, response);
}

void do_friends(int clientId, char msg[MESSAGE_SIZE]) {
    if (clientId >= MAX_NUMBER_OF_CLIENTS || clientId < 0 || clients[clientId].queueId == -1) {
        printf("Unknown client %i\n", clientId);
        return;
    }
    char list[MESSAGE_SIZE];
    char command[MESSAGE_SIZE];
    int num = sscanf(msg, "%s %s", command, list);
    if (num == EOF || num == 0) {
        printf("Scanning problem: %i\n", clientId);
        return;
    }
    if (num == 1) {
        clients[clientId].numberOfFriends = 0;
    } else if (num == 2) {
        clients[clientId].numberOfFriends = 0;
        char *elem = strtok(list, ";");
        while (elem != NULL && clients[clientId].numberOfFriends < MAX_NUMBER_OF_CLIENTS) {
            clients[clientId].friends[clients[clientId].numberOfFriends] = (int) strtol(elem, NULL, 10);
            clients[clientId].numberOfFriends++;
            elem = strtok(NULL, ";");
        }
    }
}

void do_2_all(int clientId, char msg[MESSAGE_SIZE]) {
    char response[MESSAGE_SIZE];
    char date[64];
    FILE *f = popen("date", "r");
    fread(date, sizeof(char), 64, f);
    pclose(f);
    sprintf(response, "%s\tID: %i\tDate: %s\n", msg, clientId, date);
    for(int i=0;i<MAX_NUMBER_OF_CLIENTS;i++){
        if(i!=clientId && clients[i].queueId!=-1){
            send(i, _2ALL, response);
            kill(clients[i].pid, SIGRTMIN);
        }
    }
    send(clientId, _2ALL, response);
    printf("%s", date);
}

int processResponse(struct MESSAGE *msg) {
    switch (msg->mType) {
        case STOP:
            do_stop(msg->senderId);
            break;
        case ECHO:
            do_echo(msg->senderId, msg->message);
            break;
        case INIT:
            do_init(msg->senderId, msg->message);
            break;
        case LIST:
            do_list(msg->senderId);
            break;
        case FRIENDS:
            do_friends(msg->senderId, msg->message);
            break;
        case _2ALL:
            do_2_all(msg->senderId, msg->message);
            break;
    }
    printf("%s\n", msg->message);
    return 0;
}