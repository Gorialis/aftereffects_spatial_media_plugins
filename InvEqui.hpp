#pragma once

#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096
#define PF_DEEP_COLOR_AWARE 1

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "InvEqui_Strings.hpp"
#include "Meta.hpp"

enum {
	INVEQUI_INPUT = 0,
	INVEQUI_LAYER,
	INVEQUI_FOV,
	INVEQUI_YAW,
	INVEQUI_PITCH,
	INVEQUI_KEEPLAYER,
	INVEQUI_ANTIALIAS,
	INVEQUI_NUM_PARAMS
};

enum {
	LAYER_PARAM_ID = 1,
	FOV_PARAM_ID,
	YAW_PARAM_ID,
	PITCH_PARAM_ID,
	KEEPLAYER_PARAM_ID,
	ANTIALIAS_PARAM_ID,
};


extern "C" {
	DllExport PF_Err EffectMain(
		PF_Cmd cmd,
		PF_InData *in_data,
		PF_OutData *out_data,
		PF_ParamDef *params[],
		PF_LayerDef *output,
		void *extra
	);
}
