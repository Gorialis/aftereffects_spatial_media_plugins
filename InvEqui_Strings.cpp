#include "InvEqui.hpp"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;


TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"Inverse Equirectangular",
	StrID_Description,				"A plugin that takes standard perspective footage with an FOV, yaw and pitch to convert into an equirectangular projection patch.",
	StrID_Layer_Param_Name,         "Source Layer",
	StrID_FOV_Param_Name,			"FOV",
	StrID_Yaw_Param_Name,			"Yaw",
	StrID_Pitch_Param_Name,			"Pitch",
	StrID_KeepLayer_Param_Name,		"Preserve layer",
	StrID_KeepLayer_Param_Desc,		"Keep",
};


char *GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
