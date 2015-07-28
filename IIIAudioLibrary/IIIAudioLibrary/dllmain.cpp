/*
	Includes
*/

#include <VC.CLEO.h>
#include <bass.h>
#include "stdafx.h"
#include <stdio.h>

/*
	Opcode Defines
*/

#define BASS_PLAY_STREAM 	0x0AAC
#define BASS_STOP_STREAM 0x0AAD
#define BASS_PLAY_MOD	0x7AAA
#define BASS_STOP_MOD	0x7AAB
#define BASS_PLAY_SFX	0x7AAC
#define BASS_SET_MOD_POS 0x7ABB
#define BASS_STREAM_CONTROL	0x7ABC

/*
	CLEO Version to check
*/

#define CLEO_VERSION_MAIN    2
#define CLEO_VERSION_MAJOR   0
#define CLEO_VERSION_MINOR   0
#define CLEO_VERSION_BINARY  0

#define CLEO_VERSION ((CLEO_VERSION_MAIN << 16)|(CLEO_VERSION_MAJOR << 12)|(CLEO_VERSION_MINOR << 8)|(CLEO_VERSION_BINARY))

/*
	Opcode Parameter Controller
*/

tScriptVar *Params;

/*
	Variables
*/

int device = -1; // the audio device(s)
int freq = 44100; // frequency
int loop = 0; // loop 1 channel? 
int sfxloop = 0; // loop 2 channel?

/*
	BASS Channels
*/

HSTREAM streamHandle;					
HSTREAM sfxHandle;
HMUSIC musicHandle;
BOOL isBASSLoaded = FALSE;

// 7ABC: set_stream_playing_mode channel 0 mode 0

eOpcodeResult WINAPI StreamControl(CScript* script)
{
	script->Collect(2);

	int channel = 0;
	int status = 0;

	channel = Params[0].nVar;
	status = Params[0].nVar;

	switch(status)
	{
	case 0:
		if(channel == 0)
		{
			if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING)
			{
				BASS_ChannelPause(streamHandle);
			}
		} else {
			if(BASS_ChannelIsActive(sfxHandle) == BASS_ACTIVE_PLAYING)
			{
				BASS_ChannelPause(sfxHandle);
			}
		}
	case 1:
		if(channel == 0)
		{
			if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PAUSED)
			{
				BASS_ChannelPlay(streamHandle, FALSE);
			}
		} else {
			if(BASS_ChannelIsActive(sfxHandle) == BASS_ACTIVE_PAUSED)
			{
				BASS_ChannelPlay(streamHandle, TRUE);
			}
		}
	}
	return OR_CONTINUE;
}

// 7ABB: set_mod_position 2 2

eOpcodeResult WINAPI SetMODPosition(CScript* script)
{	
	script->Collect(2);

	int order = 0;
	int row = 0;
	
	order = Params[0].nVar;
	row = Params[1].nVar;

	if(BASS_ChannelIsActive(musicHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelSetPosition(musicHandle, MAKELONG(order, row), BASS_POS_MUSIC_ORDER|BASS_MUSIC_POSRESET);
	}

	return OR_CONTINUE;
}

// 7AAC: play_audio_stream_2channel "Audio\test.wav" loop 1 volume 0.5

eOpcodeResult WINAPI SFXPlayer(CScript* script)
{
	script->Collect(3);

	char sfxpath[100];
	float sfxvolume = 0.0f;

	strcpy_s(sfxpath, Params[0].cVar); 
	sfxloop = Params[1].nVar;
	sfxvolume = Params[2].fVar;

	if(isBASSLoaded == FALSE) {
		BASS_Init(device, freq, 0, 0, NULL);
		isBASSLoaded = TRUE;
	}

	if(BASS_ChannelIsActive(sfxHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(sfxHandle);
		BASS_StreamFree(sfxHandle);
		sfxHandle = NULL;
	}

	sfxHandle = BASS_StreamCreateFile(FALSE, sfxpath, 0, 0, 0);
	BASS_ChannelPlay(sfxHandle, FALSE);
	
	if(sfxloop == 1) {
		BASS_ChannelFlags(streamHandle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	} 

	BASS_ChannelSetAttribute(streamHandle, BASS_ATTRIB_VOL, sfxvolume);

	return OR_CONTINUE;
}

// 7AAB: stop_mod_music

eOpcodeResult WINAPI StopMOD(CScript* script)
{
	if(BASS_ChannelIsActive(musicHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(musicHandle);
		BASS_MusicFree(musicHandle);
		musicHandle = NULL;
	}
	return OR_CONTINUE;
}

//7AAA: play_mod_music "test.mod"

eOpcodeResult WINAPI PlayMOD(CScript* script)
{
	script->Collect(1);

	char modpath[100];

	strcpy_s(modpath, Params[0].cVar);

	if(isBASSLoaded == FALSE) {
		BASS_Init(device, freq, 0, 0, NULL);
		isBASSLoaded = TRUE;
	}

	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
		loop = 0;
	}
	
	if(BASS_ChannelIsActive(musicHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(musicHandle);
		streamHandle = NULL;
		loop = 0;
	}

		musicHandle = BASS_MusicLoad(FALSE, modpath, 0, 0, BASS_SAMPLE_LOOP, 0);
		BASS_ChannelPlay(musicHandle, FALSE);;
	return OR_CONTINUE;
}

// 0AAD: stop_audio_stream

eOpcodeResult WINAPI StopStream(CScript* script)
{
	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
		loop = 0;
	}
	return OR_CONTINUE;
}

// 0AAC: play_audio_stream "test.mp3" loop 0 volume 0.5

eOpcodeResult WINAPI PlayStream(CScript* script)
{
	script->Collect(3);

	char path[100];
	float volume = 0.0f;

	strcpy_s(path, Params[0].cVar); 
	loop = Params[1].nVar;
	volume = Params[2].fVar;

	if(isBASSLoaded == FALSE) {
		BASS_Init(device, freq, 0, 0, NULL);
		isBASSLoaded = TRUE;
	}

	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
	}

	streamHandle = BASS_StreamCreateFile(FALSE, path, 0, 0, 0);
	BASS_ChannelPlay(streamHandle, FALSE);

	if(loop == 1) {
		BASS_ChannelFlags(streamHandle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	} 

	BASS_ChannelSetAttribute(streamHandle, BASS_ATTRIB_VOL, volume);

	return OR_CONTINUE;
}

// DLL Main Method

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		if (CLEO_GetVersion() < CLEO_VERSION)
		{
			MessageBox(HWND_DESKTOP, "Incorrect CLEO Version.\nRequired version 2.0.0.1, Installed version: ," + CLEO_GetVersion(),"AudioPlugin", MB_ICONERROR);
			return FALSE;
		}
		
		// Registers the opcodes in the game, to be used by script makers.

		Params = CLEO_GetParamsAddress();
		Opcodes::RegisterOpcode(BASS_PLAY_STREAM, PlayStream);
		Opcodes::RegisterOpcode(BASS_STOP_STREAM, StopStream);
		Opcodes::RegisterOpcode(BASS_PLAY_MOD, PlayMOD);
		Opcodes::RegisterOpcode(BASS_STOP_MOD, StopMOD);
		Opcodes::RegisterOpcode(BASS_PLAY_SFX, SFXPlayer);
		Opcodes::RegisterOpcode(BASS_SET_MOD_POS, SetMODPosition);
		Opcodes::RegisterOpcode(BASS_STREAM_CONTROL, StreamControl);
	}
	return TRUE;
}