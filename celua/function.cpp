#include "../lua53/src/lua.hpp"
#include "../Inject/luacall.hpp"
#include "../tool/md5.h"
#include "../tool/JsonObject.h"
#include "../tool/HttpConnect.h"
#include <map>
#include <atlstr.h>
#include "globalvalue.h"
#include "../Inject/inject.h"
#include "luaglobal.h"
using namespace std;

const wchar_t* ExecName[] = { L"YuanShen.exe", L"GenshinImpact.exe", 0 };
const wchar_t* GameName = L"原神";
const wchar_t* InjectDll = L"YuanShen.dll";
const wchar_t* SharedMemory = L"AllGoOutOfMyHome";

static bool isiniting = false;
bool hasinit = false;

static inline bool findGameProc() {
	const wchar_t** p = ExecName;
	for (; *p != 0; p++) {
		if (getpid(*p) != 0)
			return true;
	}
	return false;
}

static void initfunction(lua_State* L)
{
	size_t count = 0;
	while (!findGameProc() && count++ < 100)//初始化等待10秒
		Sleep(100);
	if (!findGameProc()) {
		MessageBoxA(0, "检测到游戏一直未开启，如果已经打开了游戏，请联系管理员解决问题。", "初始化失败", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"检测到游戏一直未开启，如果已经打开了游戏，请联系管理员解决问题。");
		return;
	}
	if (count != 0) {//发现游戏刚刚打开，休眠10秒等待加载进入游戏
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"检测到游戏第一次打开，现在等待10秒使游戏加载完全。。。");
		Sleep(10000);
	}
	lua_setvalue(L, 0, "YSM.printmain.Caption", L"检测到游戏已打开，正在读取游戏信息。。。");
	XXXTXXX
	{
		const wchar_t** p = ExecName;
		for (; *p != 0; p++) {
			if (getpid(*p) != 0) {
				lua_vfunction(L, 0, "openProcess", *p);
				break;
			}
		}
	}
	if (0 == lua_function<int>(L, 0, "getOpenedProcessID")) {
		MessageBoxA(0, "读取初始化信息失败，请询问管理员解决问题。", "初始化失败", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"读取进程信息失败，请询问管理员解决问题。");
		return;
	}
	XXXTXXX
	//这里要注入dll到游戏里，然后设置共享内存进行信息交互，交互完成后再设置完成标志
	CString strCurPath;
	GetModuleFileNameW(NULL, strCurPath.GetBuffer(MAX_PATH), MAX_PATH);
	strCurPath.ReleaseBuffer();
	strCurPath = strCurPath.Left(strCurPath.ReverseFind(L'\\') + 1) + InjectDll;
	if (GetFileAttributesW(strCurPath) == INVALID_FILE_ATTRIBUTES)
#ifdef USEDBVM
		strCurPath = L"D:\\WindowsProject\\YuanShen\\x64\\UseDbvm\\YuanShen.dll";
#else
		strCurPath = L"D:\\WindowsProject\\YuanShen\\x64\\OnlyDbk\\YuanShen.dll";
#endif
	//strCurPath = L"D:\\WindowsProject\\YuanShen\\x64\\OnlyDbk\\YuanShen.dll";
	//*********************注入之前设置共享内存完成验证*******************************
	char* global_message = (char*)createSharedMemory(L"ALLMESSAGEENCRYPTION", 0x1000);
	char all_md5line[0x50]{};
	sprintf(all_md5line, "87f8d729a4b3f7a5%016llx2a9c0821b5e28a0f", GetTickCount64() / 1000);
	auto md5 = MD5(all_md5line).toString();//32个字符
	for (int i = 0; i < 32; i++)
		global_message[i] = md5[i];
	if (!lua_function<bool>(L, 0, "injectDLL", strCurPath.GetString())) {
		MessageBoxA(0, "注入dll失败，请询问管理员解决问题。", "初始化失败", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"注入dll失败，请询问管理员解决问题。");
		return;
	}
	//设置共享内存
	count = 0;
	while (count++ < 100 && (SHARED = (PINT)openSharedMemory(SharedMemory, 0x1000)) == 0)//等待10秒
		Sleep(100);
	if (SHARED == 0) {
		MessageBoxA(0, "设置远程内存失败，请询问管理员解决问题。", "初始化失败", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"设置远程内存失败，请询问管理员解决问题。");
		return;
	}
	XXXTXXX
	//进行内存交互
	count = 0;
	while (count++ < 200 && *(SHARED + 0x200) == 0)//等待20秒
		Sleep(100);
	if (*(SHARED + 0x200) != 100) {// == 100 才算是成功运行
		isiniting = false;
		WCHAR msg[0x100] = {};
		if (*(SHARED + 0x200) == 0)
			wcscpy_s(msg, 0x100, L"远程进程运行超时，请联系管理员解决问题。");
		else
			wsprintf(msg, L"远程进程运行失败，错误码%d，请询问管理员解决问题。\n%s", *(SHARED + 0x200), (PWCHAR)openSharedMemory(L"SharedMessageDialog", 0x1000));
		MessageBoxW(0, msg, L"初始化失败", 0);
		lua_setvalue(L, 0, "YSM.printmain.Caption", msg);
		return;
	}
	XXXTXXX
	if (!initControl(L)) {
		MessageBoxA(0, "设置远程进程失败，请询问管理员解决问题。", "初始化失败", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"设置远程进程失败，请询问管理员解决问题。");
		return;
	}
	XXXTXXX
	lua_setvalue(L, 0, "YSM.printmain.Caption", L"初始化成功。");
	isiniting = false;
	hasinit = true;
	XXXTXXX
	extern void HotKeySet(lua_State*);
	HotKeySet(L);
}


