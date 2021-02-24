#include "main.h"

std::string lite_conv(std::string &src, UINT cp_from, UINT cp_to)
{
	if (src.empty())
		return std::string();

	int wstr_len = MultiByteToWideChar(cp_from, NULL, src.c_str(), src.length(), NULL, 0);
	std::wstring wide_str(wstr_len, L'\x00');
	MultiByteToWideChar(cp_from, NULL, src.c_str(), src.length(), &wide_str[0], wstr_len);

	int outstr_len = WideCharToMultiByte(cp_to, NULL, &wide_str[0], wide_str.length(), NULL, 0, NULL, NULL);
	std::string out_str(outstr_len, '\x00');
	WideCharToMultiByte(cp_to, NULL, &wide_str[0], wide_str.length(), &out_str[0], outstr_len, NULL, NULL);

	return out_str;
}

std::wstring get_utf16(const std::string &str, int codepage)
{
	if (str.empty()) return std::wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	std::wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

std::string HttpWebRequest(LPCWSTR type, LPCWSTR domain, LPCWSTR url, LPVOID data, size_t data_len, bool useHTTPS, LPCWSTR headers = WINHTTP_NO_ADDITIONAL_HEADERS)
{
	std::string	response;

	HINTERNET hSession = WinHttpOpen(L"VoiceInput/0.3", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession)
	{
		HINTERNET hConnect = WinHttpConnect(hSession, domain, useHTTPS ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
		if (hConnect)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hConnect, type, url, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, useHTTPS ? WINHTTP_FLAG_SECURE : NULL);
			if (hRequest)
			{
				if (WinHttpSendRequest(hRequest, headers, headers ? (ULONG)-1L : 0, data, data_len, data_len, 0))
				{
					if (WinHttpReceiveResponse(hRequest, NULL))
					{
						DWORD dwSize = 0;
						do
						{
							dwSize = 0;
							WinHttpQueryDataAvailable(hRequest, &dwSize);
							if (dwSize > 0)
							{
								char* outBuffer = new char[dwSize + 1];
								if (outBuffer)
								{
									DWORD dwDownloaded = 0;
									if (WinHttpReadData(hRequest, (LPVOID)outBuffer, dwSize, &dwDownloaded))
									{
										outBuffer[dwDownloaded] = '\0';
										response += outBuffer;
									}

									delete[] outBuffer;
								}
								else dwSize = 0;
							}

						} while (dwSize > 0);

						if (GetLastError() != ERROR_SUCCESS)
							response.clear();
					}
				}
				WinHttpCloseHandle(hRequest);
			}
			WinHttpCloseHandle(hConnect);
		}
		WinHttpCloseHandle(hSession);
	}
	return response;
}

std::string RecognizeUsingVoiceInputAPI(STTFile* file)
{
	static std::wstring URL;
	if (URL.empty())
	{
		URL = std::wstring(L"voiceinput/api.php?service=" + get_utf16(ps.used_api, CP_UTF8) + L"&lang=" + get_utf16(ps.languageCode, CP_UTF8));
	}

	std::string result;
	std::string responce = HttpWebRequest(L"POST", L"f0446239.xsph.ru", URL.c_str(), file->data(), file->size(), false);
	if (responce.empty())
	{
		SAMP->AddChatMessage(0xFFFF4545, "[VoiceInput] Unexpected error occurred.");
		return std::string();
	}

	json_t *root = json_loads(responce.c_str(), 0, NULL);
	if (json_is_object(root))
	{
		json_t *error = json_object_get(root, "error");
		if (json_is_object(error))
		{
			json_t *code = json_object_get(error, "code");
			json_t *status = json_object_get(error, "status");
			json_t *message = json_object_get(error, "message");

			if (json_is_integer(code) && json_is_string(status) && json_is_string(message))
				SAMP->AddChatMessage(0xFFFF4545, "[ERROR: %d] %s: {FFFFFF}%s", json_integer_value(code), json_string_value(status), lite_conv(std::string(json_string_value(message)), CP_UTF8, CP_ACP).c_str());
			else
				SAMP->AddChatMessage(0xFFFF4545, "[VoiceInput] Unexpected error occurred.");
		}
		else
		{
			json_t* _result = json_object_get(root, "result");
			if (json_is_object(_result))
			{
				json_t *transcript = json_object_get(_result, "transcript");
				if (json_is_string(transcript))
					result = json_string_value(transcript);
			}
		}
	}
	json_decref(root);

	return result;
}

bool string_replace(std::string &str, const char* from, const char* to)
{
	if (str.empty() || from == nullptr || to == nullptr)
		return false;

	size_t pos = str.find(from);
	if (pos != std::string::npos)
	{
		str.replace(str.begin() + pos, str.begin() + pos + strlen(from), to);
		return true;
	}
	return false;
}