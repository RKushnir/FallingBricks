#ifndef __CONSTANTS_H_INCLUDED
#define __CONSTANTS_H_INCLUDED

#include "Brick.h"

#define CELL_HORIZONTAL_SIZE 32.0f // pixels
#define CELL_VERTICAL_SIZE 32.0f // pixels

extern const RECT g_arCellTextureRectsByColor[CBrick::BCC__MAX];
extern const RECT g_rPlayBoardBackgroundTextureRect;
extern const RECT g_rPauseMenuBackgroundTextureRect;
extern const RECT g_rPauseMenuUpperCorner;
extern const RECT g_rPauseMenuLowerCorner;
extern const RECT g_rGameInfoBackgroundTextureRect;

#endif // __CONSTANTS_H_INCLUDED
