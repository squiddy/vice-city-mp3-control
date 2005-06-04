#pragma once

#include <windows.h>

#define SCREENWIDTH 0x9B48E4
#define SCREENHEIGHT 0x9B48E8

#define SCREENWIDTH_SCALEFACTOR 1.5625e-3f
#define SCREENHEIGHT_SCALEFACTOR 2.232143e-3f

typedef struct _ID3v1
{
	char tag[3]; 
	char title[30];
	char artist[30];
	char album[30];
	DWORD year;
	char comment[30];
	BYTE genre;

} ID3v1;

struct MP3FILE
{
	char szFilename[260];
	DWORD dwTrackLength;
	DWORD dwPreviousTrackLength;
	DWORD dwNextFile;
	DWORD dwUnknown1;
	DWORD dwUnknown2;

	char *szTitle;
	char *szInterpret;
	char *szAlbum;
	DWORD dwPreviousFile;
};

struct MP3STATION
{
	char *szName;
	char *szPlaylist;
	bool bShowTitle;
	bool bShuffle;
	int iFade;

	MP3FILE *mp3Start;
	int iCurrentTrack;
	int iTrackCount;
};

struct KEYS
{
	BYTE iNext;
	BYTE iPrevious;
	BYTE iToggleShuffle;
	BYTE iVolumeUp;
	BYTE iVolumeDown;
	BYTE iToggleList;
	BYTE iListScrollUp;
	BYTE iListScrollDown;
	BYTE iListChoose;

};