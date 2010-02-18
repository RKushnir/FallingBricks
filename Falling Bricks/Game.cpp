#include "StdAfx.h"
#include "Game.h"
#include "resource.h"
#include "PlayBoard.h"
#include "Brick.h"

#include "Direct3DFunctions.h"
#include "InputFunctions.h"
#include "XACTFunctions.h"
#include "Constants.h"
#include "Utilities.h"

#include <time.h>
#include <strsafe.h>

enum
{
	GAME_DIFFICULTY_LEVEL_MIN = 0,
	GAME_DIFFICULTY_LEVEL_MAX = 10,
};

#define PAUSE_MENU_INITIAL_POSITION_X -160.0f

static float g_fProcessedGameTime = 0.0f;
static const float g_fUpdateTimeInterval = 0.01f;
static float g_fTotalGameTime = 0.0f;
static float g_fLatencyAccumulator = 0.0f;
static unsigned g_nGameScore = 0;
static unsigned g_nCompletedLineCount = 0;
static unsigned g_nGameDifficultyLevel = GAME_DIFFICULTY_LEVEL_MIN;
static float g_fPauseMenuPositionX = 0.0f;

ID3DXSprite *g_psSprite = NULL;
IDirect3DTexture9 *g_ptTexture = NULL;
ID3DXFont *g_pfMainMenuFont = NULL;
ID3DXFont *g_pfCommonFont = NULL;

static IDirect3DTexture9 *g_ptRenderTargetTexture = NULL;
static IDirect3DSurface9 *g_psRenderTargetSurface = NULL;
static ID3DXRenderToSurface *g_prsRenderToSurface = NULL;
static IDirect3DPixelShader9* m_ppsPixelShader = NULL;

enum EGAMESTATE
{
	GS__MIN,

	GS_WELLCOME = GS__MIN,
	GS_MAINMENU,
	GS_PLAYING,
	GS_PAUSED,
	GS_FINISHED,

	GS__MAX,
};

static EGAMESTATE g_gsGameCurrentState = GS_MAINMENU;

enum EMAINMENUENTRIES
{
	MME__MIN,

	MME_NEW_GAME = MME__MIN,
	MME_EXIT,

	MME__MAX,
};

const TCHAR *g_aszMainMenuEntryNames[MME__MAX] =
{
	TEXT("New Game"), // MME_NEW_GAME
	TEXT("Exit"), // MME_EXIT
};

enum EPAUSEMENUENTRIES
{
	PME__MIN,

	PME_CONTINUE_GAME = PME__MIN,
	PME_RESTART_GAME,
	PME_EXIT,

	PME__MAX,
};

const TCHAR *g_aszPauseMenuEntryNames[PME__MAX] =
{
	TEXT("Continue Game"), // PME_CONTINUE_GAME
	TEXT("Restart Game"), // PME_RESTART_GAME
	TEXT("Exit"), // PME_EXIT
};

static EMAINMENUENTRIES g_meMainMenuSelectedEntry = MME__MIN; // <-- reset selected entry when showing menus
static EPAUSEMENUENTRIES g_mePauseMenuSelectedEntry = PME__MIN;

static const unsigned g_anScorePointsByCompletedLines[CBrick::MAX_LINES + 1] =
{
	0, // 0 lines
	10, // 1 line
	40, // 2 lines
	100, // 3 lines
	200, // 4 lines
};

static const unsigned g_anDifficultyLevelMinimalCompletedLineCount[GAME_DIFFICULTY_LEVEL_MAX] =
{
	0,
	10,
	20,
	30,
	40,
	50,
	60,
	70,
	80,
	90,
};

static const float g_afDifficultyLevelBrickMovePeriod[GAME_DIFFICULTY_LEVEL_MAX] =
{
	1.0f,
	0.9f,
	0.8f,
	0.7f,
	0.6f,
	0.5f,
	0.4f,
	0.3f,
	0.2f,
	0.1f,
};

static CPlayBoard g_pbPlayBoard(210.0f, 30.0f);

