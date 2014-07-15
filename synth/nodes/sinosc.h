
class ZSinOsc : public ZSynthNode
{
public:
	ZSinOsc(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);
	
private:
	double phase;
};
