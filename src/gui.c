#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include "../assets/font.h"
#include "config.h"
#include "../raylib/src/raylib.h"
#include "macros.h"

enum Player {
    WHITE_PLR = 'w',
    BLACK_PLR = 'b',
};

#define IN_BOUNDRY(p,p1,p2) 	        (p.x >= p1.x && p.x <= p2.x && p.y >= p1.y && p.y <= p2.y)
#define HASH(c)			                ((c-1)%21)
#define PADX 			                ((GetScreenWidth() - (DIM * TILE_SIZE)) / 2)
#define PADY 			                ((GetScreenHeight() - (DIM * TILE_SIZE)) / 2)
#define IN_BOARD(pos) 		            IN_BOUNDRY(pos,((Vector2){PADX,PADY}),((Vector2){PADX+TILE_SIZE*DIM,PADY+TILE_SIZE*DIM}))
#define GET_POS_FROM_TILE(tile,start)   ((Vector2){(tile).y*TILE_SIZE+start.x,(tile).x*TILE_SIZE+start.y})
#define GET_TILE_FROM_POS(pos,start)    ((Vector2){(pos.x-start.x)/TILE_SIZE,(pos.y-start.y)/TILE_SIZE})
#define GET_PLR(c) 		                ((bool)isupper(c.piece) ? WHITE_PLR : BLACK_PLR)
#define IS_PIECE(p) 		            ((bool)strchr(pieces, tolower(p)))
#define IS_TILE_EMPTY(tile) 	        (!isalpha(tile.piece))

// Vector2
#define SUB_VECTOR2(v1, v2) ((Vector2){(v1).x - (v2).x, (v1).y - (v2).y})
#define ADD_VECTOR2(v1, v2) ((Vector2){(v1).x + (v2).x, (v1).y + (v2).y})

// drawing
#define DRAW_TILE(v,c) 		    DrawRectangleV(v,(Vector2){TILE_SIZE, TILE_SIZE},c)
#define DRAW_PIECE(v,t) 	    DrawTextureV(textures[HASH(t)],v,PIECES_TINT);
#define DRAW_LABEL(x,y,l,c) 	DrawTextEx(font,(char[]){l,'\0'},(Vector2){x,y},FONT_SIZE,0,c);
#define DRAW_INACTIVE_MASK() 	DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), INACTIVE_C)

static Board b;
static Move move;

static Font font;
static Sound move_sound;
static Sound capture_sound;
static Texture2D textures[19];
static const char pieces[] = "PNBRQKpnbrqk";

void loadAssets();
bool GuiLoadFen(const char *fen);
bool is_legal_fen(const char *fen);
void initBoard();
bool handleTouch();
void drawBoard();
void GuiMakeMove(char *mv);

void GuiInitWindow(const char *fen)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
    SetWindowMinSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    InitAudioDevice();
    loadAssets();
    SetTargetFPS(FPS);
    initBoard();
    if (!GuiLoadFen(fen))
        exit(1);
}

bool GuiDrawWindow()
{
    BeginDrawing();
    drawBoard();
    ClearBackground(BG_COLOR);
    EndDrawing();
    return handleTouch();
}

void GuiCloseWindow()
{
    CloseWindow();
}

// LOCAL IMPLEMENTATION
//------------------------------------------------------------------
void loadAssets()
{
    Image img;

    char pieces_path[] = PIECES_PATH;
    char *pieces_n = strchr(pieces_path, '$');

    // load pieces images
    FOR (i, sizeof(pieces)) {
        *pieces_n = pieces[i];
        img = LoadImage(pieces_path);
        ImageResize(&img, PIECE_SIZE, PIECE_SIZE);
        textures[HASH(pieces[i])] = LoadTextureFromImage(img);
    }

    // load sound
    move_sound = LoadSound(SOUND_PATH "m.mp3");
    capture_sound = LoadSound(SOUND_PATH "c.mp3");

    // load font
    font = LoadCustomFont();
}

void drawBoard()
{
    Vector2 pos;

    // draw tiles
    FOR (i, DIM) {
        FOR (j, DIM) {
            pos = (Vector2){j*TILE_SIZE+PADX, i*TILE_SIZE+PADY};
            DRAW_TILE(pos, IS_EVEN(i+j) ? B_T_C : W_T_C);
            // Draw highlighted tiles
            if (b.mat[i][j].highlight)
                DRAW_TILE(pos, S_T_C);
        }
    }
    // Draw the pieces
    FOR (i, DIM) {
        FOR (j, DIM) {
            // if there is a piece here
            if (b.mat[i][j].piece) {
                pos = (Vector2){j*TILE_SIZE+PADX, i*TILE_SIZE+PADY};
                // center the piece in the tile
                pos.x += (TILE_SIZE-PIECE_SIZE)/2;
                pos.y += (TILE_SIZE-PIECE_SIZE)/2;

                // calculate the animation offset
                if (b.mat[i][j].offset.x)
                    b.mat[i][j].offset.x -= b.mat[i][j].offset.x / 4;
                if (b.mat[i][j].offset.y)
                    b.mat[i][j].offset.y -= b.mat[i][j].offset.y / 4;
                pos = SUB_VECTOR2(pos, b.mat[i][j].offset);

                DRAW_PIECE(pos, b.mat[i][j].piece);
            }
        }
        // draw the numbers on the edge of the board
        DRAW_LABEL(PADX+1, PADY+((DIM-i-1)*TILE_SIZE), '1'+i, IS_EVEN(i) ? B_T_C : W_T_C);
        DRAW_LABEL(PADX+((DIM-i)*TILE_SIZE)-10, PADY + DIM*TILE_SIZE - 17, 'h'-i, IS_EVEN(i) ? W_T_C : B_T_C);
    }
}

