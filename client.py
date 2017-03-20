__author__ = 'Robert'
#!/usr/bin/python           # This is client.py file
# CS 372, Introduction to Networks Project2
# 2/7/2015
# Adapted from the following sources
# http://stackoverflow.com/questions/14425401/catch-socket-error-errno-111-connection-refused-exception
# with credit to Ian Bender  @ OSU


import socket
import sys
import os

stdMsgSize = 510
setMsgSize = 0

#http://stackoverflow.com/questions/12638408/decorating-hex-function-to-pad-zeros
def convert(i, l):

    return "{0:#0{1}x}".format(i,l)


def interpreter(m):

    if (m[0:1] == "-a"):
        return 1

    return 0


#listen for a data connection
# called after there is an established connection
def listenup():

    #adapted from http://www.binarytides.com/python-socket-server-code-example/

    dport = 8888
    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.bind(('127.0.0.1', dport))
    except socket.error as msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()

    s.listen(1)

    return s


def receiveFile(s, filesz):

    try:

        #make them enter file name

        #GET LIST OF CURRENT DIR
        cfiles = [f for f in os.listdir(".")]
        msg =""


        while True:
            msg = raw_input("Enter name of file:")[:stdMsgSize]

            for i in cfiles:
                if i == msg:
                     print "Sorry that file name is taken"
                     exit(0)
            break


        if filesz <=0:
            print "no bytes to be sent file size determined < 0"
            exit(0)

        # pass send initialization
        s.send("1")

        f = open(msg, 'w')

        #receive the file in chunks
        while (filesz > 0):
            h = s.recv(filesz)
            remain = len(h)
            print remain
            cnts = str(h)
            f.write(cnts)
            filesz -= remain


    except socket.error as msg:
        print 'Bad rcv : ' + str(msg[0]) + ' Message ' + msg[1]
        s.close()



def connect(h, p):
    try:

        s = socket.socket()         # Create a socket object
        s.connect((h, p))
        print 'Connected to ' + str(s.getpeername())

    except socket.error:
        print ("Sorry, the connection must be down")
        return -1

    return s

def deliver(action, connection, msg):

    try:
        connection.send(action + msg + '\0')
        return 1

    except socket.error, e:
        print ("Sorry, couldn't confirm connection")
        return -1

#handles outgoing messages, and what to do with them i.e. wait for reply, just echo something, etc.
def out_interpreter(msg, connection):

    act = msg[0:2]
    pyld = msg[3:stdMsgSize-1]

    if (act == "-a"):
        #send an ack
        connection.send(act + pyld + '\0')
        res = inc_interpreter (connection)
        if(res < 0):
            print "server did not reply"
            return 0

        return 1

    elif (act == "-e"):

        connection.send(act + pyld)
        res = inc_interpreter (connection)
        if res < 0:
            print "server did not reply"
            return 0
        return 1

    elif (act == "-g"):

        #start listening for connection
        s = listenup()

        #aks for it
        connection.send(act + pyld) #ask for the file

        #accept connection
        try:
            dConn, addr = s.accept()
            print 'Connected with ' + addr[0] + ':' + str(addr[1])

        except socket.error as msg:
            print 'Bad connection. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]


        if(dConn):
            try:
                dConn.send("42") # just checking

            except socket.error as msg:
                print 'Bad send : ' + str(msg[0]) + ' Message ' + msg[1]
                dConn.close()

        if(dConn): #connection is on

            fileSize = dConn.recv(stdMsgSize)

            if(fileSize < 0):
                dConn.close()

            try:#receive the file
                receiveFile(dConn, long(fileSize))

            except socket.error as msg:
                print 'Bad connection. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]

        print "File received"


        dConn.close()
        return 1

    elif (act == "-l"):

        connection.send(act + "0")
        res =inc_interpreter (connection)
        if(res < 0):
            print "server did not reply"
            return 0

        #get he file
        return 1


    elif (act == "\\q"):
        #send request to server
        connection.send(act)

        #r
        connection.close()
        print "good bye"
        exit(0)
        return 0

    elif (act == "-h"):
        #list commands
        return 1

    else:
        print "Try again- bad command"
        return 1

# handles incoming messages
def inc_interpreter(connection):

    m = connection.recv(stdMsgSize)

    if not (m):
        return 0

    act = m[0:2]
    pyld = m[2:stdMsgSize]

    if (act == "-a"):
        #send an ack
        print "In heaven everything is... " + pyld
        return 1

    #simple echoing system
    elif (act == "-e"):
        #send an ack
        print pyld
        return 1

    #  quit with a reason
    elif (act == "-q"):

        print pyld
        connection.close()
        return 0

    elif (act == "-l"):

        #receive the list
        print pyld

        return 1

    elif (act == "-h"):
        #list commands
        return 1

    #set next message size
    elif (act == "-s"):

        setMsgSize = long(pyld)

        print pyld
        return 1

    else:
        return 0


if (len(sys.argv) < (3)):
    print 'REQUIRED: python2.7 ARGS: <host> <port number>'
    exit(0)

# get parameters
host = sys.argv[1]
port = int(sys.argv[2])


# establish a connection
x = connect(host, port)

if (x):

    #announce to the server the port
    #for data connection
    affirmed = deliver('-a', x, '8888')

    if affirmed <= 0:
       print ("Sorry, the server didn't understand")

    #make sure it heard you
    try:
        inc_interpreter(x)

    except socket.error:
        print ("Sorry, the connection must be down")

    while True:

            #take commands
            msg = raw_input("$:")[:stdMsgSize]

            # pass it to the interperter
            r = out_interpreter(msg, x)

            if(r < 0):
                x.close()
                break
