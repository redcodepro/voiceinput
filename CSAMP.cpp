#include "main.h"

#define SAMP_CHAT_INFO_OFFSET_R1	0x21A0E4
#define SAMP_CHAT_INFO_OFFSET_R2	0
#define SAMP_CHAT_INFO_OFFSET_R3	0x26E8C8
#define SAMP_CHAT_INFO_OFFSET_R4	0x26E9F8

#define SAMP_INPUT_INFO_OFFSET_R1	0x21A0E8
#define SAMP_INPUT_INFO_OFFSET_R2	0
#define SAMP_INPUT_INFO_OFFSET_R3	0x26E8CC
#define SAMP_INPUT_INFO_OFFSET_R4	0x26E9FC

#define SAMP_DIALOG_INFO_OFFSET_R1	0x21A0B8
#define SAMP_DIALOG_INFO_OFFSET_R2	0x21A0C0
#define SAMP_DIALOG_INFO_OFFSET_R3	0x26E898
#define SAMP_DIALOG_INFO_OFFSET_R4	0x26E9C8

DWORD	dwSAMPAddr = 0;
CSAMP	*SAMP = nullptr;

template<typename T>
inline T samp_offset(uint32_t offset)
{
	return reinterpret_cast<T>(dwSAMPAddr + offset);
}

template<typename T>
inline T samp_ptr(uint32_t offset)
{
	return *reinterpret_cast<T*>(dwSAMPAddr + offset);
}

bool CSAMP::Init()
{
	if (SAMP_Version == 0)
	{
		if ((dwSAMPAddr = (DWORD)GetModuleHandle("samp.dll")) == NULL)
			return false;

		IMAGE_NT_HEADERS *ntheader = reinterpret_cast<IMAGE_NT_HEADERS*>(dwSAMPAddr + reinterpret_cast<IMAGE_DOS_HEADER*>(dwSAMPAddr)->e_lfanew);
		uintptr_t ep = ntheader->OptionalHeader.AddressOfEntryPoint;
		switch (ep)
		{
		case 0x31DF13:
			{
				SAMP_Version = 1;

				// Init functions
				fAddChatMessage = samp_offset<AddChatMessage_t>(0x64010);

				fAddClientCommand = samp_offset<AddClientCommand_t>(0x65AD0);
				fSendCommand = samp_offset<SendCommand_t>(0x65C60);
				fSendSay = samp_offset<SendSay_t>(0x57F0);
			}
			break;
		/*case 0x3195DD:
			{
				SAMP_Version = 2;

				// Init functions
				fAddChatMessage = samp_offset<AddChatMessage_t>(0);

				fAddClientCommand = samp_offset<AddClientCommand_t>(0);
				fSendCommand = samp_offset<SendCommand_t>(0x65D30);
				fSendSay = samp_offset<SendSay_t>(0x57E0);
			}
			break;*/
		case 0xCC4D0:
			{
				SAMP_Version = 3;

				// Init functions
				fAddChatMessage = samp_offset<AddChatMessage_t>(0x67460);

				fAddClientCommand = samp_offset<AddClientCommand_t>(0x69000);
				fSendCommand = samp_offset<SendCommand_t>(0x69190);
				fSendSay = samp_offset<SendSay_t>(0x5820);
			}
			break;
		case 0xCBCB0:
			{
				SAMP_Version = 4;

				// Init functions
				fAddChatMessage = samp_offset<AddChatMessage_t>(0x680B0);

				fAddClientCommand = samp_offset<AddClientCommand_t>(0x69730);
				fSendCommand = samp_offset<SendCommand_t>(0x698C0);
				fSendSay = samp_offset<SendSay_t>(0x5A00);
			}
			break;
		default:
			{
				SAMP_Version = 0;
				MessageBoxA(G.hwnd, "Unknown SA-MP version.", "VoiceInput.asi", MB_OK | MB_ICONERROR);
			}
			break;
		}
	}

	switch (SAMP_Version)
	{
	case 1:
		{
			pChat = samp_ptr<stChatInfo*>(SAMP_CHAT_INFO_OFFSET_R1);
			if (pChat == nullptr)
				return false;

			pInput = samp_ptr<stInputInfo*>(SAMP_INPUT_INFO_OFFSET_R1);
			if (pInput == nullptr)
				return false;

			pDialog = samp_ptr<stDialogInfo*>(SAMP_DIALOG_INFO_OFFSET_R1);
			if (pDialog == nullptr)
				return false;
		}
		break;
	case 2:
		{
			pChat = samp_ptr<stChatInfo*>(SAMP_CHAT_INFO_OFFSET_R2);
			if (pChat == nullptr)
				return false;

			pInput = samp_ptr<stInputInfo*>(SAMP_INPUT_INFO_OFFSET_R2);
			if (pInput == nullptr)
				return false;

			pDialog = samp_ptr<stDialogInfo*>(SAMP_DIALOG_INFO_OFFSET_R2);
			if (pDialog == nullptr)
				return false;
		}
		break;
	case 3:
		{
			pChat = samp_ptr<stChatInfo*>(SAMP_CHAT_INFO_OFFSET_R3);
			if (pChat == nullptr)
				return false;

			pInput = samp_ptr<stInputInfo*>(SAMP_INPUT_INFO_OFFSET_R3);
			if (pInput == nullptr)
				return false;

			pDialog = samp_ptr<stDialogInfo*>(SAMP_DIALOG_INFO_OFFSET_R3);
			if (pDialog == nullptr)
				return false;
		}
		break;
	case 4:
		{
			pChat = samp_ptr<stChatInfo*>(SAMP_CHAT_INFO_OFFSET_R4);
			if (pChat == nullptr)
				return false;

			pInput = samp_ptr<stInputInfo*>(SAMP_INPUT_INFO_OFFSET_R4);
			if (pInput == nullptr)
				return false;

			pDialog = samp_ptr<stDialogInfo*>(SAMP_DIALOG_INFO_OFFSET_R4);
			if (pDialog == nullptr)
				return false;
		}
		break;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                              FUNCTIONS                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void CSAMP::AddChatMessage(D3DCOLOR color, const char *format, ...)
{
	if (format == NULL)
		return;

	va_list ap;
	char text[256];
	memset(text, 0, 256);

	va_start(ap, format);
	vsprintf_s(text, 256, format, ap);
	va_end(ap);

	fAddChatMessage(pChat, CHAT_TYPE_DEBUG, text, NULL, color, NULL);
}

void CSAMP::SendChat(const char *text)
{
	if (text == nullptr)
		return;

	if (text[0] == '/')
	{
		fSendCommand(pInput, text);
	}
	else
	{
		fSendSay(nullptr, text);
	}
}

void CSAMP::registerChatCommand(const char* name, CMDPROC callback)
{
	fAddClientCommand(pInput, name, callback);
}