#include <cstdio>
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include "mouse.h"
#include "chessBoard.h"
#include "pollEvents.h"
int main(int argc, char *argv[])
{
    SDL_Window *window, *pwindow;
	SDL_Surface *surface, *psurface;
	SDL_Renderer *renderer, *prenderer;
	int wWidth = 720, wHeight = 720;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
    {
		printf("Warning: Linear texture filtering not enabled!");
		return false;
	}
    if (argc >= 3)
    {
        int atoiNoW = atoi(argv[1]);
        int atoiNoH = atoi(argv[2]);
        wWidth = (atoiNoW >= 320 && atoiNoW <= 1080) ? atoiNoW : wWidth;
        wHeight = (atoiNoH >= 320 && atoiNoH <= 1080) ? atoiNoH : wHeight;
    }

    window = SDL_CreateWindow("Chess Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wWidth, wHeight, SDL_WINDOW_OPENGL);
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    pwindow = SDL_CreateWindow("Pieces Out", x+wWidth+12, y, 320, 320, SDL_WINDOW_HIDDEN);
    if(!window || !pwindow)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n",SDL_GetError());
		return 1;
	}
    surface = SDL_GetWindowSurface(window);
    psurface = SDL_GetWindowSurface(pwindow);
	renderer = SDL_CreateSoftwareRenderer(surface);
	prenderer = SDL_CreateSoftwareRenderer(psurface);
	if(!renderer || !prenderer)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n",SDL_GetError());
		return 1;
	}
	/* Clear the rendering surface with the specified color */
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);

    glewExperimental = GL_TRUE;
    glewInit();

    SDL_Event windowEvent;
    client* socket = new client();
    std::string c = "00";
    if (argc == 5)
    {
        c = socket->startClient(argv[3], atoi(argv[4])); // the server here returns a char to indicate the state of the player
        if (c[0] == 'w')
        {
            flipView = false;
            myTurn = true;
        }
        else if (c[0] == 'b')
        {
            flipView = true;
            myTurn =false;
        }
        else if (c[0] == 'v')
        {
            watchGame = true;
            flipView = false;
        }
        else
        {
            running = false;
            delete socket;
            socket = NULL;
            std::cout << "Could not determinate the received arguments\n";
        }
    }
    else
    {
        solo = true;
        myTurn = true;
    }
    chessBoard *cb = new chessBoard(renderer, prenderer, socket);

    //-----------the main loop-----------
    while (running)
    {
        if (pollEvents(&windowEvent) == -1)
            running = false;
        cb->DrawChessBoard(renderer);
        cb->Update();
        cb->DrawPieces(renderer, prenderer);
        if (view_second_window)
        {
            SDL_ShowWindow(pwindow);
            SDL_UpdateWindowSurface(pwindow);
        }
        else
            SDL_HideWindow(pwindow);
        SDL_UpdateWindowSurface(window);
    }
    //-----------------------------------

    delete socket;
    socket = NULL;
    SDL_Quit();
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
