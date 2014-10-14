#pragma once

class ZWatchFile : public align16
{
public:
	ZWatchFile(const char* path);
	~ZWatchFile(void);

	bool DidUpdate(void);

	char* data;
	char* freeNextUpdate;

	int32_t checkTime;
	int32_t readTime;

	HANDLE hFile;
	FILETIME ftLastRead;

	private:
		const char* path;
};