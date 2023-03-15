#include "ErrorCodes.h"
#include <stdio.h>
#include <Windows.h>
#include "wincon.h"
#include <stdlib.h>  // rand(), srand()
#include <time.h>    // time()

// CONSTANTS
#define MAX_TAIL_SIZE 100
#define WIDTH 121
#define HEIGHT 61
#define ROWS 10
#define COLS 10
#define CELL_WIDTH  (WIDTH/ROWS)-2
#define CELL_HEIGHT  (HEIGHT/COLS)-2 
#define WINDOW_TITLE "SNAKE - Game DEV"
#define FOREGROUND_WHITE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

// Struct declarations
// NODE
struct NODE
{
    COORD cGridPosition;
    COORD cDrawingAnchorPosition;
};
typedef struct NODE NODE;
// GRID
struct GRID
{
    NODE Nodes[ROWS*COLS];
};
typedef struct GRID GRID;
// APPLE
struct APPLE
{
    COORD Position;
};
typedef struct APPLE APPLE;
// HEAD
struct HEAD
    {
        COORD Position;
        COORD LastVectorPos;
    };
typedef struct HEAD HEAD;
// TAIL
struct SEGMENT
{
    COORD Position;
};
typedef struct SEGMENT SEGMENT;

int lastError;
void DrawErrorCode(HANDLE* phFrameBuffer, int code, const char* text, int padding, WORD wSettings);

// DRAWING FUNCTIONS
int GetGridIndexFromPos(int GridX_Pos, int GridY_Pos)
{
    return COLS*GridY_Pos + GridX_Pos;
}

void STOP()
{
    char* test;
    scanf("%i",&test);
}

void WriteText(HANDLE* phFrameBuffer, int X, int Y, const char* text, DWORD len, WORD wSettings)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD lastCursorPos; // Initial cursor position
    WORD wSettingsBackup = csbi.wAttributes;

    GetConsoleScreenBufferInfo(*phFrameBuffer, &csbi);
    lastCursorPos = csbi.dwCursorPosition;
    
    csbi.dwCursorPosition.X = X;
    csbi.dwCursorPosition.Y = Y;
    SetConsoleCursorPosition(*phFrameBuffer, csbi.dwCursorPosition);
    SetConsoleTextAttribute(*phFrameBuffer, wSettings);
    WriteConsole(*phFrameBuffer, text, len, NULL, NULL);
    SetConsoleTextAttribute(*phFrameBuffer, 0 | FOREGROUND_WHITE);
    SetConsoleCursorPosition(*phFrameBuffer, lastCursorPos);
}

void CLS(HANDLE* phFrameBuffer)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;

    if(!GetConsoleScreenBufferInfo(*phFrameBuffer, &csbi))
    {
        DrawErrorCode(phFrameBuffer, FRAMEBUFFER_INFO_GET_ERROR, FRAMEBUFFER_INFO_GET_ERROR_STR, 10, BACKGROUND_RED);
        return;
    }        

    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = csbi.dwSize.X;
    scrollRect.Bottom = csbi.dwSize.Y;

    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0-csbi.dwSize.Y);

    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes;

    ScrollConsoleScreenBuffer(*phFrameBuffer, &scrollRect, NULL, scrollTarget, &fill);

    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y = 0;
    if(!SetConsoleCursorPosition(*phFrameBuffer, csbi.dwCursorPosition))
    {
        DrawErrorCode(phFrameBuffer, SET_CURSOR_POSITION_ERROR, SET_CURSOR_POSITION_ERROR_STR, 10, BACKGROUND_RED);
        return;
    }
}

void WriteChar(HANDLE* phFrameBuffer, int X, int Y, WCHAR character)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD lastCursorPos; // Initial cursor position
    if(!GetConsoleScreenBufferInfo(*phFrameBuffer, &csbi))
    {
        DrawErrorCode(phFrameBuffer, FRAMEBUFFER_INFO_GET_ERROR, FRAMEBUFFER_INFO_GET_ERROR_STR, 10, BACKGROUND_RED);
        return;
    }
    lastCursorPos = csbi.dwCursorPosition;
    
    csbi.dwCursorPosition.X = X;
    csbi.dwCursorPosition.Y = Y;

    SetConsoleCursorPosition(*phFrameBuffer, csbi.dwCursorPosition);

    WriteConsole(*phFrameBuffer, &character, 1, NULL, NULL);
    if(!SetConsoleCursorPosition(*phFrameBuffer, lastCursorPos))
    {
        DrawErrorCode(phFrameBuffer, SET_CURSOR_POSITION_ERROR, SET_CURSOR_POSITION_ERROR_STR, 10, BACKGROUND_RED);
        return;
    }
}

