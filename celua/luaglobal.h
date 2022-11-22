#pragma once
#include "../lua53/src/lua.hpp"

#ifdef __cplusplus
extern "C" {
#endif
	//lua��ʼ��ʱ����
	void luaInit(lua_State* L);
	//�����¼ѡ��
	int bef_FormClose(lua_State* L);
	int bef_registerClick(lua_State* L);
	int bef_ResetClick(lua_State* L);
	int bef_chongcardClick(lua_State* L);
	int bef_logonClick(lua_State* L);//��¼��ť���
	//����ע��ѡ��
	int register_getcrcClick(lua_State* L);
	int register_registerClick(lua_State* L);
	//������������ѡ��
	int Reset_getcrcClick(lua_State* L);
	int Reset_ResetClick(lua_State* L);
	//�����ֵ����ѡ��
	int chongcard_chongcardClick(lua_State* L);
	//���ܺ���
	int YSM_initClick(lua_State* L);
	int YSM_noticeClick(lua_State* L);
	int YSM_functionalertClick(lua_State* L);
	int YSM_hotkeyalertClick(lua_State* L);
	int YSM_fixdbvmalertClick(lua_State* L);
	int YSM_FormClose(lua_State* L);
	//ʵ����Ϸ�Ĺ���
	int YSM_skillnocdChange(lua_State* L);
	int YSM_skillisenoughChange(lua_State* L);
	int YSM_runnocdChange(lua_State* L);
	int YSM_yidongsuduChange(lua_State* L);
	int YSM_yidingsudu_ddClick(lua_State* L);
	int YSM_yidingsudu_dClick(lua_State* L);
	int YSM_yidingsudu_uClick(lua_State* L);
	int YSM_yidingsudu_uuClick(lua_State* L);
	int YSM_gongjisuduChange(lua_State* L);
	int YSM_gongjisudu_ddClick(lua_State* L);
	int YSM_gongjisudu_dClick(lua_State* L);
	int YSM_gongjisudu_uClick(lua_State* L);
	int YSM_gongjisudu_uuClick(lua_State* L);
	int YSM_wudimiaoshaChange(lua_State* L);
	int YSM_miaosha_ddClick(lua_State* L);
	int YSM_miaosha_dClick(lua_State* L);
	int YSM_miaosha_uClick(lua_State* L);
	int YSM_miaosha_uuClick(lua_State* L);
	int YSM_yidongxiguaiChange(lua_State* L);
	int YSM_highlyjumpClick(lua_State* L);
	int YSM_getAllClick(lua_State* L);
	int YSM_GlobalSpeedChange(lua_State* L);
	//int YSM_muticlickFChange(lua_State* L);

#ifdef __cplusplus
}
#endif