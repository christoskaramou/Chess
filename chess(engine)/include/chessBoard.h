#ifndef CHESSBOARD_H
#define CHESSBOARD_H
#include <client.h>
#include <string>
#include <mixer.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
#else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#endif
using namespace std;
class chessBoard
{
    public:
        chessBoard(SDL_Renderer*, SDL_Renderer*, client*);
        ~chessBoard() {}
        bool whiteTurn;
        void DrawChessBoard(SDL_Renderer*);
        void DrawPieces(SDL_Renderer*, SDL_Renderer*);
        void Update();
    protected:
    private:
        SDL_Rect windowRect;
        SDL_Rect crect[64];
        SDL_Rect showMoves[64];
        SDL_Rect pieceRect[32];
        SDL_Surface* allPieces;
        SDL_Texture* piecesTex[32];
        SDL_Texture* piecesTex2[32];
        SDL_Renderer* render;
        SDL_Rect tempRect;
        SDL_Rect lastPos;

        Position lastMoved;
        vector<Position> movesPlayed;
        client* clnt;
        mixer* mix;

        int pNumber;
        int mNumber;
        int lNo;
        int pieceMNumber;
        int promotedPiece[32];

        int getPieceNoClicked();
        int getPieceNoFromRect(int);
        int getClosestRect(int, int, int);
        int getRectNoOf(int);
        int checkMovesOf(int, int);

        bool enemy;
        bool threaten;
        bool enPassant;
        bool promotion;
        bool checkingCheckMate;
        bool pieceMoved[32];
        bool excludedPiece[32];

        bool checkMate();
        bool checkIfThreaten(int);
        bool checkIfAlly(int, int);
        bool checkIfEnemy(int, int);
        bool allowMovementOf(int, int, int);
        bool IsBlocked(int, int, int);
        void setRects(SDL_Renderer*);
        void destroyPieceAndRect(int);
        void createNewTexFor(int, int);
        void resetAll(SDL_Renderer*, SDL_Renderer*);

        string encodeMsg(Position*);
        Position decodeMsg(string);
        Position chessPosToPos(ChessPosition);
        ChessPosition posToChessPos(Position);
        void doMoves(vector<Position>);

