
class ZNoiseOsc : public ZSynthNode
{
public:
	ZNoiseOsc(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);
	
private:
	ZRandom rand;
};
