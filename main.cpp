/*
   Robert Jackson
   CS372 2/7/16
   Project1

   Adapted From:
   http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/server.c
   http://www.linuxhowtos.org/C_C++/socket.htm
   */

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
#include "main.h"
#include <dirent.h>
#include <vector>
#include <sys/errno.h>
#include <sstream>
#include <fstream>
#include <streambuf>

int MSGSIZE = 510;
const char * clientPN;


using std::string;


void error(string msg)
{
    perror(msg.c_str());
    exit(1);
}


//make a list of the directory's contents
//@ params the dir, where you want to store it
// adapted from http://www.cplusplus.com/forum/beginner/107265/
int getdir (std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << std::endl;
        return 0;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(std::string(dirp->d_name));

    }
    closedir(dp);
    return 1;
}


//Get the files contents
//http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
int get_file_contents(const char *filename, std::string &contents)
{

    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {

        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents.size();
    }

    return -1;
}


//Sending Function
//@params the socket int, pointer to the message
// Send a message to the socket
// msg format (Action, size in hex, payload )

void sender( int sockfd, string msg, string act){
    int m, s, a;
    char *temp;
    s = (int) msg.size();
    a = (int) act.size();

    temp = (char*)malloc(s+a);
    strcpy(temp, act.c_str());
    strcat(temp, msg.c_str());

    m = send(sockfd, temp, s+a,0);

    if (m < 0)
        printf("SERVER ERROR writing to socket");

    free(temp);

}

//Sending Files
//@params the socket int, string file contents, sizeof the message
// send the file
void sendfile( int sockfd, string file, unsigned long size){
    int m, s;
    unsigned long offset = 0;

    std::cout << "TOTAL sending " << file.size() << std::endl;

    //SEND THE WHOLE FILE
    while (offset < file.size()) {

        m = send(sockfd, file.c_str()+offset, file.size()-offset,0);
        std::cout << "sending " << m << std::endl;
        if (m <= 0) break;
        offset += m;
    }// ADAPTED FROM http://stackoverflow.com/questions/15176213/read-the-whole-file-and-send-it-via-sockets

}

//Receiving Function
//@params the socket int, pointer to the message, sizeof the message
// calls the interpreter on the message
// returns the result from interpreter
int receiver(int sockfd, char  *msg, size_t msgBytes){
    int m;

    m = recv(sockfd, msg, msgBytes ,0);
    string temp = msg;

    if (m < 0){
        printf("SERVER ERROR receiving from socket");
        return m;
    }

    m = interpreter(temp, sockfd);
    return m;

}

//prints out the payload of the message
//using this to test mostly
void anEcho(string msg){

    std::cout << msg.substr(2, MSGSIZE-1)<< "\n" << std::endl;

}

