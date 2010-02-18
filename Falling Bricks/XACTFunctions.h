#ifndef __XACTFUNCTIONS_H_INCLUDED
#define __XACTFUNCTIONS_H_INCLUDED

#include <xact3.h>

//-----------------------------------------------------------------------------
// Struct to hold audio game state
//-----------------------------------------------------------------------------
struct AUDIO_STATE
{
	IXACT3Engine *pEngine;
	IXACT3WaveBank *pWaveBank;
	IXACT3SoundBank *pSoundBank;
	XACTINDEX iBrickDropped;
	XACTINDEX iBrickRotated;
	XACTINDEX iLineCompleted;

	VOID* pbWaveBank; // Handle to wave bank data.  Its memory mapped so call UnmapViewOfFile() upon cleanup to release file
	VOID* pbSoundBank; // Pointer to sound bank data.  Call delete on it when the sound bank is destroyed
};

extern AUDIO_STATE g_audioState;

HRESULT InitializeXACT();
VOID FinalizeXACT();

#endif // __XACTFUNCTIONS_H_INCLUDED
