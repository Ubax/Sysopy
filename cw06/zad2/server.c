#include "communication.h"

struct CLIENT {
    int queueId;
    int friends[MAX_NUMBER_OF_CLIENTS];
    int numberOfFriends;
    pid_t pid;
};

int running = 1;
mqd_t queueId = -1;
struct CLIENT clients[MAX_NUMBER_OF_CLIENTS];

int processResponse(struct MESSAGE *msg);

void do_init(int clientPID, char message[MESSAGE_SIZE]);

void do_stop(int clientId);

void do_echo(int clientId, char msg[MESSAGE_SIZE]);

void do_list(int clientId);

void do_friends(int clientId, char msg[MESSAGE_SIZE]);

void do_add(int clientId, char msg[MESSAGE_SIZE]);

void do_del(int clientId, char msg[MESSAGE_SIZE]);

void do_2_all(int clientId, char msg[MESSAGE_SIZE]);

void do_2_friends(int clientId, char msg[MESSAGE_SIZE]);

int canSendTo(int clientId) {
    return !(clientId >= MAX_NUMBER_OF_CLIENTS || clientId < 0 || clients[clientId].queueId == -1);
}

void cleanExit() {
    if (mq_close(queueId) == -1) ERROR_EXIT("Closing queue");
    if (mq_unlink(SERVER_QUEUE_NAME) == -1) ERROR_EXIT("Deleting queue");
    exit(0);
}

void exitSignal(int signalno) {
    do_stop(-1);
    cleanExit();
}

int main() {
    signal(SIGINT, exitSignal);
    for (int i = 0; i < MAX_NUMBER_OF_CLIENTS; i++) {
        clients[i].queueId = -1;
        clients[i].numberOfFriends = 0;
    }
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = MAX_QUEUE_SIZE;
    queue_attr.mq_msgsize = MESSAGE_SIZE;
    if ((queueId = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr)) == -1)
        ERROR_EXIT("Creating queue");
    struct MESSAGE msgbuf;
    while (running) {
        if (mq_receive(queueId, &msgbuf, MSGSZ, NULL) == -1) ERROR_EXIT("Receiving");
        processResponse(&msgbuf);
    }
    cleanExit(queueId);
    return 0;
}

int send(int clientId, enum COMMAND type, char text[MESSAGE_SIZE]) {
    struct MESSAGE msg;
    msg.mType = type;
    strcpy(msg.message, text);
    msg.senderId = -1;
    if (!canSendTo(clientId)) {
        printf("WRONG CLIENT ID\n");
        return 1;
    }

    if (msgsnd(clients[clientId].queueId, &msg, MSGSZ, IPC_NOWAIT)) ERROR_EXIT("Sending");
    return 0;
}

void do_echo(int clientId, char msg[MESSAGE_SIZE]) {
    printf("Echo for %i...\n", clientId);

    char response[MESSAGE_SIZE];
    char date[64];
    FILE *f = popen("date", "r");
    fread(date, sizeof(char), 64, f);
    pclose(f);
    sprintf(response, "%s\t%s", msg, date);
    send(clientId, ECHO, response);
}

void do_stop(int clientId) {
    printf("Stopping...\n");
    if (clientId >= 0)clients[clientId].queueId = -1;

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
    clients[clientId].numberOfFriends = 0;

    printf("Init of client %i with id %i...\n", clientQueueId, clientId);

    char text[MESSAGE_SIZE];
    sprintf(text, "%i", clientId);
    send(clientId, INIT, text);
}

