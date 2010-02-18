#include "StdAfx.h"
#include "PlayBoard.h"
#include "Constants.h"
#include "Direct3DFunctions.h"

#define BRICK_MOVE_DEFAULT_PERIOD 1.0f

CFadingCellPainter::CFadingCellPainter(unsigned nBoardWidth, unsigned nFadeLength):
	m_nBoardWidth(nBoardWidth),
	m_nFadeLength(nFadeLength),
	m_nTotalCellCount(0),
	m_nFirstOpaqueCell(0)
{
}

float CFadingCellPainter::operator() (unsigned nCellIndex) const
{
	float fTransparency;

	if (m_nFirstOpaqueCell >= nCellIndex + m_nFadeLength)
	{
		fTransparency = 0.0f;
	}
	else if (m_nFirstOpaqueCell >= nCellIndex + 0)
	{
		fTransparency = 1.0f - (float)(m_nFirstOpaqueCell - nCellIndex) / (float)m_nFadeLength;
	}
	else
	{
		fTransparency = 1.0f;
	}

	return fTransparency;
}

void CFadingCellPainter::Reset(unsigned nLineCount)
{
	m_nTotalCellCount = nLineCount * m_nBoardWidth;
	m_nFirstOpaqueCell = 0;
}

bool CFadingCellPainter::NextStep()
{
	bool bResult = false;

	if (m_nFirstOpaqueCell < m_nTotalCellCount + m_nFadeLength)
	{
		++m_nFirstOpaqueCell;
		bResult = true;
	}

	return bResult;
}

CPlayBoard::CPlayBoard(float fPositionX, float fPositionY):
	m_cpFadingCellPainter(BOARD_COLUMN_COUNT, 10),
	m_fLastUpdateTime(0.0f),
	m_fBrickMovePeriod(BRICK_MOVE_DEFAULT_PERIOD),
	m_fPositionX(fPositionX),
	m_fPositionY(fPositionY),
	m_nBrickCellPositionX(0),
	m_nBrickCellPositionY(0),
	m_nBrickCollisionCellPositionY(0),
	m_nFirstCompletedLine(ULONG_MAX),
	m_nLastCompletedLine(ULONG_MAX),
	m_bIsBrickDropped(false),
	m_bHasSettleDelayPassed(false)
{
}

bool CPlayBoard::RotateBrick()
{
	bool bResult = false;

	m_bActiveBrick.m_rsRotationState = (CBrick::EBRICKROTATIONSTATE)(((int)m_bActiveBrick.m_rsRotationState + 1) % (int)CBrick::BRS__MAX);

	if (!DetectCollision(0, 0))
	{
		RecalculateCollisionPosition();

		bResult = true;
	}
	else
	{
		// revert changes
		m_bActiveBrick.m_rsRotationState = (CBrick::EBRICKROTATIONSTATE)(((int)m_bActiveBrick.m_rsRotationState + CBrick::BRS__MAX - 1) % (int)CBrick::BRS__MAX);
	}

	return bResult;
}

bool CPlayBoard::MoveBrickRight()
{
	bool bResult = false;

	if (!DetectCollision(+1, 0))
	{
		++m_nBrickCellPositionX;
		RecalculateCollisionPosition();

		bResult = true;
	}

	return bResult;
}

bool CPlayBoard::MoveBrickLeft()
{
	bool bResult = false;

	if (!DetectCollision(-1, 0))
	{
		--m_nBrickCellPositionX;
		RecalculateCollisionPosition();

		bResult = true;
	}

	return bResult;
}

bool CPlayBoard::MoveBrickDown()
{
	bool bResult = false;

	if (m_nBrickCellPositionY < m_nBrickCollisionCellPositionY)
	{
		++m_nBrickCellPositionY;
		bResult = true;
	}

	return bResult;
}

void CPlayBoard::DropBrick()
{
	SetIsBrickDropped(true);
}

void CPlayBoard::Clear()
{
	memset(m_afcCells, CBrick::BCC_NONE, sizeof(m_afcCells));
	memset(m_anLineRemovingDirection, 0, sizeof(m_anLineRemovingDirection));

	m_bActiveBrick.Randomize();
	m_fNextBrick.Randomize();
	m_nBrickCellPositionX = BOARD_COLUMN_COUNT / 2 - 1;
	m_nBrickCellPositionY = 0;

	RecalculateCollisionPosition();
}

