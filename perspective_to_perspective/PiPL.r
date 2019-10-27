
#include "AEConfig.h"
#include "AE_EffectVers.h"

#ifndef AE_OS_WIN
    #include <AE_General.r>
#endif

#include "defs.hpp"

resource 'PiPL' (16000) {
    {
        Kind {
            AEEffect
        },
        Name {
            PLUGIN_NAME
        },
        Category {
            PLUGIN_CATEGORY
        },
        #ifdef AE_OS_WIN
        #ifdef AE_PROC_INTELx64
        CodeWin64X86 {"EffectMain"},
        #endif
        #ifdef AE_OS_MAC
        CodeMacIntel64 {"EffectMain"},
        #endif
        #endif,
        AE_PiPL_Version {
            MAJOR_VERSION, MINOR_VERSION
        },
        AE_Effect_Spec_Version {
            PF_PLUG_IN_VERSION,
            PF_PLUG_IN_SUBVERS
        },
        AE_Effect_Version {
            COMPOUND_VERSION
        },
        AE_Effect_Info_Flags {
            0
        },
        AE_Effect_Global_OutFlags {
            0x02000000
        },
        AE_Effect_Global_OutFlags_2 {
            0x00000000
        },
        AE_Effect_Match_Name {
            PLUGIN_UUID
        },
        AE_Reserved_Info {
            0
        }
    }
};
