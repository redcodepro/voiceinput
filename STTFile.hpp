#pragma once

#define _free(x) if (x) { free(x); x = NULL; }
#define _delete(x) if (x) { delete x; x = NULL; }

#define MAX_LENGTH_IN_SECONDS 15

#include <stdint.h>
#include <windows.h>

struct WAVEHEADER
{
	char chunkId[4] = { 'R','I','F','F' };
	uint32_t chunkSize;
	char format[4] = { 'W','A','V','E' };
	char subchunk1Id[4] = { 'f','m','t',' ' };
	uint32_t subchunk1Size = 16;
	uint16_t audioformat = WAVE_FORMAT_PCM;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	char subchunk2Id[4] = { 'd','a','t','a' };
	uint32_t datasize;
};

struct STTFile
{
private:
	byte*	buffer = nullptr;
	size_t	allocated_size = 0;
	size_t	write_pointer = 0;
	size_t	bytes_written = 0;

	WAVEHEADER wh;

	bool create_buffer(size_t size)
	{
		buffer = (byte*)malloc(sizeof(WAVEHEADER) + size);
		if (buffer)
		{
			allocated_size = sizeof(WAVEHEADER) + size;
			write_pointer = sizeof(WAVEHEADER);
			bytes_written = 0;
			memcpy(buffer, &wh, sizeof(WAVEHEADER));
			return true;
		}
		return false;
	}
public:
	STTFile(uint32_t sampleRate, uint16_t bitRate, uint16_t channels)
	{
		wh.sampleRate = sampleRate;
		wh.bitsPerSample = bitRate;
		wh.numChannels = channels;
		wh.blockAlign = wh.numChannels * (wh.bitsPerSample / 8);
		wh.byteRate = wh.sampleRate * wh.blockAlign;
		create_buffer(wh.byteRate * 3);
	}
	~STTFile()
	{
		_free(buffer);
	}
	byte *data(void) { return buffer; }
	size_t size(void) { return bytes_written + sizeof(WAVEHEADER); }
	size_t ñontent_length(void) { return (size_t)(((float)bytes_written / (float)wh.byteRate) * 1000.0f); }

	void Write(const void* data, const size_t size)
	{
		if (data == nullptr || size == 0)
			return;
		if (buffer != nullptr)
			if (write_pointer + size <= allocated_size)
			{
				memcpy(buffer + write_pointer, data, size);
				write_pointer += size;
				bytes_written += size;

				((WAVEHEADER*)buffer)->chunkSize = bytes_written + 36;
				((WAVEHEADER*)buffer)->datasize = bytes_written;
			}
	}
	bool isEnoughSize(const size_t size)
	{
		if (write_pointer + size <= allocated_size)
			return true;
		if ((write_pointer + size + wh.byteRate) <= (sizeof(WAVEHEADER) + (wh.byteRate * wh.numChannels * MAX_LENGTH_IN_SECONDS)))
			return Resize(allocated_size + wh.byteRate);
		return false;
	}
	bool Resize(const size_t new_size)
	{
		if (buffer == nullptr || allocated_size == 0)
		{
			return create_buffer(new_size);
		}
		else
		{
			buffer = (byte*)realloc(buffer, new_size);
			if (buffer)
			{
				allocated_size = new_size;
				return true;
			}
		}
		return false;
	}
};