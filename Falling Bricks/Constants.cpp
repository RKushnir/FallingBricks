#include "StdAfx.h"
#include "Constants.h"
#include "Brick.h"

const RECT g_arCellTextureRectsByColor[CBrick::BCC__MAX] =
{
	{ 0, 0, 0, 0, },
	{ 0, 0, 32, 32, },
	{ 0, 32, 32, 64, },
	{ 0, 64, 32, 96, },
	{ 0, 96, 32, 128, },
	{ 0, 128, 32, 160, },
};

const RECT g_rPlayBoardBackgroundTextureRect = {32, 0, 392, 680};
const RECT g_rPauseMenuBackgroundTextureRect = {148, 680, 328, 800};
const RECT g_rPauseMenuUpperCorner = {148, 800, 166, 820};
const RECT g_rPauseMenuLowerCorner = {166, 800, 184, 820};
const RECT g_rGameInfoBackgroundTextureRect = {0, 680, 148, 878};

