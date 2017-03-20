//
// Created by Robert on 3/2/2016.
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>

#ifndef PROJECT2SERVER_MAIN_H
#define PROJECT2SERVER_MAIN_H

int receiver(int sockfd, char  *msg, size_t msgBytes);
void anEcho(std::string msg);
int interpreter(std::string msg, int socket);
void sender( int sockfd, char *msg);
int dataConnection(const char * portno);
int controlConnection(uint16_t port);

#endif //PROJECT2SERVER_MAIN_H
