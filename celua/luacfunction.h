#pragma once
#include "luaglobal.h"

//要添加到自定义的全局C++函数就放到这里

luaL_Reg myself_cfunction[] = {
	{"luaInit", luaInit},

	{"bef_FormClose", bef_FormClose},
	{"bef_registerClick", bef_registerClick},
	{"bef_ResetClick", bef_ResetClick},
	{"bef_chongcardClick", bef_chongcardClick},
	{"bef_logonClick", bef_logonClick},

	{"register_getcrcClick", register_getcrcClick},
	{"register_registerClick", register_registerClick},

	{"Reset_getcrcClick", Reset_getcrcClick},
	{"Reset_ResetClick", Reset_ResetClick},

	{"chongcard_chongcardClick", chongcard_chongcardClick},

	{"YSM_initClick", YSM_initClick},
	{"YSM_noticeClick", YSM_noticeClick},
	{"YSM_functionalertClick", YSM_functionalertClick},
	{"YSM_hotkeyalertClick", YSM_hotkeyalertClick},
	{"YSM_fixdbvmalertClick", YSM_fixdbvmalertClick},
	{"YSM_FormClose", YSM_FormClose},

	{"YSM_skillnocdChange", YSM_skillnocdChange},
	{"YSM_skillisenoughChange", YSM_skillisenoughChange},
	{"YSM_runnocdChange", YSM_runnocdChange},
	{"YSM_yidongsuduChange", YSM_yidongsuduChange},
	{"YSM_yidingsudu_ddClick", YSM_yidingsudu_ddClick},
	{"YSM_yidingsudu_dClick", YSM_yidingsudu_dClick},
	{"YSM_yidingsudu_uClick", YSM_yidingsudu_uClick},
	{"YSM_yidingsudu_uuClick", YSM_yidingsudu_uuClick},
	{"YSM_gongjisuduChange", YSM_gongjisuduChange},
	{"YSM_gongjisudu_ddClick", YSM_gongjisudu_ddClick},
	{"YSM_gongjisudu_dClick", YSM_gongjisudu_dClick},
	{"YSM_gongjisudu_uClick", YSM_gongjisudu_uClick},
	{"YSM_gongjisudu_uuClick", YSM_gongjisudu_uuClick},
	{"YSM_wudimiaoshaChange", YSM_wudimiaoshaChange},
	{"YSM_miaosha_ddClick", YSM_miaosha_ddClick},
	{"YSM_miaosha_dClick", YSM_miaosha_dClick},
	{"YSM_miaosha_uClick", YSM_miaosha_uClick},
	{"YSM_miaosha_uuClick", YSM_miaosha_uuClick},
	{"YSM_yidongxiguaiChange", YSM_yidongxiguaiChange},
	{"YSM_highlyjumpClick", YSM_highlyjumpClick},
	{"YSM_getAllClick", YSM_getAllClick},
	{"YSM_GlobalSpeedChange", YSM_GlobalSpeedChange},
	//{"YSM_muticlickFChange", YSM_muticlickFChange},
	{NULL, NULL}
};