void DrawGameInfo(const CBrick &fNextBrick)
{
	D3DXVECTOR3 v3BackgroundPosition(20.0f, 10.0f, 0.0f);
	g_psSprite->Draw(g_ptTexture, &g_rGameInfoBackgroundTextureRect, NULL, &v3BackgroundPosition, D3DCOLOR_RGBA(255, 255, 255, 255));

	D3DXMATRIX mCurrentTransform;
	g_psSprite->GetTransform(&mCurrentTransform);
	D3DXMATRIX mBrickTranslation;
	D3DXMatrixTranslation(&mBrickTranslation, v3BackgroundPosition.x + 10.0f, v3BackgroundPosition.y + 60.0f, 0.0f);
	D3DXMATRIX mBrickScaling;
	g_psSprite->SetTransform(&(mCurrentTransform * mBrickTranslation));
	fNextBrick.Draw();
	g_psSprite->SetTransform(&mCurrentTransform);

	RECT rTextPosition = {30, 20, 158, 40};
	TCHAR szScoreStrBuffer[32];
	StringCbPrintf(szScoreStrBuffer, sizeof(szScoreStrBuffer), TEXT("Score: %d"), g_nGameScore);
	g_pfCommonFont->DrawText(g_psSprite, szScoreStrBuffer, -1, &rTextPosition, DT_LEFT, D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f));

	rTextPosition.top += 20;
	rTextPosition.bottom += 20;
	TCHAR szDifficultyStrBuffer[32];
	StringCbPrintf(szDifficultyStrBuffer, sizeof(szDifficultyStrBuffer), TEXT("Level: %d"), g_nGameDifficultyLevel);
	g_pfCommonFont->DrawText(g_psSprite, szDifficultyStrBuffer, -1, &rTextPosition, DT_LEFT, D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f));
}

void ResetGame()
{
	g_pbPlayBoard.Clear();
	g_nGameScore = 0;
	g_nCompletedLineCount = 0;
	g_nGameDifficultyLevel = GAME_DIFFICULTY_LEVEL_MIN;
	g_pbPlayBoard.SetBrickMovePeriod(g_afDifficultyLevelBrickMovePeriod[g_nGameDifficultyLevel]);
}

bool InitializeGame()
{
	bool bResult = false;

	srand((unsigned)time(NULL));

	do 
	{
		const HRESULT hrSpriteCreateResult = D3DXCreateSprite(g_pDirect3DDevice, &g_psSprite);
		if (hrSpriteCreateResult == D3D_OK)
		{
			const HRESULT hrMainTextureCreateResult = D3DXCreateTextureFromResource(g_pDirect3DDevice, NULL, MAKEINTRESOURCE(IDB_TEXTURE), &g_ptTexture);
			if (hrMainTextureCreateResult == D3D_OK)
			{
				D3DVIEWPORT9 vpViewport;
				g_pDirect3DDevice->GetViewport(&vpViewport);

				const HRESULT hrRenderTargetCreateResult = D3DXCreateTexture(g_pDirect3DDevice, vpViewport.Width, vpViewport.Height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_ptRenderTargetTexture);
				if (hrRenderTargetCreateResult == D3D_OK)
				{
					D3DSURFACE_DESC sdRenderTargetDesc;
					g_ptRenderTargetTexture->GetSurfaceLevel(0, &g_psRenderTargetSurface);
					g_psRenderTargetSurface->GetDesc(&sdRenderTargetDesc);

					const HRESULT hrRenderToSurfaceCreateResult = D3DXCreateRenderToSurface(g_pDirect3DDevice, sdRenderTargetDesc.Width, sdRenderTargetDesc.Height, sdRenderTargetDesc.Format, FALSE, D3DFMT_UNKNOWN, &g_prsRenderToSurface);
					if (hrRenderToSurfaceCreateResult == D3D_OK)
					{
						D3DXFONT_DESC fdMenuFontDescription = {24,
							0,
							400,
							0,
							false,
							DEFAULT_CHARSET,
							OUT_TT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							DEFAULT_PITCH,
							TEXT("Arial")};

						const HRESULT hrMenuFontCreateResult = D3DXCreateFontIndirect(g_pDirect3DDevice, &fdMenuFontDescription, &g_pfMainMenuFont);
						if (hrMenuFontCreateResult == D3D_OK)
						{
							D3DXFONT_DESC fdCommonFontDescription = {18,
								0,
								600,
								0,
								false,
								DEFAULT_CHARSET,
								OUT_TT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								DEFAULT_PITCH,
								TEXT("Verdana")};

							const HRESULT hrCommonFontCreateResult = D3DXCreateFontIndirect(g_pDirect3DDevice, &fdCommonFontDescription, &g_pfCommonFont);
							if (hrCommonFontCreateResult == D3D_OK)
							{
								D3DCAPS9 cD3DDeviceCaps;
								ZeroMemory(&cD3DDeviceCaps, sizeof(D3DCAPS9));
								g_pDirect3DDevice->GetDeviceCaps(&cD3DDeviceCaps);
								
								const bool bIsPixelShader20Supported = cD3DDeviceCaps.PixelShaderVersion >= D3DPS_VERSION(2, 0); 
								if (!bIsPixelShader20Supported || CreatePixelShaderFromResource(MAKEINTRESOURCE(IDR_MILKY_BLUR_SHADER), &m_ppsPixelShader))
								{
									bResult = true;
									break;
								}

								g_pfCommonFont->Release();
								g_pfCommonFont = NULL;
							}

							g_pfMainMenuFont->Release();
							g_pfMainMenuFont = NULL;
						}

						g_prsRenderToSurface->Release();
						g_prsRenderToSurface = NULL;
					}

					g_psRenderTargetSurface->Release();
					g_psRenderTargetSurface = NULL;

					g_ptRenderTargetTexture->Release();
					g_ptRenderTargetTexture = NULL;
				}

				g_ptTexture->Release();
				g_ptTexture = NULL;
			}

			g_psSprite->Release();
			g_psSprite = NULL;
		}
	}
	while (false);

	return bResult;
}

