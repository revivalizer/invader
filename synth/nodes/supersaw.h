
class ZSuperSaw : public ZSynthNode
{
public:
	static const uint32_t kNumOsc = 7;

	ZSuperSaw(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	
private:
	sample_t phase[kNumOsc];
	ZRandom rand;
	ZFilterZD24 filter;
	ShapingTransform unisonShape;
};
