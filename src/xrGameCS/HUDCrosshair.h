// HUDCrosshair.h:  ������� �������, ������������ ������� ���������
// 
//////////////////////////////////////////////////////////////////////

#pragma once

#define HUD_CURSOR_SECTION "hud_cursor"

#include "ui_defs.h"


class CHUDCrosshair	
{
private:
	float			cross_length_perc;
	float			min_radius_perc;
	float			max_radius_perc;

	//������� ������ �������
	float			radius;
	float			target_radius;
#ifdef DEBUG
	float			fb_radius;
#endif
	//ref_geom 		hGeomLine;
	ui_shader		hShader;
public:
	u32				cross_color;

					CHUDCrosshair	();
					~CHUDCrosshair	();

			void	OnRender		(const float& x, const float& y);
			void	SetDispersion	(float disp);
#ifdef DEBUG
			void	SetFirstBulletDispertion(float fbdisp);
			void	OnRenderFirstBulletDispertion();
#endif

			void	Load			();
};