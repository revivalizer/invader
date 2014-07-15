#pragma once

namespace invader {

class ZOnepoleFilter;

class ZLevelFollower
{
public:
	ZLevelFollower(double filterCutoff = 10);
	~ZLevelFollower(void);

	void Reset(void);
	void SetFilterCutoff(double filterCutoff);

	void Process(const sample_t& input);

	double GetdBLevel(void);

private:
	ZOnepoleFilter* lpFilter;
	double level;

};

template<uint32_t oversamplingFactor>
void ZLevelFollowerProcessBlock(ZLevelFollower& follower, ZBlockBuffer<oversamplingFactor>& block);

} // namespace invader