void do_list(int clientId) {
    printf("Sending list to %i...\n", clientId);
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

void do_2_all(int clientId, char msg[MESSAGE_SIZE]) {
    printf("Sending message to all...\n");
    char response[MESSAGE_SIZE];
    char date[64];
    FILE *f = popen("date", "r");
    fread(date, sizeof(char), 64, f);
    pclose(f);
    sprintf(response, "%s\tID: %i\tDate: %s\n", msg, clientId, date);
    for (int i = 0; i < MAX_NUMBER_OF_CLIENTS; i++) {
        if (i != clientId && clients[i].queueId != -1) {
            send(i, _2ALL, response);
            kill(clients[i].pid, SIGRTMIN);
        }
    }
}

void do_2_friends(int clientId, char msg[MESSAGE_SIZE]) {
    printf("Sending message to friends...\n");
    char response[MESSAGE_SIZE];
    char date[64];
    FILE *f = popen("date", "r");
    fread(date, sizeof(char), 64, f);
    pclose(f);
    sprintf(response, "%s\tID: %i\tDate: %s\n", msg, clientId, date);
    for (int i = 0; i < clients[clientId].numberOfFriends; i++) {
        int to = clients[clientId].friends[i];
        if (canSendTo(to)) {
            printf("%i\t", to);
            send(to, _2FRIENDS, response);
            kill(clients[to].pid, SIGRTMIN);
        }
    }
    printf("\n");
}

void do_2_one(int clientId, char msg[MESSAGE_SIZE]) {
    printf("Sending message to one...\n");
    char text[MESSAGE_SIZE];
    char response[MESSAGE_SIZE];
    char date[64];
    int to;
    FILE *f = popen("date", "r");
    fread(date, sizeof(char), 64, f);
    pclose(f);
    sscanf(msg, "%i %s", &to, text);
    sprintf(response, "%s\tID: %i\tDate: %s\n", text, clientId, date);
    if (canSendTo(to)) {
        send(to, _2FRIENDS, response);
        kill(clients[to].pid, SIGRTMIN);
    }
}

void add_friends(int clientId, char list[MESSAGE_SIZE]) {
    char *elem = strtok(list, LIST_DELIMITER);

    while (elem != NULL && clients[clientId].numberOfFriends < MAX_NUMBER_OF_CLIENTS) {
        int id = (int) strtol(elem, NULL, 10);
        for (int i = 0; i < clients[clientId].numberOfFriends; i++)
            if (id == clients[clientId].friends[i])id = -1;
        if (id < MAX_NUMBER_OF_CLIENTS && id >= 0 && id != clientId) {
            clients[clientId].friends[clients[clientId].numberOfFriends] = id;
            printf("\t\t%i\n", clients[clientId].friends[clients[clientId].numberOfFriends]);
            clients[clientId].numberOfFriends++;
        }
        elem = strtok(NULL, LIST_DELIMITER);
    }
}

void do_friends(int clientId, char msg[MESSAGE_SIZE]) {
    if (!canSendTo(clientId)) {
        printf("Unknown client %i\n", clientId);
        return;
    }
    printf("Setting friends of %i...\n", clientId);

    char list[MESSAGE_SIZE];
    int num = sscanf(msg, "%s", list);

    clients[clientId].numberOfFriends = 0;
    if (num == 1) {
        printf("\tfriends:\n");
        add_friends(clientId, list);
    }
}

void do_add(int clientId, char msg[MESSAGE_SIZE]) {
    if (!canSendTo(clientId)) {
        printf("Unknown client %i\n", clientId);
        return;
    }
    printf("Adding friends of %i...\n", clientId);

    char list[MESSAGE_SIZE];
    int num = sscanf(msg, "%s", list);

    if (num == EOF || num == 0) {
        printf("Scanning problem: %i\n", clientId);
        return;
    } else if (num == 1) {
        printf("\tfriends:\n");
        add_friends(clientId, list);
    }
}

void do_del(int clientId, char msg[MESSAGE_SIZE]) {
    if (!canSendTo(clientId)) {
        printf("Unknown client %i\n", clientId);
        return;
    }
    printf("Deleting friends of %i...\n", clientId);

    char list[MESSAGE_SIZE];
    int num = sscanf(msg, "%s", list);
    char *elem = NULL;
    int *numberOfFriends = &clients[clientId].numberOfFriends;
    int id = -1;
    int i = 0;

    if (num == EOF || num == 0) {
        printf("Scanning problem: %i\n", clientId);
        return;
    } else if (num == 1) {
        printf("\tfriends:\n");
        elem = strtok(list, LIST_DELIMITER);
        while (elem != NULL && (*numberOfFriends) > 0) {
            id = (int) strtol(elem, NULL, 10);
            i = 0;
            for (; i < (*numberOfFriends); i++)
                if (id == clients[clientId].friends[i])break;
            if (i >= (*numberOfFriends))id = -1;
            if (id < MAX_NUMBER_OF_CLIENTS && id >= 0 && id != clientId) {
                printf("\t\t%i\n", clients[clientId].friends[i]);
                clients[clientId].friends[i] = clients[clientId].friends[(*numberOfFriends) - 1];
                (*numberOfFriends)--;
            }
            elem = strtok(NULL, LIST_DELIMITER);
        }
    }
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
        case _2FRIENDS:
            do_2_friends(msg->senderId, msg->message);
            break;
        case _2ONE:
            do_2_one(msg->senderId, msg->message);
            break;
        case ADD:
            do_add(msg->senderId, msg->message);
            break;
        case DEL:
            do_del(msg->senderId, msg->message);
            break;
    }
    return 0;
}