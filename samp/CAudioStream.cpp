/*
	This is a SAMP (0.3.7-R1) API project file.
	Developer: LUCHARE <luchare.dev@gmail.com>
	
	See more here https://github.com/LUCHARE/SAMP-API
	
	Copyright (c) 2018 BlastHack Team <BlastHack.Net>. All rights reserved.
*/

#include "CAudioStream.h"

int &SAMP::CAudioStream::hStream = *(int *)SAMP_ADDROF(0x11A60C);
bool &SAMP::CAudioStream::bNeedsToDestroy = *(bool *)SAMP_ADDROF(0xF03AA);
SAMP::CVector &SAMP::CAudioStream::position = *(CVector *)SAMP_ADDROF(0x11A614);
bool &SAMP::CAudioStream::bIsPlaying = *(bool *)SAMP_ADDROF(0x11A610);
char *SAMP::CAudioStream::szUrl = (char *)SAMP_ADDROF(0x11A2F8);
bool &SAMP::CAudioStream::bIs3d = *(bool *)SAMP_ADDROF(0x11A620);
SAMP::CAudioStream *&SAMP::pAudioStream = *(SAMP::CAudioStream **)SAMP_ADDROF(0x21A0F0);
char *SAMP::CAudioStream::szInfo = (char *)SAMP_ADDROF(0x11A400);
char *SAMP::CAudioStream::szIcyName = (char *)SAMP_ADDROF(0x11A1F0);
char *SAMP::CAudioStream::szIcyUrl = (char *)SAMP_ADDROF(0x11A508);
float &SAMP::CAudioStream::fRadius = *(float *)SAMP_ADDROF(0xF03AC);

BOOL SAMP::CAudioStream::Reset() {
	return ((BOOL(__thiscall *)(CAudioStream *))SAMP_ADDROF(0x628C0))(this);
}

BOOL SAMP::CAudioStream::Stop(bool bWait) {
	return ((BOOL(__thiscall *)(CAudioStream *, bool))SAMP_ADDROF(0x629A0))(this, bWait);
}

void SAMP::CAudioStream::ConstructInfo() {
	((void(__cdecl *)())SAMP_ADDROF(0x62A00))();
}

void SAMP::CAudioStream::SyncProc(int handle, int channel, int data, void *user) {
	((void(__stdcall *)(int, int, int, void *))SAMP_ADDROF(0x62B30))(handle, channel, data, user);
}

void SAMP::CAudioStream::Process(void *arglist) {
	((void(__cdecl *)(void *))SAMP_ADDROF(0x62B40))(arglist);
}

BOOL SAMP::CAudioStream::Play(const char *szUrl, CVector position, float fRadius, bool bIs3d) {
	return ((BOOL(__thiscall *)(CAudioStream *, const char *, CVector, float, bool))SAMP_ADDROF(0x62DA0))(this, szUrl, position, fRadius, bIs3d);
}

void SAMP::CAudioStream::ControlGameRadio() {
	((void(__thiscall *)(CAudioStream *))SAMP_ADDROF(0x62EC0))(this);
}

void SAMP::CAudioStream::DrawInfo() {
	((void(__thiscall *)(CAudioStream *))SAMP_ADDROF(0x62EF0))(this);
}