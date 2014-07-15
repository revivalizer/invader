
class ZSawOsc : public ZSynthNode
{
public:
	ZSawOsc(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);
	
private:
	double phase;
};
