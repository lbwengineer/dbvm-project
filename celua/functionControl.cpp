#include "../lua53/src/lua.hpp"
#include "../Inject/luacall.hpp"
#include "../Inject/inject.h"
#include "../tool/md5.h"
#include "../tool/JsonObject.h"
#include "../tool/HttpConnect.h"
#include "globalvalue.h"
#include "luaglobal.h"
#include <set>

using namespace std;
map<std::string, code_phy> name_hack;

PINT SHARED;
PINT cd_off;
PINT skill_off;
PINT runcd_off;
PINT movespeed_off;
PFLOAT movespeed_ratio;
PINT attackspeed_off;
PFLOAT attackspeed_ratio;
PINT wudimiaosha_off;
PINT wudimiaosha_ratio;
PINT yidongxiguai_total;
PINT gaotiao_click;
PINT xiguai_click;
PINT speedDialog_off;

static inline INT32 dbvm_cloak(lua_State* L, INT64 physical, INT64 vir_addr, bool is_active)
{
	static map<INT64, INT32> active;
	if (is_active) {
		if (active[physical]++ == 0) {
#ifdef USEDBVM
			if (lua_function<bool>(L, 0, "dbvm_cloak_activate", physical, vir_addr))
				return 1;
			active[physical]--;
			return -1;
#endif
		}
	}
	else {
		if (active[physical] == 0) {
			return 0;
		}
		if (--active[physical] == 0) {
#ifdef USEDBVM
			if (lua_function<bool>(L, 0, "dbvm_cloak_deactivate", physical))
				return 1;
			active[physical]++;
			return -1;
#endif
		}
	}
	return 0;
}
//仅仅只是测试和准备，不做实际的hook
static inline bool HOOKSTART(lua_State* L, std::string func_name, PVOID base_addr, PVOID new_addr, PVOID map_addr, PCSTR original, size_t code_len)
{
	char saveline[0x100];
	sprintf(saveline, "%p:\njmp %p\n%p:\njmp %p\n%p:\n%s\njmp %p",
		base_addr, (PBYTE)map_addr + 0x800, (PBYTE)map_addr + 0x800, new_addr, map_addr, original, (PBYTE)base_addr + code_len);
	
	//Eile ff("E:\\aaa.txt");
	//ff.write("%s\n\n============", saveline);

	auto start = (INT64)base_addr & (INT64)~0xfff;
	lua_lfunction(L, 0, "getMemoryViewForm");
	lua_setvalue(L, -1, "DisassemblerView.TopAddress", start);
	auto physicalAddress = lua_function<INT64>(L, 0, "dbk_getPhysicalAddress", start);
	if (physicalAddress == 0 || !lua_function<bool>(L, 0, "autoAssembleCheck", saveline)) {
		return false;
	}
	char points[0x50];
	sprintf(points, "%p:\n%s", base_addr, original);
	name_hack.emplace(func_name, code_phy{ saveline, points, 0, physicalAddress, start });
	return true;
}
//仅仅只是测试和准备，不做实际的hook
static inline bool HOOKMID(lua_State* L, std::string func_name, PVOID inline_addr, PVOID map_addr, PVOID call_addr, PCSTR call_before, PCSTR call_after1, PCSTR call_after2, PCSTR original, size_t code_len)
{
	char saveline[0x400];
	const char* pushline = "sub rsp,d0\nmovups [rsp],xmm0\nmovups [rsp+10],xmm1\nmovups [rsp+20],xmm2\nmovups [rsp+30],xmm3\nmovups [rsp+40],xmm4\nmovups [rsp+50],xmm5\nmov [rsp+60],rax\nmov [rsp+68],rbx\nmov [rsp+70],rcx\nmov [rsp+78],rdx\nmov [rsp+80],rsi\nmov [rsp+88],rdi\nmov [rsp+90],r8\nmov [rsp+98],r9\nmov [rsp+a0],r10\nmov [rsp+a8],r11\nmov [rsp+b0],r12\nmov [rsp+b8],r13\nmov [rsp+c0],r14\nmov [rsp+c8],r15\n";
	const char* popline = "movups xmm0,[rsp]\nmovups xmm1,[rsp+10]\nmovups xmm2,[rsp+20]\nmovups xmm3,[rsp+30]\nmovups xmm4,[rsp+40]\nmovups xmm5,[rsp+50]\nmov rax,[rsp+60]\nmov rbx,[rsp+68]\nmov rcx,[rsp+70]\nmov rdx,[rsp+78]\nmov rsi,[rsp+80]\nmov rdi,[rsp+88]\nmov r8,[rsp+90]\nmov r9,[rsp+98]\nmov r10,[rsp+a0]\nmov r11,[rsp+a8]\nmov r10,[rsp+b0]\nmov r11,[rsp+b8]\nmov r10,[rsp+c0]\nmov r11,[rsp+c8]\nadd rsp,d0\n";
	sprintf(saveline, "%p:\njmp %p\n%p:\n%s\n%s\ncall %p\n%s\n%s\n%s\njmp %p",
		inline_addr, map_addr, map_addr, pushline, call_before, call_addr, call_after1, popline, call_after2, (PBYTE)inline_addr + code_len);
	
	//Eile ff("E:\\aaa.txt");
	//ff.write("%s\n\n", saveline);

	auto start = (INT64)inline_addr & (INT64)~0xfff;
	lua_lfunction(L, 0, "getMemoryViewForm");
	lua_setvalue(L, -1, "DisassemblerView.TopAddress", start);
	auto physicalAddress = lua_function<INT64>(L, 0, "dbk_getPhysicalAddress", start);
	if (physicalAddress == 0 || !lua_function<bool>(L, 0, "autoAssembleCheck", saveline)) {
		return false;
	}
	char points[0x50];
	sprintf(points, "%p:\n%s", inline_addr, original);
	name_hack.emplace(func_name, code_phy{ saveline, points, 0,physicalAddress, start });
	return true;
}