int YSM_initClick(lua_State* L)
{
	XXXTXXX
	if (isiniting) {
		MessageBoxA(0, "正在初始化中，请稍等。。。", "初始化提示", MB_OK);
		return 0;
	}
	if (hasinit) {
		MessageBoxA(0, "已经初始化成功，开启功能就可以了。", "初始化提示", MB_OK);
		return 0;
	}
	setlocale(0, "");
	XXXTXXX
	isiniting = true;
#ifdef USEDBVM
	if (findGameProc()) {//发现已经打开游戏，开始弹出警告框，点击否开始退出初始化
		char confirm[0x100] = {};
		sprintf(confirm, "你已经打开了%S，确定要继续初始化吗？这可能导致电脑蓝屏。", GameName);
		if (MessageBoxA(0, confirm, "继续初始化警告", MB_OKCANCEL) != IDOK) {
			isiniting = false;
			lua_setvalue(L, 0, "YSM.printmain.Caption", L"请关闭游戏后再初始化。");
			return 0;
		}
	}
	XXXTXXX
	auto ok = lua_function<bool>(L, 0, "dbvm_initialize", true, L"请确保你已经做好了备份工作！");
	if (ok == false) {
		MessageBoxA(0, "你的电脑可能不支持cpu虚拟化，关于如何开启cpu虚拟化请上网搜索。", "初始化失败", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"初始化失败，请开启cpu虚拟化。");
		return 0;
	}
#endif
	lua_setvalue(L, 0, "YSM.printmain.Caption", L"初始化中，请手动打开游戏。。。");
	XXXTXXX
	lua_function<int>(L, 0, "dbk_useKernelmodeOpenProcess");
	lua_function<int>(L, 0, "dbk_useKernelmodeProcessMemoryAccess");
	lua_function<int>(L, 0, "activateProtection");
	lua_function<int>(L, 0, "enableDRM");
	XXXTXXX
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)initfunction, L, 0, 0);
	return 0;
}
int YSM_noticeClick(lua_State* L)
{
	MessageBoxA(0,
#ifdef USERDBVM
		"在打开游戏前点击初始化，点击初始化后打开游戏等待初始化完成。点击初始化后请在10秒内打开游戏。点击初始化会有一定概率导致电脑蓝屏，所以在点击初始化之前一定要做好备份工作。建议在初始化之前先重启一次电脑。",
#else
		"什么时候点击初始化都可以，但是建议在进入游戏前点击初始化。点击初始化后请在10秒内打开游戏。",
#endif
		"初始化说明", 0);
	return 0;
}
int YSM_functionalertClick(lua_State* L)
{
	MessageBoxA(0,
#ifdef USERDBVM
		"低调使用就不会封号。联机必封。可以打深境螺旋。",
#else
		"可能封号，请使用小号玩耍。",
#endif
		"防封说明", 0);
	return 0;
}
int YSM_hotkeyalertClick(lua_State* L)
{
	MessageBoxA(0,
		"【ctrl + F】：开启移动速度\n"
		"【alt + F】：关闭移动速度\n"
		"【ctrl + G】：开启无敌秒杀\n"
		"【alt + G】：关闭无敌秒杀\n"
		"【ctrl + H】：开启高跳和吸人\n"
		"【alt + H】：关闭高跳和吸人\n"
		"【ctrl + space】：高跳100米\n"
		"【V】：全屏吸人\n"
		"【caps lock】：开启或关闭全局加速\n",
		"热键说明", 0);
	return 0;
}
int YSM_fixdbvmalertClick(lua_State* L)
{
	MessageBoxA(0,
		"WIN+R打开一个运行窗口，输入msconfig，回车，这时打开系统配置窗口，选择引导，高级设置，处理器个数改到最大，点确定，应用，重启。重启后应该就好了。",
		"蓝屏问题说明", 0);
	return 0;
}

int YSM_FormClose(lua_State* L)
{
	extern void closeAllFunction(lua_State*);
	closeAllFunction(L);
	lua_vfunction(L, 0, "closeCE");
	lua_pushinteger(L, 2);
	return 1;
}