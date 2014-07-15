#pragma once

namespace invader {

class ZSyncInfo
{
public:
	ZSyncInfo(void);
	~ZSyncInfo(void);

	double pos;
	double time;
	double bpm;
	double bps;

	void Advance(uint32_t samples);
	void AdvanceBlock() { Advance(kBlockSize*kDefaultOversampling); }
};

} // namespace invader