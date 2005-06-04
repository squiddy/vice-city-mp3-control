#include <windows.h>
#include <stdio.h>

#include "main.h"
#include "font.h"
#include "sound.h"

DWORD* dwCurrentRadiostation	= (DWORD*)0x9839BC;
BYTE* bIsMp3Active				= (BYTE*)0xA10B50;
BYTE* bRadioNameShown			= (BYTE*)0x7839C4;
DWORD* dwMp3Files				= (DWORD*)0x9753E0;
DWORD* dwTrackCount				= (DWORD*)0xA108B0;
DWORD* dwCurrentTrack			= (DWORD*)0x97881C;
DWORD* pCurrentStream			= (DWORD*)0x978668;
DWORD* pHDIGDRIVER				= (DWORD*)0x978550;
BYTE* pbMusicVolume				= (BYTE*)0x983B91;
HWND* hGameWnd					= (HWND*)0x7897A4;
BYTE *bDisableKeyboard			= (BYTE*)0x9B48FC;
BYTE *bDisableKeyboard2			= (BYTE*)0xA10AE4;

WNDPROC hOldWndProc;
GameSound *pSound;
CFont* gFont;

KEYS gKeys;
MP3STATION gMp3Stations[10];
int giCurMp3Station;
int giStationCount = 0;
int giSkipping = 0;
int giFade = 0;
bool gbStationChanged = false;

bool gbList = false;
int giListCurrentItem = 0;
float gfListAlpha = 255.0f;
float gfListFade = 0.0f;

void UpdateMp3InfoWidthID3v1Tags(MP3FILE *pFile)
{
	FILE* mp3 = fopen(pFile->szFilename, "r");
	if(mp3)
	{
		ID3v1 id3tag;
		fseek(mp3, -128, SEEK_END);
		fread((void*)&id3tag, 128, 1, mp3);

		if(id3tag.tag[0] == 'T' &&
			id3tag.tag[1] == 'A' &&
			id3tag.tag[2] == 'G')
		{
			pFile->szAlbum = (char*)malloc(31);
			memcpy(pFile->szAlbum, &id3tag.album[0], 30);
			pFile->szAlbum[30] = 0;

			pFile->szInterpret = (char*)malloc(31);
			memcpy(pFile->szInterpret, &id3tag.artist[0], 30);
			pFile->szInterpret[30] = 0;

			pFile->szTitle = (char*)malloc(31);
			memcpy(pFile->szTitle, &id3tag.title[0], 30);
			pFile->szTitle[30] = 0;
		}

		fclose(mp3);
	}
}

void UpdateMp3InfoWithID3v2Tags(MP3FILE *pFile)
{
	FILE* f = fopen(pFile->szFilename, "r");
	if(f)
	{
		BYTE buffer[50000];

		fread(&buffer, 3, 1, f);
		if(buffer[0] != 'I' &&
			buffer[1] != 'D' &&
			buffer[2] != '3') return;

		fseek(f, 6, 0);
		fread(&buffer, 4, 1, f);
		int iTagSize = (int)(buffer[0] << 24) + (int)(buffer[1] << 16) + (int)(buffer[2] << 8) + (int)(buffer[3] << 0);
		
		fseek(f, 10, 0);
		fread(&buffer, iTagSize, 1, f);

		BYTE *tag = &buffer[0];
		while(tag < &buffer[iTagSize])
		{
			char *frame = (char*)tag;
			BYTE *size = (BYTE*)&tag[4];
			int framesize = (int)(size[0] << 24) + (int)(size[1] << 16) + (int)(size[2] << 8) + (int)(size[3] << 0);
			if(framesize == 0) break;

			if(frame[0] == 'T')
			{
				BYTE *encoded = (BYTE*)(size+6);

				char temp[255];
				memset((void*)&temp[0], 0, 255);
				if(*encoded == 0)
				{
					memcpy((void*)&temp[0], (void*)(encoded+1), framesize - 1);
				}
				else if(*encoded == 1)
				{
					wchar_t wtemp[255];
					memset((void*)&wtemp[0], 0, 255);
					memcpy((void*)&wtemp[0], (void*)(encoded+3), framesize - 1);
					WideCharToMultiByte(CP_ACP, 0, wtemp, -1, temp, 255, NULL, NULL);
				}

				if(!strcmp(frame, "TALB"))
				{
					pFile->szAlbum = (char*)malloc(strlen(temp)+1);
					sprintf(pFile->szAlbum, "%s", &temp[0]);
				}
				else if(!strcmp(frame, "TPE1"))
				{
					pFile->szInterpret = (char*)malloc(strlen(temp)+1);
					sprintf(pFile->szInterpret, "%s", &temp[0]);
				}
				else if(!strcmp(frame, "TIT2"))
				{
					pFile->szTitle = (char*)malloc(strlen(temp)+1);
					sprintf(pFile->szTitle, "%s", &temp[0]);
				}
			}
			tag += 10+framesize;
		}

		fclose(f);
	}
}