void FinalizeGame()
{
	if (g_psSprite)
	{
		if (g_ptTexture)
		{
			if (g_ptRenderTargetTexture)
			{
				if (g_prsRenderToSurface)
				{
					if (g_pfMainMenuFont)
					{
						if (m_ppsPixelShader)
						{
							m_ppsPixelShader->Release();
							m_ppsPixelShader = NULL;
						}

						g_pfMainMenuFont->Release();
						g_pfMainMenuFont = NULL;
					}

					g_prsRenderToSurface->Release();
					g_prsRenderToSurface = NULL;
				}

				g_psRenderTargetSurface->Release();
				g_psRenderTargetSurface = NULL;

				g_ptRenderTargetTexture->Release();
				g_ptRenderTargetTexture = NULL;
			}

			g_ptTexture->Release();
			g_ptTexture = NULL;
		}

		g_psSprite->Release();
		g_psSprite = NULL;
	}
}

void UpdateGame_Playing()
{
	bool bIsBrickSettled;
	unsigned nCompletedLineCount;
	bool bIsBoardFull;
	g_pbPlayBoard.Update(g_fProcessedGameTime, bIsBrickSettled, nCompletedLineCount, bIsBoardFull);

	if (bIsBrickSettled)
	{
		g_audioState.pSoundBank->Play(g_audioState.iBrickDropped, 0, 0, NULL);

		if (nCompletedLineCount > 0)
		{
			ASSERT(nCompletedLineCount <= CBrick::MAX_LINES);
			g_audioState.pSoundBank->Play(g_audioState.iLineCompleted, 0, 0, NULL);
			g_nGameScore += g_anScorePointsByCompletedLines[nCompletedLineCount];
			g_nCompletedLineCount += nCompletedLineCount;

			while (g_nGameDifficultyLevel < GAME_DIFFICULTY_LEVEL_MAX - 1 && g_nCompletedLineCount >= g_anDifficultyLevelMinimalCompletedLineCount[g_nGameDifficultyLevel + 1])
			{
				++g_nGameDifficultyLevel;
				g_pbPlayBoard.SetBrickMovePeriod(g_afDifficultyLevelBrickMovePeriod[g_nGameDifficultyLevel]);
			}
		}

		if (bIsBoardFull)
		{
			g_gsGameCurrentState = GS_FINISHED;
		}
	}
}

void UpdateGame_Paused()
{
	if (g_fPauseMenuPositionX < 0.0f)
	{
		g_fPauseMenuPositionX += 5.0f;
	}
}

