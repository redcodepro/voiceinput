#include "main.h"

extern class CSAMP *SAMP;

enum DialogStyle
{
	DIALOG_STYLE_MSGBOX = 0,
	DIALOG_STYLE_INPUT = 1,
	DIALOG_STYLE_LIST = 2,
	DIALOG_STYLE_PASSWORD = 3,
	DIALOG_STYLE_TABLIST = 4,
	DIALOG_STYLE_TABLIST_HEADERS = 5
};

enum Limits
{
	SAMP_MAX_ACTORS = 1000,
	SAMP_MAX_PLAYERS = 1004,
	SAMP_MAX_VEHICLES = 2000,
	SAMP_MAX_PICKUPS = 4096,
	SAMP_MAX_OBJECTS = 1000,
	SAMP_MAX_GANGZONES = 1024,
	SAMP_MAX_3DTEXTS = 2048,
	SAMP_MAX_TEXTDRAWS = 2048,
	SAMP_MAX_PLAYERTEXTDRAWS = 256,
	SAMP_MAX_CLIENTCMDS = 144,
	SAMP_MAX_MENUS = 128,
	SAMP_MAX_PLAYER_NAME = 24,
	SAMP_ALLOWED_PLAYER_NAME_LENGTH = 20
};

enum ChatMessageType
{
	CHAT_TYPE_NONE = 0,
	CHAT_TYPE_CHAT = 2,
	CHAT_TYPE_INFO = 4,
	CHAT_TYPE_DEBUG = 8
};

enum ChatDisplayMode
{
	CHAT_WINDOW_MODE_OFF = 0,
	CHAT_WINDOW_MODE_LIGHT = 1,
	CHAT_WINDOW_MODE_FULL = 2
};

#pragma pack( push, 1 )

struct stChatEntry
{
	uint32_t								SystemTime;
	char									szPrefix[28];
	char									szText[144];
	uint8_t									unknown[64];
	int										iType;			// 2 - text + prefix, 4 - text (server msg), 8 - text (debug)
	D3DCOLOR								clTextColor;
	D3DCOLOR								clPrefixColor;	// or textOnly colour
};

struct stFontRenderer //size = 0x20
{
	ID3DXFont								*m_pChatFont;
	ID3DXFont								*m_pLittleFont;
	ID3DXFont								*m_pChatShadowFont;
	ID3DXFont								*m_pLittleShadowFont;
	ID3DXFont								*m_pCarNumberFont;
	ID3DXSprite								*m_pTempSprite;
	IDirect3DDevice9						*m_pD3DDevice;
	char									*m_pszTextBuffer;
};

struct stChatInfo
{
	int										pagesize;
	void									*pUnk;
	int										iChatWindowMode;
	uint8_t									bTimestamps;
	uint32_t								iLogFileExist;
	char									logFilePathChatLog[MAX_PATH + 1];
	void									*pGameUI; // CDXUTDialog
	struct stDXUTEditBox					*pEditBackground; // CDXUTEditBox
	struct stDXUTScrollBar					*pDXUTScrollBar;
	D3DCOLOR								clTextColor;
	D3DCOLOR								clInfoColor;
	D3DCOLOR								clDebugColor;
	uint32_t								ulChatWindowBottom;
	stChatEntry								chatEntry[100];
	stFontRenderer							*m_pFontRenderer;
	ID3DXSprite								*m_pChatTextSprite;
	ID3DXSprite								*m_pSprite;
	IDirect3DDevice9						*m_pD3DDevice;
	int										m_iRenderMode; // 0 - Direct Mode (slow), 1 - Normal mode
	ID3DXRenderToSurface					*pID3DXRenderToSurface;
	IDirect3DTexture9						*m_pTexture;
	IDirect3DSurface9						*pSurface;
	D3DDISPLAYMODE							*pD3DDisplayMode;
	int										iUnk1[3];
	int										iUnk2; // smth related to drawing in direct mode
	int										m_iRedraw;
	int										m_nPrevScrollBarPosition;
	int										m_iFontSizeY;
	int										m_iTimestampWidth;
};

typedef void(__cdecl *CMDPROC) (PCHAR);
struct stInputInfo
{
	void									*pD3DDevice;
	void									*pDXUTDialog;
	stDXUTEditBox							*pDXUTEditBox;
	CMDPROC									pCMDs[SAMP_MAX_CLIENTCMDS];
	char									szCMDNames[SAMP_MAX_CLIENTCMDS][33];
	int										iCMDCount;
	int										iInputEnabled;
	char									szInputBuffer[129];
	char									szRecallBufffer[10][129];
	char									szCurrentBuffer[129];
	int										iCurrentRecall;
	int										iTotalRecalls;
	CMDPROC									pszDefaultCMD;
};

struct stDialogInfo
{
	void									*pVTBL;
	int										iTextPoxX;
	int										iTextPoxY;
	int										iTextSizeX;
	int										iTextSizeY;
	int										iBtnOffsetX;
	int										iBtnOffsetY;
	struct stDXUTDialog						*pDialog;
	struct stDXUTListBox					*pList;
	struct stDXUTEditBox					*pEditBox;
	int										iIsActive;
	int										iType;
	uint32_t								DialogID;
	char									*pText;
	uint32_t								font;
	uint32_t								font2;
	char									szCaption[64];
	uint8_t									byteUnknown;
	int										bServerside;
};

#pragma pack(pop)

class CSAMP
{
private:
	typedef void(__thiscall *AddChatMessage_t)(stChatInfo*, ChatMessageType, const char*, const char*, D3DCOLOR, D3DCOLOR);

	typedef void(__thiscall *AddClientCommand_t)(stInputInfo*, const char*, CMDPROC);
	typedef void(__thiscall *SendCommand_t)(stInputInfo*, const char*);
	typedef void(__thiscall *SendSay_t)(void*, const char*);

	int					SAMP_Version = 0;

	AddChatMessage_t	fAddChatMessage = nullptr;

	AddClientCommand_t	fAddClientCommand = nullptr;
	SendCommand_t		fSendCommand = nullptr;
	SendSay_t			fSendSay = nullptr;

	stChatInfo			*pChat = nullptr;
	stInputInfo			*pInput = nullptr;
	stDialogInfo		*pDialog = nullptr;
public:
	bool				Init();

	stChatInfo			*getChat(void) { return pChat; };
	stInputInfo			*getInput(void) { return pInput; };
	stDialogInfo		*getDialog(void) { return pDialog; };

	void				AddChatMessage(D3DCOLOR color, const char *format, ...);

	void				SendChat(const char *text);
	void				registerChatCommand(const char* command, CMDPROC callback);
};