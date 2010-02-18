#ifndef __PLAYBOARD_H_INCLUDED
#define __PLAYBOARD_H_INCLUDED

#define BOARD_COLUMN_COUNT 10 // cells
#define BOARD_LINE_COUNT 20 // cells

#include "Brick.h"

class CFadingCellPainter
{
public:
	CFadingCellPainter(unsigned nBoardWidth, unsigned nFadeLength);

	float operator() (unsigned nCellIndex) const;
	void Reset(unsigned nLineCount);
	bool NextStep();

private:
	const unsigned		m_nBoardWidth;
	const unsigned		m_nFadeLength;
	unsigned			m_nTotalCellCount;
	unsigned			m_nFirstOpaqueCell;
};

class CPlayBoard
{
public:
	CPlayBoard(float fPositionX, float fPositionY);

	bool RotateBrick();
	bool MoveBrickRight();
	bool MoveBrickLeft();
	bool MoveBrickDown();
	void DropBrick();
	void Clear();

	void Update(float fCurrentTime, bool &bOutIsBrickSettled, unsigned &nOutCompletedLineCount, bool &bOutIsBoardFull);
	void Draw() const;

private:
	void RecalculateCollisionPosition();
	bool DetectCollision(int nOffsetX, int nOffsetY) const;
	void SettleBrick();
	bool IsLineCompleted(unsigned nLineIndex);
	unsigned MarkCompletedLinesForRemoval(unsigned nFromLine, unsigned nToLine);
	bool GetNextRemovedCell(unsigned &nVarLineNumber, unsigned &nVarColumnNumber) const;
	void RemoveCompletedLines();
	void RemoveLine(int nRemovedLineIndex);

	void DrawActiveBrick(const CBrick &bSourceBrick) const;
	void DrawActiveBrickFootprint(const CBrick &bSourceBrick) const;
	void DrawSingleLine(unsigned nLineIndex) const;

public:
	const CBrick &GetNextBrick() const { return m_fNextBrick; }
	float GetBrickMovePeriod() const { return m_fBrickMovePeriod; }
	void SetBrickMovePeriod(float fMovePeriod) { m_fBrickMovePeriod = fMovePeriod; }

private:
	bool GetIsAnyLineCompleted() const;
	void ResetIsAnyLineCompleted();
	bool GetIsBrickDropped() const { return m_bIsBrickDropped; }
	void SetIsBrickDropped(bool bValue) { m_bIsBrickDropped = bValue; }
	bool GetHasSettleDelayPassed() const { return m_bHasSettleDelayPassed; }
	void SetHasSettleDelayPassed(bool bValue) { m_bHasSettleDelayPassed = bValue; }

private:
	CFadingCellPainter	m_cpFadingCellPainter;
	CBrick::EBRICKCELLCOLOR	m_afcCells[BOARD_LINE_COUNT][BOARD_COLUMN_COUNT];
	int					m_anLineRemovingDirection[BOARD_LINE_COUNT];
	float				m_fLastUpdateTime;
	float				m_fBrickMovePeriod;
	CBrick				m_bActiveBrick;
	CBrick				m_fNextBrick;
	float				m_fPositionX;
	float				m_fPositionY;
	unsigned			m_nBrickCellPositionX;
	unsigned			m_nBrickCellPositionY;
	unsigned			m_nBrickCollisionCellPositionY;
	unsigned			m_nFirstCompletedLine;
	unsigned			m_nLastCompletedLine;
	bool				m_bIsBrickDropped;
	bool				m_bHasSettleDelayPassed;
};

#endif // __PLAYBOARD_H_INCLUDED
