#include <iostream>
#include <SDL.h>
#include <SDL_net.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n;
    return stm.str();
}

int main(int argc,char** argv)
{
    char msg[11];
    char rcvmsg[11];
    int receivedByteCount = 0;
    bool run = true;
    int clientNo = 0;
    int portNum = 0;

    std::vector<TCPsocket> socketVector;
    IPaddress ip;
    TCPsocket server = NULL;
    vector<TCPsocket> client = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    vector<string> movesPlayed;
    SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(11);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDLNet_Init();

    if (argc == 1)
    {
        std::cout << "Port Number: ";
        std::cin >> portNum;
        std::cout << std::endl;
    }
    else
        portNum = atoi(argv[1]);

    if(SDLNet_ResolveHost(&ip,NULL,portNum)==-1) // server
    {
       printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
       exit(1);
    }
    server=SDLNet_TCP_Open(&ip);
    if(!server)
    {
       printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
       exit(2);
    }
    SDLNet_TCP_AddSocket(socketSet, server);

    std::cout << "Server is now listening at port (" << portNum << ") ..." << std::endl;

    while(run)
    {
        SDLNet_CheckSockets(socketSet, 300);
        int serverActivty = SDLNet_SocketReady(server);
        if (serverActivty != 0) // if there is activity on our server socket
        {
            if (clientNo < 2) // that means 2 clients allowed for playing
            {
                printf("waiting for players...\n");
                client[clientNo]=SDLNet_TCP_Accept(server);
                if(client[clientNo])
                {

                    SDLNet_TCP_AddSocket(socketSet, client[clientNo]);
                    if (clientNo == 0) strcpy( msg, "w");
                    else strcpy( msg, "b");
                    int msgLength = strlen(msg) + 1;
                    SDLNet_TCP_Send(client[clientNo], msg, msgLength);
                    std::cout << "New Connection: " << SDLNet_TCP_GetPeerAddress(client[clientNo]) << "ID: " << clientNo << std::endl;
                    clientNo++;
                }
            }
            else if (clientNo < 10)
            {
                client[clientNo]=SDLNet_TCP_Accept(server);
                if(client[clientNo])
                {
                    SDLNet_TCP_AddSocket(socketSet, client[clientNo]);
                    strcpy( msg, "v");
                    int msgLength = strlen(msg) + 1;
                    SDLNet_TCP_Send(client[clientNo], msg, msgLength);
                    SDLNet_CheckSockets(socketSet, 0);
//                    if (SDLNet_SocketReady(client[clientNo]) != 0)
//                    {
//                        for (string mov : movesPlayed)
//                        {
//                            SDLNet_TCP_Send(client[clientNo], mov.c_str(), strlen(mov.c_str()));
//                        }
//                    }
                    std::cout << "New Connection: " << SDLNet_TCP_GetPeerAddress(client[clientNo]) << "ID: " << clientNo << std::endl;
                    clientNo++;
                }
            }
            else // If we don't have room for new clients...
            {
                cout << "*** Maximum client count reached - rejecting client connection ***" << endl;

                // Accept the client connection to clear it from the incoming connections list
                TCPsocket tempSock = SDLNet_TCP_Accept(server);

                // Send a message to the client saying "FULL" to tell the client to go away
                strcpy(msg, "FULL SERVER" );
                int msgLength = strlen(msg) + 1;
                SDLNet_TCP_Send(tempSock, msg, msgLength);

                // Shutdown, disconnect, and close the socket to the client
                SDLNet_TCP_Close(tempSock);
            }
        } // End of if server socket is has activity check

        // Loop to check all possible client sockets for activity
        for (int i=0; i<client.size(); i++)
        {
            if (client[i] != NULL)
            {
                int clientSocketActivity = SDLNet_SocketReady(client[i]);
                if (clientSocketActivity != 0)
                {
                    // If the socket is ready (i.e. it has data we can read)... (SDLNet_SocketReady returns non-zero if there is activity on the socket, and zero if there is no activity)
                    receivedByteCount = SDLNet_TCP_Recv(client[i], rcvmsg, 10);
                    if (receivedByteCount <= 0) // the player disconnected
                    {
                        printf("The player with ID:%d has disconnected\n", i);
                        //... remove the socket from the socket set, then close and reset the socket ready for re-use and finally...
                        SDLNet_TCP_DelSocket(socketSet, client[i]);
                        SDLNet_TCP_Close(client[i]);
                        client[i] = NULL;
                        if (client[0] == NULL || client[1] == NULL)
                        {
                            run = false;
                        }
                    }
                    else
                    {
                        int originatingClient = i;
                        cout << "Received from player with ID: " << i << " the message: " << rcvmsg << endl;
                        movesPlayed.push_back(to_string(rcvmsg));
                        for (int loop = 0; loop < client.size(); loop++)
                        {
                            if (client[loop] != NULL)
                            {
                                // Send a message to the client saying "OK" to indicate the incoming connection has been accepted
                                //strcpy( buffer, SERVER_NOT_FULL.c_str() );
                                int msgLength = strlen(rcvmsg);// + 1;

                                // If the message length is more than 1 (i.e. client pressed enter without entering any other text), then
                                // send the message to all connected clients except the client who originated the message in the first place
                                if (msgLength > 1 && loop != originatingClient)
                                {
                                    cout << "Retransmitting message: " << rcvmsg << " (" << msgLength << " bytes) to client number: " << loop << endl;
                                    SDLNet_TCP_Send(client[loop], rcvmsg, msgLength);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    SDLNet_FreeSocketSet(socketSet);
    for (auto &x : client)
        SDLNet_TCP_Close(x);
    SDLNet_TCP_Close(server);

    SDLNet_Quit();
    SDL_Quit();

    return 0;
}
