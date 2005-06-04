#pragma once

#include <windows.h>

#define ADDR_AIL_STREAM_MS_POSITION		0x6F25B8
#define ADDR_AIL_SET_STREAM_MS_POSITION 0x6F25CC
#define ADDR_AIL_OPEN_STREAM			0x6F25C4
#define ADDR_AIL_CLOSE_STREAM			0x6F25C0
#define ADDR_AIL_SET_STREAM_VOLUME		0x6F25B0
#define ADDR_AIL_PAUSE_STREAM			0x6F25BC

typedef void (__stdcall * p_AIL_close_stream)(DWORD);
typedef void (__stdcall * p_AIL_stream_ms_position)(DWORD, DWORD*, DWORD*);
typedef void (__stdcall * p_AIL_set_stream_ms_position)(DWORD, DWORD);
typedef DWORD (__stdcall * p_AIL_open_stream)(DWORD, char *, DWORD);
typedef void (__stdcall * p_AIL_pause_stream)(DWORD, DWORD);

typedef struct _GameSound
{
	p_AIL_close_stream CloseStream;
	p_AIL_stream_ms_position StreamMsPosition;
	p_AIL_set_stream_ms_position SetStreamMsPosition;
	p_AIL_open_stream OpenStream;
	p_AIL_pause_stream PauseStream;

} GameSound;

void InitGameSound(GameSound* pSound);