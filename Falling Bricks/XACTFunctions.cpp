#include "StdAfx.h"
#include "XACTFunctions.h"
#include "resource.h"

AUDIO_STATE g_audioState;

//-----------------------------------------------------------------------------------------
// This function does the following:
//
//      1. Initialize XACT by calling pEngine->Initialize 
//      2. Create the XACT wave bank(s) you want to use
//      3. Create the XACT sound bank(s) you want to use
//      4. Store indices to the XACT cue(s) your game uses
//-----------------------------------------------------------------------------------------
HRESULT InitializeXACT()
{
	// Clear struct
	ZeroMemory(&g_audioState, sizeof(AUDIO_STATE));

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);  // COINIT_APARTMENTTHREADED will work too
	if (SUCCEEDED(hr))
	{
		DWORD dwCreationFlags = 0;
		hr = XACT3CreateEngine(dwCreationFlags, &g_audioState.pEngine);
	}
	
	if (FAILED(hr) || g_audioState.pEngine == NULL)
		return E_FAIL;

	// Initialize & create the XACT runtime 
	XACT_RUNTIME_PARAMETERS xrParams = {0};
	xrParams.lookAheadTime = XACT_ENGINE_LOOKAHEAD_DEFAULT;
	hr = g_audioState.pEngine->Initialize( &xrParams );
	if (FAILED(hr))
		return hr;

 	const HRSRC hrcWaveBank = FindResource(NULL, MAKEINTRESOURCE(IDR_WAVE_BANK), RT_RCDATA);
	const HGLOBAL hgWaveBank = LoadResource(NULL, hrcWaveBank);
	const void *pvWaveBank = LockResource(hgWaveBank);
	const DWORD dwWaveBankSize = SizeofResource(NULL, hrcWaveBank);

	hr = g_audioState.pEngine->CreateInMemoryWaveBank(pvWaveBank, dwWaveBankSize, 0, 0, &g_audioState.pWaveBank);

	if (FAILED(hr))
		return E_FAIL; // CleanupXACT() will cleanup state before exiting

	const HRSRC hrcSoundBank = FindResource(NULL, MAKEINTRESOURCE(IDR_SOUND_BANK), RT_RCDATA);
	const HGLOBAL hgSoundBank = LoadResource(NULL, hrcSoundBank);
	const void *pvSoundBank = LockResource(hgSoundBank);
	const DWORD dwSoundBankSize = SizeofResource(NULL, hrcSoundBank);

	g_audioState.pbSoundBank = new BYTE[dwSoundBankSize];
	hr = g_audioState.pEngine->CreateSoundBank(pvSoundBank, dwSoundBankSize, 0, 0, &g_audioState.pSoundBank);

	if (FAILED(hr))
		return E_FAIL; // CleanupXACT() will cleanup state before exiting

	// Get the sound cue index from the sound bank
	//
	// Note that if the cue does not exist in the sound bank, the index will be XACTINDEX_INVALID
	// however this is ok especially during development.  The Play or Prepare call will just fail.
	g_audioState.iBrickDropped = g_audioState.pSoundBank->GetCueIndex("BrickDropped");
	g_audioState.iBrickRotated = g_audioState.pSoundBank->GetCueIndex("BrickRotated");
	g_audioState.iLineCompleted = g_audioState.pSoundBank->GetCueIndex("LineCompleted");

	return S_OK;
}

//-----------------------------------------------------------------------------
// Releases all previously initialized XACT objects
//-----------------------------------------------------------------------------
VOID FinalizeXACT()
{
	// Shutdown XACT
	//
	// Note that pEngine->ShutDown is synchronous and will take some time to complete 
	// if there are still playing cues.  Also pEngine->ShutDown() is generally only 
	// called when a game exits and is not the preferred method of changing audio 
	// resources. To know when it is safe to free wave/sound bank data without 
	// shutting down the XACT engine, use the XACTNOTIFICATIONTYPE_SOUNDBANKDESTROYED 
	// or XACTNOTIFICATIONTYPE_WAVEBANKDESTROYED notifications 
	if (g_audioState.pEngine)
	{
		g_audioState.pEngine->ShutDown();
		g_audioState.pEngine->Release();
	}

	if (g_audioState.pbSoundBank)
	{
		delete[] g_audioState.pbSoundBank;
		g_audioState.pbSoundBank = NULL;
	}

	g_audioState.pbWaveBank = NULL;

	CoUninitialize();
}

