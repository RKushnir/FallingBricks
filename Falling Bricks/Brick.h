#ifndef __BRICK_H_INCLUDED
#define __BRICK_H_INCLUDED

typedef unsigned short CBrickBitmap;

struct CBrick
{
	enum EBRICKTYPE
	{
		BT__MIN,

		BT_T = BT__MIN,
		BT_L,
		BT_J,
		BT_I,
		BT_S,
		BT_Z,
		BT_D,

		BT__MAX,
	};

	enum EBRICKROTATIONSTATE
	{
		BRS__MIN,

		BRS_0DEG = BRS__MIN,
		BRS_90DEG,
		BRS_180DEG,
		BRS_270DEG,

		BRS__MAX,
	};

	enum EBRICKCELLCOLOR
	{
		BCC__MIN,

		BCC_NONE = BCC__MIN,
		BCC_VISIBLE_MIN,

		BCC_VISIBLE_MAX = BCC_VISIBLE_MIN + 5,

		BCC__MAX = BCC_VISIBLE_MAX,
	};

	enum
	{
		MAX_COLUMNS = 4,
		MAX_LINES = 4,
	};

	CBrick():
		m_btType(BT_T),
		m_rsRotationState(BRS_0DEG),
		m_ccColor(BCC_NONE)
	{}

	CBrick(EBRICKTYPE ftType, EBRICKCELLCOLOR ccColor):
		m_btType(ftType),
		m_rsRotationState(BRS_0DEG),
		m_ccColor(ccColor)
	{}

	void Draw(float fOpacity=1.0f) const;
	void Randomize();

	EBRICKTYPE				m_btType;
	EBRICKROTATIONSTATE		m_rsRotationState;
	EBRICKCELLCOLOR			m_ccColor;

	static const CBrickBitmap m_abbBrickBitmaps[BT__MAX][BRS__MAX];
};

#endif // __BRICK_H_INCLUDED
