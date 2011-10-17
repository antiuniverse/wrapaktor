//////////////////////////////////////////////////////////////////////////

class ScopedCriticalSectionLock
{
public:
	ScopedCriticalSectionLock(CRITICAL_SECTION* pCritSec)
	:	m_pCriticalSection(pCritSec)
	{
		EnterCriticalSection(m_pCriticalSection);
	}

	~ScopedCriticalSectionLock()
	{
		LeaveCriticalSection(m_pCriticalSection);
	}

private:
	CRITICAL_SECTION* m_pCriticalSection;
};

//////////////////////////////////////////////////////////////////////////

class VSTPluginWrapper
{
public:
	VSTPluginWrapper();

	bool Init(audioMasterCallback pfnAudioMaster, AEffect** OUT pAEffectOut);
	void Shutdown();

protected:
	virtual void PreInitWrappedPlugin() { ; }
	virtual void PostInitWrappedPlugin() { ; }

	virtual VstIntPtr AEffectDispatch( AEffect* aeffect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt );

private:
	void DetermineWrappedModulePath();
	void ClearWrappedModulePath();
	bool LoadWrappedPlugin();
	void UnloadWrappedPlugin();
	bool InitWrappedPlugin(audioMasterCallback pfnAudioMaster);

private:
	_TCHAR m_szWrappedModulePath[MAX_PATH];
	HMODULE m_hWrappedModule;

	AEffect* m_pWrappedAEffect;

	typedef AEffect* (*PluginEntryProc)(audioMasterCallback pfnAudioMaster);
	PluginEntryProc m_pfnWrappedVSTEntry;
	AEffectDispatcherProc m_pfnWrappedDispatcher;

//////////////////////////////////////////////////////////////////////////
// STATIC MEMBERS

public:
	static void _StaticInit(HINSTANCE hInst);
	static void _StaticShutdown();

	static VstIntPtr AEffectDispatchHook(AEffect* aeffect, VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

private:
	static void MapInstance(AEffect* key, VSTPluginWrapper* value);
	static void UnmapInstance(AEffect* key, VSTPluginWrapper* value);
	static inline VSTPluginWrapper* GetInstance(AEffect* pAEffect);

private:
	static HINSTANCE sm_hInst;
	static CRITICAL_SECTION sm_critSec;

	typedef std::map<AEffect*, VSTPluginWrapper*> InstanceMap;
	static InstanceMap sm_instances;
};


inline VSTPluginWrapper* VSTPluginWrapper::GetInstance(AEffect* pAEffect)
{
	ScopedCriticalSectionLock lock(&sm_critSec);

	InstanceMap::iterator mappedInstance = sm_instances.find(pAEffect);

	if (mappedInstance != sm_instances.end())
		return mappedInstance->second;
	else
		return NULL;
}