void CPlayBoard::Update(float fCurrentTime, bool &bOutIsBrickSettled, unsigned &nOutCompletedLineCount, bool &bOutIsBoardFull)
{
	bOutIsBrickSettled = false;
	nOutCompletedLineCount = 0;
	bOutIsBoardFull = false;

	if (m_nBrickCollisionCellPositionY > m_nBrickCellPositionY)
	{
		if (fCurrentTime - m_fLastUpdateTime >= GetBrickMovePeriod() || GetIsBrickDropped())
		{
			++m_nBrickCellPositionY;
			m_fLastUpdateTime = fCurrentTime;
		}
	}
	else if(!GetHasSettleDelayPassed() && !GetIsBrickDropped())
	{
		if (fCurrentTime - m_fLastUpdateTime >= 0.5f)
		{
			SetHasSettleDelayPassed(true);
			m_fLastUpdateTime = fCurrentTime;
		}
	}
	else
	{
		if (!GetIsAnyLineCompleted())
		{
			SettleBrick();
			const unsigned nLinesRemoved = MarkCompletedLinesForRemoval(m_nBrickCellPositionY, m_nBrickCellPositionY + CBrick::MAX_LINES - 1);
			SetHasSettleDelayPassed(true);
			SetIsBrickDropped(false);

			bOutIsBrickSettled = true;
			nOutCompletedLineCount = nLinesRemoved;
		}
		else
		{
			if (fCurrentTime - m_fLastUpdateTime >= 0.01f)
			{
				if (!m_cpFadingCellPainter.NextStep())
				{
					RemoveCompletedLines();
				}

				m_fLastUpdateTime = fCurrentTime;
			}
		}

		if (!GetIsAnyLineCompleted())
		{
			m_bActiveBrick = m_fNextBrick;
			m_fNextBrick.Randomize();
			m_nBrickCellPositionX = BOARD_COLUMN_COUNT / 2 - 1;
			m_nBrickCellPositionY = 0;

			if (DetectCollision(0, 0))
			{
				bOutIsBoardFull = true;
			}
			else
			{
				RecalculateCollisionPosition();
			}

			SetHasSettleDelayPassed(false);
		}
	}
}

void CPlayBoard::Draw() const
{
	D3DXVECTOR3 v3BackgroundPosition(m_fPositionX - 20.0f, m_fPositionY - 20.0f, 0.0f);
	g_psSprite->Draw(g_ptTexture, &g_rPlayBoardBackgroundTextureRect, NULL, &v3BackgroundPosition, D3DCOLOR_RGBA(255, 255, 255, 255));

	for (int nLineIndex = 0; nLineIndex != BOARD_LINE_COUNT; ++nLineIndex)
	{
		if (m_anLineRemovingDirection[nLineIndex] == 0)
		{
			DrawSingleLine(nLineIndex);
		}
	}

	if (GetIsAnyLineCompleted())
	{
		unsigned nLineIndex = m_nFirstCompletedLine;
		unsigned nColumnIndex = (m_anLineRemovingDirection[nLineIndex] == 1) ? 0 : (BOARD_COLUMN_COUNT - 1);
		unsigned nCurrentCellIndex = 0;

		do
		{
			const CBrick::EBRICKCELLCOLOR fcColor = m_afcCells[nLineIndex][nColumnIndex];
			const RECT *prTextureSourceRect = &g_arCellTextureRectsByColor[fcColor];
			D3DXVECTOR3 v3CellPosition(m_fPositionX + nColumnIndex * CELL_HORIZONTAL_SIZE, m_fPositionY + nLineIndex * CELL_VERTICAL_SIZE, 0.0f);

			g_psSprite->Draw(g_ptTexture, prTextureSourceRect, NULL, &v3CellPosition, D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, m_cpFadingCellPainter(nCurrentCellIndex)));
			++nCurrentCellIndex;
		}
		while (GetNextRemovedCell(nLineIndex, nColumnIndex));
	}
	else
	{
		if (m_nBrickCollisionCellPositionY != m_nBrickCellPositionY)
		{
			DrawActiveBrickFootprint(m_bActiveBrick);
		}

		DrawActiveBrick(m_bActiveBrick);
	}
}

void CPlayBoard::RecalculateCollisionPosition()
{
	unsigned nDistanceToCollision = 1;

	while (!DetectCollision(0, nDistanceToCollision))
	{
		++nDistanceToCollision;
	}

	m_nBrickCollisionCellPositionY = m_nBrickCellPositionY + nDistanceToCollision - 1;
}

