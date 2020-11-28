#ifndef __MAIN__
#define __MAIN__

#pragma warning(disable: 4238 4244 4305)

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <queue>
#include <assert.h>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include "resource.h"

#include "bass.h"
#pragma comment(lib, "bass.lib")

#include "hook/MinHook.h"
#pragma comment(lib, "hook/libMinHook.x86.lib")

#include "d3drender.h"
#include "keyhook.h"

#include "samp/CChat.h"
#include "samp/CDialog.h"
#include "samp/CInput.h"
#include "samp/CNetGame.h"

#include "jansson/jansson.h"

#include "STTFile.hpp"

#include "INET.h"

#include "render.h"

#define VT_FUNC(vt, i)	((void**)(*(void***)(vt))[i])

using namespace std;

typedef void(__cdecl* hookedMainloop_t)(void);

struct ___globals
{
	HMODULE				selfModule = NULL;
	WNDPROC				orig_wndproc = NULL;
	IDirect3DDevice9*	&pD3DDevice = *reinterpret_cast<IDirect3DDevice9**>(0xC97C28);
	HWND				&hwnd = *reinterpret_cast<HWND*>(0xC97C1C);
	uint32_t			&gameState = *reinterpret_cast<uint32_t*>(0xC8D4C0);
	bool				&isMenuOpened = *reinterpret_cast<bool*>(0xBA67A4);
};

extern struct ___globals G;

enum
{
	STATE_NOTAVAIL,
	STATE_ALREADY,
	STATE_RECORDINIT,
	STATE_RECORDING,
	STATE_REQUESTING,
};

struct CHAT_PROFILE
{
	int		keys[2] = {0, 0};
	string	format;
	CHAT_PROFILE() {};
	CHAT_PROFILE(const char* fmt, int key1, int key2 = NULL)
	{
		format = fmt;
		keys[0] = key1;
		keys[1] = key2;
	};
};

struct PLUGIN_DATA
{
	// TEMPORARY
	DWORD dwPluginState = STATE_NOTAVAIL;
	uint32_t iPresetID = 0; // = GENERIC

	// main
	vector<CHAT_PROFILE> presets = {
		CHAT_PROFILE("%s", VK_F2),
		CHAT_PROFILE("/b %s", 0x42),
		CHAT_PROFILE("/r %s", 0x50),
		CHAT_PROFILE("/rb %s", VK_LCONTROL, 0x50),
		CHAT_PROFILE("/s %s", 0x4F),
		CHAT_PROFILE("/me %s", 0x4D),
		CHAT_PROFILE("/do %s", VK_LCONTROL, 0x4D)
	};

	// AUDIO
	int	sampleRate	= 16000;
	int bitRate		= 16;
	int numChannels	= 1;

	// render
	int32_t	pos_x = 6;
	int32_t	pos_y = 236;

	// reco settings
	string hostname = "f0446239.xsph.ru";
	string url		= "crc-api/stt.php";
	string used_api	= "default";
	string languageCode = "ru-RU";

	
	//bool animations = true;
	bool saveLastRecord = false;
};

extern PLUGIN_DATA ps;

void load_config(const char* filename);
void save_config(const char* filename);

void SAMP_AddChatMessage(D3DCOLOR color, const char *format, ...);
void SAMP_SendChat(const char* text);

#endif