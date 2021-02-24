#include "main.h"

struct ___globals	G;
struct PLUGIN_DATA	ps;

STTFile	*file = nullptr;
HRECORD	record = 0;

std::queue<std::string> speech;

BOOL CALLBACK RecordProc(HRECORD hRec, const void *buf, DWORD length, void *user)
{
	if (hRec != record)
		return true;

	if (file == nullptr || (uint32_t)user != ps.iPresetID)
		return false;

	static DWORD ticks = 0;

	if (!(KEY_DOWN(ps.presets.at(ps.iPresetID).keys[0]) || (ps.presets.at(ps.iPresetID).keys[1] && KEY_DOWN(ps.presets.at(ps.iPresetID).keys[1]))) && ticks == 0)
		ticks = GetTickCount();

	if (!file->isEnoughSize(length) || (ticks && GetTickCount() - ticks > 250))
	{
		ps.dwPluginState = STATE_REQUESTING;
		ticks = 0;
		return false;
	}

	file->Write(buf, length);
	ps.dwPluginState = STATE_RECORDING;
	return true;
}

HANDLE hThread, hStopEvent;
DWORD WINAPI POST_Thread(LPVOID arg)
{
	while (WaitForSingleObject(hStopEvent, 10) == WAIT_TIMEOUT)
	{
		if (ps.dwPluginState == STATE_RECORDINIT)
		{
			file = new STTFile(ps.sampleRate, ps.bitRate, ps.numChannels);
			if (file != nullptr)
			{
				record = BASS_RecordStart(ps.sampleRate, ps.numChannels, MAKELONG(NULL, 50), RecordProc, (void*)ps.iPresetID);

				ps.dwPluginState = STATE_RECORDING;
			}
		}
		if (ps.dwPluginState == STATE_REQUESTING)
		{
			if (file && file->ñontent_length() > 500)
			{
				std::string result = RecognizeUsingVoiceInputAPI(file);

				if (!result.empty())
				{
					std::string ansi = lite_conv(result, CP_UTF8, CP_ACP);
					size_t max_len = 127 - ps.presets[ps.iPresetID].format.length() - 2;
					if (ansi.length() > max_len)
						ansi.resize(max_len);

					char outstr[128];
					sprintf_s(outstr, 128, ps.presets.at(ps.iPresetID).format.c_str(), ansi.c_str());
					speech.push(outstr);
				}

				if (ps.saveLastRecord)
				{
					FILE *temp = fopen(".\\VoiceOutput.wav", "wb");
					if (temp)
					{
						fwrite(file->data(), sizeof(byte), file->size(), temp);
						fclose(temp);
					}
				}
			}
			ps.dwPluginState = STATE_ALREADY;

			_delete(file);
		}
	}
	ExitThread(0);
}

hookedMainloop_t orig_mainloop;
void __cdecl hooked_mainloop()
{
	static bool init = false;
	if (!init)
	{
		if (G.gameState != 9 || !SAMP->Init())
			return orig_mainloop();

		load_config(".\\VoiceInput.json");

		if ((hStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL)) && (hThread = CreateThread(NULL, 0, POST_Thread, 0, 0, 0)))
		{
			G.orig_wndproc = (WNDPROC)SetWindowLongA(G.hwnd, GWL_WNDPROC, (LONG)wnd_proc);

			if (BASS_RecordInit(-1) || BASS_ErrorGetCode() == BASS_ERROR_ALREADY)
				ps.dwPluginState = STATE_ALREADY;

			renderInit();
		}
		init = true;
	}
	
	while (!speech.empty())
	{
		SAMP->SendChat(speech.front().c_str());
		speech.pop();
	}

	if (ps.dwPluginState == STATE_ALREADY && !SAMP->getInput()->iInputEnabled && !SAMP->getDialog()->iIsActive)
	{
		int preset_id = -1;

		for (size_t i = 0; i < ps.presets.size(); ++i)
			if (ps.presets[i].keys[1])
			{
				if (KEY_DOWN(ps.presets[i].keys[0]) && KEY_PRESSED(ps.presets[i].keys[1]))
				{
					preset_id = i;
					break;
				}
			}
			else
			{
				if (KEY_PRESSED(ps.presets[i].keys[0]))
				{
					preset_id = i;
				}
			}
		if (preset_id != -1)
		{
			ps.iPresetID = preset_id;

			ps.dwPluginState = STATE_RECORDINIT;
		}
	}

	if (!G.isMenuOpened && ps.dwPluginState != STATE_NOTAVAIL)
	{
		static bool move = false;
		static int offset[2] = { 0, 0 };

		POINT &curPos = keyhook_get_mouse_position();

		if (move)
		{
			if (KEY_DOWN(VK_LBUTTON))
			{
				ps.pos_x = curPos.x - offset[0];
				ps.pos_y = curPos.y - offset[1];

				if (ps.pos_x < 0) ps.pos_x = 0;
				if (ps.pos_y < 0) ps.pos_y = 0;
				if (ps.pos_x + 32 > G.screen_x) ps.pos_x = G.screen_x - 32;
				if (ps.pos_y + 32 > G.screen_y) ps.pos_y = G.screen_y - 32;
			}
			else move = false;
		}
		else
		{
			if (SAMP->getInput()->iInputEnabled && KEY_PRESSED(VK_LBUTTON) && MOUSE_HOVERED(ps.pos_x, ps.pos_y, 32, 32))
			{
				offset[0] = curPos.x - ps.pos_x;
				offset[1] = curPos.y - ps.pos_y;
				move = true;
			}
		}
	}

	if (G.isMenuOpened)
	{
		keyhook_clear_states();
	}
	else
	{
		keyhook_run();
	}
	return orig_mainloop();
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	G.selfModule = hModule;

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);

			SAMP = new CSAMP();

			MH_Initialize();
			MH_CreateHookEx((void*)0x561B10, &hooked_mainloop, &orig_mainloop);
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			renderFree();

			MH_RemoveHook((void*)0x561B10);

			if (hStopEvent && hThread)
			{
				SetEvent(hStopEvent);
				WaitForSingleObject(hThread, INFINITE);
				CloseHandle(hThread);
				CloseHandle(hStopEvent);
			}
			BASS_RecordFree();

			_delete(SAMP);

			MH_Uninitialize();
		}
		break;
	}
	return true;
}

