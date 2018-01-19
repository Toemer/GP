class CClientEntityList
{
public:

	CBaseEntity* GetClientEntity( int Index )
	{
		typedef CBaseEntity*(__thiscall* Fn)(void*, int);
		return CallVFunction<Fn>(this, 3)(this, Index);
	}

	int GetHighestEntityIndex()
	{
		typedef int(__thiscall* Fn)(void*);
		return CallVFunction<Fn>(this, 8)(this);
	}

	CBaseEntity* GetClientEntityFromHandle( HANDLE Handle )
	{
		typedef CBaseEntity*(__thiscall* Fn)(void*,HANDLE);
		return CallVFunction<Fn>(this, 7)(this, Handle);
	}
	CBaseEntity *	GetClientEntityFromHandle(ULONG hEnt)
	{
		typedef CBaseEntity*(__thiscall* Fn)(void*, ULONG);
		return CallVFunction<Fn>(this, 4)(this, hEnt);
	}
	CBaseEntity *	GetClientEntityFromHandle_D(DWORD hEnt)
	{
		typedef CBaseEntity*(__thiscall* Fn)(void*, int);
		return CallVFunction<Fn>(this, 4)(this, hEnt);
	}

};

#define TE_BEAMPOINTS 0		// beam effect between two points

class C_Beam;
class Beam_t;

struct BeamInfo_t
{
	int			m_nType;

	// Entities
	CBaseEntity* m_pStartEnt;
	int			m_nStartAttachment;
	CBaseEntity* m_pEndEnt;
	int			m_nEndAttachment;

	// Points
	Vector		m_vecStart;
	Vector		m_vecEnd;

	int			m_nModelIndex;
	const char	*m_pszModelName;

	int			m_nHaloIndex;
	const char	*m_pszHaloName;
	float		m_flHaloScale;

	float		m_flLife;
	float		m_flWidth;
	float		m_flEndWidth;
	float		m_flFadeLength;
	float		m_flAmplitude;

	float		m_flBrightness;
	float		m_flSpeed;

	int			m_nStartFrame;
	float		m_flFrameRate;

	float		m_flRed;
	float		m_flGreen;
	float		m_flBlue;

	bool		m_bRenderable;

	int			m_nSegments;

	int			m_nFlags;

	// Rings
	Vector		m_vecCenter;
	float		m_flStartRadius;
	float		m_flEndRadius;

	BeamInfo_t()
	{
		m_nType = TE_BEAMPOINTS;
		m_nSegments = -1;
		m_pszModelName = NULL;
		m_pszHaloName = NULL;
		m_nModelIndex = -1;
		m_nHaloIndex = -1;
		m_bRenderable = true;
		m_nFlags = 0;
	}
};

class ITraceFilter;
class ITexture;

class IViewRenderBeams
{
public:
	// Construction
public:
	IViewRenderBeams(void);
	virtual				~IViewRenderBeams(void);

	// Implement IViewRenderBeams
public:
	virtual	void		InitBeams(void);
	virtual	void		ShutdownBeams(void);
	virtual	void		ClearBeams(void);

	// Updates the state of the temp ent beams
	virtual void		UpdateTempEntBeams();

	virtual void		DrawBeam(Beam_t *pbeam);
	virtual void		DrawBeam(C_Beam* pbeam, ITraceFilter *pEntityBeamTraceFilter = NULL);

	virtual	void		KillDeadBeams(CBaseEntity *pDeadEntity);

	virtual	void		CreateBeamEnts(int startEnt, int endEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int type = -1);
	virtual Beam_t		*CreateBeamEnts(BeamInfo_t &beamInfo);

	virtual	void		CreateBeamEntPoint(int	nStartEntity, const Vector *pStart, int nEndEntity, const Vector* pEnd,
		int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b);
	virtual Beam_t		*CreateBeamEntPoint(BeamInfo_t &beamInfo);

	virtual	void		CreateBeamPoints(Vector& start, Vector& end, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b);
	virtual	Beam_t		*CreateBeamPoints(BeamInfo_t &beamInfo);

	virtual	void		CreateBeamRing(int startEnt, int endEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int flags);
	virtual Beam_t		*CreateBeamRing(BeamInfo_t &beamInfo);

	virtual void		CreateBeamRingPoint(const Vector& center, float start_radius, float end_radius, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int flags);
	virtual Beam_t		*CreateBeamRingPoint(BeamInfo_t &beamInfo);

	virtual	void		CreateBeamCirclePoints(int type, Vector& start, Vector& end,
		int modelIndex, int haloIndex, float haloScale, float life, float width,
		float endWidth, float fadeLength, float amplitude, float brightness, float speed,
		int startFrame, float framerate, float r, float g, float b);
	virtual Beam_t		*CreateBeamCirclePoints(BeamInfo_t &beamInfo);

	virtual	void		CreateBeamFollow(int startEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float r, float g, float b,
		float brightness);
	virtual Beam_t		*CreateBeamFollow(BeamInfo_t &beamInfo);
	/*virtual void	InitBeams(void) = 0;
	virtual void	ShutdownBeams(void) = 0;
	virtual void	ClearBeams(void) = 0;

	// Updates the state of the temp ent beams
	virtual void	UpdateTempEntBeams() = 0;

	virtual void	DrawBeam(C_Beam* pbeam, ITraceFilter *pEntityBeamTraceFilter = NULL) = 0;
	virtual void	DrawBeam(Beam_t *pbeam) = 0;

	virtual void	KillDeadBeams(IClientEntity* pEnt) = 0;

	// New interfaces!
	virtual Beam_t	*CreateBeamEnts(BeamInfo_t &beamInfo) = 0;
	virtual Beam_t	*CreateBeamEntPoint(BeamInfo_t &beamInfo) = 0;
	virtual	Beam_t	*CreateBeamPoints(BeamInfo_t &beamInfo) = 0;
	virtual Beam_t	*CreateBeamRing(BeamInfo_t &beamInfo) = 0;
	virtual Beam_t	*CreateBeamRingPoint(BeamInfo_t &beamInfo) = 0;
	virtual Beam_t	*CreateBeamCirclePoints(BeamInfo_t &beamInfo) = 0;
	virtual Beam_t	*CreateBeamFollow(BeamInfo_t &beamInfo) = 0;

	virtual void	FreeBeam(Beam_t *pBeam) = 0;
	virtual void	UpdateBeamInfo(Beam_t *pBeam, BeamInfo_t &beamInfo) = 0;*/
};