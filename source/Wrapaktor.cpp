#include <stdafx.h>
#include "Wrapaktor.h"



//////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID /*lpvReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		VSTPluginWrapper::_StaticInit(hInst);
	else if (dwReason == DLL_PROCESS_DETACH)
		VSTPluginWrapper::_StaticShutdown();

	return TRUE;
}

extern "C" AEffect* VSTPluginMain(audioMasterCallback audioMaster)
{
	VSTPluginWrapper* pWrapper = new Wrapaktor;

	AEffect* pWrappedAEffect = NULL;

	pWrapper->Init(audioMaster, &pWrappedAEffect);

	return pWrappedAEffect;
}

//////////////////////////////////////////////////////////////////////////

Wrapaktor::Wrapaktor()
:	m_dwOriginalPriority(0),
	m_dwOriginalProcessAffinityMask(0),
	m_dwOriginalSystemAffinityMask(0)
{

}

void Wrapaktor::PreInitWrappedPlugin()
{
	m_dwOriginalPriority = GetPriorityClass(GetCurrentProcess());
	GetProcessAffinityMask(GetCurrentProcess(), &m_dwOriginalProcessAffinityMask, &m_dwOriginalSystemAffinityMask);
}

void Wrapaktor::PostInitWrappedPlugin()
{
	// For Reaktor VSTi/effects
	RestorePriorityAndAffinity();
}

VstInt32 Wrapaktor::AEffectDispatch( AEffect* aeffect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt )
{
	VstInt32 retVal = VSTPluginWrapper::AEffectDispatch(aeffect, opCode, index, value, ptr, opt);

	// For Kore Player (triggers this once or twice on program change)
// 	if (opCode == 69)
// 		RestorePriorityAndAffinity();

	return retVal;
}

void Wrapaktor::RestorePriorityAndAffinity()
{
	SetPriorityClass(GetCurrentProcess(), m_dwOriginalPriority);
	SetProcessAffinityMask(GetCurrentProcess(), m_dwOriginalProcessAffinityMask);
}
