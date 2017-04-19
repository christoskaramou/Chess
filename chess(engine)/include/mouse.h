#ifndef MOUSE_H
#define MOUSE_H
#include <string>
#include <sstream>

template <typename T> std::string to_string(const T& n)
{
    std::ostringstream stm ;
    stm << n;
    return stm.str();
}
struct Point
{
    int x;
    int y;
};
struct Position
{
    int piece;
    int fromNo;
    int toNo;
};
struct ChessPosition
{
    std::string pieceName;
    std::string from;
    std::string to;
};

Point mouse {0, 0};
bool view_second_window = false;
bool r_mouse_down = false;
bool r_mouse_up = true;
bool l_mouse_down = false;
bool l_mouse_up = true;
bool running = true;
bool lock = false;
bool checkingMoves = false;
bool flipView = false;
bool myTurn = false;
bool watchGame = false;
bool solo = false;

#endif // MOUSE_H
