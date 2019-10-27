#pragma once

#include <cstring>
#include <map>
#include <memory>
#include <vector>

#include "ae_definitions.hpp"


class ParameterABC {
    public:
        ParameterABC(const char* name) : name(name) {}
        virtual PF_Err declare(PF_InData* in_data, int id) {
            return PF_Err_NONE;
        }

        const char* name;
};

class AngleParameter : public ParameterABC {
    public:
        AngleParameter(
            const char* name, float default_value = 0.0f
        ) : ParameterABC(name), default_value(default_value) {}

        virtual PF_Err declare(PF_InData* in_data, int id) {
            PF_ParamDef def;
            AEFX_CLR_STRUCT(def);

            PF_ADD_ANGLE(
                name,
                default_value,
                id
            );

            return PF_Err_NONE;
        }

        float default_value;
};

class CheckboxParameter : public ParameterABC {
    public:
        CheckboxParameter(
            const char* name, const char* description = "", int default_value = FALSE, int flags = 0
        ) : ParameterABC(name), description(description), default_value(default_value), flags(flags) {}

        virtual PF_Err declare(PF_InData* in_data, int id) {
            PF_ParamDef def;
            AEFX_CLR_STRUCT(def);

            PF_ADD_CHECKBOX(
                name,
                description,
                default_value,
                flags,
                id
            );

            return PF_Err_NONE;
        }

        const char* description;
        int default_value;
        int flags;
};

class LayerParameter : public ParameterABC {
    public:
        LayerParameter(
            const char* name, int default_value = PF_LayerDefault_MYSELF
        ) : ParameterABC(name), default_value(default_value) {}

        virtual PF_Err declare(PF_InData* in_data, int id) {
            PF_ParamDef def;
            AEFX_CLR_STRUCT(def);

            PF_ADD_LAYER(
                name,
                default_value,
                id
            );

            return PF_Err_NONE;
        }

        int default_value;
};

class PopupParameter : public ParameterABC {
    public:
        PopupParameter(
            const char* name, int choice_count, const char* choices, int default_value = 0
        ) : ParameterABC(name), choice_count(choice_count), choices(choices), default_value(default_value) {}

        virtual PF_Err declare(PF_InData* in_data, int id) {
            PF_ParamDef def;
            AEFX_CLR_STRUCT(def);

            PF_ADD_POPUP(
                name,
                choice_count,
                default_value,
                choices,
                id
            );

            return PF_Err_NONE;
        }

        int choice_count;
        const char* choices;
        int default_value;
};

namespace {
    struct StringComparer {
        bool operator()(const char* a, const char* b) const {
            return std::strcmp(a, b) < 0;
        }
    };
}

class ParameterGroup {
    public:
        ParameterGroup(std::vector<std::shared_ptr<ParameterABC>> params) : internal(params) {
            for (std::vector<ParameterABC>::size_type i = 0; i < params.size(); i++) {
                lookup.insert(
                    std::pair<const char*, std::vector<ParameterABC>::size_type>(
                        params[i]->name,
                        i + 1
                    )
                );
            }
        };

        PF_Err declare(PF_InData* in_data, PF_OutData* out_data) {
            PF_Err err = PF_Err_NONE;

            for (std::vector<std::shared_ptr<ParameterABC>>::size_type i = 0; i < internal.size(); i++) {
                if (err = internal[i]->declare(in_data, i + 1)) return err;
            }

            out_data->num_params = internal.size() + 1;
            return err;
        }

        void update(PF_ParamDef* params[]) {
            param_defs = params;
        }

        PF_ParamDef* operator[](std::vector<std::shared_ptr<ParameterABC>>::size_type i) {
            return param_defs[i + 1];
        }

        PF_ParamDef* operator[](const char* i) {
            auto search = lookup.find(i);

            if (search != lookup.end()) {
                return param_defs[search->second];
            } else {
                throw PF_Err_INVALID_INDEX;
            }
        }

        PF_ParamDef* self() {
            return param_defs[0];
        }

        PF_ParamDef** param_defs;

    private:
        std::vector<std::shared_ptr<ParameterABC>> internal;
        std::map<const char*, std::vector<std::shared_ptr<ParameterABC>>::size_type, StringComparer> lookup;

};