void Log(char *szFormat, ...)
{
	char chBuffer[256];

	va_list v;
	va_start(v, szFormat);
	_vsnprintf(chBuffer, 255, szFormat, v);
	chBuffer[255] = '\0';

	FILE *log = fopen("mp3control.log", "a");
	if(log)
	{
		fprintf(log, "%s\n", chBuffer);
		fclose(log);
	}
}

void PatchCode(DWORD dwAddress, DWORD dwNumberOfBytes, void *patch)
{
	DWORD dwProt;
	VirtualProtect((void*)dwAddress, dwNumberOfBytes, PAGE_EXECUTE_READWRITE, &dwProt);
	memcpy((void*)dwAddress, patch, dwNumberOfBytes);
	VirtualProtect((void*)dwAddress, dwNumberOfBytes, dwProt, &dwProt);
}

void ChangeFunctionCall(DWORD dwAddress, DWORD dwCallTarget, DWORD dwNopBytes)
{
	DWORD dwProt;
	VirtualProtect((void*)dwAddress, 5, PAGE_EXECUTE_READWRITE, &dwProt);
	*(BYTE*)dwAddress = 0xE8;
	*(DWORD*)(dwAddress+1) = dwCallTarget-dwAddress-0x5;
	memset((void*)(dwAddress+5), 0x90, dwNopBytes);
	VirtualProtect((void*)dwAddress, 5, dwProt, &dwProt);
}

void ReadConfig()
{
	FILE* config = fopen("mp3control.cfg", "r");
	if(config)
	{
		char szLine[500];
		while(fgets(szLine, 500, config))
		{
			szLine[strlen(szLine)-1] = 0;
			if(szLine[0] != '#' && szLine[0] != 0)
			{
				char* tok1 = strtok(szLine, " ");
								
				if(tok1[0] == 'K') 
				{
					tok1 += 2;

					char* tok2 = strtok(NULL, " ");

					if(!strcmp(tok1, "NEXT_TRACK"))
						gKeys.iNext = atoi(tok2);

					if(!strcmp(tok1, "PREVIOUS_TRACK"))
						gKeys.iPrevious = atoi(tok2);
					
					if(!strcmp(tok1, "TOGGLE_SHUFFLE"))
						gKeys.iToggleShuffle = atoi(tok2);

					if(!strcmp(tok1, "VOLUME_UP"))
						gKeys.iVolumeUp = atoi(tok2);

					if(!strcmp(tok1, "VOLUME_DOWN"))
						gKeys.iVolumeDown = atoi(tok2);

					if(!strcmp(tok1, "BROWSE_LIST"))
						gKeys.iToggleList = atoi(tok2);

					if(!strcmp(tok1, "LIST_SCROLL_UP"))
						gKeys.iListScrollUp = atoi(tok2);

					if(!strcmp(tok1, "LIST_SCROLL_DOWN"))
						gKeys.iListScrollDown = atoi(tok2);

					if(!strcmp(tok1, "LIST_CHOOSE"))
						gKeys.iListChoose = atoi(tok2);
				}
				else
				{
					char* playlist = strtok(NULL, "\"");
					strtok(NULL, "\"");
					char* name = strtok(NULL, "\"");
					char* showtitle = strtok(NULL, " ");
					char* fadeout = strtok(NULL, " ");
					char* shuffle = strtok(NULL, " ");

					if(giStationCount < 9)
					{
						gMp3Stations[giStationCount].szName = (char*)malloc(strlen(name));
						strcpy(gMp3Stations[giStationCount].szName, name);
						gMp3Stations[giStationCount].szPlaylist = (char*)malloc(strlen(playlist));
						strcpy(gMp3Stations[giStationCount].szPlaylist, playlist);
						gMp3Stations[giStationCount].bShowTitle = atoi(showtitle);
						gMp3Stations[giStationCount].iFade = atoi(fadeout);
						gMp3Stations[giStationCount].bShuffle = atoi(shuffle);

						giStationCount++;
					}
				}
			}
		}
		fclose(config);
	}
}