//必须先调用前面两个才能hook
static inline bool DOHOOK(lua_State* L, std::string func_name)
{
	auto find = name_hack.find(func_name);
	if (find == name_hack.end()) {
		MessageBoxA(0, (func_name + "未初始化").c_str(), "开启功能失败", 0);
		return false;
	}
	auto dbvm = dbvm_cloak(L, (*find).second.phy, (*find).second.vir, true);
	if (dbvm == -1) {
		MessageBoxA(0, (func_name + "开启失败，此功能所在的内存页分配内存失败").c_str(), "开启功能失败", 0);
		return false;
	}
	if (dbvm >= 0 && ((*find).second.mod++ == 0 && !lua_function<bool>(L, 0, "autoAssemble", (*find).second.code))) {
		MessageBoxA(0, (func_name + "开启失败").c_str(), "开启功能失败", 0);
		(*find).second.mod--;
		return false;
	}
	return true;
}
//必须先调用前面两个才能dehook
static inline bool DEHOOK(lua_State* L, std::string func_name)
{
	auto find = name_hack.find(func_name);
	if (find == name_hack.end()) {
		MessageBoxA(0, (func_name + "未初始化").c_str(), "关闭功能失败", 0);
		return false;
	}
	auto dbvm = dbvm_cloak(L, (*find).second.phy, (*find).second.vir, false);
	if (dbvm == -1) {
		MessageBoxA(0, (func_name + "关闭失败，此功能所在的内存页分配内存失败").c_str(), "关闭功能失败", 0);
		return false;
	}
	else if (dbvm == 0) {
		if ((*find).second.mod == 1 && !lua_function<bool>(L, 0, "autoAssemble", (*find).second.original)) {
			MessageBoxA(0, (func_name + "关闭失败").c_str(), "关闭功能失败", 0);
			return false;
		}
		else if ((*find).second.mod != 0)
			(*find).second.mod--;
	}
	else if (dbvm == 1)
		(*find).second.mod = 0;
	return true;
}

