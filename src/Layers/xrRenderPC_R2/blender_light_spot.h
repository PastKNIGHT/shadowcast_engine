#pragma once

#ifdef _EDITOR
#include "r2_types.h"
#include "Layers/xrRenderPC_R2/stdafx.h"
#endif

class CBlender_accum_spot : public IBlender  
{
public:
	virtual		LPCSTR		getComment()	{ return "INTERNAL: accumulate spot light";	}
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C);

	CBlender_accum_spot();
	virtual ~CBlender_accum_spot();
};