void ChangeMp3Station(int iStation)
{
	if(giCurMp3Station != iStation && iStation < giStationCount)
	{
		DWORD dwTotalTime;
		if(*pCurrentStream)
		{
			pSound->StreamMsPosition(*pCurrentStream, &dwTotalTime, 0);
			pSound->SetStreamMsPosition(*pCurrentStream, dwTotalTime-10);
		}

		gMp3Stations[giCurMp3Station].iCurrentTrack = *dwCurrentTrack;

		*dwMp3Files = (DWORD)gMp3Stations[iStation].mp3Start;
		*dwTrackCount = gMp3Stations[iStation].iTrackCount;
		*dwCurrentTrack = gMp3Stations[iStation].iCurrentTrack;
		giListCurrentItem = gMp3Stations[iStation].iCurrentTrack;

		if(gMp3Stations[iStation].iFade > 0)
		{
			giFade = gMp3Stations[iStation].iFade;
		}

		giCurMp3Station = iStation;

		gbStationChanged = true;
	}
}

void HandleKeyPress(int bKey)
{
	/*switch(bKey)
	{
	case VK_F9:
		*(DWORD*)(0x9B48F0) = 1;
		break;
	}*/

	if(*bIsMp3Active == 0) return;
	
	giSkipping = 0;

	if(bKey == gKeys.iToggleList)
	{
		gbList = !gbList;
		*bDisableKeyboard2 = gbList;

		if(gbList)
		{
			gfListAlpha = 255.0f;
			gfListFade = 0.0f;
		}
	}
	else if(gbList && bKey == gKeys.iListChoose)
	{
		MP3FILE* temp = (MP3FILE*)*dwMp3Files;
		int i = 0;
		while (temp && i != giListCurrentItem)
		{
			temp = (MP3FILE*)temp->dwNextFile;
			i++;
		}
		*dwCurrentTrack = i;

		DWORD dwTotalTime;
		pSound->StreamMsPosition(*pCurrentStream, &dwTotalTime, 0);
		pSound->SetStreamMsPosition(*pCurrentStream, dwTotalTime-10);

		gbStationChanged = true;

		gfListFade = -20.0f;
	}
	else if(gbList && bKey == gKeys.iListScrollUp)
	{
		if(giListCurrentItem > 0) giListCurrentItem -= 1;
	}
	else if(gbList && bKey == gKeys.iListScrollDown)
	{
		if(giListCurrentItem < *dwTrackCount - 1) giListCurrentItem += 1;
	}
	else if(bKey >= 0x31 && bKey <= 0x39)
	{
		ChangeMp3Station(bKey - 0x31);
	}
	else if(bKey == gKeys.iPrevious)
	{
		DWORD dwTotalTime;
		pSound->StreamMsPosition(*pCurrentStream, &dwTotalTime, 0);
		pSound->SetStreamMsPosition(*pCurrentStream, dwTotalTime-10);

		giSkipping = -1;
	}
	else if(bKey == gKeys.iNext)
	{
		DWORD dwTotalTime;
		pSound->StreamMsPosition(*pCurrentStream, &dwTotalTime, 0);
		pSound->SetStreamMsPosition(*pCurrentStream, dwTotalTime-10);
		
		giSkipping = +1;
	}
	else if(bKey == gKeys.iToggleShuffle)
	{
		gMp3Stations[giCurMp3Station].bShuffle = !gMp3Stations[giCurMp3Station].bShuffle;
	}
	else if(bKey == gKeys.iVolumeDown)
	{
		if(*pbMusicVolume < 65)
		{
			*pbMusicVolume = *pbMusicVolume + 1;
		}
	}
	else if(bKey == gKeys.iVolumeDown)
	{
		if(*pbMusicVolume > 0)
		{
			*pbMusicVolume = *pbMusicVolume - 1;
		}
	}
}