bool initControl(lua_State* L)
{
	cd_off = (PINT)(SHARED + 0x101);
	skill_off = (PINT)(SHARED + 0x102);
	runcd_off = (PINT)(SHARED + 0x103);
	movespeed_off = (PINT)(SHARED + 0x104);
	movespeed_ratio = (PFLOAT)(SHARED + 0x105);
	attackspeed_off = (PINT)(SHARED + 0x106);
	attackspeed_ratio = (PFLOAT)(SHARED + 0x107);
	wudimiaosha_off = (PINT)(SHARED + 0x108);
	wudimiaosha_ratio = (PINT)(SHARED + 0x109);
	yidongxiguai_total = (PINT)(SHARED + 0x110);
	gaotiao_click = (PINT)(SHARED + 0x111);
	xiguai_click = (PINT)(SHARED + 0x112);
	speedDialog_off = (PINT)(SHARED + 0x120);


	PBYTE* line = (PBYTE* )SHARED;
	char jmpcode[0x60];
	UINT64 physicalAddress = 0;
	
	//技能无cd
	if (!HOOKSTART(L, "技能无cd",  * line, *(line + 7), *(line + 8), "mov [rsp+08],rbx", 5))
		return false;

	//无限大招
	if (!HOOKSTART(L, "无限大招", *(line + 1), *(line + 9), *(line + 10), "push rbx\nsub rsp,40", 6))
		return false;

	//无限耐力
	if (!HOOKSTART(L, "无限耐力", *(line + 2), *(line + 11), *(line + 12), "mov [rsp+08],rbx", 5))
		return false;

	//无敌秒杀
	if (!HOOKSTART(L, "无敌秒杀", *(line + 3), *(line + 13), *(line + 14), "mov [rsp+18],rbp", 5))
		return false;

	//速度
	if (!HOOKMID(L, "速度", *(line + 4), *(line + 15), *(line + 16), "movss [rsp-10],xmm6\nlea rdx,[rsp-10]", "movss xmm6,[rsp-10]", "", "movss xmm6,[rcx+r15+38]", 7))
		return false;

	//位置初始化
	if (!HOOKMID(L, "位置初始化", *(line + 5), *(line + 17), *(line + 18), "mov rdx,rbx", "", "movss xmm9,[rbx+0C]", "movss xmm9,[rbx+0C]", 6))
		return false;

	//高跳吸怪
	if (!HOOKMID(L, "高跳吸怪", *(line + 6), *(line + 19), *(line + 20),
		"movss [rbp+10],xmm2\nmovss [rbp+14],xmm3\nmovss [rbp+18],xmm4\nmov rcx,rbp", "", "", "movss [rbp+10],xmm2\nmovss [rbp+14],xmm3\nmovss [rbp+18],xmm4", 15))
		return false;

	return true;
}

void closeAllFunction(lua_State* L) {
	if (!hasinit)
		return;
	for (auto& get : name_hack) {
		auto fucks = get.second.mod;
		while (fucks-- > 0)
			DEHOOK(L, get.first);
	}
	*speedDialog_off = 0;
}

//全局加速
int YSM_GlobalSpeedChange(lua_State* L) {
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*speedDialog_off = lua_getvalue<bool>(L, -1, "Checked");
}

/*
技能无cd
*/
int YSM_skillnocdChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*cd_off = lua_getvalue<bool>(L, -1, "Checked");
	if (*cd_off)
		DOHOOK(L, "技能无cd");
	else
		DEHOOK(L, "技能无cd");
	return 0;
}
/*
无限大招
*/
int YSM_skillisenoughChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*skill_off = lua_getvalue<bool>(L, -1, "Checked");
	if (*skill_off)
		DOHOOK(L, "无限大招");
	else
		DEHOOK(L, "无限大招");
	return 0;
}
/*
无限耐力
*/
int YSM_runnocdChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*runcd_off = lua_getvalue<bool>(L, -1, "Checked");
	if (*runcd_off)
		DOHOOK(L, "无限耐力");
	else
		DEHOOK(L, "无限耐力");
	return 0;
}
/*
移动速度
*/
int YSM_yidongsuduChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*movespeed_off = lua_getvalue<bool>(L, -1, "Checked");
	if (*movespeed_off)
		DOHOOK(L, "速度");
	else
		DEHOOK(L, "速度");
	*movespeed_ratio = strtof(lua_getvalue<std::string>(L, 0, "YSM.yidingsudu_edit.Text").c_str(), 0);
	return 0;
}
int YSM_yidingsudu_ddClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.yidingsudu_edit.Text").c_str(), 0);
	*movespeed_ratio = speed - 1;
	lua_setvalue(L, 0, "YSM.yidingsudu_edit.Text", *movespeed_ratio);
	return 0;
}
int YSM_yidingsudu_dClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.yidingsudu_edit.Text").c_str(), 0);
	*movespeed_ratio = speed - 0.1;
	lua_setvalue(L, 0, "YSM.yidingsudu_edit.Text", *movespeed_ratio);
	return 0;
}
int YSM_yidingsudu_uClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.yidingsudu_edit.Text").c_str(), 0);
	*movespeed_ratio = speed + 0.1;
	lua_setvalue(L, 0, "YSM.yidingsudu_edit.Text", *movespeed_ratio);
	return 0;
}
int YSM_yidingsudu_uuClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.yidingsudu_edit.Text").c_str(), 0);
	*movespeed_ratio = speed + 1;
	lua_setvalue(L, 0, "YSM.yidingsudu_edit.Text", *movespeed_ratio);
	return 0;
}