void DrawGrid(HANDLE* phFrameBuffer)
{
    // Horizontal lines
    for(int y = 0; y <= HEIGHT; y+=HEIGHT/ROWS)
    {
        for(int x = 0; x <= WIDTH; x++)
        {
            WriteChar(phFrameBuffer, x, y, L'=');
        }
    }

    // Vertical lines
    for(int x = 0; x <= WIDTH; x+=WIDTH/COLS)
    {
        for(int y = 0; y <= HEIGHT; y++)
        {
            WriteChar(phFrameBuffer, x, y, L'|');
        }
    }
}

void DrawRect(HANDLE *phFrameBuffer, int X_pos, int Y_pos, int width, int height, WORD wSettings)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;

    if(!GetConsoleScreenBufferInfo(*phFrameBuffer, &csbi))
    {
        DrawErrorCode(phFrameBuffer, FRAMEBUFFER_INFO_GET_ERROR, FRAMEBUFFER_INFO_GET_ERROR_STR, 10, BACKGROUND_RED);
        return;
    }

    scrollRect.Left = X_pos;
    scrollRect.Top = Y_pos;
    scrollRect.Right = X_pos + width;
    scrollRect.Bottom = Y_pos + height;

    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0-csbi.dwSize.Y);

    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes | wSettings;

    if(!ScrollConsoleScreenBuffer(*phFrameBuffer, &scrollRect, NULL, scrollTarget, &fill))
    {
        DrawErrorCode(phFrameBuffer, SCROLLBUFFER_ERROR, SCROLLBUFFER_ERROR_STR, 10, BACKGROUND_RED);
        return;
    }
}

void DrawErrorCode(HANDLE* phFrameBuffer, int code, const char* text, int padding, WORD wSettings)
{
    DWORD textSize = 0;
    for(int i = 0; i < 255; i++)
    {
        textSize++;
        if(text[i] == 0) break;
    }
    
    DrawRect(
        phFrameBuffer,
        WIDTH/2-(int)(textSize/2)-padding,
        HEIGHT/2-(padding/2),
        2*padding+textSize,
        padding/2 * 2,
        BACKGROUND_RED
        );
    const char sError[] = "ERROR";
    const int errorSize = sizeof("ERROR")/sizeof(char);
    WriteText(phFrameBuffer, WIDTH/2-(int)(errorSize/2), HEIGHT/2-1, sError, errorSize, 0 | BACKGROUND_RED | FOREGROUND_WHITE);
    WriteText(phFrameBuffer, WIDTH/2-(int)(textSize/2), HEIGHT/2, text, textSize, 0 | BACKGROUND_RED | FOREGROUND_WHITE);
    STOP();
}

void DrawRectAtGridPos(HANDLE *phFrameBuffer, GRID* pgGrid, int GridX_Pos, int GridY_Pos, WORD wSettings)
{
    int realIndex = GetGridIndexFromPos(GridX_Pos, GridY_Pos);
    int pixelX = pgGrid->Nodes[realIndex].cDrawingAnchorPosition.X;
    int pixelY = pgGrid->Nodes[realIndex].cDrawingAnchorPosition.Y;
    DrawRect(phFrameBuffer, pixelX, pixelY, CELL_WIDTH, CELL_HEIGHT, wSettings);
}

void ClearRectAtGridPos(HANDLE *phFrameBuffer, GRID* pgGrid, int GridX_Pos, int GridY_Pos, WORD wSettings)
{
    int realIndex = GetGridIndexFromPos(GridX_Pos, GridY_Pos);
    int pixelX = pgGrid->Nodes[realIndex].cDrawingAnchorPosition.X;
    int pixelY = pgGrid->Nodes[realIndex].cDrawingAnchorPosition.Y;
    DrawRect(phFrameBuffer, pixelX, pixelY, CELL_WIDTH, CELL_HEIGHT, wSettings);
}