bool CPlayBoard::DetectCollision(int nOffsetX, int nOffsetY) const
{
	bool bResult = false;

	CBrickBitmap bbBitmap = CBrick::m_abbBrickBitmaps[m_bActiveBrick.m_btType][m_bActiveBrick.m_rsRotationState];

	int nCellIndexY = 0;
	while (bbBitmap && !bResult)
	{
		for (int nCellIndexX = 0; nCellIndexX != CBrick::MAX_COLUMNS && bbBitmap; ++nCellIndexX)
		{
			if (bbBitmap & 0x8000)
			{
				const int nFinalIndexX = m_nBrickCellPositionX + nCellIndexX + nOffsetX;
				const int nFinalIndexY = m_nBrickCellPositionY + nCellIndexY + nOffsetY;

				if (nFinalIndexX < 0 || nFinalIndexX > BOARD_COLUMN_COUNT - 1||
					nFinalIndexY < 0 || nFinalIndexY > BOARD_LINE_COUNT - 1 ||
					m_afcCells[nFinalIndexY][nFinalIndexX] != CBrick::BCC_NONE)
				{
					bResult = true;
					break;
				}
			}

			bbBitmap <<= 1;
		}

		++nCellIndexY;
	}

	return bResult;
}

void CPlayBoard::SettleBrick()
{
	CBrickBitmap bbBitmap = CBrick::m_abbBrickBitmaps[m_bActiveBrick.m_btType][m_bActiveBrick.m_rsRotationState];
	const CBrick::EBRICKCELLCOLOR bcBrickColor = m_bActiveBrick.m_ccColor;

	int nCellIndexY = 0;
	while (bbBitmap)
	{
		for (int nCellIndexX = 0; nCellIndexX != CBrick::MAX_COLUMNS && bbBitmap; ++nCellIndexX)
		{
			if (bbBitmap & 0x8000)
			{
				const int nFinalIndexX = m_nBrickCellPositionX + nCellIndexX;
				const int nFinalIndexY = m_nBrickCellPositionY + nCellIndexY;

				m_afcCells[nFinalIndexY][nFinalIndexX] = bcBrickColor;
			}

			bbBitmap <<= 1;
		}

		++nCellIndexY;
	}
}

bool CPlayBoard::IsLineCompleted(unsigned nLineIndex)
{
	int nCellIndex = 0;

	for ( ; nCellIndex != BOARD_COLUMN_COUNT; ++nCellIndex)
	{
		if (m_afcCells[nLineIndex][nCellIndex] == CBrick::BCC_NONE)
		{
			break;
		}
	}

	return nCellIndex == BOARD_COLUMN_COUNT;
}

unsigned CPlayBoard::MarkCompletedLinesForRemoval(unsigned nFromLine, unsigned nToLine)
{
	unsigned nMarkedLineCount = 0;
	const unsigned nAdjustedToLine = min(nToLine, BOARD_LINE_COUNT - 1);

	int nRemovingDirection = 1;
	bool bIsFirstLineForRemovalSet = false;

	for (unsigned nLineIndex = nAdjustedToLine + 1; nLineIndex > nFromLine; --nLineIndex) //  +1 to avoid underflow when nFromLine is 0
	{
		const unsigned nAdjustedLineIndex = nLineIndex - 1; // make nLineIndex 0-based back

		if (IsLineCompleted(nAdjustedLineIndex))
		{
			if (!bIsFirstLineForRemovalSet)
			{
				m_nFirstCompletedLine = nAdjustedLineIndex;
				bIsFirstLineForRemovalSet = true;
			}

			m_nLastCompletedLine = nAdjustedLineIndex;

			m_anLineRemovingDirection[nAdjustedLineIndex] = nRemovingDirection;
			nRemovingDirection = -nRemovingDirection;
			++nMarkedLineCount;
		}
	}

	m_cpFadingCellPainter.Reset(nMarkedLineCount);

	return nMarkedLineCount;
}

bool CPlayBoard::GetNextRemovedCell(unsigned &nVarLineNumber, unsigned &nVarColumnNumber) const
{
	bool bResult = false;

	const int nRemovingDirection = m_anLineRemovingDirection[nVarLineNumber];

	do
	{
		if (nVarColumnNumber == 0 && nRemovingDirection == -1)
		{
			if (nVarLineNumber == m_nLastCompletedLine)
			{
				break;
			}

			while (nVarLineNumber != m_nLastCompletedLine)
			{
				--nVarLineNumber;
				if (m_anLineRemovingDirection[nVarLineNumber] != 0)
				{
					break;
				}
			}

			nVarColumnNumber = 0;
		}
		else if (nVarColumnNumber == BOARD_COLUMN_COUNT - 1 && nRemovingDirection == +1)
		{
			if (nVarLineNumber == m_nLastCompletedLine)
			{
				break;
			}

			while (nVarLineNumber != m_nLastCompletedLine)
			{
				--nVarLineNumber;
				if (m_anLineRemovingDirection[nVarLineNumber] != 0)
				{
					break;
				}
			}

			nVarColumnNumber = BOARD_COLUMN_COUNT - 1;
		}
		else
		{
			nVarColumnNumber += nRemovingDirection;
		}

		bResult = true;
	}
	while (false);

	return bResult;
}

