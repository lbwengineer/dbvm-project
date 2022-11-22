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
const wchar_t* GameName = L"ԭ��";
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
	while (!findGameProc() && count++ < 100)//��ʼ���ȴ�10��
		Sleep(100);
	if (!findGameProc()) {
		MessageBoxA(0, "��⵽��Ϸһֱδ����������Ѿ�������Ϸ������ϵ����Ա������⡣", "��ʼ��ʧ��", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"��⵽��Ϸһֱδ����������Ѿ�������Ϸ������ϵ����Ա������⡣");
		return;
	}
	if (count != 0) {//������Ϸ�ոմ򿪣�����10��ȴ����ؽ�����Ϸ
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"��⵽��Ϸ��һ�δ򿪣����ڵȴ�10��ʹ��Ϸ������ȫ������");
		Sleep(10000);
	}
	lua_setvalue(L, 0, "YSM.printmain.Caption", L"��⵽��Ϸ�Ѵ򿪣����ڶ�ȡ��Ϸ��Ϣ������");
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
		MessageBoxA(0, "��ȡ��ʼ����Ϣʧ�ܣ���ѯ�ʹ���Ա������⡣", "��ʼ��ʧ��", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"��ȡ������Ϣʧ�ܣ���ѯ�ʹ���Ա������⡣");
		return;
	}
	XXXTXXX
	//����Ҫע��dll����Ϸ�Ȼ�����ù����ڴ������Ϣ������������ɺ���������ɱ�־
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
	//*********************ע��֮ǰ���ù����ڴ������֤*******************************
	char* global_message = (char*)createSharedMemory(L"ALLMESSAGEENCRYPTION", 0x1000);
	char all_md5line[0x50]{};
	sprintf(all_md5line, "87f8d729a4b3f7a5%016llx2a9c0821b5e28a0f", GetTickCount64() / 1000);
	auto md5 = MD5(all_md5line).toString();//32���ַ�
	for (int i = 0; i < 32; i++)
		global_message[i] = md5[i];
	if (!lua_function<bool>(L, 0, "injectDLL", strCurPath.GetString())) {
		MessageBoxA(0, "ע��dllʧ�ܣ���ѯ�ʹ���Ա������⡣", "��ʼ��ʧ��", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"ע��dllʧ�ܣ���ѯ�ʹ���Ա������⡣");
		return;
	}
	//���ù����ڴ�
	count = 0;
	while (count++ < 100 && (SHARED = (PINT)openSharedMemory(SharedMemory, 0x1000)) == 0)//�ȴ�10��
		Sleep(100);
	if (SHARED == 0) {
		MessageBoxA(0, "����Զ���ڴ�ʧ�ܣ���ѯ�ʹ���Ա������⡣", "��ʼ��ʧ��", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"����Զ���ڴ�ʧ�ܣ���ѯ�ʹ���Ա������⡣");
		return;
	}
	XXXTXXX
	//�����ڴ潻��
	count = 0;
	while (count++ < 200 && *(SHARED + 0x200) == 0)//�ȴ�20��
		Sleep(100);
	if (*(SHARED + 0x200) != 100) {// == 100 �����ǳɹ�����
		isiniting = false;
		WCHAR msg[0x100] = {};
		if (*(SHARED + 0x200) == 0)
			wcscpy_s(msg, 0x100, L"Զ�̽������г�ʱ������ϵ����Ա������⡣");
		else
			wsprintf(msg, L"Զ�̽�������ʧ�ܣ�������%d����ѯ�ʹ���Ա������⡣\n%s", *(SHARED + 0x200), (PWCHAR)openSharedMemory(L"SharedMessageDialog", 0x1000));
		MessageBoxW(0, msg, L"��ʼ��ʧ��", 0);
		lua_setvalue(L, 0, "YSM.printmain.Caption", msg);
		return;
	}
	XXXTXXX
	if (!initControl(L)) {
		MessageBoxA(0, "����Զ�̽���ʧ�ܣ���ѯ�ʹ���Ա������⡣", "��ʼ��ʧ��", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"����Զ�̽���ʧ�ܣ���ѯ�ʹ���Ա������⡣");
		return;
	}
	XXXTXXX
	lua_setvalue(L, 0, "YSM.printmain.Caption", L"��ʼ���ɹ���");
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
		MessageBoxA(0, "���ڳ�ʼ���У����Եȡ�����", "��ʼ����ʾ", MB_OK);
		return 0;
	}
	if (hasinit) {
		MessageBoxA(0, "�Ѿ���ʼ���ɹ����������ܾͿ����ˡ�", "��ʼ����ʾ", MB_OK);
		return 0;
	}
	setlocale(0, "");
	XXXTXXX
	isiniting = true;