// UTLITY FUNCTIONS
int RandomIntInRange(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

// GAME LOGIC FUNCTIONS
void GetMovementVector(int* direction_, int* x_, int* y_, HANDLE* phFrameBuffer)
{
    int x = 0;
    int y = 0;
    int direction = *direction_;
    switch (direction)
    {
    case 0: // UP
        if(HIBYTE(GetKeyState(0x44)) > 0) // D key
        {
            x += 1;
        }
        if(HIBYTE(GetKeyState(0x41)) > 0) // A Key
        {
            x -= 1;
        }
        if(x < 0)
            direction = 3;
        else if(x > 0)
            direction = 1;
        else
        {
            direction = 0;
            y -= 1;
        }
        break;
    case 1: // RIGHT
        if(HIBYTE(GetKeyState(0x53)) > 0) // S Key
        {
            y += 1;
        }
        if(HIBYTE(GetKeyState(0x57)) > 0) // W Key
        {
            y -= 1;
        }
        if(y < 0)
            direction = 0;
        else if(y > 0)
            direction = 2;
        else
        {
            direction = 1;
            x += 1;
        }
        break;
    case 2: // DOWN
        if(HIBYTE(GetKeyState(0x44)) > 0) // D key
        {
            x += 1;
        }
        if(HIBYTE(GetKeyState(0x41)) > 0) // A Key
        {
            x -= 1;
        }
        if(x < 0)
            direction = 3;
        else if(x > 0)
            direction = 1;
        else
        {
            direction = 2;
            y += 1;
        }
            
        break;    
    case 3: // LEFT
        if(HIBYTE(GetKeyState(0x53)) > 0) // S Key
        {
            y += 1;
        }
        if(HIBYTE(GetKeyState(0x57)) > 0) // W Key
        {
            y -= 1;
        }
        
        if(y < 0)
            direction = 0;
        else if(y > 0)
            direction = 2;
        else
        {
            direction = 3;
            x -= 1;
        }
        break;
    default:
        DrawErrorCode(phFrameBuffer, MOVEMENT_SWITCH_ERROR, MOVEMENT_SWITCH_ERROR_STR, 10, BACKGROUND_RED);
        break;
    }
    *direction_ = direction;
    *x_ = x;
    *y_ = y;
}

void ShiftArray(SEGMENT* tail, int tCount, int newX, int newY)
{
    SEGMENT copy = tail[tCount-1]; // Copy of last element
    // Assign new position to element
    copy.Position.X = newX;
    copy.Position.Y = newY;
    // SHIFT RIGHT
    for(int i = tCount-1; i > 0; i--)
    {
        tail[i] = tail[i-1];
    }
    // assign first value
    tail[0] = copy;
}

/// @brief Funkcja ktora ma za zadanie utworzyc nowy bufor, ustawic glowny bufor na nowo utworzony, ustawic widocznosc kursora na false, ustawic tytul okna oraz wielkosc okna
/// @param phFrameBuffer [out] Wskaznik do nowo utworzonego bufora 
/// @return 0 - jezeli dzialanie funkcji zakonczy sie powodzeniem, > 0 - kiedy funkcja konczy niepowodzeniem
int Setup(HANDLE* phFrameBuffer)
{
    TCHAR szTitle[MAX_PATH];
    COORD WindowSize;
    CONSOLE_CURSOR_INFO cursorSettings;
    DWORD defaultConsoleMode;

    // Create new buffer
    *phFrameBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if(*phFrameBuffer == INVALID_HANDLE_VALUE)
    {
        DrawErrorCode(phFrameBuffer, FRAMEBUFFER_CREATE_ERROR, FRAMEBUFFER_CREATE_ERROR_STR, 10, BACKGROUND_RED);
        lastError = FRAMEBUFFER_CREATE_ERROR;
        return FRAMEBUFFER_CREATE_ERROR;
    }
    if(!SetConsoleActiveScreenBuffer(*phFrameBuffer))
    {
        DrawErrorCode(phFrameBuffer, CONSOLE_ACTIVE_BUFFER_CHANGE_ERROR, CONSOLE_ACTIVE_BUFFER_CHANGE_ERROR_STR, 10, BACKGROUND_RED);
        lastError = CONSOLE_ACTIVE_BUFFER_CHANGE_ERROR;
        return CONSOLE_ACTIVE_BUFFER_CHANGE_ERROR;
    }
    // Console mode a'ka cursor status
    if(!GetConsoleMode(*phFrameBuffer, &defaultConsoleMode))
    {
        DrawErrorCode(phFrameBuffer, CONSOLE_GET_MODE_ERROR, CONSOLE_GET_MODE_ERROR_STR, 10, BACKGROUND_RED);
        lastError = CONSOLE_GET_MODE_ERROR;
        return CONSOLE_GET_MODE_ERROR;
    }
    // Disable line input console mode
    if(!SetConsoleMode(*phFrameBuffer, defaultConsoleMode & ~ENABLE_LINE_INPUT))
    {
        DrawErrorCode(phFrameBuffer, CONSOLE_SET_MODE_ERROR, CONSOLE_SET_MODE_ERROR_STR, 10, BACKGROUND_RED);
        lastError = CONSOLE_SET_MODE_ERROR;
        return CONSOLE_SET_MODE_ERROR;
    }

    // Set window
    WindowSize.X = WIDTH;
    WindowSize.Y = HEIGHT;

    // Change size of cmd window
    char SystemCall[12];
    sprintf(SystemCall, "MODE %i, %i", WIDTH, HEIGHT);
    system(SystemCall);

    // Set title
    strcpy(szTitle, WINDOW_TITLE);
    SetConsoleTitle(szTitle);

    // Set window size
    if(!SetConsoleScreenBufferSize(*phFrameBuffer, WindowSize))
    {
        DrawErrorCode(phFrameBuffer, CONSOLE_SET_FRAMEBUFFER_SIZE_ERROR, CONSOLE_ACTIVE_BUFFER_CHANGE_ERROR_STR, 10, BACKGROUND_RED);
        lastError = CONSOLE_SET_FRAMEBUFFER_SIZE_ERROR;
        return CONSOLE_SET_FRAMEBUFFER_SIZE_ERROR;
    }

    // Disable cursor visibility
    cursorSettings.bVisible = FALSE;
    cursorSettings.dwSize = 1;
    if(!SetConsoleCursorInfo(*phFrameBuffer, &cursorSettings))
    {
        DrawErrorCode(phFrameBuffer, SET_CURSOR_VISIBLITY_ERROR, SET_CURSOR_VISIBLITY_ERROR_STR, 10, BACKGROUND_RED);
        lastError = SET_CURSOR_VISIBLITY_ERROR;
        return SET_CURSOR_VISIBLITY_ERROR;
    }
    return 0;
}

void InitGame(GRID* pGrid, HEAD* pHead, APPLE* pApple, SEGMENT* tail, int* tailCount)
{
    // Init grid object;
    for(int y = 0; y < ROWS; y++)
    {
        for(int x = 0; x < COLS; x++)
        {
            pGrid->Nodes[GetGridIndexFromPos(x, y)].cGridPosition.X = x;
            pGrid->Nodes[GetGridIndexFromPos(x, y)].cGridPosition.Y = y;
            pGrid->Nodes[GetGridIndexFromPos(x,y)].cDrawingAnchorPosition.X = x*(CELL_WIDTH+2)+1;
            pGrid->Nodes[GetGridIndexFromPos(x,y)].cDrawingAnchorPosition.Y = y*(CELL_HEIGHT+2)+1;
        }
    }
    // INIT HEAD POS
    pHead->Position.X = 4;
    pHead->Position.Y = 1;
    // INIT APPLE POS
    pApple->Position.X = RandomIntInRange(0, 9);
    pApple->Position.Y = RandomIntInRange(0, 9);
    // Init first 3 tail segments
    tail[0].Position.X = pHead->Position.X - 1;
    tail[0].Position.Y = pHead->Position.Y;
    tail[1].Position.X = pHead->Position.X - 2;
    tail[1].Position.Y = pHead->Position.Y;
    tail[2].Position.X = pHead->Position.X - 3;
    tail[2].Position.Y = pHead->Position.Y;
    *tailCount = 3;
}

void StartGame(HANDLE *phFrameBuffer, short* isRun)
{
    CLS(phFrameBuffer);
    DrawGrid(phFrameBuffer);
    *isRun = 1;
}

short isCollidingWithApple(HEAD* pHead, APPLE* pApple)
{
    if(pHead->Position.X == pApple->Position.X && pHead->Position.Y == pApple->Position.Y) return 1;
    return 0;
}

void CollisionWithApple(SEGMENT* tail, HEAD* pHead, APPLE* pApple, int* tailCount)
{
    // CREATE NEW SEGMENT
    *tailCount += 1;
    SEGMENT newSegment;
    newSegment.Position.X = pHead->LastVectorPos.X;
    newSegment.Position.Y = pHead->LastVectorPos.Y;
    for(int i = (*tailCount)-1; i > 0; i--)
    {
        tail[i] = tail[i-1];
    }
    tail[0] = newSegment;
    // CHECK FOR CORRECT POSITION
    short correctPos = 0;
    short flag_1 = 0;
    short flag_2 = 0;
    do
    {
        flag_1 = 0;
        flag_2 = 0;
        pApple->Position.X = RandomIntInRange(0, ROWS-1);
        pApple->Position.Y = RandomIntInRange(0, COLS-1);
        for(int i = 0; i < *tailCount; i++)
        {
            if(tail[i].Position.X == pApple->Position.X && tail[i].Position.Y == pApple->Position.Y)
            {
                flag_1 = 1;
                break;
            }
        }
        if(pApple->Position.X == pHead->Position.X && pApple->Position.Y == pHead->Position.Y)
        {
            flag_2 = 1;
        }
        
        if(flag_1 || flag_2)
        {
            correctPos = 0;
        }
        else
        {
            correctPos = 1;
        }
    }while(!correctPos);
}

short isCollidingWithWall(HEAD* pHead)
{
    if((pHead->Position.X > COLS-1 || pHead->Position.X < 0) || (pHead->Position.X > ROWS-1 || pHead->Position.Y < 0)) return 1;
    else return 0;
}

short isCollidingWithSegment(HEAD* pHead, SEGMENT* tail, int* tailCount)
{
    for(int i = 0; i < *tailCount; i++)
    {
        if(pHead->Position.X == tail[i].Position.X && pHead->Position.Y == tail[i].Position.Y)
        {
            return 1;
        }
    }
    return 0;
}

void DrawMainMenu(HANDLE* pFrameBuffer, short* isRunning)
{
    int choose = 0;
    // 0 - Start game
    // 1 - Settings
    // 2 - Exit game
    DrawRect(pFrameBuffer, 10, 3, WIDTH-20, HEIGHT-6, BACKGROUND_RED);
    const char sStartgame[] = "Start Game";
    const char sSettings[] = "Settings";
    const char sExit[] = "Exit Game";
    int yOff = 2;
    int xOff_1 = 5;
    int xOff_2 = 4;
    WriteText(pFrameBuffer, WIDTH/2-xOff_1, HEIGHT/2-yOff, sStartgame, 10, BACKGROUND_RED | FOREGROUND_GREEN);
    WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2, sSettings, 8, BACKGROUND_RED | FOREGROUND_WHITE);
    WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2+yOff, sExit, 9, BACKGROUND_RED | FOREGROUND_WHITE);
    
    while (1)
    {
        if(HIBYTE(GetKeyState(0x26)) > 0)
        {
            choose--;
            if(choose < 0) choose = 2;
        }
        else if(HIBYTE(GetKeyState(0x28)) > 0)
        {
            choose++;
            if(choose > 2) choose = 0;
        }

        if(HIBYTE(GetKeyState(VK_RETURN)) > 0)
        {
            *isRunning = 1;
            break;
        }

        switch (choose)
        {
        case 0:
            WriteText(pFrameBuffer, WIDTH/2-xOff_1, HEIGHT/2-yOff, sStartgame, 10, BACKGROUND_RED | FOREGROUND_GREEN);
            WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2, sSettings, 8, BACKGROUND_RED | FOREGROUND_WHITE);
            WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2+yOff, sExit, 9, BACKGROUND_RED | FOREGROUND_WHITE);
            break;
        case 1:
            WriteText(pFrameBuffer, WIDTH/2-xOff_1, HEIGHT/2-yOff, sStartgame, 10, BACKGROUND_RED | FOREGROUND_WHITE);
            WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2, sSettings, 8, BACKGROUND_RED | FOREGROUND_GREEN);
            WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2+yOff, sExit, 9, BACKGROUND_RED | FOREGROUND_WHITE);
            break;
        case 2:
            WriteText(pFrameBuffer, WIDTH/2-xOff_1, HEIGHT/2-yOff, sStartgame, 10, BACKGROUND_RED | FOREGROUND_WHITE);
            WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2, sSettings, 8, BACKGROUND_RED | FOREGROUND_WHITE);
            WriteText(pFrameBuffer, WIDTH/2-xOff_2, HEIGHT/2+yOff, sExit, 9, BACKGROUND_RED | FOREGROUND_GREEN);
            break;
        default:
            break;
        }
        Sleep((DWORD)150);
    }
}