/*
攻击速度
*/
int YSM_gongjisuduChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*attackspeed_off = lua_getvalue<bool>(L, -1, "Checked");
	if (*attackspeed_off)
		DOHOOK(L, "速度");
	else
		DEHOOK(L, "速度");
	*attackspeed_ratio = strtof(lua_getvalue<std::string>(L, 0, "YSM.gongjisudu_edit.Text").c_str(), 0);
	return 0;
}
int YSM_gongjisudu_ddClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.gongjisudu_edit.Text").c_str(), 0);
	*attackspeed_ratio = speed - 1;
	lua_setvalue(L, 0, "YSM.gongjisudu_edit.Text", *attackspeed_ratio);
	return 0;
}
int YSM_gongjisudu_dClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.gongjisudu_edit.Text").c_str(), 0);
	*attackspeed_ratio = speed - 0.1;
	lua_setvalue(L, 0, "YSM.gongjisudu_edit.Text", *attackspeed_ratio);
	return 0;
}
int YSM_gongjisudu_uClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.gongjisudu_edit.Text").c_str(), 0);
	*attackspeed_ratio = speed + 0.1;
	lua_setvalue(L, 0, "YSM.gongjisudu_edit.Text", *attackspeed_ratio);
	return 0;
}
int YSM_gongjisudu_uuClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto speed = strtof(lua_getvalue<std::string>(L, 0, "YSM.gongjisudu_edit.Text").c_str(), 0);
	*attackspeed_ratio = speed + 1;
	lua_setvalue(L, 0, "YSM.gongjisudu_edit.Text", *attackspeed_ratio);
	return 0;
}
/*
无敌秒杀
*/
int YSM_wudimiaoshaChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*wudimiaosha_off = lua_getvalue<bool>(L, -1, "Checked");
	if (*wudimiaosha_off)
		DOHOOK(L, "无敌秒杀");
	else
		DEHOOK(L, "无敌秒杀");
	*wudimiaosha_ratio = strtof(lua_getvalue<std::string>(L, 0, "YSM.miaosha_edit.Text").c_str(), 0);
	return 0;
}
int YSM_miaosha_ddClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto ratio = strtol(lua_getvalue<std::string>(L, 0, "YSM.miaosha_edit.Text").c_str(), 0, 10);
	*wudimiaosha_ratio = ratio - 10;
	lua_setvalue(L, 0, "YSM.miaosha_edit.Text", *wudimiaosha_ratio);
	return 0;
}
int YSM_miaosha_dClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto ratio = strtol(lua_getvalue<std::string>(L, 0, "YSM.miaosha_edit.Text").c_str(), 0, 10);
	*wudimiaosha_ratio = ratio - 1;
	lua_setvalue(L, 0, "YSM.miaosha_edit.Text", *wudimiaosha_ratio);
	return 0;
}
int YSM_miaosha_uClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto ratio = strtol(lua_getvalue<std::string>(L, 0, "YSM.miaosha_edit.Text").c_str(), 0, 10);
	*wudimiaosha_ratio = ratio + 1;
	lua_setvalue(L, 0, "YSM.miaosha_edit.Text", *wudimiaosha_ratio);
	return 0;
}
int YSM_miaosha_uuClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	auto ratio = strtol(lua_getvalue<std::string>(L, 0, "YSM.miaosha_edit.Text").c_str(), 0, 10);
	*wudimiaosha_ratio = ratio + 10;
	lua_setvalue(L, 0, "YSM.miaosha_edit.Text", *wudimiaosha_ratio);
	return 0;
}

//bool threadF_off;
//void threadF() {
//	while (threadF_off) {
//		keybd_event('F', 0, 0, 0);
//		Sleep(1);
//		keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
//		Sleep(1);
//	}
//}

//int YSM_muticlickFChange(lua_State* L)
//{
//	if (!hasinit) {
//		lua_setvalue(L, -1, "Checked", false);
//		return 0;
//	}
//	if (lua_getvalue<bool>(L, -1, "Checked")) {
//		threadF_off = true;
//		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)threadF, 0, 0, 0);
//	}
//	else {
//		threadF_off = false;
//	}
//}

/*
吸怪吸人高跳
*/
int YSM_yidongxiguaiChange(lua_State* L)
{
	if (!hasinit) {
		lua_setvalue(L, -1, "Checked", false);
		return 0;
	}
	*yidongxiguai_total = lua_getvalue<bool>(L, -1, "Checked");
	if (*yidongxiguai_total)
		DOHOOK(L, "位置初始化"), DOHOOK(L, "高跳吸怪");
	else
		DEHOOK(L, "位置初始化"), DEHOOK(L, "高跳吸怪");
	return 0;
}