void InjectPlaylist(int iMp3Station)
{
	Log("Mp3 Control: Reading playlist %i", iMp3Station);

	FILE* f = fopen(gMp3Stations[iMp3Station].szPlaylist, "r");
	if(f)
	{
		MP3FILE* previous = NULL;

		char* line = (char*)malloc(260);
		while(fgets(line, 260, f))
		{
			line[strlen(line)-1] = 0;
			if(line[0] != '#' && line[0] != 0)
			{
				DWORD dwStream;

				MP3FILE* temp = (MP3FILE*)malloc(sizeof(MP3FILE));
				strcpy(temp->szFilename, line);
				temp->dwNextFile = 0;
				temp->dwUnknown1 = 0;
				temp->dwUnknown2 = 0;
				
				temp->dwPreviousFile = 0;
				temp->szAlbum = NULL;
				temp->szInterpret = NULL;
				temp->szTitle = NULL;

				dwStream = pSound->OpenStream(*pHDIGDRIVER, &temp->szFilename[0], 0);
				if(dwStream)
				{
					UpdateMp3InfoWithID3v2Tags(temp);
					if(temp->szTitle == NULL)
					{
						UpdateMp3InfoWidthID3v1Tags(temp);
						if(temp->szTitle == NULL)
						{
							char *szTemp = temp->szFilename;
							szTemp += strlen(temp->szFilename);
							while(szTemp != temp->szFilename && *szTemp != '\\')
							{
								szTemp--;
							}
							szTemp++;
						
							temp->szTitle = (char*)malloc(temp->szFilename + strlen(temp->szFilename) - szTemp + 1);
							sprintf(temp->szTitle, "%s", szTemp);
						}
					}
	
					pSound->StreamMsPosition(dwStream, &temp->dwTrackLength, 0);
					pSound->CloseStream(dwStream);

					if(gMp3Stations[iMp3Station].mp3Start == NULL)
					{
						gMp3Stations[iMp3Station].mp3Start = temp;
					}
				
					gMp3Stations[iMp3Station].iTrackCount += 1;
				
					if(previous)
					{
						temp->dwPreviousFile = (DWORD)previous;
						previous->dwNextFile = (DWORD)temp;
					}
					previous = temp;
				}
				else
				{
					Log("Mp3 Control: Not supported file: %s", &temp->szFilename[0]);
				}
			}
		}
		free(line);

		fclose(f);

		if(gMp3Stations[iMp3Station].iTrackCount > 0)
		{
			Log("Mp3 Control: Found %i mp3 files", gMp3Stations[iMp3Station].iTrackCount);
		}
		else
		{
			Log("Mp3 Control: No files found. Ignoring station.");
		}
	}
	else
	{
		Log("Mp3 Control: Error! Couldn't open '%s'", gMp3Stations[iMp3Station].szPlaylist);
	}
}

void GetMp3Files()
{
	*dwMp3Files = NULL;
	*dwTrackCount = 0;
	*dwCurrentTrack = 0;

	Log("Mp3 Control: Reading playlists");

	for(int i = 0; i < giStationCount; i++)
	{
		InjectPlaylist(i);
	}

	*dwMp3Files = (DWORD)gMp3Stations[0].mp3Start;
	*dwTrackCount = gMp3Stations[0].iTrackCount;
}

void _declspec(naked) NextTrack()
{
	__asm push eax
	
	if(!gbStationChanged)
	{
		if(gMp3Stations[giCurMp3Station].bShuffle)
		{
			*dwCurrentTrack = rand()%(*dwTrackCount);
		}
		else
		{
			*dwCurrentTrack = *dwCurrentTrack + giSkipping;
			if(*dwCurrentTrack == -1) *dwCurrentTrack = *dwTrackCount - 1;
			else if(*dwCurrentTrack == *dwTrackCount) *dwCurrentTrack = 0;
		}

		gMp3Stations[giCurMp3Station].iCurrentTrack = *dwCurrentTrack;
	}
	else gbStationChanged = false;

	if(gMp3Stations[giCurMp3Station].iFade > 0)
	{
		giFade = gMp3Stations[giCurMp3Station].iFade;
	}

	__asm pop eax
	__asm ret
}


MP3FILE* GetMp3Track(int iNumber)
{
	MP3FILE *temp = (MP3FILE*)*dwMp3Files;

	int i = 0;
	while(i < iNumber && temp)
	{
		temp = (MP3FILE*)temp->dwNextFile; 
		i++;
	}

	return temp;
}

