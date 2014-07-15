#include "pch.h"

namespace invader {

ZSyncInfo::ZSyncInfo(void)
{
}

ZSyncInfo::~ZSyncInfo(void)
{
}

void ZSyncInfo::Advance(uint32_t samples)
{
	double deltaTime = samples/kSampleRate;
	time += deltaTime;
	pos  += deltaTime*bps;
}

} // namespace invader