void load_config(const char* filename)
{
	json_error_t error;
	json_t* settings = json_load_file(".\\VoiceInput.json", 0, &error);

	if (json_is_object(settings))
	{
		json_t* config = json_object_get(settings, "config");
		if (json_is_object(config))
		{
			json_t *slr = json_object_get(config, "saveLastRecord");
			if (json_is_boolean(slr))
			{
				ps.saveLastRecord = json_boolean_value(slr);
			}
			json_t *stt = json_object_get(config, "stt");
			if (json_is_object(stt))
			{
				json_t* usedAPI = json_object_get(stt, "API");
				if (json_is_string(usedAPI))
					ps.used_api = json_string_value(usedAPI);

				json_t* languageCode = json_object_get(stt, "languageCode");
				if (json_is_string(languageCode))
					ps.languageCode = json_string_value(languageCode);
			}
			json_t *render = json_object_get(config, "render");
			if (json_is_object(render))
			{
				json_t *pos = json_object_get(render, "pos");
				if (json_is_object(pos))
				{
					json_t* pos_x = json_object_get(pos, "x");
					if (json_is_integer(pos_x))
						ps.pos_x = json_integer_value(pos_x);

					json_t* pos_y = json_object_get(pos, "y");
					if (json_is_integer(pos_y))
						ps.pos_y = json_integer_value(pos_y);
				}
			}
			json_t *chat_formats = json_object_get(config, "presets");
			if (json_is_array(chat_formats))
			{
				ps.presets.clear();

				size_t i, j;
				json_t *val0, *val1;

				json_array_foreach(chat_formats, i, val0)
				{
					if (json_is_object(val0))
					{
						CHAT_PROFILE temp;

						json_t* fmtstr = json_object_get(val0, "format");
						if (json_is_string(fmtstr))
						{
							temp.format = json_string_value(fmtstr);
							temp.format.erase(std::remove(temp.format.begin(), temp.format.end(), '%'), temp.format.end());
							if (!string_replace(temp.format, "<text>", "%s"))
								continue;
						}

						json_t* keys = json_object_get(val0, "keys");
						if (json_is_array(keys))
						{
							json_array_foreach(keys, j, val1)
							{
								if (json_is_string(val1) && j < 2)
									temp.keys[j] = key_lookup(json_string_value(val1));
							}
						}
						ps.presets.push_back(temp);
					}
				}
			}
		}
	}
	json_decref(settings);

	save_config(filename);
}

void save_config(const char* filename)
{
	json_t *to_save = json_object();
	if (json_is_object(to_save))
	{
		json_t* config = json_object();
		if (json_is_object(config))
		{
			json_object_set(config, "saveLastRecord", json_boolean(ps.saveLastRecord));
			json_t* stt = json_object();
			if (json_is_object(stt))
			{
				json_object_set(stt, "API", json_string(ps.used_api.c_str()));
				json_object_set(stt, "languageCode", json_string(ps.languageCode.c_str()));

				json_object_set(config, "stt", stt);
			}

			json_t* render = json_object();
			if (json_is_object(render))
			{
				json_t* pos = json_object();
				if (json_is_object(pos))
				{
					json_object_set(pos, "x", json_integer(ps.pos_x));
					json_object_set(pos, "y", json_integer(ps.pos_y));

					json_object_set(render, "pos", pos);
				}

				json_object_set(config, "render", render);
			}

			json_t* chat_formats = json_array();
			if (json_is_array(chat_formats))
			{
				for (uint32_t i = 0; i < ps.presets.size(); ++i)
				{
					json_t* obj00 = json_object();
					if (json_is_object(obj00))
					{
						std::string temp = ps.presets.at(i).format;
						string_replace(temp, "%s", "<text>");
						json_object_set(obj00, "format", json_string(temp.c_str()));

						json_t* keys = json_array();
						if (json_is_array(keys))
						{
							json_array_append(keys, json_string(key_delookup(ps.presets[i].keys[0]).c_str()));
							if (ps.presets[i].keys[1])
								json_array_append(keys, json_string(key_delookup(ps.presets[i].keys[1]).c_str()));

							json_object_set(obj00, "keys", keys);
						}
						json_array_append(chat_formats, obj00);
					}
				}
				json_object_set(config, "presets", chat_formats);
			}

			json_object_set(to_save, "config", config);
		}
	}

	json_dump_file(to_save, filename, JSON_INDENT(2));

	json_decref(to_save);
}
