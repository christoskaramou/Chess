#ifndef CLIENT_H
#define CLIENT_H
#include <iostream>
#include <string>
#include <vector>
using namespace std;
class client
{
    public:
        client();
        ~client();
        string startClient(const char*, const int);
        string getMsg();
        void sendMsg(string);
        SDLNet_SocketSet socketSet;
        TCPsocket Client;
    protected:
    private:
        IPaddress ip;
};
client::client()
{
    socketSet = SDLNet_AllocSocketSet(1);
}
client::~client()
{
    SDLNet_TCP_Close(Client);
    SDLNet_Quit();
}
string client::startClient(const char* address, const int port)
{
    char message[10];
    SDLNet_Init();
    if(SDLNet_ResolveHost(&ip, address, port) == -1) // client
    {
       printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
       exit(1);
    }
    Client = SDLNet_TCP_Open(&ip);
    if(!Client)
    {
       printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
       exit(2);
    }
    SDLNet_TCP_AddSocket(socketSet, Client);
    int result = SDLNet_TCP_Recv(Client, message, 10);
    if(result > 0)
    {
        //std::cout << "Received message: " << string(message) << endl;
    }
    else
    {
        std::cout << "Cannot connect to " << address << ": " << SDLNet_GetError() << endl;
        exit(1);
    }
    return string(message);
}
void client::sendMsg(string msg)
{
    if(!Client)
    {
       printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
       running = false;
    }
    int result = SDLNet_TCP_Send(Client,msg.c_str(),10);
    if(result == 10)
    {
         //std::cout << "Sent move: " << msg << std::endl;
    }
    else
    {
        // invalid socket
        printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        running = false;
    }
}
string client::getMsg()
{
    char message[10];
    if(!Client)
    {
       printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
       running = false;
    }
    SDLNet_CheckSockets(socketSet, 0);
    if (SDLNet_SocketReady(Client))
    {
        int bufferMsgNum = SDLNet_TCP_Recv(Client,message,10);
        if (bufferMsgNum > 0)
            std::cout << "Received move: " << message << std::endl;
        return string(message);
    }
    else return "00 00 00 \n";
}

#endif // CLIENT_H
