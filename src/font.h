#pragma once

#include "main.h"
#include <stdio.h>
#include <windows.h>

#define ADDR_CFONT_SETDROPSHADOWPOSITION		0x54ff20
#define ADDR_CFONT_SETDROPCOLOR					0x54ff30
#define ADDR_CFONT_SETALPHAFADE					0x54ffc0
#define ADDR_CFONT_SETRIGHTJUSTIFYWRAP			0x54ffd0
#define ADDR_CFONT_SETFONTSTYLE					0x54ffe0
#define ADDR_CFONT_SETPROPON					0x550020
#define ADDR_CFONT_SETPROPOFF					0x550030
#define ADDR_CFONT_SETRIGHTJUSTIFYOFF			0x550040
#define ADDR_CFONT_SETRIGHTJUSTIFYON			0x550060
#define ADDR_CFONT_SETBACKGROUNDONLYTEXTOFF		0x550080
#define ADDR_CFONT_SETBACKGROUNDONLYTEXTON		0x550090
#define ADDR_CFONT_SETBACKGROUNDCOLOR			0x5500a0
#define ADDR_CFONT_SETBACKGROUNDOFF				0x5500d0
#define ADDR_CFONT_SETBACKGROUNDON				0x5500e0
#define ADDR_CFONT_SETCENTRESIZE				0x5500f0
#define ADDR_CFONT_SETWRAPX						0x550100
#define ADDR_CFONT_SETCENTREOFF					0x550110
#define ADDR_CFONT_SETCENTREON					0x550120
#define ADDR_CFONT_SETJUSTIFYOFF				0x550140
#define ADDR_CFONT_SETJUSTIFYON					0x550150
#define ADDR_CFONT_SETCOLOR						0x550170
#define ADDR_CFONT_SETSCALE						0x550230
#define ADDR_CFONT_PRINTSTRING					0x551040
#define ADDR_ASCIITOUNICODE						0x552500
#define ADDR_CFONT_DRAWFONTS					0x550250

#define OFF_FONTSCALEX							0x97F824
#define OFF_FONTSCALEY							0x97F828

typedef unsigned int RGBA;

typedef void (__cdecl * p_DrawFonts)(void);
typedef void (__cdecl * p_SetDropShadowPosition)(short);
typedef void (__cdecl * p_SetDropColor)(RGBA*);
typedef void (__cdecl * p_SetAlphaFade)(float);
typedef void (__cdecl * p_SetRightJustifyWrap)(float);
typedef void (__cdecl * p_SetFontStyle)(short);
typedef void (__cdecl * p_SetPropOn)(void);
typedef void (__cdecl * p_SetPropOff)(void);
typedef void (__cdecl * p_SetRightJustifyOff)(void);
typedef void (__cdecl * p_SetRightJustifyOn)(void);
typedef void (__cdecl * p_SetBackGroundOnlyTextOff)(void);
typedef void (__cdecl * p_SetBackGroundOnlyTextOn)(void);
typedef void (__cdecl * p_SetBackgroundColor)(RGBA*);
typedef void (__cdecl * p_SetBackGroundOff)(void);
typedef void (__cdecl * p_SetBackGroundOn)(void);
typedef void (__cdecl * p_SetCentreSize)(float);
typedef void (__cdecl * p_SetWrapX)(float);
typedef void (__cdecl * p_SetCentreOff)(void);
typedef void (__cdecl * p_SetCentreOn)(void);
typedef void (__cdecl * p_SetJustifyOff)(void);
typedef void (__cdecl * p_SetJustifyOn)(void);
typedef void (__cdecl * p_SetColor)(RGBA*);
typedef void (__cdecl * p_SetScale)(float, float);
typedef void (__cdecl * p_PrintString)(float,float,unsigned short int *);
typedef void (__cdecl * p_AsciiToUnicode)(char *, unsigned short int *);

typedef struct _GameFont
{
	p_SetDropShadowPosition	SetDropShadowPosition;
	p_SetDropColor			SetDropColor;
	p_SetAlphaFade			SetAlphaFade;
	p_SetRightJustifyWrap	SetRightJustifyWrap;
	p_SetFontStyle			SetFontStyle;
	p_SetPropOn				SetPropOn;
	p_SetPropOff			SetPropOff;
	p_SetRightJustifyOff	SetRightJustifyOff;
	p_SetRightJustifyOn		SetRightJustifyOn;
	p_SetBackGroundOnlyTextOff	SetBackGroundOnlyTextOff;
	p_SetBackGroundOnlyTextOn	SetBackGroundOnlyTextOn;
	p_SetBackgroundColor	SetBackgroundColor;
	p_SetBackGroundOff		SetBackGroundOff;
	p_SetBackGroundOn		SetBackGroundOn;
	p_SetCentreSize			SetCentreSize;
	p_SetWrapX				SetWrapX;
	p_SetCentreOff			SetCentreOff;
	p_SetCentreOn			SetCentreOn;
	p_SetJustifyOff			SetJustifyOff;
	p_SetJustifyOn			SetJustifyOn;
	p_SetColor				SetColor;
	p_SetScale				SetScale;
	p_PrintString			PrintString;
	p_AsciiToUnicode		AsciiToUnicode;
	p_DrawFonts				DrawFonts;

} GameFont;

class CFont
{
public:
	GameFont mpFont;

	void SetDropShadowPosition(short iPosition);
	void SetDropColor(RGBA color);
	void SetAlphaFade(float fFade);
	void SetRightJustifyWrap(float fWrap);
	void SetFontStyle(int iStyle);
	void SetPropOn();
	void SetPropOff();
	void SetRightJustifyOff();
	void SetRightJustifyOn();
	void SetBackGroundOnlyTextOff();
	void SetBackGroundOnlyTextOn();
	void SetBackgroundColor(RGBA color);
	void SetBackGroundOff();
	void SetBackGroundOn();
	void SetCentreSize(float fSize);
	void SetWrapX(float fWrap);
	void SetCentreOff();
	void SetCentreOn();
	void SetJustifyOff();
	void SetJustifyOn();
	void SetColor(RGBA color);
	void SetScale(float fX, float fY);
	void PrintString(float fPosX, float fPosY, char *szFormat, ...);
	void AsciiToUnicode(char *szSrc, unsigned short int *szDest);
	void DrawFonts();

	CFont() {};
	~CFont() {};
};

void InitGameFont(GameFont *pFont);