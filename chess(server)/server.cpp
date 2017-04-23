#include <iostream>
#include <SDL.h>
#include <SDL_net.h>
#include <cstring>

int main(int argc,char** argv)
{
        SDL_Init(SDL_INIT_EVERYTHING);
        SDLNet_Init();

        IPaddress ip;
        //write "127.0.0.1",1234 to connect to the server.cpp on your local machine
        SDLNet_ResolveHost(&ip,"www.linux.org",80);

        const char* http="GET / HTTP/1.1\nHost: www.linux.org\n\n";

        TCPsocket client=SDLNet_TCP_Open(&ip);

        SDLNet_TCP_Send(client,http,strlen(http)+1);

        char text[10000];

        while(SDLNet_TCP_Recv(client,text,10000))
                std::cout << text;

        SDLNet_TCP_Close(client);

        SDLNet_Quit();
        SDL_Quit();
}