void RenderList()
{
	float fScale = 0.6f;
	
	gFont->SetJustifyOff();
	gFont->SetBackGroundOff();
	gFont->SetBackgroundColor(0xBB000000);
	gFont->SetPropOn();
	gFont->SetDropShadowPosition(2);
	gFont->SetDropColor(0xFF000000);
	gFont->SetFontStyle(1);
	gFont->SetCentreOn();
	
	gFont->SetScale(fScale, fScale * 1.6f);
	gFont->SetColor(0xFFF7C261);

	MP3FILE *selected = GetMp3Track(giListCurrentItem);

	if(selected->szInterpret == NULL)
	{
		gFont->PrintString(320.0f, 240.0f, "%s", selected->szTitle);
	}
	else
	{
		gFont->PrintString(320.0f, 240.0f, "%s - %s", selected->szInterpret, selected->szTitle);
	}
	gFont->PrintString(320.0f, 400.0f, "%i/%i", giListCurrentItem + 1, *dwTrackCount);


	int iTopCount = 0;
	int iBottomCount = 0;

	if(*dwTrackCount < 7)
	{
		iTopCount = giListCurrentItem;
		iBottomCount = *dwTrackCount - iTopCount - 1;
	}
	else if(giListCurrentItem < 3)
	{
		iTopCount = giListCurrentItem;
		iBottomCount = 3;
	}
	else if(giListCurrentItem > *dwTrackCount - 4)
	{
		iTopCount = 3;
		iBottomCount = *dwTrackCount - giListCurrentItem - 1;
	}
	else
	{
		iTopCount = 3;
		iBottomCount = 3;
	}

	float fPosY = 220.0f;
	MP3FILE *temp = GetMp3Track(giListCurrentItem - 1);
	fScale = 0.4f;

	for(int i = 0; i < iTopCount; i++)
	{
		gFont->SetScale(fScale, fScale * 1.6f);
		gFont->SetColor(0xFFFFFFFF);
		gFont->SetDropColor(0xFF000000);
		if(temp->szInterpret == NULL)
		{
			gFont->PrintString(320.0f, fPosY, "%s", temp->szTitle);
		}
		else
		{
			gFont->PrintString(320.0f, fPosY, "%s - %s", temp->szInterpret, temp->szTitle);
		}
		fScale -= 0.05f;
		fPosY = fPosY - 15.0f;
		temp = (MP3FILE*)temp->dwPreviousFile;
	}

	fPosY = 260.0f;
	temp = GetMp3Track(giListCurrentItem + 1);
	fScale = 0.4f;
	for(int i = 0; i < iBottomCount; i++)
	{
		gFont->SetCentreOn();
		gFont->SetScale(fScale, fScale * 1.6f);
		gFont->SetColor(0xFFFFFFFF);
		gFont->SetDropColor(0xFF000000);
		if(temp->szInterpret == NULL)
		{
			gFont->PrintString(320.0f, fPosY, "%s", temp->szTitle, temp);
		}
		else
		{
			gFont->PrintString(320.0f, fPosY, "%s - %s", temp->szInterpret, temp->szTitle);
		}
		fScale -= 0.05f;
		fPosY = fPosY + 15.0f;
		temp = (MP3FILE*)temp->dwNextFile;
	}
}

