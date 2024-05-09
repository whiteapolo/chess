#ifndef GUI_H
#define GUI_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "raylib.h"
#include "config.h"
#include "types_and_macros.h"



// PUBLIC FUNCTIONS
//------------------------------------------------------------------
void GuiInitWindow(const char *fen);
void GuiDrawWindow();
void GuiCloseWindow();

// return true if loading was successfull
// false otherwise
bool GuiLoadFen(const char *fen);

// return true if there was a move
bool GuiGetUserMove(char mv[4]);
void GuiMakeMove(char *mv);
void GuiGetFen(char dest[100]);

#endif
