#include "font.h"

void InitGameFont(GameFont *pFont)
{
	pFont->SetDropShadowPosition = (p_SetDropShadowPosition)ADDR_CFONT_SETDROPSHADOWPOSITION;
	pFont->SetDropColor = (p_SetDropColor)ADDR_CFONT_SETDROPCOLOR;
	pFont->SetAlphaFade = (p_SetAlphaFade)ADDR_CFONT_SETALPHAFADE;
	pFont->SetRightJustifyWrap = (p_SetRightJustifyWrap)ADDR_CFONT_SETRIGHTJUSTIFYWRAP;
	pFont->SetFontStyle = (p_SetFontStyle)ADDR_CFONT_SETFONTSTYLE;
	pFont->SetPropOn = (p_SetPropOn)ADDR_CFONT_SETPROPON;
	pFont->SetPropOff = (p_SetPropOff)ADDR_CFONT_SETPROPOFF;
	pFont->SetRightJustifyOff = (p_SetRightJustifyOff)ADDR_CFONT_SETRIGHTJUSTIFYOFF;
	pFont->SetRightJustifyOn = (p_SetRightJustifyOn)ADDR_CFONT_SETRIGHTJUSTIFYON;
	pFont->SetBackGroundOnlyTextOff = (p_SetBackGroundOnlyTextOff)ADDR_CFONT_SETBACKGROUNDONLYTEXTOFF;
	pFont->SetBackGroundOnlyTextOn = (p_SetBackGroundOnlyTextOn)ADDR_CFONT_SETBACKGROUNDONLYTEXTON;
	pFont->SetBackgroundColor = (p_SetBackgroundColor)ADDR_CFONT_SETBACKGROUNDCOLOR;
	pFont->SetBackGroundOff = (p_SetBackGroundOff)ADDR_CFONT_SETBACKGROUNDOFF;
	pFont->SetBackGroundOn = (p_SetBackGroundOn)ADDR_CFONT_SETBACKGROUNDON;
	pFont->SetCentreSize = (p_SetCentreSize)ADDR_CFONT_SETCENTRESIZE;
	pFont->SetWrapX = (p_SetWrapX)ADDR_CFONT_SETWRAPX;
	pFont->SetCentreOff = (p_SetCentreOff)ADDR_CFONT_SETCENTREOFF;
	pFont->SetCentreOn = (p_SetCentreOn)ADDR_CFONT_SETCENTREON;
	pFont->SetJustifyOff = (p_SetJustifyOff)ADDR_CFONT_SETJUSTIFYOFF;
	pFont->SetJustifyOn = (p_SetJustifyOn)ADDR_CFONT_SETJUSTIFYON;
	pFont->SetColor = (p_SetColor)ADDR_CFONT_SETCOLOR;
	pFont->SetScale = (p_SetScale)ADDR_CFONT_SETSCALE;
	pFont->PrintString = (p_PrintString)ADDR_CFONT_PRINTSTRING;
	pFont->AsciiToUnicode = (p_AsciiToUnicode)ADDR_ASCIITOUNICODE;
	pFont->DrawFonts = (p_DrawFonts)ADDR_CFONT_DRAWFONTS;

}

void CFont::SetDropShadowPosition(short iPosition)
{
	this->mpFont.SetDropShadowPosition(iPosition);
}

void CFont::SetDropColor(RGBA color)
{
	this->mpFont.SetDropColor(&color);
}

void CFont::SetAlphaFade(float fFade)
{
	this->mpFont.SetAlphaFade(fFade);
}

void CFont::SetRightJustifyWrap(float fWrap)
{
	this->mpFont.SetRightJustifyWrap(fWrap);
}

void CFont::SetFontStyle(int iStyle)
{
	this->mpFont.SetFontStyle(iStyle);
}

void CFont::SetPropOn()
{
	this->mpFont.SetPropOn();
}

void CFont::SetPropOff()
{
	this->mpFont.SetPropOff();
}

void CFont::SetRightJustifyOff()
{
	this->mpFont.SetRightJustifyOff();
}

void CFont::SetRightJustifyOn()
{
	this->mpFont.SetRightJustifyOn();
}

void CFont::SetBackGroundOnlyTextOff()
{
	this->mpFont.SetBackGroundOnlyTextOff();
}

void CFont::SetBackGroundOnlyTextOn()
{
	this->mpFont.SetBackGroundOnlyTextOn();
}

void CFont::SetBackgroundColor(RGBA color)
{
	this->mpFont.SetBackgroundColor(&color);
}

void CFont::SetBackGroundOff()
{
	this->mpFont.SetBackGroundOff();
}

void CFont::SetBackGroundOn()
{
	this->mpFont.SetBackGroundOn();
}

void CFont::SetCentreSize(float fSize)
{
	this->mpFont.SetCentreSize(fSize);
}

void CFont::SetWrapX(float fWrap)
{
	this->mpFont.SetWrapX(fWrap);
}

void CFont::SetCentreOff()
{
	this->mpFont.SetCentreOff();
}

void CFont::SetCentreOn()
{
	this->mpFont.SetCentreOn();
}

void CFont::SetJustifyOff()
{
	this->mpFont.SetJustifyOff();
}

void CFont::SetJustifyOn()
{
	this->mpFont.SetJustifyOn();
}

void CFont::SetColor(RGBA color)
{
	this->mpFont.SetColor(&color);
}

void CFont::SetScale(float fX, float fY)
{
	float fScaleX, fScaleY;
	fScaleX = (float)*(DWORD*)SCREENWIDTH;
	fScaleX *= SCREENWIDTH_SCALEFACTOR;
	fScaleX *= fX;

	fScaleY = (float)*(DWORD*)SCREENHEIGHT;
	fScaleY *= SCREENHEIGHT_SCALEFACTOR;
	fScaleY *= fY;

	this->mpFont.SetScale(fScaleX, fScaleY);
}

void CFont::PrintString(float fPosX, float fPosY, char *szFormat, ...)
{
	char szBuffer[250];
	unsigned short int szUnicode[500];

	va_list v;
	va_start(v, szFormat);
	vsnprintf(szBuffer, 250, szFormat, v);
	szBuffer[249] = '\0';

	this->AsciiToUnicode(&szBuffer[0], szUnicode);

	float fScaledPosX, fScaledPosY;
	fScaledPosX = (float)*(DWORD*)SCREENWIDTH;
	fScaledPosX *= SCREENWIDTH_SCALEFACTOR;
	fScaledPosX *= fPosX;

	fScaledPosY = (float)*(DWORD*)SCREENHEIGHT;
	fScaledPosY *= SCREENHEIGHT_SCALEFACTOR;
	fScaledPosY *= fPosY;

	this->mpFont.PrintString(fScaledPosX, fScaledPosY, szUnicode);
}

void CFont::AsciiToUnicode(char *szSrc, unsigned short int *szDest)
{
	this->mpFont.AsciiToUnicode(szSrc, szDest);
}

void CFont::DrawFonts()
{
	this->mpFont.DrawFonts();
}