bool handleTouch()
{
    // check if there was a click
    if (!IsMouseButtonPressed(0))
        return false;

    Vector2 pos = GetMousePosition();

    // check if the click was on board
    if (!IN_BOARD(pos))
        return false;

    // calculate the tile that was clicked
    pos = GET_TILE_FROM_POS(pos, ((Vector2){PADX,PADY}));
    u8 x = pos.x;
    u8 y = pos.y;

    // same tile as before was selected...
    // deselecting it...
    if (&b.mat[y][x] == move.src) {
        move.src = NULL;
        b.mat[y][x].highlight = false;
        // if there a piece on tile and its the same color as the current player
    } else if (!IS_TILE_EMPTY(b.mat[y][x]) && GET_PLR(b.mat[y][x]) == b.player) {
        if (move.src != NULL)
            move.src->highlight = false;
        move.src = &b.mat[y][x];
        move.src->highlight = true;
        // make the move.
    } else if (move.src != NULL) {
        move.dest = &b.mat[y][x];
        // deheighlight the tiles
        move.src->highlight = false;
        move.dest->highlight = false;
        return true;
    }
    return false;
}

void initBoard()
{
    b = (Board){0};
    move = (Move){0};
    FOR (i, DIM) {
        FOR (j, DIM) {
            b.mat[i][j].x = i;
            b.mat[i][j].y = j;
        }
    }
}

// check if a fen is legal
bool GuiIsLegalFen(const char *fen)
{
    u16 i = 0;
    u16 slash = 0;

    while (*fen && *fen != ' ') {
        if (isdigit(*fen))
            i += *fen - '0';
        else if (*fen == '/')
            slash++;
        else if (IS_PIECE(*fen))
            i++;
        else
            return false;
        fen++;
    }

    if (!(*(fen++)) ||
            i != 64 ||
            slash != 8 - 1 ||
            !strchr("wb", *(fen++)) ||
            (!(*(fen++)) && !(*fen + 1)))
        return false;

    FOR(i, 4) {
        if (!strchr("-KQkq", *(fen++)))
            return false;
    }

    if (*fen != '\0' && *fen != ' ')
        return false;

    return true;
}

bool GuiLoadFen(const char *fen)
{
    if (!GuiIsLegalFen(fen)) {
        printf("Chess: This Fen is not valid: \"%s\"\n", fen);
        return false;
    }

    FOR(i, DIM*DIM)
        b.mat[i%8][i/8].piece = 0;

    for (u16 i = 0; *fen != ' '; fen++) {
        if (isdigit(*fen)) {
            i += *fen - '0';
        }
        else if (isalpha(*fen)) {
            b.mat[i/8][i%8].piece = *fen;
            i++;
        }
    }
    b.player = *(++fen);
    return true;
}

void GuiMakeMove(char *mv)
{
    // convert mv to a Move
    move.src = &b.mat[-(mv[0] - '8')][mv[1] - 'a'];
    move.dest = &b.mat[-(mv[2] - '8')][mv[3] - 'a'];

    // play sound
    move.dest->piece ? PlaySound(capture_sound) : PlaySound(move_sound);

    // make the actual move
    move.dest->piece = move.src->piece;
    move.src->piece = 0;

    // calculate the animation offset
    move.dest->offset = SUB_VECTOR2(
            GET_POS_FROM_TILE(*(move.dest), ((Vector2){PADX, PADY})),
            GET_POS_FROM_TILE(*(move.src), ((Vector2){PADX, PADY})));

    // if the move was a promoting pawn
    // promote it...
    if (tolower(move.dest->piece) == 'p' && (move.dest->x == 0 || move.dest->x == DIM-1))
        move.dest->piece = isupper(move.dest->piece) ? 'Q' : 'q';

    move = (Move){0};

    // switch players
    b.player = (b.player == WHITE_PLR ? BLACK_PLR : WHITE_PLR);
}

// return true if there was a move
bool GuiGetUserMove(char mv[4])
{
    if (move.dest != NULL) {
        mv[0] = '8' - move.src->x;
        mv[1] = 'a' + move.src->y;
        mv[2] = '8' - move.dest->x;
        mv[3] = 'a' + move.dest->y;
        move = (Move){0};
        return true;
    }
    return false;
}

char *GuiGetFen()
{
    char *fen = MALLOC(char, 100);
    char *start = fen;

    FOR (i, DIM) {
        FOR (j, DIM) {
            if (b.mat[i][j].piece == 0) {
                if (isdigit(*(fen-1))) {
                    (*(fen-1))++;
                    fen--;
                } else {
                    *fen = '1';
                }
            } else {
                *fen = b.mat[i][j].piece;
            }
            fen++;
        }
        *(fen++) = '/';
    }
    *(fen-1) = ' ';
    *fen = b.player;
    *(fen+1) = '\0';
    return start;
}
