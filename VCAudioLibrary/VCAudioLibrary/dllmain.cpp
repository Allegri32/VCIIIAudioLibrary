#include <VC.CLEO.h>
#include <bass.h>
#include "stdafx.h"
#include <stdio.h>

#define BASS_PLAY_STREAM 	0x7AAA
#define BASS_LOAD	0x7AAB
#define BASS_STOP_STREAM 0x7AAC
#define BASS_PLAY_MOD	0x7ABB
#define BASS_STOP_MOD	0x7AB1

#define CLEO_VERSION_MAIN    2
#define CLEO_VERSION_MAJOR   0
#define CLEO_VERSION_MINOR   0
#define CLEO_VERSION_BINARY  0

#define CLEO_VERSION ((CLEO_VERSION_MAIN << 16)|(CLEO_VERSION_MAJOR << 12)|(CLEO_VERSION_MINOR << 8)|(CLEO_VERSION_BINARY))

tScriptVar *Params;

int device = -1;
int freq = 44100;
int loop = 0;
HSTREAM streamHandle;
HMUSIC musicHandle;

eOpcodeResult WINAPI StopMOD(CScript* script)
{
	if(BASS_ChannelIsActive(musicHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(musicHandle);
		BASS_MusicFree(musicHandle);
		musicHandle = NULL;
	}
	return OR_CONTINUE;
}

eOpcodeResult WINAPI PlayMOD(CScript* script)
{
	script->Collect(1);

	char modpath[100];

	strcpy_s(modpath, Params[0].cVar);

	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
		loop = 0;
	}
		musicHandle = BASS_MusicLoad(FALSE, modpath, 0, 0, BASS_SAMPLE_LOOP, 0);
		BASS_ChannelPlay(musicHandle, FALSE);
	return OR_CONTINUE;
}

eOpcodeResult WINAPI StopStream(CScript* script)
{
	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
		loop = 0;
	}
	return OR_CONTINUE;
}

eOpcodeResult WINAPI BassLoader(CScript* script)
{
	BASS_Init(device, freq, 0, 0, NULL);
	return OR_CONTINUE;
}

eOpcodeResult WINAPI PlayStream(CScript* script)
{
	script->Collect(2);

	char path[100];

	strcpy_s(path, Params[0].cVar); 
	loop = Params[1].nVar;
	
	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
	}

	streamHandle = BASS_StreamCreateFile(FALSE, path, 0, 0, 0);
	BASS_ChannelPlay(streamHandle, FALSE);

	if(loop == 1) {
		BASS_ChannelFlags(streamHandle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	} 

	return OR_CONTINUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		if (CLEO_GetVersion() < CLEO_VERSION)
		{
			MessageBox(HWND_DESKTOP, "Failed to load the BASS library for GTA III.\nMake sure, that the version of CLEO is 2.0.0.1.", "BASS Loader", MB_ICONERROR);
			return FALSE;
		}

		Params = CLEO_GetParamsAddress();
		Opcodes::RegisterOpcode(BASS_PLAY_STREAM, PlayStream);
		Opcodes::RegisterOpcode(BASS_LOAD, BassLoader);
		Opcodes::RegisterOpcode(BASS_STOP_STREAM, StopStream);
		Opcodes::RegisterOpcode(BASS_PLAY_MOD, PlayMOD);
		Opcodes::RegisterOpcode(BASS_STOP_MOD, StopMOD);
	}
	return TRUE;
}

