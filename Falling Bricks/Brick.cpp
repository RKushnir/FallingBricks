#include "StdAfx.h"
#include "Brick.h"
#include "Constants.h"
#include "Direct3DFunctions.h"
#include "Utilities.h"

const CBrickBitmap CBrick::m_abbBrickBitmaps[BT__MAX][BRS__MAX] =
{
	// BT_T
	{
		0xE400, // BRS_0DEG
		0x4C40, // BRS_90DEG
		0x4E00, // BRS_180DEG
		0x8C80, // BRS_270DEG
	},
	// BT_L
	{
		0xE800, // BRS_0DEG
		0xC440, // BRS_90DEG
		0x2E00, // BRS_180DEG
		0x88C0, // BRS_270DEG
	},
	// BT_J
	{
		0xE200, // BRS_0DEG
		0x44C0, // BRS_90DEG
		0x8E00, // BRS_180DEG
		0xC880, // BRS_270DEG
	},
	// BT_I
	{
		0xF000, // BRS_0DEG
		0x8888, // BRS_90DEG
		0xF000, // BRS_180DEG
		0x8888, // BRS_270DEG
	},
	// BT_S
	{
		0x6C00, // BRS_0DEG
		0x8C40, // BRS_90DEG
		0x6C00, // BRS_180DEG
		0x8C40, // BRS_270DEG
	},
	// BT_Z
	{
		0xC600, // BRS_0DEG
		0x4C80, // BRS_90DEG
		0xC600, // BRS_180DEG
		0x4C80, // BRS_270DEG
	},
	// BT_D
	{
		0xCC00, // BRS_0DEG
		0xCC00, // BRS_90DEG
		0xCC00, // BRS_180DEG
		0xCC00, // BRS_270DEG
	},
};

void CBrick::Draw(float fOpacity) const
{
	CBrickBitmap bbBitmap = m_abbBrickBitmaps[m_btType][m_rsRotationState];
	const RECT *prTextureSourceRect = &g_arCellTextureRectsByColor[m_ccColor];
	D3DXVECTOR3 v3CurrentPosition(0.0f, 0.0f, 0.0f);

	int nIndexY = 0;
	while (bbBitmap)
	{
		for (int nIndexX = 0; nIndexX != CBrick::MAX_COLUMNS && bbBitmap; ++nIndexX)
		{
			if (bbBitmap & 0x8000)
			{
				v3CurrentPosition.x = nIndexX * CELL_HORIZONTAL_SIZE;
				v3CurrentPosition.y = nIndexY * CELL_VERTICAL_SIZE;

				g_psSprite->Draw(g_ptTexture, prTextureSourceRect, NULL, &v3CurrentPosition, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, fOpacity));
			}

			bbBitmap <<= 1;
		}

		++nIndexY;
	}
}

void CBrick::Randomize()
{
	m_ccColor = (EBRICKCELLCOLOR)GetRandomNumber(BCC_VISIBLE_MIN, BCC_VISIBLE_MAX);
	m_btType = (EBRICKTYPE)GetRandomNumber(BT__MIN, BT__MAX);
	m_rsRotationState = (EBRICKROTATIONSTATE)GetRandomNumber(BRS__MIN, BRS__MAX);
}