int YSM_highlyjumpClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	*gaotiao_click = 1;
	return 0;
}

int YSM_getAllClick(lua_State* L)
{
	if (!hasinit) {
		return 0;
	}
	*xiguai_click = 1;
	return 0;
}

////【ctrl + C】：开启自动拾取
//int clickF_on(lua_State* L) {
//	lua_setvalue(L, 0, "YSM.muticlickF.Checked", true);
//	threadF_off = true;
//	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)threadF, 0, 0, 0);
//	return 0;
//}
////【ctrl + C】：关闭自动拾取
//int clickF_off(lua_State* L) {
//	lua_setvalue(L, 0, "YSM.muticlickF.Checked", false);
//	threadF_off = false;
//	return 0;
//}
//【ctrl + F】：开启移动速度
int yidongsudu_on(lua_State* L) {
	lua_setvalue(L, 0, "YSM.yidongsudu.Checked", true);
	*movespeed_off = 1;
	return 0;
}
//【alt + F】：关闭移动速度
int yidongsudu_off(lua_State* L) {
	lua_setvalue(L, 0, "YSM.yidongsudu.Checked", false);
	*movespeed_off = 0;
	return 0;
}
//【ctrl + G】：开启无敌秒杀
int widumiaosha_on(lua_State* L) {
	lua_setvalue(L, 0, "YSM.wudimiaosha.Checked", true);
	*wudimiaosha_off = 1;
	return 0;
}
//【alt + G】：关闭无敌秒杀
int widumiaosha_off(lua_State* L) {
	lua_setvalue(L, 0, "YSM.wudimiaosha.Checked", false);
	*wudimiaosha_off = 0;
	return 0;
}
//【ctrl + H】：开启高跳和吸人
int gaotiaoxiren_on(lua_State* L) {
	lua_setvalue(L, 0, "YSM.yidongxiguai.Checked", true);
	*yidongxiguai_total = 1;
	return 0;
}
//【alt + H】：关闭高跳和吸人
int gaotiaoxiren_off(lua_State* L) {
	lua_setvalue(L, 0, "YSM.yidongxiguai.Checked", false);
	*yidongxiguai_total = 0;
	return 0;
}
//【ctrl + space】：高跳100米
int gaotiao_on(lua_State* L) {
	*gaotiao_click = 1;
	return 0;
}
//【V】：全屏吸人
int xiguai_on(lua_State* L) {
	*xiguai_click = 1;
	return 0;
}
//【tab】：开启或关闭全局加速
int globalspeed_on(lua_State* L) {
	lua_setvalue(L, 0, "YSM.GlobalSpeed.Checked", !lua_getvalue<bool>(L, 0, "YSM.GlobalSpeed.Checked"));
	return 0;
}

//ctrl		17
//alt		18

void HotKeySet(lua_State* L) {
	//lua_register(L, "clickF_on", clickF_on);
	//lua_register(L, "clickF_off", clickF_off);
	lua_register(L, "yidongsudu_on", yidongsudu_on);
	lua_register(L, "yidongsudu_off", yidongsudu_off);
	lua_register(L, "widumiaosha_on", widumiaosha_on);
	lua_register(L, "widumiaosha_off", widumiaosha_off);
	lua_register(L, "gaotiaoxiren_on", gaotiaoxiren_on);
	lua_register(L, "gaotiaoxiren_off", gaotiaoxiren_off);
	lua_register(L, "gaotiao_on", gaotiao_on);
	lua_register(L, "xiguai_on", xiguai_on);
	lua_register(L, "globalspeed_on", globalspeed_on);
	const char* LuaString = "--createHotkey(clickF_on, 17, 67)\n"
		"--createHotkey(clickF_off, 18, 67)\n"
		"createHotkey(yidongsudu_on, 17, 70)\n"
		"createHotkey(yidongsudu_off, 18, 70)\n"
		"createHotkey(widumiaosha_on, 17, 71)\n"
		"createHotkey(widumiaosha_off, 18, 71)\n"
		"createHotkey(gaotiaoxiren_on, 17, 72)\n"
		"createHotkey(gaotiaoxiren_off, 18, 72)\n"
		"createHotkey(gaotiao_on, 17, 32)\n"
		"createHotkey(xiguai_on, 86)\n"
		"createHotkey(globalspeed_on, 20)\n";
	luaL_dostring(L, LuaString);
}
