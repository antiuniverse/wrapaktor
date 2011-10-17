#include "VSTPluginWrapper.h"

class Wrapaktor : public VSTPluginWrapper
{
public:
	Wrapaktor();

private:
	virtual void PreInitWrappedPlugin();
	virtual void PostInitWrappedPlugin();

	virtual VstInt32 AEffectDispatch(AEffect* aeffect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

private:
	void RestorePriorityAndAffinity();

	DWORD m_dwOriginalPriority;
	DWORD m_dwOriginalProcessAffinityMask;
	DWORD m_dwOriginalSystemAffinityMask;
};