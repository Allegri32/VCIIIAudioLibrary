/*
	Includes
*/

#include "stdafx.h"
#include "allog.h"

allog logfile;

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
#define BASS_GET_DURATION	0x7BBB

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
int musDuration = 0; // gets music duration (Streams)

/*
	BASS Channels
*/

HSTREAM streamHandle;					
HSTREAM sfxHandle;
HMUSIC musicHandle;
BOOL isBASSLoaded = FALSE;

// 7ACD: $500 = get_stream_duration

eOpcodeResult WINAPI GetDuration(CScript* script)
{
	QWORD len = BASS_ChannelGetLength(streamHandle, BASS_POS_BYTE); // the length in bytes
	double time = BASS_ChannelBytes2Seconds(streamHandle, len); // the length in seconds

	
	Params[0].nVar = time;
	script->Store(1);

	logfile.write("got duration:");
	logfile.writeint(time);
	return OR_CONTINUE;
}
// 7ABC: set_stream_playing_mode channel 0 mode 0
eOpcodeResult WINAPI StreamControl(CScript* script)
{
	
	script->Collect(1);

	int status = 0;

	status = Params[0].nVar;

	if(status == 0) {
		if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING)
		{
			BASS_ChannelPause(streamHandle);
		}
	} else {
		if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PAUSED)
			{
				BASS_ChannelPlay(streamHandle, FALSE);
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

	if(!BASS_ChannelIsActive(sfxHandle)) {
		MessageBox(HWND_DESKTOP, "Failed to play audio file.\nCheck the path, and try again.", "AudioPlugin", MB_ICONWARNING);
	}
	
	if(sfxloop == 1) {
		BASS_ChannelFlags(streamHandle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	} 

	BASS_ChannelSetAttribute(streamHandle, BASS_ATTRIB_VOL, sfxvolume);

	return OR_CONTINUE;
}

// 7AAB: stop_mod_music

eOpcodeResult WINAPI StopMOD(CScript* script)
{
	logfile.write("Stopping MOD playback.");
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
	logfile.write("Starting MOD playback.");
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

// 0AAD: stop_audio_stream channel 0

eOpcodeResult WINAPI StopStream(CScript* script)
{
	logfile.write("Stopping stream...");
	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {	
		logfile.write("Playback stopped.");
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
		loop = 0;
	}
	return OR_CONTINUE;
}

// 0AAC: play_audio_stream "test.mp3" loop 0 volume 0.5

eOpcodeResult WINAPI PlayStream(CScript* script)
{	
	logfile.write("PLAY_STREAM called.");
	script->Collect(3);

	char path[100];
	float volume = 0.0f;


	strcpy_s(path, Params[0].cVar); 
	loop = Params[1].nVar;
	volume = Params[2].fVar;

	if(isBASSLoaded == FALSE) {
		logfile.write("BASS library is not initialized. Initalizing...");
		BASS_Init(device, freq, 0, 0, NULL);
		logfile.write("BASS is initalized.");
		isBASSLoaded = TRUE;
	}

	if(BASS_ChannelIsActive(streamHandle) == BASS_ACTIVE_PLAYING) {
		BASS_ChannelStop(streamHandle);
		streamHandle = NULL;
	}

	streamHandle = BASS_StreamCreateFile(FALSE, path, 0, 0, 0);
	BASS_ChannelPlay(streamHandle, FALSE);

	if(!BASS_ChannelIsActive(streamHandle)) {
		logfile.write("ERROR: couldn't open specified audio file.");
		MessageBox(HWND_DESKTOP, "Error: couldn't open audio file.", "AudioPlugin", MB_ICONWARNING);
	}

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
		logfile.start();
		logfile.write("CLEO_PLUGIN loaded.");
		// Registers the opcodes in the game, to be used by script makers.

		logfile.write("Getting addresses of params...");
		Params = CLEO_GetParamsAddress();
		Opcodes::RegisterOpcode(BASS_PLAY_STREAM, PlayStream);
		Opcodes::RegisterOpcode(BASS_STOP_STREAM, StopStream);
		Opcodes::RegisterOpcode(BASS_PLAY_MOD, PlayMOD);
		Opcodes::RegisterOpcode(BASS_STOP_MOD, StopMOD);
		Opcodes::RegisterOpcode(BASS_PLAY_SFX, SFXPlayer);
		Opcodes::RegisterOpcode(BASS_SET_MOD_POS, SetMODPosition);
		Opcodes::RegisterOpcode(BASS_STREAM_CONTROL, StreamControl);
		Opcodes::RegisterOpcode(BASS_GET_DURATION, GetDuration);
		logfile.write("All opcodes registered.");
	}
	return TRUE;
}