void DisplayMp3Station()
{
	gFont->SetScale(0.5f, 0.89f);
	gFont->SetJustifyOff();
	gFont->SetBackGroundOff();
	gFont->SetPropOn();
	gFont->SetDropShadowPosition(1);
	gFont->SetDropColor(0xFF000000);
	gFont->SetCentreOff();

	gFont->SetFontStyle(2);
	gFont->SetColor(0xFFF7C261);
	gFont->PrintString(10.0f, 22.0f, "%s", gMp3Stations[giCurMp3Station].szName);

	MP3FILE* pFile = (MP3FILE*)*dwMp3Files;
	int i = 0;
	while(i < *dwCurrentTrack && pFile)
	{
		pFile = (MP3FILE*)(pFile->dwNextFile);
		i++;
	}

	gFont->SetFontStyle(1);

	if(pFile->szInterpret)
	{
		gFont->SetColor(0xEEF7C261);
		gFont->PrintString(10.0f, 50.0f, "%s", pFile->szInterpret);
	}
	gFont->SetScale(0.4f, 0.71f);
	gFont->SetColor(0xDDF7C261);
	if(pFile->szAlbum)
	{
		gFont->PrintString(10.0f, 66.0f, "%s", pFile->szAlbum);
	}
	gFont->PrintString(10.0f, 78.0f, "%s", pFile->szTitle);

	DWORD iTotalPos, iCurPos;

	pSound->StreamMsPosition(*pCurrentStream, &iTotalPos, &iCurPos);

	iCurPos = (DWORD)(iCurPos / 1000);
	iTotalPos = (DWORD)(iTotalPos / 1000);

	DWORD iCurPosMin = (DWORD)(iCurPos / 60);
	DWORD iCurPosSec = iCurPos - (iCurPosMin * 60);
	DWORD iTotalPosMin = (DWORD)(iTotalPos / 60);
	DWORD iTotalPosSec = iTotalPos - (iTotalPosMin * 60);

	gFont->SetColor(0xAAF7C261);
	gFont->SetScale(0.3f, 0.53f);
	gFont->PrintString(10.0f, 90.0f, "%i/%i %i:%02i/%i:%02i", 
			*dwCurrentTrack + 1, 
			*dwTrackCount, 
			iCurPosMin,
			iCurPosSec,
			iTotalPosMin,
			iTotalPosSec
		);

	return;
}

void Display()
{
	if(gbList)
	{
		if(gfListAlpha > 50.0f)
		{
			gFont->SetAlphaFade(gfListAlpha);
			RenderList();
			gfListAlpha += gfListFade;
		}
		else
		{
			gbList = false;
			*bDisableKeyboard2 = gbList;
		}
	}

	gFont->SetAlphaFade(255.0f);

	if(gMp3Stations[giCurMp3Station].iFade > 0)
	{
		if(giFade == 0)
		{
			gFont->DrawFonts();
			return;
		}
		giFade -= 1;

		gFont->SetAlphaFade(giFade * 50.0f);
	}

	DisplayMp3Station();

	gFont->SetAlphaFade(255.0f);

	gFont->DrawFonts();
}

void DeleteStations()
{
	for(int i = 0; i < giStationCount; i++)
	{
		free(gMp3Stations[i].szName);
		free(gMp3Stations[i].szPlaylist);

		MP3FILE* temp = gMp3Stations[i].mp3Start;
		while(temp != 0)
		{
			MP3FILE* next = (MP3FILE*)temp->dwNextFile;
			
			free(temp->szAlbum);
			free(temp->szInterpret);
			free(temp->szTitle);
			free(temp);
			
			temp = next;
		}
	}
}

LRESULT APIENTRY NewWndProc( HWND hwnd,UINT uMsg,
							 WPARAM wParam,LPARAM lParam ) 
{ 
	switch(uMsg) {
		case WM_KEYDOWN:
			HandleKeyPress((int)wParam);
			break;
	}
	return CallWindowProc(hOldWndProc, *hGameWnd, uMsg, wParam, lParam);
}

void Initialization()
{
	gFont = new CFont();
	InitGameFont(&gFont->mpFont);

	pSound = new GameSound;
	InitGameSound(pSound);

	ReadConfig();

	FILE *log = fopen("mp3control.log", "w");
	if(log) fclose(log);

	if(giStationCount > 0)
	{
		giCurMp3Station = 0;

		Log("Mp3 Control: Found %i stations", giStationCount);

		// Hook into mp3 processing
		ChangeFunctionCall(0x5D7EA4, (DWORD)&GetMp3Files, 0);
		ChangeFunctionCall(0x5D604F, (DWORD)&NextTrack, 1);
		ChangeFunctionCall(0x5D7340, (DWORD)&DeleteStations, 0);

		// Text drawing
		ChangeFunctionCall(0x5FA0D8, (DWORD)&Display, 0);
		BYTE jmp[] = { 0xEB, 0x15 };
		PatchCode(0x5FA0DD, 2, (void*)jmp);

		// Hook Window proc
		hOldWndProc = (WNDPROC)GetWindowLong(*hGameWnd, GWL_WNDPROC);
		SetWindowLong(*hGameWnd, GWL_WNDPROC, (LONG)NewWndProc);
	}
	else
	{
		Log("Mp3 Control: No stations found, exiting.");
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReason,
                       LPVOID lpReserved
					 )
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls((HMODULE) hModule);

			Initialization();
		}
	
		case DLL_PROCESS_DETACH:
		{
		}
	}

    return TRUE;
}