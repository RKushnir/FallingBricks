#ifndef __GAME_H_INCLUDED
#define __GAME_H_INCLUDED

extern struct ID3DXSprite *g_psSprite;
extern struct IDirect3DTexture9 *g_ptTexture;
extern struct ID3DXFont *g_pfMainMenuFont;
extern struct ID3DXFont *g_pfCommonFont;

bool InitializeGame();
void FinalizeGame();
void UpdateGame();
void RenderGame();
void ProcessUserInput(LPARAM lParam);

#endif // __GAME_H_INCLUDED
