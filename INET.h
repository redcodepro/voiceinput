#pragma once

#include "main.h"

std::string lite_conv(std::string &src, UINT cp_from, UINT cp_to);
std::string RecognizeUsingVoiceInputAPI(STTFile* file);
bool string_replace(std::string &src, const char* from, const char* to);