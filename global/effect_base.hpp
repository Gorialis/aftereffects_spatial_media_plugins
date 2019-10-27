#pragma once

#include <string>

#include "ae_definitions.hpp"
#include "parameter_handling.hpp"
#include "version.hpp"

#define THROW_RETURN(CALL)  if(PF_Err err = (CALL)) throw err;


class ParameterCheckout {
    public:
        ParameterCheckout(PF_InData* in_data, int identifier) : in_data(in_data) {
            if(PF_Err err = PF_CHECKOUT_PARAM(
                in_data,
                identifier,
                in_data->current_time,
                in_data->time_step,
                in_data->time_scale,
                &def
            )) {
                throw err;
            }
        }

        ~ParameterCheckout() {
            PF_CHECKIN_PARAM(
                in_data,
                &def
            );
        }

        PF_ParamDef def;

    private:
        PF_InData* in_data;
};

class EffectPlugin {
    public:
        EffectPlugin();

        PF_Err about(PF_InData* in_data, PF_OutData* out_data, PF_LayerDef* output) {
            AEGP_SuiteHandler suites(in_data->pica_basicP);

            suites.ANSICallbacksSuite1()->sprintf(
                out_data->return_msg,
                "%s v%d.%d\r\n\r\nSpatial Media Plugins by Devon (Gorialis) R",
                name,
                MAJOR_VERSION,
                MINOR_VERSION
            );

            return PF_Err_NONE;
        }

        PF_Err global_setup(PF_InData* in_data, PF_OutData* out_data, PF_LayerDef* output) {
            out_data->my_version = PF_VERSION(
                MAJOR_VERSION,
                MINOR_VERSION,
                BUG_VERSION,
                STAGE_VERSION,
                BUILD_VERSION
            );

            out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE;

            return PF_Err_NONE;
        }

        PF_Err parameter_setup(PF_InData* in_data, PF_OutData* out_data, PF_LayerDef* output) {
            return parameters.declare(in_data, out_data);
        }

        PF_Err render(PF_InData* in_data, PF_OutData* out_data, PF_LayerDef* output);

        const char* name;
        const char* uuid;
        ParameterGroup parameters;
};

extern "C" {
    DllExport PF_Err EffectMain(
        PF_Cmd cmd,
        PF_InData* in_data,
        PF_OutData* out_data,
        PF_ParamDef* params[],
        PF_LayerDef* output,
        void* extra
    ) {
        EffectPlugin plugin;
        plugin.parameters.update(params);

        try {
            switch (cmd) {
                case PF_Cmd_ABOUT:
                    THROW_RETURN(plugin.about(in_data, out_data, output));
                    break;
                case PF_Cmd_GLOBAL_SETUP:
                    THROW_RETURN(plugin.global_setup(in_data, out_data, output));
                    break;
                case PF_Cmd_PARAMS_SETUP:
                    THROW_RETURN(plugin.parameter_setup(in_data, out_data, output));
                    break;
                case PF_Cmd_RENDER:
                    THROW_RETURN(plugin.render(in_data, out_data, output));
                    break;
            }
        } catch (PF_Err& thrown_err) {
            return thrown_err;
        }

        return PF_Err_NONE;
    };

    DllExport PF_Err PluginDataEntryFunction(
        PF_PluginDataPtr in_ptr,
        PF_PluginDataCB in_plugin_data_callback_ptr,
        SPBasicSuite* in_sp_basic_suite_ptr,
        const char* in_host_name,
        const char* in_host_version
    ) {
        EffectPlugin plugin;
        PF_Err result = PF_Err_INVALID_CALLBACK;

        result = PF_REGISTER_EFFECT(
            in_ptr,
            in_plugin_data_callback_ptr,
            plugin.name,
            plugin.uuid,
            PLUGIN_CATEGORY,
            AE_RESERVED_INFO
        );

        return result;
    };
}