#ifdef USEDBVM
	if (findGameProc()) {//�����Ѿ�����Ϸ����ʼ��������򣬵����ʼ�˳���ʼ��
		char confirm[0x100] = {};
		sprintf(confirm, "���Ѿ�����%S��ȷ��Ҫ������ʼ��������ܵ��µ���������", GameName);
		if (MessageBoxA(0, confirm, "������ʼ������", MB_OKCANCEL) != IDOK) {
			isiniting = false;
			lua_setvalue(L, 0, "YSM.printmain.Caption", L"��ر���Ϸ���ٳ�ʼ����");
			return 0;
		}
	}
	XXXTXXX
	auto ok = lua_function<bool>(L, 0, "dbvm_initialize", true, L"��ȷ�����Ѿ������˱��ݹ�����");
	if (ok == false) {
		MessageBoxA(0, "��ĵ��Կ��ܲ�֧��cpu���⻯��������ο���cpu���⻯������������", "��ʼ��ʧ��", 0);
		isiniting = false;
		lua_setvalue(L, 0, "YSM.printmain.Caption", L"��ʼ��ʧ�ܣ��뿪��cpu���⻯��");
		return 0;
	}
#endif
	lua_setvalue(L, 0, "YSM.printmain.Caption", L"��ʼ���У����ֶ�����Ϸ������");
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
		"�ڴ���Ϸǰ�����ʼ���������ʼ�������Ϸ�ȴ���ʼ����ɡ������ʼ��������10���ڴ���Ϸ�������ʼ������һ�����ʵ��µ��������������ڵ����ʼ��֮ǰһ��Ҫ���ñ��ݹ����������ڳ�ʼ��֮ǰ������һ�ε��ԡ�",
#else
		"ʲôʱ������ʼ�������ԣ����ǽ����ڽ�����Ϸǰ�����ʼ���������ʼ��������10���ڴ���Ϸ��",
#endif
		"��ʼ��˵��", 0);
	return 0;
}
int YSM_functionalertClick(lua_State* L)
{
	MessageBoxA(0,
#ifdef USERDBVM
		"�͵�ʹ�þͲ����š������ط⡣���Դ��������",
#else
		"���ܷ�ţ���ʹ��С����ˣ��",
#endif
		"����˵��", 0);
	return 0;
}
int YSM_hotkeyalertClick(lua_State* L)
{
	MessageBoxA(0,
		"��ctrl + F���������ƶ��ٶ�\n"
		"��alt + F�����ر��ƶ��ٶ�\n"
		"��ctrl + G���������޵���ɱ\n"
		"��alt + G�����ر��޵���ɱ\n"
		"��ctrl + H������������������\n"
		"��alt + H�����رո���������\n"
		"��ctrl + space��������100��\n"
		"��V����ȫ������\n"
		"��caps lock����������ر�ȫ�ּ���\n",
		"�ȼ�˵��", 0);
	return 0;
}
int YSM_fixdbvmalertClick(lua_State* L)
{
	MessageBoxA(0,
		"WIN+R��һ�����д��ڣ�����msconfig���س�����ʱ��ϵͳ���ô��ڣ�ѡ���������߼����ã������������ĵ���󣬵�ȷ����Ӧ�ã�������������Ӧ�þͺ��ˡ�",
		"��������˵��", 0);
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