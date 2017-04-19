#ifndef POLLEVENTS_H
#define POLLEVENTS_H

int pollEvents(SDL_Event* windowEvent)
{
    while (SDL_PollEvent(windowEvent))
    {
        switch(windowEvent->type)
        {
        case SDL_QUIT:
            return -1;
            break;
        case SDL_KEYUP:
            switch (windowEvent->key.keysym.sym)
            {
            case SDLK_ESCAPE:
                return -1;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            switch (windowEvent->button.button)
            {
            case SDL_BUTTON_LEFT:
                l_mouse_down = true;
                mouse.x = windowEvent->motion.x;
                mouse.y = windowEvent->motion.y;
                break;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            switch (windowEvent->button.button)
            {
            case SDL_BUTTON_LEFT:
                l_mouse_down = false;
                lock = false;
                checkingMoves = false;
                break;
            case SDL_BUTTON_RIGHT:
                view_second_window = !view_second_window;
                break;
            }
            break;
        case SDL_MOUSEMOTION:
            if (l_mouse_down)
            {
                mouse.x = windowEvent->motion.x;
                mouse.y = windowEvent->motion.y;
            }
            break;
        }
    }
    return 0;
}

#endif // POLLEVENTS_H