void CPlayBoard::RemoveCompletedLines()
{
	ASSERT(GetIsAnyLineCompleted());
	ASSERT(m_nLastCompletedLine <= m_nFirstCompletedLine); // order is reversed because lines disappear from bottom to top

	for (unsigned nLineIndex = m_nLastCompletedLine; nLineIndex <= m_nFirstCompletedLine; ++nLineIndex)
	{
		if (m_anLineRemovingDirection[nLineIndex] != 0)
		{
			RemoveLine(nLineIndex);
		}
	}

	const size_t nZeroedMemorySize = (const char *)(m_anLineRemovingDirection + m_nFirstCompletedLine + 1) - (const char *)(m_anLineRemovingDirection + m_nLastCompletedLine);
	memset(m_anLineRemovingDirection + m_nLastCompletedLine, 0, nZeroedMemorySize);

	ResetIsAnyLineCompleted();
}

void CPlayBoard::RemoveLine(int nRemovedLineIndex)
{
	if (nRemovedLineIndex > 0)
	{
		for (int nLineIndex = nRemovedLineIndex; nLineIndex != 0; --nLineIndex)
		{
			memcpy(m_afcCells[nLineIndex], m_afcCells[nLineIndex - 1], sizeof(m_afcCells[nLineIndex]));
		}
	}

	memset(m_afcCells[0], CBrick::BCC_NONE, sizeof(m_afcCells[0]));
}

void CPlayBoard::DrawActiveBrick(const CBrick &bSourceBrick) const
{
	D3DXMATRIX mCurrentTransform;
	g_psSprite->GetTransform(&mCurrentTransform);
	D3DXMATRIX mBrickTranslation;
	D3DXMatrixTranslation(&mBrickTranslation, m_fPositionX + m_nBrickCellPositionX * CELL_HORIZONTAL_SIZE, m_fPositionY + m_nBrickCellPositionY * CELL_VERTICAL_SIZE, 0.0f);
	D3DXMATRIX mBrickScaling;

	g_psSprite->SetTransform(&(mCurrentTransform * mBrickTranslation));
	bSourceBrick.Draw();
	g_psSprite->SetTransform(&mCurrentTransform);
}

void CPlayBoard::DrawActiveBrickFootprint(const CBrick &bSourceBrick) const
{
	D3DXMATRIX mCurrentTransform;
	g_psSprite->GetTransform(&mCurrentTransform);
	D3DXMATRIX mBrickTranslation;
	D3DXMatrixTranslation(&mBrickTranslation, m_fPositionX + m_nBrickCellPositionX * CELL_HORIZONTAL_SIZE, m_fPositionY + m_nBrickCollisionCellPositionY * CELL_VERTICAL_SIZE, 0.0f);
	D3DXMATRIX mBrickScaling;

	g_psSprite->SetTransform(&(mCurrentTransform * mBrickTranslation));
	bSourceBrick.Draw(0.2f);
	g_psSprite->SetTransform(&mCurrentTransform);
}

void CPlayBoard::DrawSingleLine(unsigned nLineIndex) const
{
	D3DXVECTOR3 v3CellPosition(m_fPositionX, m_fPositionY + nLineIndex * CELL_VERTICAL_SIZE, 0.0f);

	for (int nColumnIndex = 0; nColumnIndex != BOARD_COLUMN_COUNT; ++nColumnIndex)
	{
		const CBrick::EBRICKCELLCOLOR fcColor = m_afcCells[nLineIndex][nColumnIndex];

		if (fcColor != CBrick::BCC_NONE)
		{
			const RECT *prTextureSourceRect = &g_arCellTextureRectsByColor[fcColor];
			g_psSprite->Draw(g_ptTexture, prTextureSourceRect, NULL, &v3CellPosition, D3DCOLOR_RGBA(255, 255, 255, 255));
		}

		v3CellPosition.x += CELL_HORIZONTAL_SIZE;
	}
}

bool CPlayBoard::GetIsAnyLineCompleted() const
{
	return m_nFirstCompletedLine != ULONG_MAX;
}

void CPlayBoard::ResetIsAnyLineCompleted()
{
	m_nFirstCompletedLine = ULONG_MAX;
	m_nLastCompletedLine = ULONG_MAX;
}