void UpdateGame()
{
	if(g_audioState.pEngine)
	{
		g_audioState.pEngine->DoWork();
	}

	const float fNewGameTime = GetGameTime();
	float fTimeSincePreviousUpdate = fNewGameTime - g_fTotalGameTime;
	g_fTotalGameTime = fNewGameTime;

	if (fTimeSincePreviousUpdate > 0.25f)
	{
		fTimeSincePreviousUpdate = 0.25f;
	}

	g_fLatencyAccumulator += fTimeSincePreviousUpdate;

	while (g_fLatencyAccumulator >= g_fUpdateTimeInterval)
	{
		g_fLatencyAccumulator -= g_fUpdateTimeInterval;

		switch(g_gsGameCurrentState)
		{
			case GS_PLAYING:
			{
				UpdateGame_Playing();

				break;
			}

			case GS_PAUSED:
			{
				UpdateGame_Paused();

				break;
			}

			default:
			{
				break;
			}
		}

		g_fProcessedGameTime += g_fUpdateTimeInterval;
	}
}

void RenderGame_MainMenu()
{
	g_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0f, 0);

	if (SUCCEEDED(g_pDirect3DDevice->BeginScene()))
	{
		g_psSprite->Begin(D3DXSPRITE_ALPHABLEND);

		// draw main menu
		RECT rMenuEntryRect = { 50, 50, 160, 80 };

		for (size_t nMenuEntryIndex = 0; nMenuEntryIndex != MME__MAX; ++nMenuEntryIndex)
		{
			D3DCOLOR cTextColor = nMenuEntryIndex == g_meMainMenuSelectedEntry ? D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f) : D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 0.5f);
			g_pfMainMenuFont->DrawText(g_psSprite, g_aszMainMenuEntryNames[nMenuEntryIndex], -1, &rMenuEntryRect, DT_LEFT, cTextColor);
			rMenuEntryRect.top += 30;
			rMenuEntryRect.bottom += 30;
		}

		g_psSprite->End();

		g_pDirect3DDevice->EndScene();
	}

	g_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
}

void RenderGame_Playing()
{
	g_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(36, 114, 219), 1.0f, 0);

	if (SUCCEEDED(g_pDirect3DDevice->BeginScene()))
	{
		g_psSprite->Begin(D3DXSPRITE_ALPHABLEND);

		g_pbPlayBoard.Draw();

		const CBrick &bNextBrick = g_pbPlayBoard.GetNextBrick();
		DrawGameInfo(bNextBrick);

		g_psSprite->End();

		g_pDirect3DDevice->EndScene();
	}

	g_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
}

void RenderGame_Paused()
{
	IDirect3DSurface9 *psBackBuffer;
	g_pDirect3DDevice->GetRenderTarget(0, &psBackBuffer);
	g_pDirect3DDevice->SetRenderTarget(0, g_psRenderTargetSurface);

	g_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(36, 114, 219), 1.0f, 0);

	if (SUCCEEDED(g_prsRenderToSurface->BeginScene(g_psRenderTargetSurface, NULL)))
	{
		g_psSprite->Begin(D3DXSPRITE_ALPHABLEND);

		g_pbPlayBoard.Draw();

		const CBrick &bNextBrick = g_pbPlayBoard.GetNextBrick();
		DrawGameInfo(bNextBrick);

		g_psSprite->End();

		g_prsRenderToSurface->EndScene(0);
	}

	g_pDirect3DDevice->SetRenderTarget(0, psBackBuffer);
	psBackBuffer->Release();

	if (SUCCEEDED(g_pDirect3DDevice->BeginScene()))
	{
		g_psSprite->Begin(D3DXSPRITE_ALPHABLEND);

		g_pDirect3DDevice->SetPixelShader(m_ppsPixelShader);
		g_psSprite->Draw(g_ptRenderTargetTexture, NULL, NULL, NULL, D3DCOLOR_RGBA(255, 255, 255, 255));
		g_psSprite->Flush();

		g_pDirect3DDevice->SetPixelShader(NULL);

		D3DXMATRIX mCurrentTransformation;
		g_psSprite->GetTransform(&mCurrentTransformation);

		D3DXMATRIX mTranslation;
		D3DXMatrixTranslation(&mTranslation, g_fPauseMenuPositionX, 0.0f, 0.0f);
		g_psSprite->SetTransform(&(mCurrentTransformation * mTranslation));

		const D3DXVECTOR3 v3PauseMenuPosition(0.0f, 20.0f, 0.0f);
		g_psSprite->Draw(g_ptTexture, &g_rPauseMenuBackgroundTextureRect, NULL, &v3PauseMenuPosition, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 0.8f));

		// draw pause menu
		RECT rMenuEntryRect = { 10, 30, 160, 80 };

		for (size_t nMenuEntryIndex = 0; nMenuEntryIndex != PME__MAX; ++nMenuEntryIndex)
		{
			const D3DCOLOR cTextColor = nMenuEntryIndex == g_mePauseMenuSelectedEntry ? D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f) : D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 0.5f);
			g_pfMainMenuFont->DrawText(g_psSprite, g_aszPauseMenuEntryNames[nMenuEntryIndex], -1, &rMenuEntryRect, DT_LEFT, cTextColor);
			rMenuEntryRect.top += 30;
			rMenuEntryRect.bottom += 30;
		}

		g_psSprite->SetTransform(&mCurrentTransformation);

		g_psSprite->Draw(g_ptTexture, &g_rPauseMenuUpperCorner, NULL, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 0.8f));
		g_psSprite->Draw(g_ptTexture, &g_rPauseMenuLowerCorner, NULL, &D3DXVECTOR3(0.0f, 140.0f, 0.0f), D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 0.8f));

		g_psSprite->End();
		g_pDirect3DDevice->EndScene();
	}

	g_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
}


