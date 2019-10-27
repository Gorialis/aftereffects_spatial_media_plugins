#pragma once

#ifndef GIT_REV_COUNT
#define GIT_REV_COUNT 0
#endif

#define MAJOR_VERSION        2
#define MINOR_VERSION        0
#define BUG_VERSION          0
#define STAGE_VERSION        0  // PF_Stage_DEVELOP
#define BUILD_VERSION        1

#define COMPOUND_VERSION     MAJOR_VERSION * 524288 + MINOR_VERSION * 32768 + BUG_VERSION * 2048 + STAGE_VERSION * 512 + BUILD_VERSION

#define PLUGIN_CATEGORY      "Devon's Spatial Media Plugins"
