#pragma once

#include "../xrEngine/CustomHUD.h"
#include "HitMarker.h"

class CHUDTarget;
class CUIGameCustom;

class CHUDManager :
	public CCustomHUD
{
	friend class CUI;
private:
    CUIGameCustom*			pUIGame;
	CHitMarker				HitMarker;
	CHUDTarget*				m_pHUDTarget;
	bool					b_online;
public:
							CHUDManager			();
	virtual					~CHUDManager		();
	virtual		void		OnEvent				(EVENT E, u64 P1, u64 P2);

	virtual		void		Load				();
	
	virtual		void		Render_First		();
	virtual		void		Render_Last			();	   
	virtual		void		OnFrame				();

	virtual		void		RenderUI			();

    CUIGameCustom*		GetGameUI			(){return pUIGame;}

				void		Hit					(int idx, float power, const Fvector& dir);
	//������� ������� �� ������� ������� HUD
	collide::rq_result&		GetCurrentRayQuery	();


	//�������� �������� ���� ������� � ����������� �� ������� ���������
	void					SetCrosshairDisp	(float dispf, float disps = 0.f);
	void					ShowCrosshair		(bool show);

	void					SetHitmarkType		(LPCSTR tex_name);
	virtual void			OnScreenResolutionChanged();
	virtual void			OnDisconnected		();
	virtual void			OnConnected			();

	virtual void			RenderActiveItemUI	();
	virtual bool			RenderActiveItemUIQuery();


	virtual void			net_Relcase			(CObject *object);
};

IC CHUDManager& HUD() { return *((CHUDManager*)g_hud); }