void RenderGame_Finished()
{
	g_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0f, 0);

	if (SUCCEEDED(g_pDirect3DDevice->BeginScene()))
	{
		D3DVIEWPORT9 vpViewPort;
		g_pDirect3DDevice->GetViewport(&vpViewPort);
		RECT rFontPosition = {0, 100, vpViewPort.Width, vpViewPort.Height};

		g_psSprite->Begin(D3DXSPRITE_ALPHABLEND);

		g_pfMainMenuFont->DrawText(g_psSprite, TEXT("Game Over"), -1, &rFontPosition, DT_CENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
		rFontPosition.top += 30;
		TCHAR szScoreStrBuffer[32];
		StringCbPrintf(szScoreStrBuffer, sizeof(szScoreStrBuffer), TEXT("Your score: %d"), g_nGameScore);
		g_pfCommonFont->DrawText(g_psSprite, szScoreStrBuffer, -1, &rFontPosition, DT_CENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));
		rFontPosition.top += 30;
		g_pfCommonFont->DrawText(g_psSprite, TEXT("(Press Space to continue)"), -1, &rFontPosition, DT_CENTER, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f));

		g_psSprite->End();

		g_pDirect3DDevice->EndScene();
	}

	g_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
}

void RenderGame()
{
	switch(g_gsGameCurrentState)
	{
		case GS_MAINMENU:
		{
			RenderGame_MainMenu();

			break;
		}

		case GS_PLAYING:
		{
			RenderGame_Playing();

			break;
		}

		case GS_PAUSED:
		{
			RenderGame_Paused();

			break;
		}

		case GS_FINISHED:
		{
			RenderGame_Finished();

			break;
		}

		default:
		{
			break;
		}
	}
}

void ProcessUserInput_MainMenu(const RAWINPUT &riInputData)
{
	if (riInputData.data.keyboard.VKey == VK_RETURN && riInputData.data.keyboard.Message == WM_KEYDOWN)
	{
		switch (g_meMainMenuSelectedEntry)
		{
		case MME_NEW_GAME:
			{
				ResetGame();
				g_gsGameCurrentState = GS_PLAYING;

				break;
			}

		case MME_EXIT:
			{
				PostQuitMessage(0);

				break;
			}

		default:
			{
				break;
			}
		}
	}

	if (riInputData.data.keyboard.VKey == VK_UP && riInputData.data.keyboard.Message == WM_KEYDOWN)
	{
		if (g_meMainMenuSelectedEntry == MME__MIN)
		{
			g_meMainMenuSelectedEntry = MME__MAX;
		}

		--g_meMainMenuSelectedEntry;
	}

	if (riInputData.data.keyboard.VKey == VK_DOWN && riInputData.data.keyboard.Message == WM_KEYDOWN)
	{
		++g_meMainMenuSelectedEntry;

		if (g_meMainMenuSelectedEntry == MME__MAX)
		{
			g_meMainMenuSelectedEntry = MME__MIN;
		}
	}
}