short UISwitch = 1;
short UIChoose = 0;
int main()
{
    // SETUP
    HANDLE hFrameBuffer;
    if(Setup(&hFrameBuffer)) return lastError;

    // SNAKE COMPONENTS
    GRID gGrid;
    HEAD hHead;
    APPLE apple;
    SEGMENT tail[MAX_TAIL_SIZE];
    int tailCount;

    // LOGIC's
    short isRunning = 0;
    // Init game stats
    InitGame(&gGrid, &hHead, &apple, tail, &tailCount);
    while (!isRunning)
    {
        switch (UIChoose)
        {
        case 0:
            DrawMainMenu(&hFrameBuffer, &isRunning);
            break;
        
        default:
            DrawErrorCode(&hFrameBuffer, UI_SWITCH_ERROR, UI_SWITCH_ERROR_STR, 10, BACKGROUND_RED);
            return UI_SWITCH_ERROR;
            break;
        }
    }
    
    // START GAME
    StartGame(&hFrameBuffer, &isRunning);
    // Head control data
    int direction = 1;
    int offsetX = 0;
    int offsetY = 0;

    while (isRunning)
    {
        // ASSIGN PRE MOVE POSITION
        hHead.LastVectorPos.X = hHead.Position.X;
        hHead.LastVectorPos.Y = hHead.Position.Y;
        // CLEAR WHOLE TAIL
        for(int i = 0; i < tailCount; i++)
        {
            ClearRectAtGridPos(&hFrameBuffer, &gGrid, tail[i].Position.X, tail[i].Position.Y, 0);
        }
        // CALCULATE MOVEMENT VECTOR
        GetMovementVector(&direction, &offsetX, &offsetY, &hFrameBuffer);
        // MOVE HEAD TO NEW POSITION
        hHead.Position.X += offsetX;
        hHead.Position.Y += offsetY;
        // CHECK COLLISION WITH APPLE
        short isAppleCollision = isCollidingWithApple(&hHead, &apple);
        if(isAppleCollision) CollisionWithApple(tail, &hHead, &apple, &tailCount); // Do getApple logic
        else ShiftArray(tail, tailCount, hHead.LastVectorPos.X, hHead.LastVectorPos.Y); // SWAP and SHIFT tail
        // RESET OFFSETS
        offsetX = 0;
        offsetY = 0;

        short isWallCollision = isCollidingWithWall(&hHead);
        // CHECK COLLISION WITH WALL
        if(isWallCollision)
        {
            DrawErrorCode(&hFrameBuffer, COLLISION_WALL, COLLISION_WALL_STR, 10, BACKGROUND_RED);
            return COLLISION_WALL;
        }
        // DRAW APPLE
        DrawRectAtGridPos(&hFrameBuffer, &gGrid, apple.Position.X, apple.Position.Y, BACKGROUND_RED);
        // HEAD Draw
        DrawRectAtGridPos(&hFrameBuffer, &gGrid, hHead.Position.X,hHead.Position.Y, BACKGROUND_BLUE);

        // TAIL Draw
        for(int i = 0; i < tailCount; i++)
        {
            DrawRectAtGridPos(&hFrameBuffer, &gGrid, tail[i].Position.X, tail[i].Position.Y, BACKGROUND_GREEN);
        }

        // CHECK COLLISION WITH SEGMENTS
        short isSegmentCollision = isCollidingWithSegment(&hHead, tail, &tailCount);
        if(isSegmentCollision)
        {
            DrawErrorCode(&hFrameBuffer, COLLISION_TAIL, COLLISION_TAIL_STR, 10, BACKGROUND_RED);
            return COLLISION_TAIL;
        }
        if(isWallCollision || isSegmentCollision) break;
        Sleep((DWORD)500); // ~1sec sleep
    }
    return 0;
}