//Interprets the messages sent into actions
// @ params the message, the socket
// returns -1 for bad info
// returns 0 for fail ->shutdown
// returns 1 on success
int interpreter(string msg, int socky ){
    int action = NULL;
    string act = "";
    string pyld = "";

    //break message to cmd and filename
    if (msg.compare("") == 0){
        //return 0;
        std::cout << "Nothing sent" << msg << std::endl;
        sender(socky, "bad send", "-e");

        return 1;

    }else{
        act = msg.substr(0,2);
        pyld = msg.substr(2, MSGSIZE-1);
    }

    //Sender didn't send anything
    if( msg.size() < 2){
        //payl = msg.substr(2, msg.size());
        //act = msg.substr(0,1);

        //TODO SEND WARNING
        std::cout << "the action is " << act << std::endl;
        sender(socky, "uhh...Send something next time?", "-e");
        return -1;
    }

    //send the directory contents
    else if (act.compare("-l") == 0){

        //reply = listFiles();
        std::cout << "Listing Directories " <<  std::endl;
        std::vector<string> list;
        std::stringstream strm;
        int r = getdir(".", list );

        //get the dir into a string
        if(r != 0) {
            for (int k = 0; k <= list.size() - 1; k++) {

                strm << list[k];
                strm << "\n";
            }
            strm << "\0";
        }

        //make it a string and send it
        std::string s = strm.str();
        sender(socky, s, "-l");
        return 1;


    }

    //SEND A FILE
    else if(act.compare("-g") == 0){

        int r;
        long f;
        char intl[2];

        std::string fileContent;
        std::string s;
        std::cout << "the action is " << act << std::endl;
        std::stringstream ss;

        //establish second connection
        int c = dataConnection(clientPN);
        if (c < 0 )
            error("Connection lost");

        //get the file contents into a string
        f = get_file_contents(pyld.c_str(), fileContent);
        ss << f;

        //getfilecontents returns file size or -1 if non existent
        if(f < 0){
            sender(socky, "You entered a Bad file name", "-q");
            error("borked File name");
        }

        //tell the client what the file size is
        sender(c, ss.str(), "" ); // tell client what the file size is

        //Wait for transmission initiation
        r = recv(c, intl, 2 ,0);
        if(r < 0)
            error("no initialization of transmission");

        //Send it
        sendfile(c, fileContent, fileContent.size());

        return 1;
    }


    // initial ack sent establishing hosts port listening for the data connection
    //for file transfer
    else if(act.compare("-a") == 0){

        std::cout << "Connected. Data port is " << act << std::endl;
        anEcho(msg);

        //assign cleint port number
        clientPN = pyld.c_str();

        sender(socky, "fine", "-a");

        return 1;
    }

    // This just sends an echo
    else if(act.compare("-e") == 0){

        std::cout << "the action is " << act << std::endl;
        anEcho(msg);
        sender(socky, pyld, "-e");
        return 1;
    }

    else if(act.compare("\\q") == 0){
        close(socky);
        return 0;
    }
    else{

        std::cout << "Dont send me junk " << act << std::endl;
        sender(socky, "Bad Commend try again", "-e");
    }

    return 0;

}


// Starts the data connection
// @ params a port number
// retuns the socket to trasmit on
int dataConnection(const char * portno){

    int status, sockfd, m;
    char buff[3] = "no";
    char *b;
    struct addrinfo hints;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    std::string file;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_protocol = 0;

    sockfd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

    if (sockfd > 0)
        std::cout << "connecting..." << std::endl;

    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    //zero out the structs
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // not sure if this carries over from what I already set

    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    serv_addr.sin_port = htons(8888);

    //connect to the socket
    if ((m= connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0)
        error("ERROR connecting");

    //receive the ok i.e number 42
    m = recv(sockfd, buff, 2 ,0);  // jsut a check not doing anything with it
    std::cout << buff << std::endl;
    return sockfd;

    //adapted a bit from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
    // adapted from http://www.linuxhowtos.org/data/6/client.c
}


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int controlConnection(uint16_t port){

    int sockfd, newsockfd, m;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr, client_fd;
    char *message;
    size_t msgBytes = 512;

    //string welc = "WELCOME!\n\n";

    //check arguments are present.
    if (port < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    //establish socket type
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // clear out the bytes
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //set serv_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    //check for binding
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("SERVER ERROR on binding");


    while(1) {

        listen(sockfd, 5);

        clilen = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
            error("SERVER ERROR on Accept");

        message = (char *) malloc(msgBytes);
        bzero(message, msgBytes);

        while (1) {

            m = receiver(newsockfd, message, msgBytes);
            bzero(message, msgBytes);

            // if quit is sent
            if (m <= 0) {
                close(newsockfd);
                free(message);
                break;
            }

        }
        close(newsockfd);
        free(message);
    }
    //adapted a bit from http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
    // adapted from http://www.linuxhowtos.org/data/6/client.c
}
#pragma clang diagnostic pop


int main(int argc, char *argv[])
{
    controlConnection(atoi(argv[1]));


    return 0;
}