void ProcessUserInput_Playing(const RAWINPUT &riInputData, CPlayBoard &pbPlayBoard)
{
	if (KEYDOWN(riInputData, VK_SPACE))
	{
		pbPlayBoard.DropBrick();
	}

	if (KEYDOWN(riInputData, VK_UP))
	{
		if (pbPlayBoard.RotateBrick())
		{
			g_audioState.pSoundBank->Play(g_audioState.iBrickRotated, 0, 0, NULL);
		}
		else
		{
			Beep(1000, 100);
		}
	}

	if (KEYDOWN(riInputData, VK_RIGHT))
	{
		if (!pbPlayBoard.MoveBrickRight())
		{
			Beep(1000, 100);
		}
	}

	if (KEYDOWN(riInputData, VK_LEFT))
	{
		if (!pbPlayBoard.MoveBrickLeft())
		{
			Beep(1000, 100);
		}
	}

	if (KEYDOWN(riInputData, VK_DOWN))
	{
		if (!pbPlayBoard.MoveBrickDown())
		{
			Beep(1000, 100);
		}
	}

	if (KEYDOWN(riInputData, VK_ESCAPE))
	{
		g_gsGameCurrentState = GS_PAUSED;
		g_fPauseMenuPositionX = PAUSE_MENU_INITIAL_POSITION_X;
	}
}

void ProcessUserInput_Paused(const RAWINPUT &riInputData, CPlayBoard &pbPlayBoard)
{
	if (KEYDOWN(riInputData, VK_ESCAPE))
	{
		g_gsGameCurrentState = GS_PLAYING;
	}

	if (KEYDOWN(riInputData, VK_RETURN))
	{
		switch (g_mePauseMenuSelectedEntry)
		{
		case PME_CONTINUE_GAME:
			{
				g_gsGameCurrentState = GS_PLAYING;

				break;
			}

		case PME_RESTART_GAME:
			{
				ResetGame();
				g_gsGameCurrentState = GS_PLAYING;

				break;
			}

		case PME_EXIT:
			{
				PostQuitMessage(0);

				break;
			}

		default:
			{
				break;
			}
		}
	}

	if (KEYDOWN(riInputData, VK_UP))
	{
		if (g_mePauseMenuSelectedEntry == PME__MIN)
		{
			g_mePauseMenuSelectedEntry = PME__MAX;
		}

		--g_mePauseMenuSelectedEntry;
	}

	if (KEYDOWN(riInputData, VK_DOWN))
	{
		++g_mePauseMenuSelectedEntry;

		if (g_mePauseMenuSelectedEntry == PME__MAX)
		{
			g_mePauseMenuSelectedEntry = PME__MIN;
		}
	}
}

void ProcessUserInput_Finished(const RAWINPUT &riInputData)
{
	if (KEYDOWN(riInputData, VK_SPACE))
	{
		g_gsGameCurrentState = GS_MAINMENU;
	}
}

void ProcessUserInput(LPARAM lParam)
{
	UINT dwSize;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	void *pvInputDataBuffer = new BYTE[dwSize];

	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, pvInputDataBuffer, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
	{
		const RAWINPUT &riInputData = *(RAWINPUT *)pvInputDataBuffer;

		if (riInputData.header.dwType == RIM_TYPEKEYBOARD) 
		{
			switch(g_gsGameCurrentState)
			{
				case GS_MAINMENU:
				{
					ProcessUserInput_MainMenu(riInputData);

					break;
				}

				case GS_PLAYING:
				{
					ProcessUserInput_Playing(riInputData, g_pbPlayBoard);

					break;
				}

				case GS_PAUSED:
				{
					ProcessUserInput_Paused(riInputData, g_pbPlayBoard);

					break;
				}

				case GS_FINISHED:
				{
					ProcessUserInput_Finished(riInputData);

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}

	delete[] pvInputDataBuffer; 
}