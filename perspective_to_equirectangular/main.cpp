
#include "../global/effect_base.hpp"

#include "../global/effectworld_handling.hpp"
#include "../global/perspective_math.hpp"

#include "defs.hpp"

EffectPlugin::EffectPlugin() : name(PLUGIN_NAME), uuid(PLUGIN_UUID), parameters(
    {
        std::make_shared<PopupParameter>("FOV direction", 3, "Vertical|Horizontal|Diagonal", 0),
        std::make_shared<AngleParameter>("FOV", 90.0f),
        std::make_shared<LayerParameter>("Source layer"),
        std::make_shared<AngleParameter>("Yaw"),
        std::make_shared<AngleParameter>("Pitch"),
        std::make_shared<AngleParameter>("Roll"),
        std::make_shared<CheckboxParameter>("Antialiasing"),
        std::make_shared<CheckboxParameter>("Retain background"),
    }
) {}

struct RenderArgs {
    EffectPlugin* plugin;
    ParameterCheckout* layer;
    PF_LayerDef* output;
};

static PF_Err wipe(void* refptr, A_long x, A_long y, PF_Pixel8* in_pixel, PF_Pixel8* out_pixel) {
    RenderArgs* args = reinterpret_cast<RenderArgs*>(refptr);

    PF_EffectWorld sampler = args->layer->def.u.ld;
    A_long direction = args->plugin->parameters[static_cast<size_t>(0)]->u.pd.value;

    glm::vec3 globevec = get_globevec_from_equirectangular({x, y}, {args->output->width, args->output->height});
    globevec = transform_globevec_outside_in(
        globevec,
        glm::radians(static_cast<float>(FIX_2_FLOAT(args->plugin->parameters[4]->u.ad.value))),
        glm::radians(static_cast<float>(FIX_2_FLOAT(args->plugin->parameters[3]->u.ad.value))),
        glm::radians(static_cast<float>(FIX_2_FLOAT(args->plugin->parameters[5]->u.ad.value)))
    );
    glm::vec2 sample_loc = get_perspective_from_globevec(
        globevec,
        {sampler.width, sampler.height},
        glm::radians(static_cast<float>(FIX_2_FLOAT(args->plugin->parameters[1]->u.ad.value)) / 2.0f),
        direction == 3 ? FOV_Type_Diagonal :
        direction == 2 ? FOV_Type_Horizontal :
        FOV_Type_Vertical
    );

    bool antialias = args->plugin->parameters[6]->u.bd.value;
    bool retain = args->plugin->parameters[7]->u.bd.value;

    PF_Pixel8 pix = antialias ? sample_safe_lanczos(sampler, sample_loc.x, sample_loc.y) : sample_safe(sampler, sample_loc.x, sample_loc.y);
    *out_pixel = retain ? composite_pixel(pix, *in_pixel) : pix;

    return PF_Err_NONE;
}

PF_Err EffectPlugin::render(PF_InData* in_data, PF_OutData* out_data, PF_LayerDef* output) {
    PF_Err err = PF_Err_NONE;

    AEGP_SuiteHandler suites(in_data->pica_basicP);
    ParameterCheckout sourceLayer(in_data, 3);

    RenderArgs args = {this, &sourceLayer, output};

    ERR(
        suites.Iterate8Suite1()->iterate(
            in_data,
            0,
            1,
            &parameters.self()->u.ld,
            NULL,
            reinterpret_cast<void*>(&args),
            wipe,
            output
        )
    );

    return err;
}
