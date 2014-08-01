#include "pch.h"

namespace invader { 

ZLevelFollower::ZLevelFollower(double filterCutoff /*= 10*/)
{
	lpFilter = new ZOnepoleFilter;
	SetFilterCutoff(filterCutoff);

	Reset();
}

ZLevelFollower::~ZLevelFollower(void)
{
	delete lpFilter;
}

void ZLevelFollower::Reset(void)
{
	lpFilter->Reset();
}

void ZLevelFollower::SetFilterCutoff(double filterCutoff)
{
	lpFilter->SetCutoff(filterCutoff);
}

void ZLevelFollower::Process(const sample_t& input)
{
	sample_t squared;
	squared.v = _mm_mul_pd(input.v, input.v);
	sample_t filtered;
	filtered = lpFilter->Lowpass(squared);
	level = zsqrtd(zmax(filtered.d[0], filtered.d[1]));
}

double ZLevelFollower::GetdBLevel(void)
{
	return gainTodB(level);
}

template<uint32_t oversamplingFactor>
void ZLevelFollowerProcessBlock(ZLevelFollower& follower, ZBlockBuffer<oversamplingFactor>& block)
{
	for (uint32_t i=0; i<block.numSamples; i++)
		follower.Process(block.samples[i]);
}

template void ZLevelFollowerProcessBlock(ZLevelFollower& follower, ZBlockBuffer<kDefaultOversampling>& block);

} // namespace invader