        char* buffer;
};
chessBoard::chessBoard(SDL_Renderer* renderer, SDL_Renderer* prenderer, client* clnt)
{
    this->clnt = clnt;
    buffer = new char[20];
    render = renderer;
    windowRect.x = 0;
    windowRect.y = 0;
    SDL_GetRendererOutputSize(renderer, &windowRect.w, &windowRect.h);
    mix = new mixer();
    resetAll(renderer, prenderer);
    if (watchGame)
    {
        Position p;
        SDLNet_CheckSockets(clnt->socketSet, 300);
        while (SDLNet_SocketReady(clnt->Client) != 0)
        {
            p = decodeMsg(clnt->getMsg());
            movesPlayed.push_back(p);
        }
        doMoves(movesPlayed);
    }
}
void chessBoard::setRects(SDL_Renderer* renderer) // set all rectangles
{
    SDL_Rect darea;
	/* Get the Size of drawing surface */
	SDL_RenderGetViewport(renderer, &darea);
    if (!flipView)
    {
        for (int row=0; row < 8; row++) // general area rect
        {
            for (int coloum=0; coloum < 8; coloum++)
            {
                crect[row*8+coloum].w = darea.w/8;
                crect[row*8+coloum].h = darea.h/8;
                crect[row*8+coloum].x = coloum*(crect[(row*8)+coloum].w);
                crect[row*8+coloum].y = row*(crect[(row*8)+coloum].h);
                showMoves[row*8+coloum].x = windowRect.w;
                showMoves[row*8+coloum].y = windowRect.h;
                showMoves[row*8+coloum].w = ((2*darea.w)/3)/8;
                showMoves[row*8+coloum].h = darea.h/8;
            }
        }
    }
    else
    {
        for (int row=7; row >= 0; row--) // general area rect
        {
            for (int coloum=7; coloum >= 0; coloum--)
            {
                crect[row*8+coloum].w = darea.w/8;
                crect[row*8+coloum].h = darea.h/8;
                crect[row*8+coloum].x = (7-coloum)*(crect[(row*8)+coloum].w);
                crect[row*8+coloum].y = (7-row)*(crect[(row*8)+coloum].h);
                showMoves[row*8+coloum].x = windowRect.w;
                showMoves[row*8+coloum].y = windowRect.h;
                showMoves[row*8+coloum].w = ((2*darea.w)/3)/8;
                showMoves[row*8+coloum].h = darea.h/8;
            }
        }
    }
    for (int i=0; i < 4; i++)
    {
        for (int j=0; j < 8; j++)
        {
            if (i == 0)
                pieceRect[j] = crect[j];
            else if (i == 1)
                pieceRect[i*8+j] = crect[8+j];
            else if (i == 2)
                pieceRect[i*8+j] = crect[6*8+j];
            else
                pieceRect[i*8+j] = crect[7*8+j];
        }
    }
    lastPos.x = 0; lastPos.y = 0; lastPos.w = 0; lastPos.h = 0; // random initialize
}
void chessBoard::resetAll(SDL_Renderer* renderer, SDL_Renderer* prenderer) // resets all positions and textures
{
    myTurn = !flipView;
    pNumber = -1;
    mNumber = -1;
    lNo = -1;
    pieceMNumber = -1;
    whiteTurn = true;
    enemy = false;
    threaten = false;
    checkingCheckMate = false;
    enPassant = false;
    promotion = false;
    for (bool &x : pieceMoved) x = false;
    for (int &x : promotedPiece) x = -1;
    for (bool &x : excludedPiece) x = false;
    lastMoved.piece = 0;
    lastMoved.fromNo = 0;
    lastMoved.toNo = 0;

    allPieces = IMG_Load("images\\ChessPieces.png");
    SDL_Rect darea; darea.x = 0; darea.y = 0; darea.w = 155; darea.h = 192;
    SDL_Rect rec; rec.x = 0; rec.y = 0; rec.w = 155; rec.h = 192;

    //the actual cuts from all pieces image surface
    for (int i=0; i<4 ; i++)
    {
        for (int j=0; j<8 ;j++)
        {
            if (i == 0)
            {
                if (j == 0 || j == 7)      // rook black
                {
                    rec.x = 0 ; rec.y = 0;
                }
                else if (j == 1 || j == 6)  // knight black
                {
                    rec.x = 715 ; rec.y = 0;
                }
                else if (j == 2 || j == 5)  // bishop black
                {
                    rec.x = 170 ; rec.y = 0;
                }
                else if(j == 3)             // queen black
                {
                    rec.x = 360 ; rec.y = 0;
                }
                else                        // king black
                {
                    rec.x = 535 ; rec.y = 0;
                }
            }
            else if (i == 1)                // pawn black
            {
                rec.x = 890 ; rec.y = 0;
            }
            else if (i == 2)                // pawn white
            {
                rec.x = 890 ; rec.y = 210;
            }
            else if (i == 3)
            {
                if (j == 0 || j == 7)       // rook white
                {
                    rec.x = 0 ; rec.y = 210;
                }
                else if (j == 1 || j == 6)  // knight white
                {
                    rec.x = 715 ; rec.y = 210;
                }
                else if (j == 2 || j == 5)  // bishop white
                {
                    rec.x = 170 ; rec.y = 210;
                }
                else if(j == 3)             // queen white
                {
                    rec.x = 360 ; rec.y = 210;
                }
                else                        // king white
                {
                    rec.x = 535 ; rec.y = 210;
                }
            }
            //Blit surface in pieces of textures
            SDL_Surface* tempS = SDL_CreateRGBSurface(0, darea.w, darea.h, 32, rmask, gmask, bmask, amask);
            SDL_BlitSurface(allPieces, &rec, tempS, &darea);
            piecesTex[i*8+j] = SDL_CreateTextureFromSurface(renderer, tempS);

            tempS = SDL_CreateRGBSurface(0, darea.w, darea.h, 32, rmask, gmask, bmask, amask);
            SDL_BlitSurface(allPieces, &rec, tempS, &darea);
            piecesTex2[i*8+j] = SDL_CreateTextureFromSurface(prenderer, tempS);

            SDL_FreeSurface(tempS);
        }
        setRects(renderer);
    }
}
void chessBoard::destroyPieceAndRect(int pieceNo) // when a piece has been cut
{
    if (pieceNo >= 0 && pieceNo <= 31)
    {
        SDL_DestroyTexture(piecesTex[pieceNo]);
        pieceRect[pieceNo].x = windowRect.w;
        pieceRect[pieceNo].y = windowRect.h;
        pieceRect[pieceNo].w = 0;
        pieceRect[pieceNo].h = 0;
        excludedPiece[pieceNo] = true;
    }
}
void chessBoard::createNewTexFor(int fromPiece, int intoPiece) // converts a piece into an other
{
    int i = intoPiece/8;
    int j = intoPiece%8;
    SDL_Rect darea; darea.x = 0; darea.y = 0; darea.w = 155; darea.h = 192;
    SDL_Rect rec; rec.x = 0; rec.y = 0; rec.w = 155; rec.h = 192;

    if (i == 0)
    {
        if (j == 0 || j == 7)      // rook black
        {
            rec.x = 0 ; rec.y = 0;
        }
        else if (j == 1 || j == 6)  // knight black
        {
            rec.x = 715 ; rec.y = 0;
        }
        else if (j == 2 || j == 5)  // bishop black
        {
            rec.x = 170 ; rec.y = 0;
        }
        else if(j == 3)             // queen black
        {
            rec.x = 360 ; rec.y = 0;
        }
        else                        // king black
        {
            rec.x = 535 ; rec.y = 0;
        }
    }
    else if (i == 1)                // pawn black
    {
        rec.x = 890 ; rec.y = 0;
    }
    else if (i == 2)                // pawn white
    {
        rec.x = 890 ; rec.y = 210;
    }
    else if (i == 3)
    {
        if (j == 0 || j == 7)       // rook white
        {
            rec.x = 0 ; rec.y = 210;
        }
        else if (j == 1 || j == 6)  // knight white
        {
            rec.x = 715 ; rec.y = 210;
        }
        else if (j == 2 || j == 5)  // bishop white
        {
            rec.x = 170 ; rec.y = 210;
        }
        else if(j == 3)             // queen white
        {
            rec.x = 360 ; rec.y = 210;
        }
        else                        // king white
        {
            rec.x = 535 ; rec.y = 210;
        }
    }
    //Blit surface in pieces of textures
    SDL_Surface* tempS = SDL_CreateRGBSurface(0, darea.w, darea.h, 32, rmask, gmask, bmask, amask);
    SDL_BlitSurface(allPieces, &rec, tempS, &darea);
    SDL_DestroyTexture(piecesTex[fromPiece]);
    piecesTex[fromPiece] = SDL_CreateTextureFromSurface(render, tempS);
    SDL_FreeSurface(tempS);
}
void chessBoard::DrawChessBoard(SDL_Renderer* renderer) // the every frame chessboard and possible moves drawer
{
	SDL_Rect rect, darea;

	/* Get the Size of drawing surface */
	SDL_RenderGetViewport(renderer, &darea);

	for(int row = 0;row < 8; row++)
	{
	    if (row%2 == 0)
        {
            for(int coloum = 0; coloum < 8; coloum++)
            {
                if (coloum%2 != 0)
                {
                    rect.w = darea.w/8;
                    rect.h = darea.h/8;
                    rect.x = coloum * rect.w;
                    rect.y = row * rect.h;
                    SDL_SetRenderDrawColor(renderer, 96, 96, 96, 0xFF);
                    SDL_RenderFillRect(renderer, &rect);
                }
                else
                {
                    rect.w = darea.w/8;
                    rect.h = darea.h/8;
                    rect.x = coloum * rect.w;
                    rect.y = row * rect.h;
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 0xFF);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        else
        {
            for(int coloum = 0; coloum < 8; coloum++)
            {
                if (coloum%2 == 0)
                {
                    rect.w = darea.w/8;
                    rect.h = darea.h/8;
                    rect.x = coloum * rect.w;
                    rect.y = row * rect.h;
                    SDL_SetRenderDrawColor(renderer, 96, 96, 96, 0xFF);
                    SDL_RenderFillRect(renderer, &rect);
                }
                else
                {
                    rect.w = darea.w/8;
                    rect.h = darea.h/8;
                    rect.x = coloum * rect.w;
                    rect.y = row * rect.h;
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 0xFF);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
	}
	if (checkingMoves)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for(int i=0; i<64; i++)
        {
            if (showMoves[i].x == windowRect.w && showMoves[i].y == windowRect.h)
                continue;
            SDL_SetRenderDrawColor(renderer, 155, 200, 255, 100);
            SDL_RenderFillRect(renderer, &showMoves[i]);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}
void chessBoard::DrawPieces(SDL_Renderer* renderer, SDL_Renderer* prenderer) // the piece drawer
{
    for (int i=0; i<32 ; i++)
    {
        if (!excludedPiece[i])
            SDL_RenderCopy(renderer, piecesTex[i], NULL, &pieceRect[i]);
    }
    if (view_second_window)
    {
        SDL_Rect rect, darea;
        SDL_RenderGetViewport(prenderer, &darea); rect.w = darea.w/8; rect.h = darea.h/8;

        SDL_SetRenderDrawColor(prenderer, 200, 200, 200, 0xFF);
        SDL_RenderFillRect(prenderer, &darea);

        int counter = 0;
        int counter2 = 0;
        for (int i=0; i<32; i++)
        {
            if (excludedPiece[i])
            {
                if (i >= 0 && i <= 15)
                {
                    rect.x = (counter%8)*rect.w;
                    rect.y = (counter/8+6)*rect.h;
                    counter++;
                }
                else
                {
                    rect.x = (counter2%8)*rect.w;
                    rect.y = (counter2/8)*rect.h;
                    counter2++;
                }
                SDL_RenderCopy(prenderer, piecesTex2[i], NULL, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
    SDL_RenderPresent(prenderer);
}
Position chessBoard::decodeMsg(string buf) // decodes the message (or even if not), always returns a Position
{
    Position pos;
    if (buf == "00 00 00 \n")
    {
        pos.piece = -1;
        pos.fromNo = 0;
        pos.toNo = 0;
        return pos;
    }
    int msg[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int i=0;
    while(buf[i] != '\n')
    {
        if (buf[i] != ' ')
            msg[i] = buf[i] - '0'; // convert char to integer
        i++;
    }
    if (msg[0] == 0)
        pos.piece = msg[1];
    else
        pos.piece = msg[0]*10 + msg[1];
    if (msg[3] == 0)
        pos.fromNo = msg[4];
    else
        pos.fromNo = msg[3]*10 + msg[4];
    if (msg[6] == 0)
        pos.toNo = msg[7];
    else
        pos.toNo = msg[6]*10 + msg[7];
    cout << "Decoded message: " << pos.piece << " " << pos.fromNo << " " << pos.toNo << endl;
    return pos;
}
string chessBoard::encodeMsg(Position* pos) // encodes a Position to string
{
    string buf = "1234567890"; // 10 characters string
    string p = to_string(pos->piece);
    string f = to_string(pos->fromNo);
    string t = to_string(pos->toNo);
    if (pos->piece < 10)
    {
        buf[0] = '0';
        buf[1] = p[0];
    }
    else
    {
        buf[0] = p[0];
        buf[1] = p[1];
    }
    buf[2] = ' ';
    if (pos->fromNo < 10)
    {
        buf[3] = '0';
        buf[4] = f[0];
    }
    else
    {
        buf[3] = f[0];
        buf[4] = f[1];
    }
    buf[5] = ' ';
    if (pos->toNo < 10)
    {
        buf[6] = '0';
        buf[7] = t[0];
    }
    else
    {
        buf[6] = t[0];
        buf[7] = t[1];
    }
    buf[8] = ' ';
    buf[9] = '\n';

    cout << "Encoded msg: " << buf << endl;
    return buf;
}
ChessPosition chessBoard::posToChessPos(Position pos) // coverts the int Positions to Chess real positions
{
    ChessPosition cpos;
    switch (pos.piece)
    {
    case 0:
    case 7:
        cpos.pieceName = "Black Rook";
        break;
    case 1:
    case 6:
        cpos.pieceName = "Black Knight";
        break;
    case 2:
    case 5:
        cpos.pieceName = "Black Bishop";
        break;
    case 3:
        cpos.pieceName = "Black Queen";
        break;
    case 4:
        cpos.pieceName = "Black King";
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        cpos.pieceName = "Black Pawn";
        break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        cpos.pieceName = "White Pawn";
        break;
    case 24:
    case 31:
        cpos.pieceName = "White Rook";
        break;
    case 25:
    case 30:
        cpos.pieceName = "White Knight";
        break;
    case 26:
    case 29:
        cpos.pieceName = "White Bishop";
        break;
    case 27:
        cpos.pieceName = "White Queen";
        break;
    case 28:
        cpos.pieceName = "White King";
        break;

    default:
        cpos.pieceName = "The One";
    }
    char a = pos.fromNo%8 + 'a';
    char b = '8' - pos.fromNo/8;
    cpos.from = to_string(a) + to_string(b);
    a = pos.toNo%8 + 'a';
    b = '8' - pos.toNo/8;
    cpos.to = to_string(a) + to_string(b);

    return cpos;
}
int chessBoard::getPieceNoClicked() // if the clicked rectangle contains a piece, gets its piece number
{
    int offset; // this offset disallow to select opposite pieces
    if (whiteTurn)
        offset=16;
    else
        offset=0;
    for (int i=0; i<16; i++)
    {
        if (mouse.x >= pieceRect[i+offset].x
            && mouse.y >= pieceRect[i+offset].y
            && mouse.x <= pieceRect[i+offset].x + pieceRect[i+offset].w
            && mouse.y <= pieceRect[i+offset].y + pieceRect[i+offset].h)
                return i+offset;
    }
    return -1;
}
int chessBoard::getRectNoOf(int pieceNumb) // gets the rectangle number of a piece
{
    if (pieceNumb >= 0 && pieceNumb <= 31)
    {
        for (int i=0; i<64; i++)
        {
            if (SDL_RectEquals(&pieceRect[pieceNumb], &crect[i]))
                return i;
        }
    }
    return -1;

}
int chessBoard::getPieceNoFromRect(int rectNo) // if the rectangle contains a piece, gets its piece number
{
    if (rectNo >= 0 && rectNo <= 64)
    {
        for (int i=0; i<32; i++)
        {
            if (SDL_RectEquals(&crect[rectNo], &pieceRect[i]))
                return i;
        }
    }
    return -1;
}
int chessBoard::getClosestRect(int pieceNo, int x, int y) // gets the rectangle number by given mouse cords
{
    if (pieceNo < 0)
        return -1;
    for (int i=0; i<64; i++)
    {
        if (x >= crect[i].x
            && y >= crect[i].y
            && x <= crect[i].x + crect[i].w
            && y <= crect[i].y + crect[i].h)
            return i;
    }
    return -1;
}
bool chessBoard::checkIfAlly(int piece, int targetRectNo) // checks if the move is targeting allied piece
{
    int pNoFR = getPieceNoFromRect(targetRectNo);
    if (pNoFR == -1)
        return false;
    if (piece < 16)
    {
        if (pNoFR >= 16)
            return false;
        else
            return true;
    }
    else
    {
        if (pNoFR < 16)
            return false;
        else
            return true;
    }
}
bool chessBoard::checkIfEnemy(int pieceToCheck, int targetRectNo) // checks if the move is targeting enemy piece
{
    int pNoFR = getPieceNoFromRect(targetRectNo);
    if (pNoFR == -1)
        return false;
    if (pieceToCheck < 16)
    {
        if (pNoFR >= 16)
            return true;
        else
            return false;
    }
    else
    {
        if (pNoFR < 16)
            return true;
        else
            return false;
    }
}
bool chessBoard::checkMate() /*currently not working*/ // checks if it is checkmate
{

    int offset = whiteTurn ? 0 : 16; // 0 to 15 are black pieces and 16 to 31 are white
    int totalMoves = 0;

    for(int i=offset; i<16+offset; i++)
    {
        if (!excludedPiece[i])
            totalMoves += checkMovesOf(i, getRectNoOf(i));
    }
    return totalMoves == 0;
}
int chessBoard::checkMovesOf(int pieceNo, int fromNo) // checking for allowable moves
{
    int positionsCounter = 0;
    SDL_Rect lPos;
    int targetPieceNo = -1;
    bool enemyCheck = false;
    bool kingThreat = false;

    for (int i=0; i<64; i++)
    {
        // resetting showMoves x and y first to default values that
        //  we chose, so the DrawPieces() wont draw the last ones
        showMoves[i].x = windowRect.w;
        showMoves[i].y = windowRect.h;
        if (allowMovementOf(pieceNo, fromNo, i))
        {
            enemyCheck = checkIfEnemy(pieceNo, i);
            if (enemyCheck)
            {
                targetPieceNo = getPieceNoFromRect(i);
                excludedPiece[targetPieceNo] = true;
            }
            lPos = pieceRect[pieceNo];
            pieceRect[pieceNo] = crect[i];
            kingThreat = whiteTurn ? checkIfThreaten(getRectNoOf(28)) : checkIfThreaten(getRectNoOf(4));
            if (!kingThreat)
            {
                if (!checkingCheckMate)
                    showMoves[i] = crect[i];
                positionsCounter++;
            }

            // set back position and rectangle if it was an enemy
            if (enemyCheck)
                excludedPiece[targetPieceNo] = false;
            pieceRect[pieceNo] = lPos;
        }
    }
    return positionsCounter;
}
bool chessBoard::checkIfThreaten(int targetRect) // checks only if this rectangle is targeted by enemy
{
    // check if the opposite pieces are legal to go to that target rectangle
    int rectNo;
    int offset = whiteTurn ? 0 : 16; // 0 to 15 are black pieces and 16 to 31 are white
    for (int i=offset; i<16+offset; i++)
    {
        rectNo = getRectNoOf(i);
        // to avoid checking excluded pieces
        if (excludedPiece[i])
                continue;
        if (allowMovementOf(i, rectNo, targetRect)) // checks if the enemy can target this rectangle
            return true;
    }
    return false;

}
bool chessBoard::IsBlocked(int pieceN, int srcRect, int destRect) // pieces that can move more or equal than 2 squares often are blocked by other pieces
{
    // this function checks if an other piece is blocking the way of
    // this particular piece, in horizontal and vertical motions
    bool diagonal = false;
    bool orthogonal = false;
    int distance = destRect > srcRect ? destRect - srcRect : srcRect - destRect;
    int direction = destRect > srcRect ? 1 : -1;
    int x = 0, y = 0, counter = 0;
    int startX = srcRect%8;
    int startY = srcRect/8;
    int destX = destRect%8;
    int destY = destRect/8;

    if (startX == destX || startY == destY)
    {
        orthogonal = true;
        diagonal = false;
    }
    else
    {
        orthogonal = false;
        diagonal = true;
    }


    if (pieceN == 0 || pieceN == 7 || pieceN == 24 || pieceN == 31) // rooks
    {
        if (orthogonal)
        {
            if (startY == destY)    // horizontal
            {
                for(int i=srcRect+direction; i!=destRect; i=i+direction)
                {
                    if (getPieceNoFromRect(i) != -1)
                        return true;
                }
            }
            else                    // vertical
            {
                for(int i=startY+direction; i!=destY; i=i+direction)
                {
                    y=i;
                    x=startX;
                    if (getPieceNoFromRect(y*8+x) != -1)
                        return true;
                }
            }
        }
    }
    else if (pieceN == 2 || pieceN == 5 || pieceN == 26 || pieceN == 29) // bishops
    {
        if (diagonal)
        {
            for(int i=startY+direction; i!=destY; i=i+direction)
            {
                counter++;
                y=i;
                if((destX > startX && destY < startY) || (destX < startX && destY > startY))
                    x=((srcRect+(8*counter)*direction)%8)-counter*direction;
                else
                    x=((srcRect+(8*counter)*direction)%8)+counter*direction;
                if (getPieceNoFromRect(y*8+x) != -1)
                    return true;
            }
            counter = 0;
        }
    }
    else if (pieceN == 3 || pieceN == 27) // queens
    {
        if (diagonal)
        {
            for(int i=startY+direction; i!=destY; i=i+direction)
            {
                counter++;
                y=i;
                if((destX > startX && destY < startY) || (destX < startX && destY > startY))
                    x=((srcRect+(8*counter)*direction)%8)-counter*direction;
                else
                    x=((srcRect+(8*counter)*direction)%8)+counter*direction;
                if (getPieceNoFromRect(y*8+x) != -1)
                    return true;
            }
            counter = 0;
        }
        else if (orthogonal)
        {
            if (startY == destY)    // horizontal
            {
                for(int i=srcRect+direction; i!=destRect; i=i+direction)
                {
                    if (getPieceNoFromRect(i) != -1)
                        return true;
                }
            }
            else                    // vertical
            {
                for(int i=startY+direction; i!=destY; i=i+direction)
                {
                    y=i;
                    x=startX;
                    if (getPieceNoFromRect(y*8+x) != -1)
                        return true;
                }
            }
        }
    }
    else if ((pieceN == 4 && srcRect == 4) || (pieceN == 28 && srcRect == 60)) // kings in start position
    {
        if (distance == 2 && startY == destY)
        {
            for (int i=srcRect+direction; i!=destRect; i=i+direction)
            {
                if (getPieceNoFromRect(i) != -1)
                        return true;
            }
        }
    }
    else if (pieceN >= 8 && pieceN <= 23) // pawns
    {
        if (startY == 6 || startY == 1) // in start position
        {
            if (srcRect+(16*direction) == destRect)
                for (int i=startY+direction; i!=destY; i=i+direction)
                {
                    y=i;
                    x=startX;
                    if (getPieceNoFromRect(y*8+x) != -1)
                        return true;
                }
        }
    }
    return false;
}
bool chessBoard::allowMovementOf(int pieceNo, int fromNo, int toNo) // checks the allowed "base" moves of a piece plus if its not blocked and it is not going on an ally
{
    if (pieceNo < 0 || pieceNo > 31 || fromNo < 0 || fromNo > 64 || toNo < 0 || toNo > 64)
        return false;
    if (fromNo == toNo)
        return false;
    int realPieceNo = pieceNo;
    if (promotedPiece[pieceNo] >= 0)
        pieceNo = promotedPiece[pieceNo];
    int x=0, y=0, i=0, j=0;
    bool allow = false;
    int startX = fromNo%8;
    int startY = fromNo/8;
    int destX = toNo%8;
    int destY = toNo/8;
    SDL_Rect enemyRect = crect[toNo];
    SDL_Rect tempPositionAlly = {windowRect.w, windowRect.h, windowRect.w/8, windowRect.h/8};
    enemy = checkIfEnemy(pieceNo, toNo);

    switch(pieceNo)
    {
    case 0: // rook black
    case 7:
        if (fromNo%8 == toNo%8 || crect[fromNo].y == crect[toNo].y)
            allow = true;
        break;
    case 1: // knight black
    case 6:
        if (fromNo+10 == toNo || fromNo+15 == toNo
            || fromNo+17 == toNo || fromNo+6 == toNo
            || fromNo-10 == toNo || fromNo-17 == toNo
            || fromNo-15 == toNo || fromNo-6 == toNo)
            {
                if ((destX-startX) <= 2 && (destX-startX) >= -2 &&
                    (destY-startY) <= 2 && (destY-startY) >= -2)
                    allow = true;
            }
        break;
    case 2: // bishop black
    case 5:
        x=fromNo%8;
        y=fromNo/8;
        i=toNo%8;
        j=toNo/8;
        for (int k=1; k<8; k++)
        {
            if ((x+k == i && y+k == j)
                || (x-k == i && y-k == j)
                || (x+k == i && y-k == j)
                || (x-k == i && y+k == j))
            {
                allow = true;
                break;
            }
        }
        break;
    case 3: // queen black
        if (fromNo%8 == toNo%8 || crect[fromNo].y == crect[toNo].y)
        {
            allow = true;
            break;
        }
        x=fromNo%8;
        y=fromNo/8;
        i=toNo%8;
        j=toNo/8;
        for (int k=1; k<8; k++)
        {
            if ((x+k == i && y+k == j)
                || (x-k == i && y-k == j)
                || (x+k == i && y-k == j)
                || (x-k == i && y+k == j))
            {
                allow = true;
                break;
            }
        }
        break;
    case 4: // king black
        if (fromNo-1 == toNo || fromNo-9 == toNo
            || fromNo-8 == toNo || fromNo-7 == toNo
            || fromNo+1 == toNo || fromNo+9 == toNo
            || fromNo+8 == toNo || fromNo+7 == toNo)
            {
                if ((destX-startX) <= 1 && (destX-startX) >= -1 &&
                    (destY-startY) <= 1 && (destY-startY) >= -1)
                    {
                        if (!enemy)
                        {
                            tempPositionAlly = pieceRect[pieceNo];
                            pieceRect[pieceNo] = crect[toNo];
                            if (!checkIfThreaten(toNo))
                                allow = true;
                            pieceRect[pieceNo] = tempPositionAlly;
                        }
                        else
                        {   // damned needed checks and moves else it's not going to work for the king (it took me a while to figure this out)
                            int enemyPNo = getPieceNoFromRect(toNo);
                            enemyRect = crect[toNo];
                            pieceRect[enemyPNo].x = windowRect.w;
                            pieceRect[enemyPNo].y = windowRect.h;
                            excludedPiece[enemyPNo] = true;
                            tempPositionAlly = pieceRect[pieceNo];
                            pieceRect[pieceNo] = crect[toNo];
                            if (!checkIfThreaten(toNo))
                                allow = true;
                            pieceRect[pieceNo] = tempPositionAlly;
                            pieceRect[enemyPNo] = enemyRect;
                            excludedPiece[enemyPNo] = false;
                        }
                    }
            }
        else if (fromNo == 4 && !checkIfAlly(pieceNo, toNo) && !IsBlocked(pieceNo, fromNo, toNo))
        {
            if (toNo == 2 && !pieceMoved[4] && !pieceMoved[0])
            {
                if (!IsBlocked(0, 0, 3))
                {
                    if (!checkIfThreaten(4) && !checkIfThreaten(3) && !checkIfThreaten(2))
                    {
                        if (!checkingMoves && !checkingCheckMate)
                        {
                            pieceRect[0] = crect[3];
                            pieceMoved[0] = true;
                        }
                        return true; // only return works at this point
                    }
                }
            }
            else if (toNo == 6 && !pieceMoved[4] && !pieceMoved[7])
            {
                if (!IsBlocked(7, 7, 5))
                {
                    if (!checkIfThreaten(4) && !checkIfThreaten(5) && !checkIfThreaten(6))
                    {

                        if (!checkingMoves && !checkingCheckMate)
                        {
                            pieceRect[7] = crect[5];
                            pieceMoved[5] = true;
                        }
                        return true; // only return works at this point
                    }
                }
            }
        }
        break;
    case 8: // pawn black
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        if (fromNo+7 == toNo || fromNo+9 == toNo)
        {
            if ((destX-startX) != -1 || (destX-startX) != 1)
            {
                if (enemy)
                    allow = true;
                else if(fromNo/8 == 4 && lastMoved.piece >= 16 && lastMoved.piece <= 23 &&
                        lastMoved.fromNo/8 == 6)
                {
                    if (lastMoved.toNo%8 == toNo%8)
                    {
                        if (!checkingMoves && !checkingCheckMate)
                            enPassant = true;
                        allow = true;
                    }
                }
            }
        }
        else if (fromNo+8 == toNo)
        {
            if(!enemy)
                allow = true;
        }
        else if (fromNo/8 == 1 && fromNo+16 == toNo)
        {
            if (!enemy)
                allow = true;
        }
        break;
    case 16: // pawn white
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
        if (fromNo-7 == toNo || fromNo-9 == toNo)
        {
            if ((destX-startX) != -1 || (destX-startX) != 1)
            {
                if (enemy)
                    allow = true;
                else if(fromNo/8 == 3 && lastMoved.piece >= 0 && lastMoved.piece <= 15 &&
                        lastMoved.fromNo/8 == 1)
                {
                    if (lastMoved.toNo%8 == toNo%8)
                    {
                        if (!checkingMoves && !checkingCheckMate)
                            enPassant = true;
                        allow = true;
                    }
                }
            }
        }
        else if (fromNo-8 == toNo)
        {
            if(!enemy)
                allow = true;
        }
        else if (fromNo/8 == 6 && fromNo-16 == toNo)
        {
            if (!enemy)
                allow = true;
        }
        break;
    case 24: // rook white
    case 31:
        if (fromNo%8 == toNo%8 || crect[fromNo].y == crect[toNo].y)
            allow = true;
        break;
    case 25: // knight white
    case 30:
        if (fromNo+10 == toNo || fromNo+15 == toNo
            || fromNo+17 == toNo || fromNo+6 == toNo
            || fromNo-10 == toNo || fromNo-17 == toNo
            || fromNo-15 == toNo || fromNo-6 == toNo)
            {
                if ((destX-startX) <= 2 && (destX-startX) >= -2 &&
                    (destY-startY) <= 2 && (destY-startY) >= -2)
                    allow = true;
            }
        break;
    case 26: // bishop white
    case 29:
        x=fromNo%8;
        y=fromNo/8;
        i=toNo%8;
        j=toNo/8;
        for (int k=1; k<8; k++)
        {
            if ((x+k == i && y+k == j)
                || (x-k == i && y-k == j)
                || (x+k == i && y-k == j)
                || (x-k == i && y+k == j))
            {
                allow = true;
                break;
            }
        }
        break;
    case 27: // queen white
        if (fromNo%8 == toNo%8 || crect[fromNo].y == crect[toNo].y)
        {
            allow = true;
            break;
        }
        x=fromNo%8;
        y=fromNo/8;
        i=toNo%8;
        j=toNo/8;
        for (int k=1; k<8; k++)
        {
            if ((x+k == i && y+k == j)
                || (x-k == i && y-k == j)
                || (x+k == i && y-k == j)
                || (x-k == i && y+k == j))
            {
                allow = true;
                break;
            }
        }
        break;
    case 28: // king white
        if (fromNo-1 == toNo || fromNo-9 == toNo
            || fromNo-8 == toNo || fromNo-7 == toNo
            || fromNo+1 == toNo || fromNo+9 == toNo
            || fromNo+8 == toNo || fromNo+7 == toNo)
            {
                if ((destX-startX) <= 1 && (destX-startX) >= -1 &&
                    (destY-startY) <= 1 && (destY-startY) >= -1)
                    {
                        if (!enemy)
                        {
                            tempPositionAlly = pieceRect[pieceNo];
                            pieceRect[pieceNo] = crect[toNo];
                            if (!checkIfThreaten(toNo))
                                allow = true;
                            pieceRect[pieceNo] = tempPositionAlly;
                        }
                        else
                        {   // damned needed checks and moves else it's not going to work for the king (it took me a while to figure this out)
                            int enemyPNo = getPieceNoFromRect(toNo);
                            enemyRect = crect[toNo];
                            pieceRect[enemyPNo].x = windowRect.w;
                            pieceRect[enemyPNo].y = windowRect.h;
                            excludedPiece[enemyPNo] = true;
                            tempPositionAlly = pieceRect[pieceNo];
                            pieceRect[pieceNo] = crect[toNo];
                            if (!checkIfThreaten(toNo))
                                allow = true;
                            pieceRect[pieceNo] = tempPositionAlly;
                            pieceRect[enemyPNo] = enemyRect;
                            excludedPiece[enemyPNo] = false;
                        }
                    }
            }
        else if (fromNo == 60 && !checkIfAlly(pieceNo, toNo) && !IsBlocked(pieceNo, fromNo, toNo))
        {
            if (toNo == 58 && !pieceMoved[28] && !pieceMoved[24])
            {
                if (!IsBlocked(24, 56, 59))
                {
                    if (!checkIfThreaten(60) && !checkIfThreaten(59) && !checkIfThreaten(58))
                    {
                        if (!checkingMoves && !checkingCheckMate)
                        {
                            pieceRect[24] = crect[59];
                            pieceMoved[24] = true;
                        }
                        return true; // only return works at this point
                    }
                }
            }
            else if (toNo == 62 && !pieceMoved[28] && !pieceMoved[31])
            {
                if (!IsBlocked(31, 63, 61))
                {
                    if (!checkIfThreaten(60) && !checkIfThreaten(61) && !checkIfThreaten(62))
                    {
                        if (!checkingMoves && !checkingCheckMate)
                        {
                            pieceRect[31] = crect[61];
                            pieceMoved[31] = true;
                        }
                        return true; // only return works at this point
                    }
                }
            }
        }
        break;
    default:
        allow = false;
    }
    if (allow) // if base movement is allowed
    {
        if (!checkIfAlly(pieceNo, toNo) && !IsBlocked(pieceNo, fromNo, toNo))
        {
            if (!checkingMoves && !checkingCheckMate)
            {
                if (pieceNo >= 8 && pieceNo <= 15 && destY == 7) // this is a black pawn candidate for promotion
                {
                    promotedPiece[realPieceNo] = 3;
                    promotion = true;
                }
                if (pieceNo >= 16 && pieceNo <= 23 && destY == 0) // this is a white pawn candidate for promotion
                {
                    promotedPiece[realPieceNo] = 27;
                    promotion = true;
                }
            }
            return true;
        }
        else
            return false;
    }
    else
        return false;

}
void chessBoard::doMoves(vector<Position> moves) /* not used yet*/ // does multiple moves based on a vector of Positions
{
     // must be valid moves else, who knows!

    for (Position &pos : moves)
    {
        pNumber = pos.piece;
        lNo = pos.fromNo;
        mNumber = pos.toNo;
        // mandatory to hold position before is changed
        pieceMNumber = getPieceNoFromRect(mNumber);

        // checks the base movement of pieces
        if (allowMovementOf(pNumber, lNo, mNumber))
        {
            // checks if it gets an enemy
            bool enemyCheck = checkIfEnemy(pNumber, mNumber);

            // do the move, so that checkIfThreaten checks if the move is OK
            pieceRect[pNumber] = crect[mNumber];

            // undo the temporally set off of the enemy
            if (enemyCheck)
                destroyPieceAndRect(pieceMNumber);
            if (enPassant) // a special move that needs to be checked here
            {
                destroyPieceAndRect(lastMoved.piece);
                enPassant = false;
            }
            if (promotion)
            {
                createNewTexFor(pNumber, promotedPiece[pNumber]);
                promotion = false;
            }

            pieceMoved[pNumber] = true;

            lastMoved.piece = pNumber;
            lastMoved.fromNo = lNo;
            lastMoved.toNo = mNumber;
            movesPlayed.push_back(lastMoved);

            whiteTurn = !whiteTurn;
            if (!solo)
                myTurn = !myTurn;
        }
    }
}
void chessBoard::Update() // the main update function with the logic of chess
{

    if (l_mouse_down && myTurn && !watchGame)
    {
        // lock is starting with a false value, thus it will get in here after l_mouse_down state is true
        if (!lock) // lock is a secure variable used to help not to get multiple mouse events when left click is down
        {
            pNumber = getPieceNoClicked();
            lNo = getRectNoOf(pNumber);
            lastPos = crect[lNo];
            checkingMoves = true;
            checkMovesOf(pNumber, lNo);
            lock = true;
        }
        // if a piece is clicked and if mouse button is locked down make this piece follow the mouse cords
        if (pNumber != -1 && lock == true)
        {
            if (mouse.x <= 0)
                mouse.x = 0;
            if (mouse.x >= (windowRect.w/8)*8 - 2)
                mouse.x = (windowRect.w/8)*8 - 2;
            if (mouse.y <= 0)
                mouse.y = 0;
            if (mouse.y >= (windowRect.h/8)*8 - 2)
                mouse.y = (windowRect.h/8)*8 - 2;
            if (mouse.x > pieceRect[pNumber].w/2 && mouse.x < windowRect.w - pieceRect[pNumber].w/2)
                pieceRect[pNumber].x = mouse.x - pieceRect[pNumber].w/2;
            if (mouse.y > pieceRect[pNumber].h/2 && mouse.y < windowRect.h - pieceRect[pNumber].h/2)
                pieceRect[pNumber].y = mouse.y - pieceRect[pNumber].h/2;
        }
    }
    else // if the mouse click is not down
    {
        if (watchGame) // watch game means that this player cannot move any piece, just receive moves
        {
            Position p = decodeMsg(clnt->getMsg());
            pNumber = p.piece;
            lNo = p.fromNo;
            mNumber = p.toNo;
        }
        else if (!myTurn) // if myTurn is false, then we need to get the move over the network
        {
            Position p = decodeMsg(clnt->getMsg());
            pNumber = p.piece;
            lNo = p.fromNo;
            mNumber = p.toNo;
        }
        else
            mNumber = getClosestRect(pNumber, mouse.x, mouse.y); // mNumber will have a rectangle number, unless pNumber equals -1

        if (mNumber > -1) // this also checks if pNumber is > -1 to avoid segmentation faults
        {
            // mandatory to hold the position before is changed
            pieceMNumber = getPieceNoFromRect(mNumber);

            // checks the base movement of pieces
            if (allowMovementOf(pNumber, lNo, mNumber))
            {
                // checks if it gets an enemy
                bool enemyCheck = checkIfEnemy(pNumber, mNumber);

                // if it is enemy, set off its rectangle position
                // for checking purposes in checkIfThreaten() later
                if (enemyCheck)
                    excludedPiece[pieceMNumber] = true; // setting the *enemy* temporally off of its current position

                // do the move, so that checkIfThreaten checks if the move is OK
                pieceRect[pNumber] = crect[mNumber];

                // explicitly check if the king is threatened by enemy based on whiteTurn value
                threaten = whiteTurn ? checkIfThreaten(getRectNoOf(28)) : checkIfThreaten(getRectNoOf(4));

                // undo the temporally set off of the enemy
                if (enemyCheck)
                    excludedPiece[pieceMNumber] = false;

                // if this piece move does not affect the king by the enemy, go on
                if (!threaten)
                {
                    // if there is, go and destroy the enemy piece
                    if (enemyCheck)
                        destroyPieceAndRect(pieceMNumber);
                    if (enPassant) // a special move that needs to be checked here
                    {
                        destroyPieceAndRect(lastMoved.piece);
                        enPassant = false;
                        std::cout << "Special move: en passant" << endl;
                    }
                    // promotion happens when one piece reaches the opposite kings row
                    // and it is checked in allowMovement() function
                    if (promotion)
                    {
                        createNewTexFor(pNumber, promotedPiece[pNumber]);
                        promotion = false;
                        std::cout << "Special move: promotion to Queen" << endl;
                    }

                    pieceMoved[pNumber] = true;
                    mix->playDown(); // play a small sound when a move occurs

                    lastMoved.piece = pNumber;
                    lastMoved.fromNo = lNo;
                    lastMoved.toNo = mNumber;
                    movesPlayed.push_back(lastMoved);

                    ChessPosition cpos = posToChessPos(lastMoved);
                    std::cout << cpos.pieceName << " (" <<  cpos.from << ", " << cpos.to << ")" << endl << endl;

                    if (myTurn && !watchGame && !solo)
                        clnt->sendMsg(encodeMsg(&lastMoved)); // sent the move over network



//                    checkingCheckMate = true;
//                    if (checkMate())
//                        resetAll(render);
//                    checkingCheckMate = false;

                    whiteTurn = !whiteTurn;
                    if (!solo)
                        myTurn = !myTurn;
                }
                else // else if king is threaten return this piece to the last position
                {
                    pieceRect[pNumber] = lastPos;
                    enPassant = false;
                    if (promotion)
                        promotedPiece[pNumber] = -1;
                    promotion = false;
                }
            }
            else // if movement is disallowed return to last position
            {
                if (promotion)
                        promotedPiece[pNumber] = -1;
                pieceRect[pNumber] = lastPos;
                promotion = false;
            }


        }
        pNumber = -1; // this variable needs to be always -1 after a move or a check
    }
}


#endif // CHESSBOARD_H
