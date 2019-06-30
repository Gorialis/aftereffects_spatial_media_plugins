
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

	AEFX_CLR_STRUCT(def);

	PF_ADD_CHECKBOX(
		STR(StrID_Antialias_Param_Name),
		STR(StrID_Antialias_Param_Desc),
		TRUE,
		0,
		ANTIALIAS_PARAM_ID
	);

	out_data->num_params = INVEQUI_NUM_PARAMS;

	return PF_Err_NONE;
}

struct RenderArgs {
	PF_InData *in_data;
	PF_ParamDef **params;
	PF_ParamDef *scanLayer;
	Math3D::TransformRenderer<float> renderer;
};

inline PF_Pixel8 *sampleIntegral8(PF_EffectWorld &def, int x, int y) {
	return (PF_Pixel8*)((char*)def.data + (y * def.rowbytes) + (x * sizeof(PF_Pixel8)));
}

inline PF_Pixel8 boundedIntegral(PF_EffectWorld &def, int x, int y) {
	if (x < 0 || y < 0)
		return {0, 0, 0, 0};
	if (x >= def.width || y >= def.height)
		return {0, 0, 0, 0};

	PF_Pixel8 *ptr = sampleIntegral8(def, x, y);
	if (ptr == nullptr)
		return {0, 0, 0, 0};

	return *ptr;
}

inline PF_Pixel8 lanczosSample(PF_EffectWorld &def, float x, float y) {
	int xi = static_cast<int>(x);
	int yi = static_cast<int>(y);
	float xo = x - static_cast<float>(xi);
	float yo = y - static_cast<float>(yi);

	float lx, ly;
	float a, r, g, b;
	float y_a, y_r, y_g, y_b;

	float x_coeffs[7];
	for (int ix_pre = -3; ix_pre <= 3; ix_pre++) {
		x_coeffs[ix_pre + 3] = Math3D::lanczos(xo - ix_pre);
	}

	a = r = g = b = 0.0f;

	for (int iy = -3; iy <= 3; iy++) {
		y_a = y_r = y_g = y_b = 0.0f;

		for (int ix = -3; ix <= 3; ix++) {
			lx = x_coeffs[ix + 3];
			PF_Pixel px = boundedIntegral(def, xi + ix, yi + iy);

			y_a += lx * px.alpha;
			y_r += lx * px.red;
			y_g += lx * px.green;
			y_b += lx * px.blue;
		}

		ly = Math3D::lanczos(yo - iy);

		a += ly * y_a;
		r += ly * y_r;
		g += ly * y_g;
		b += ly * y_b;
	}

	return {
		static_cast<A_u_char>(fmaxf(fminf(255.0f, a), 0.0f)),
		static_cast<A_u_char>(fmaxf(fminf(255.0f, r), 0.0f)),
		static_cast<A_u_char>(fmaxf(fminf(255.0f, g), 0.0f)),
		static_cast<A_u_char>(fmaxf(fminf(255.0f, b), 0.0f))
	};
}

constexpr uint8_t PRECISION_BITS = 7;

#define divshift(a)\
    ((((a) >> 8) + a) >> 8)

static PF_Pixel CompositePixel(PF_Pixel fg, PF_Pixel bg) {
	if (fg.alpha == 0)
		return bg;
	if (fg.alpha == 255)
		return fg;

	uint32_t tmpr, tmpg, tmpb;
	uint32_t blend = bg.alpha * (255 - fg.alpha);
	uint32_t oa = fg.alpha * 255 + blend;

	uint32_t coef1 = fg.alpha * 255 * 255 * (1 << PRECISION_BITS) / oa;
	uint32_t coef2 = 255 * (1 << PRECISION_BITS) - coef1;

	tmpr = fg.red * coef1 + bg.red * coef2;
	tmpg = fg.green * coef1 + bg.green * coef2;
	tmpb = fg.blue * coef1 + bg.blue * coef2;

	return {
		static_cast<A_u_char>(divshift(oa + 0x80)),
		static_cast<A_u_char>(divshift(tmpr + (0x80 << PRECISION_BITS)) >> PRECISION_BITS),
		static_cast<A_u_char>(divshift(tmpg + (0x80 << PRECISION_BITS)) >> PRECISION_BITS),
		static_cast<A_u_char>(divshift(tmpb + (0x80 << PRECISION_BITS)) >> PRECISION_BITS)
	};
}

static PF_Err SubSample_Pixel8(void *refcon, A_long x, A_long y, PF_Pixel8 *inP, PF_Pixel8 *outP) {
	PF_Err err = PF_Err_NONE;
	RenderArgs* rArgs = reinterpret_cast<RenderArgs*>(refcon);

	PF_EffectWorld sample = rArgs->scanLayer->u.ld;

	bool antialias = static_cast<bool>(rArgs->params[INVEQUI_ANTIALIAS]->u.bd.value);
	bool preserve = static_cast<bool>(rArgs->params[INVEQUI_KEEPLAYER]->u.bd.value);

	glm::vec3 screen = rArgs->renderer.Render(x, y);

	PF_Pixel out;

	if (!rArgs->renderer.inBounds(screen)) {
		out = { 0, 0, 0, 0 };
	} else {
		if (antialias) {
			out = lanczosSample(sample, screen.x, screen.y);
		}
		else {
			int sX = static_cast<int>(screen.x);
			int sY = static_cast<int>(screen.y);

			out = boundedIntegral(sample, sX, sY);
		}
	}

	if (preserve)
		out = CompositePixel(out, *inP);

	*outP = out;

	return err;
}

static PF_Err Render (PF_InData *in_data, PF_OutData *out_data, PF_ParamDef *params[], PF_LayerDef *output) {
	PF_Err	err  = PF_Err_NONE,
			err2 = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	PF_ParamDef scanLayer;

	A_long progress_baseL = 0, progress_finalL = 4;

	ERR(PF_CHECKOUT_PARAM(
		in_data,
		INVEQUI_LAYER,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&scanLayer
	));

	progress_baseL++;

	if (scanLayer.u.ld.width == 0 || scanLayer.u.ld.height == 0) {
		progress_baseL += 3;
		return err;
	}

	RenderArgs rArgs = {
		in_data,
		params,
		&scanLayer,
		Math3D::TransformRenderer<float>(
			glm::vec2(
				static_cast<float>(scanLayer.u.ld.width),
				static_cast<float>(scanLayer.u.ld.height)
			),
			glm::vec2(
				static_cast<float>(output->width),
				static_cast<float>(output->height)
			),
			glm::radians(static_cast<float>(FIX_2_FLOAT(params[INVEQUI_FOV]->u.ad.value))),
			glm::radians(static_cast<float>(FIX_2_FLOAT(params[INVEQUI_PITCH]->u.ad.value))),
			glm::radians(static_cast<float>(FIX_2_FLOAT(params[INVEQUI_YAW]->u.ad.value)))
		)
	};

	progress_baseL++;

	PF_LayerDef* drawLayer = &params[INVEQUI_INPUT]->u.ld;

	ERR(suites.Iterate8Suite1()->iterate(
		in_data,
		progress_baseL,
		progress_finalL,
		drawLayer,
		NULL,
		reinterpret_cast<void*>(&rArgs),
		SubSample_Pixel8,
		output
	));

	progress_baseL++;

	ERR2(PF_CHECKIN_PARAM(in_data, &scanLayer));

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

