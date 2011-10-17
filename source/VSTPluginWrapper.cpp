#include <stdafx.h>
#include "VSTPluginWrapper.h"


HINSTANCE                     VSTPluginWrapper::sm_hInst = NULL;
CRITICAL_SECTION              VSTPluginWrapper::sm_critSec;
VSTPluginWrapper::InstanceMap VSTPluginWrapper::sm_instances;



VSTPluginWrapper::VSTPluginWrapper()
:	m_hWrappedModule(NULL),
	m_pfnWrappedVSTEntry(NULL),
	m_pfnWrappedDispatcher(NULL),
	m_pWrappedAEffect(NULL)
{
	ClearWrappedModulePath();
}

bool VSTPluginWrapper::Init(audioMasterCallback pfnAudioMaster, AEffect** OUT pAEffectOut)
{
	PreInitWrappedPlugin();

	if (!InitWrappedPlugin(pfnAudioMaster))
		return false;

	PostInitWrappedPlugin();

	// Install host-to-plugin hook
	m_pfnWrappedDispatcher = m_pWrappedAEffect->dispatcher;
	m_pWrappedAEffect->dispatcher = &VSTPluginWrapper::AEffectDispatchHook;

	// Map the new AEffect instance to this wrapper instance
	MapInstance(m_pWrappedAEffect, this);

	*pAEffectOut = m_pWrappedAEffect;

	return true;
}

void VSTPluginWrapper::Shutdown()
{
	UnmapInstance(m_pWrappedAEffect, this);
	UnloadWrappedPlugin();
}


bool VSTPluginWrapper::InitWrappedPlugin(audioMasterCallback pfnAudioMaster)
{
	if (!LoadWrappedPlugin())
	{
		UnloadWrappedPlugin();
		return false;
	}

	m_pWrappedAEffect = m_pfnWrappedVSTEntry(pfnAudioMaster);

	return m_pWrappedAEffect != NULL;
}

void VSTPluginWrapper::DetermineWrappedModulePath()
{
	ClearWrappedModulePath();

	// Get the full path to our DLL
	_TCHAR ourModulePath[MAX_PATH] = {0};
	GetModuleFileName(sm_hInst, ourModulePath, MAX_PATH);

	// Wrapped module file name is the same as ours, except for the extension
	_TCHAR* moduleDllExtension = _tcsrchr(ourModulePath, _T('.'));
	size_t extensionIdx = size_t(moduleDllExtension - ourModulePath);

	_tcsncpy(m_szWrappedModulePath, ourModulePath, extensionIdx);
	_tcsncpy(m_szWrappedModulePath + extensionIdx, _T(".priowrap_dll"), MAX_PATH - extensionIdx);
}

void VSTPluginWrapper::ClearWrappedModulePath()
{
	memset(m_szWrappedModulePath, 0, sizeof(m_szWrappedModulePath));
}


void VSTPluginWrapper::UnloadWrappedPlugin()
{
	if (m_hWrappedModule)
	{
		FreeLibrary(m_hWrappedModule);
		m_hWrappedModule = NULL;
	}

	m_pWrappedAEffect = NULL;
	m_pfnWrappedVSTEntry = NULL;
	m_pfnWrappedDispatcher = NULL;
}

bool VSTPluginWrapper::LoadWrappedPlugin()
{
	UnloadWrappedPlugin();

	DetermineWrappedModulePath();

	m_hWrappedModule = LoadLibrary(m_szWrappedModulePath);

	if (m_hWrappedModule)
	{
		m_pfnWrappedVSTEntry = (PluginEntryProc)GetProcAddress(m_hWrappedModule, "VSTPluginMain");

		if (!m_pfnWrappedVSTEntry)
			m_pfnWrappedVSTEntry = (PluginEntryProc)GetProcAddress(m_hWrappedModule, "main");
	}

	return m_pfnWrappedVSTEntry != NULL;
}

VstIntPtr VSTPluginWrapper::AEffectDispatch( AEffect* aeffect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt )
{
	if (m_pfnWrappedDispatcher)
		return m_pfnWrappedDispatcher(aeffect, opCode, index, value, ptr, opt);
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////

VstIntPtr VSTPluginWrapper::AEffectDispatchHook(AEffect* aeffect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VSTPluginWrapper* pWrapper = GetInstance(aeffect);

	if (!pWrapper)
		return 0;

	// Dispatch to the wrapped plugin
	VstIntPtr retVal = pWrapper->AEffectDispatch(aeffect, opCode, index, value, ptr, opt);

	// Handle cleanup on plugin closed
	if (opCode == effClose)
	{
		pWrapper->Shutdown();
		delete pWrapper;
	}

	return retVal;
}

void VSTPluginWrapper::_StaticInit(HINSTANCE hInst)
{
	sm_hInst = hInst;
	InitializeCriticalSection(&sm_critSec);
}

void VSTPluginWrapper::_StaticShutdown()
{
	DeleteCriticalSection(&sm_critSec);
}

void VSTPluginWrapper::MapInstance(AEffect* key, VSTPluginWrapper* value)
{
	ScopedCriticalSectionLock lock(&sm_critSec);
	sm_instances[key] = value;
}

void VSTPluginWrapper::UnmapInstance(AEffect* key, VSTPluginWrapper* /*value*/)
{
	ScopedCriticalSectionLock lock(&sm_critSec);

	InstanceMap::iterator mappedInstance = sm_instances.find(key);

	if (mappedInstance != sm_instances.end())
		sm_instances.erase(mappedInstance);
}