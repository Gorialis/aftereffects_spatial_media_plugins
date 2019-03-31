
#include "Math3D.hpp"
#include "InvEqui.hpp"

static PF_Err About (PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output) {
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	suites.ANSICallbacksSuite1()->sprintf(
		out_data->return_msg,
		"%s v%d.%d\r%s",
		STR(StrID_Name),
		MAJOR_VERSION,
		MINOR_VERSION,
		STR(StrID_Description)
	);

	return PF_Err_NONE;
}

static PF_Err GlobalSetup (PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output) {
	out_data->my_version = PF_VERSION(
		MAJOR_VERSION,
		MINOR_VERSION,
		BUG_VERSION,
		STAGE_VERSION,
		BUILD_VERSION
	);

	out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;

	return PF_Err_NONE;
}

static PF_Err ParamsSetup (PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output) {
	PF_ParamDef	def;

	AEFX_CLR_STRUCT(def);

	PF_ADD_LAYER(
		STR(StrID_Layer_Param_Name),
		PF_LayerDefault_MYSELF,
		LAYER_PARAM_ID
	);

	AEFX_CLR_STRUCT(def);

	PF_ADD_ANGLE(
		STR(StrID_FOV_Param_Name),
		90.0f,
		FOV_PARAM_ID
	);

	AEFX_CLR_STRUCT(def);

	PF_ADD_ANGLE(
		STR(StrID_Yaw_Param_Name),
		0.0f,
		YAW_PARAM_ID
	);

	AEFX_CLR_STRUCT(def);

	PF_ADD_ANGLE(
		STR(StrID_Pitch_Param_Name),
		0.0f,
		PITCH_PARAM_ID
	);

	AEFX_CLR_STRUCT(def);

	PF_ADD_CHECKBOX(
		STR(StrID_KeepLayer_Param_Name),
		STR(StrID_KeepLayer_Param_Desc),
		TRUE,
		0,
		KEEPLAYER_PARAM_ID
	);

	out_data->num_params = INVEQUI_NUM_PARAMS;

	return PF_Err_NONE;
}

struct RenderArgs {
	PF_InData *in_data;
	PF_ParamDef **params;
	PF_ParamDef *scanLayer;
};

inline PF_Pixel *sampleIntegral8(PF_EffectWorld &def, int x, int y) {
	return (PF_Pixel8*)((char*)def.data + (y * def.rowbytes) + (x * sizeof(PF_Pixel8)));
}

static PF_Err SubSample_Pixel8(void *refcon, A_long x, A_long y, PF_Pixel8 *inP, PF_Pixel8 *outP) {
	PF_Err err = PF_Err_NONE;
	RenderArgs* rArgs = reinterpret_cast<RenderArgs*>(refcon);

	float yawDeg = (static_cast<float>(x) * 360.0f / static_cast<float>(rArgs->in_data->width)) - 180.0f;
	float pitchDeg = (static_cast<float>(y) * 180.0f / static_cast<float>(rArgs->in_data->height)) - 90.0f;

	PF_EffectWorld sample = rArgs->scanLayer->u.ld;

	float yaw = static_cast<float>(FIX_2_FLOAT(rArgs->params[INVEQUI_YAW]->u.ad.value));
	float pitch = static_cast<float>(FIX_2_FLOAT(rArgs->params[INVEQUI_PITCH]->u.ad.value));

	float fov = static_cast<float>(FIX_2_FLOAT(rArgs->params[INVEQUI_FOV]->u.ad.value));
	fov = fminf(fmaxf(fov, 0.1f), 179.9f);

	bool keepLayer = static_cast<bool>(rArgs->params[INVEQUI_KEEPLAYER]->u.bd.value);

	Math3D::Vector3D<float> model =
		Math3D::rotate(
			Math3D::rotate(
				Math3D::Vector3D<float>(0.0f, 0.0f, -1.0f),
				Math3D::radians(pitchDeg),
				Math3D::Vector3D<float>(1.0f, 0.0f, 0.0f)
			),
			Math3D::radians(yawDeg),
			Math3D::Vector3D<float>(0.0f, 1.0f, 0.0f)
		);

	Math3D::Vector3D<float> view =
		Math3D::rotate(
			Math3D::rotate(
				model,
				Math3D::radians(-yaw),
				Math3D::Vector3D<float>(0.0f, 1.0f, 0.0f)
			),
			Math3D::radians(-pitch),
			Math3D::Vector3D<float>(1.0f, 0.0f, 0.0f)
		);

	Math3D::Vector3D<float> screen =
		Math3D::perspective(
			view,
			Math3D::radians(fov),
			static_cast<float>(sample.width) / static_cast<float>(sample.height)
		);

	if (screen.z < -1.0f || screen.z > 1.0f || screen.x < -1.0f || screen.x > 1.0f || screen.y < -1.0f || screen.y > 1.0f) {
		if (keepLayer) {
			outP->red = inP->red;
			outP->green = inP->green;
			outP->blue = inP->blue;
			outP->alpha = inP->alpha;
		}
		else {
			outP->red = 0;
			outP->green = 0;
			outP->blue = 0;
			outP->alpha = 0;
		}
	}
	else {
		float samplePosX = (screen.x + 1.0f) * 0.5f * static_cast<float>(sample.width);
		float samplePosY = (1.0f - ((screen.y + 1.0f) * 0.5f)) * static_cast<float>(sample.height);

		int sX = static_cast<int>(samplePosX);
		int sY = static_cast<int>(samplePosY);

		PF_Pixel* sampleP = sampleIntegral8(sample, sX, sY);

		if (sampleP != nullptr) {
			outP->red = sampleP->red;
			outP->green = sampleP->green;
			outP->blue = sampleP->blue;
			outP->alpha = sampleP->alpha;
		}
		else {
			outP->red = 255;
			outP->green = 255;
			outP->blue = 0;
			outP->alpha = 255;
		}
	}

	return err;
}

static PF_Err Render (PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output) {
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	PF_ParamDef scanLayer;

	RenderArgs rArgs = {
		in_data,
		params,
		&scanLayer
	};

	A_long progress_baseL = 0, progress_finalL = 1;

	ERR(PF_CHECKOUT_PARAM(
		in_data,
		INVEQUI_LAYER,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&scanLayer
	));

	ERR(suites.Iterate8Suite1()->iterate(
		in_data,
		progress_baseL,
		progress_finalL,
		&params[INVEQUI_INPUT]->u.ld,
		NULL,
		reinterpret_cast<void*>(&rArgs),
		SubSample_Pixel8,
		output
	));

	progress_baseL++;

	return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"Inverse Equirectangular",
		"GORIALIS InvEquirectangular",
		"Devon's Plug-ins",
		AE_RESERVED_INFO
	);

	return result;
}


PF_Err EffectMain(PF_Cmd cmd, PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output, void *extra) {
	PF_Err		err = PF_Err_NONE;

	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:
				err = About(
					in_data,
					out_data,
					params,
					output
				);
				break;

			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(
					in_data,
					out_data,
					params,
					output
				);
				break;

			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(
					in_data,
					out_data,
					params,
					output
				);
				break;

			case PF_Cmd_RENDER:
				err = Render(
					in_data,
					out_data,
					params,
					output
				);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

