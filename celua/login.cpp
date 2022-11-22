#include <WinSock2.h>
#include "../lua53/src/lua.hpp"
#include "../Inject/luacall.hpp"
#include "../tool/md5.h"
#include "../tool/JsonObject.h"
#include "connect.h"
#include "globalvalue.h"
#include "luaglobal.h"

std::string TOKEN;
const char* accountPath = "\\HnB4Xc7Rf9Yc4Xs6VrO0YbF7iLf3Z5Vh7X1Km";
int SUCCESS;

using namespace std;

int bef_FormClose(lua_State* L)
{
	lua_function<int>(L, 0, "closeCE");
	lua_pushinteger(L, 2);//返回caFree
	return 1;
}

int bef_registerClick(lua_State* L)
{
	auto email = lua_getvalue<std::string>(L, 0, "bef.account.Text");
	lua_setvalue(L, 0, "register.email.Text", email);
	lua_function<int>(L, 0, "register.show");
	return 0;
}

int bef_ResetClick(lua_State* L)
{
	auto email = lua_getvalue<std::string>(L, 0, "bef.account.Text");
	lua_setvalue(L, 0, "Reset.email.Text", email);
	lua_function<int>(L, 0, "Reset.show");
	return 0;
}

int bef_chongcardClick(lua_State* L)
{
	auto email = lua_getvalue<std::string>(L, 0, "bef.account.Text");
	lua_setvalue(L, 0, "chongcard.account.Text", email);
	lua_function<int>(L, 0, "chongcard.show");
	return 0;
}


//使用_WINSOCK_DEPRECATED_NO_WARNINGS来继续使用旧版本的API
static inline std::string getIP()
{
	WSADATA WSAData;                                //WSADATA结构被用来储存调用AfxSocketInit全局函数返回的Windows Sockets初始化信息。
	if (WSAStartup(MAKEWORD(2, 0), &WSAData))        // 初始化Windows sockets API
	{
		return "";        //异常退出 
	}
	char hostName[0x300];
	if (gethostname(hostName, sizeof(hostName)))        //获取主机名
	{
		return "";        //异常退出 
	}
	std::string name(hostName);
	hostent* host = gethostbyname(hostName);    // 根据主机名获取主机信息. 
	if (host == NULL) {
		return name;        //异常退出 
	}
	sprintf_s(hostName, 0x300, "%u-%u", host->h_addrtype, host->h_length);
	name += hostName;
	name += host->h_name;
	WSACleanup();
	return name;
}
static inline std::string getBoisIDByCmd() {
	const long MAX_COMMAND_SIZE = 10000; // 命令行输出缓冲大小	

	WCHAR szFetCmd[] = L"wmic csproduct get UUID"; // 获取BOIS命令行	
	const string strEnSearch = "UUID"; // 主板序列号的前导信息

	BOOL   bret = FALSE;
	HANDLE hReadPipe = NULL; //读取管道
	HANDLE hWritePipe = NULL; //写入管道	
	PROCESS_INFORMATION pi; //进程信息	
	memset(&pi, 0, sizeof(pi));
	STARTUPINFO	si;	//控制命令行窗口信息
	memset(&si, 0, sizeof(si));
	SECURITY_ATTRIBUTES sa; //安全属性
	memset(&sa, 0, sizeof(sa));

	char szBuffer[MAX_COMMAND_SIZE + 1] = { 0 }; // 放置命令行结果的输出缓冲区
	string	strBuffer;
	unsigned long count = 0;
	long ipos = 0;

	pi.hProcess = NULL;
	pi.hThread = NULL;
	si.cb = sizeof(STARTUPINFO);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//1.创建管道
	bret = CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
	if (!bret) {
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		return "";
	}

	//2.设置命令行窗口的信息为指定的读写管道
	GetStartupInfo(&si);
	si.hStdError = hWritePipe;
	si.hStdOutput = hWritePipe;
	si.wShowWindow = SW_HIDE; //隐藏命令行窗口
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	//3.创建获取命令行的进程
	bret = CreateProcess(NULL, szFetCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	if (!bret) {
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return "";
	}

	//4.读取返回的数据
	WaitForSingleObject(pi.hProcess, 200);
	bret = ReadFile(hReadPipe, szBuffer, MAX_COMMAND_SIZE, &count, 0);
	if (!bret) {
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return "";
	}

	//5.查找主板ID
	strBuffer = szBuffer;
	ipos = strBuffer.find(strEnSearch);
	if (ipos < 0) { // 没有找到
		CloseHandle(hWritePipe);
		CloseHandle(hReadPipe);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return "";
	}
	else {
		strBuffer = strBuffer.substr(ipos + strEnSearch.length());
	}

	memset(szBuffer, 0x00, sizeof(szBuffer));
	strcpy_s(szBuffer, strBuffer.c_str());

	//去掉中间的空格 \r \n
	char lpszBaseBoard[0x300] = {};
	int j = 0;
	for (int i = 0; i < strlen(szBuffer); i++) {
		if (szBuffer[i] != ' ' && szBuffer[i] != '\n' && szBuffer[i] != '\r') {
			lpszBaseBoard[j] = szBuffer[i];
			j++;
		}
	}
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return lpszBaseBoard;
}
static inline std::string getSysInfo()
{
	SYSTEM_INFO  sysInfo;    //该结构体包含了当前计算机的信息：计算机的体系结构、中央处理器的类型、系统中中央处理器的数量、页面的大小以及其他信息。
	OSVERSIONINFOEX osvi;
	GetSystemInfo(&sysInfo);
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	char name[0x300];
	std::string get;
#pragma warning(disable: 4996)
	if (GetVersionEx((LPOSVERSIONINFOW)&osvi))
	{
		sprintf_s(name, 0x300, "%u.%u.%u", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
		get += name;
		sprintf_s(name, 0x300, "%u.%u", osvi.wServicePackMajor, osvi.wServicePackMinor);
		get += name;
	}
	sprintf_s(name, 0x300, "%hx.%hx.%hx.%llx.%lx.%lx", sysInfo.wProcessorArchitecture, sysInfo.wProcessorLevel, sysInfo.wProcessorRevision,
		sysInfo.dwActiveProcessorMask, sysInfo.dwNumberOfProcessors, sysInfo.dwProcessorType);
	get += name;
	return get;
}


int bef_logonClick(lua_State* L)
{

	//lua_function<int>(L, 0, "bef.hide");
	//lua_function<int>(L, 0, "YSM.show");
	//SUCCESS = 0x327fc942;//判断成功过的标志
	//return 0;


	auto account = lua_getvalue<std::string>(L, 0, "bef.account.Text");
	auto password = lua_getvalue<std::string>(L, 0, "bef.password.Text");
	{
		FILE* fw;
		if (fopen_s(&fw, (string(temp_path) + accountPath).c_str(), "w") == 0) {
			fprintf_s(fw, "account=%s password=%s ", account.c_str(), password.c_str());
			fclose(fw);
		}
	}
	//password = "account=" + account + "&password=" + password;
	string ppath(temp_path);
	ppath += "\\GmAiOl9Vx6RdC6Y8Ih4CiKl5DO0Xr2V7GxRJKm4F7J";
	char read[60] = {};
	std::string device = MD5(getIP() + getSysInfo() + getBoisIDByCmd()).toString();
	//获取设备校验码
	auto getmd5 = [&]() -> std::string {
		char ts[0x20];
		sprintf_s(ts, 0x20, "%llu", time(0) / 10);//时间小10倍
		//md5加密方式
		device = MD5(getIP() + getSysInfo() + getBoisIDByCmd()).toString();
		auto md5 = MD5(device + ts).toString();
		return md5;
	};
	//写入或者覆盖操作
	auto writein = [&](const std::string& md5) {
		FILE* fw;
		if (fopen_s(&fw, ppath.c_str(), "w") != 0) {
			MessageBoxW(0, L"写入Temp目录文件失败，请询问管理员解决问题。", L"执行错误", MB_ICONERROR);
			exit(102);
		}
		auto newtext = md5 + "==JbVr4Xs8Yg0Jl6C2X5FI***==";
		fwrite(newtext.c_str(), 1, newtext.size(), fw);
		fclose(fw);
		strcpy_s(read, 60, md5.c_str());
	};
	FILE* fp;
	if (fopen_s(&fp, ppath.c_str(), "r") != 0)
		writein(getmd5());
	else {
		fread_s(read, 60, 1, 59, fp);
		if (strstr(read, "==JbVr4Xs8Yg0Jl6C2X5FI***==") == NULL) {
			fclose(fp);
			writein(getmd5());
		}
		fclose(fp);
		read[32] = 0;//现在read等于device + time的md5字符串
	}
	auto nowtime = time(0);
	bool find = false;
	for (size_t i = 0; i < 60 * 60 * 10 / 10; i++)//10个小时改变一次机器码
	{
		char ts[0x20] = {};
		sprintf_s(ts, 0x20, "%llu", nowtime / 10 - i);
		if (MD5(device + ts).toString() == read) {
			find = true;
			break;
		}
	}
	if (!find)
		writein(getmd5());
	std::string data[6]{"username", account, "password", password ,"machine_code",read};
	auto response = encPost("https://www.baidu.com", data, 3);
	auto json = det::JsonObject(response);
	find = false;
	if (json.getInt("code") == 200) {
		auto newmd5 = getmd5();
		data[5] = newmd5;
		response = encPost("https://www.baidu.com", data, 3);
		json = det::JsonObject(response);
		if (json.getInt("code") == 200) {
			writein(newmd5);
			TOKEN = json.getJsonObject("data").getString("token");
			find = true;
		}
	}
	if (!find) {
		if (json.getInt("code") == 0)
			MessageBoxW(0, L"请检查网络是否正常", L"登陆失败", 0);
		else
			MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "登陆失败", 0);
		return 0;
	}
	data[0] = "token";
	data[1] = TOKEN;
	data[2] = "key";
	char times[0x20] = {};
	sprintf_s(times, 0x20, "%d", time_enc());
	data[3] = times;
	response = encPost("https://www.baidu.com", data, 2);
	json = det::JsonObject(response);
	if (json.getInt("code") != 200) {
		if (json.getInt("code") == 0)
			MessageBoxW(0, L"请检查网络是否正常", L"校验失败", 0);
		else 
			MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "校验失败", 0);
		return 0;
	}
	if (!time_dec(json.getJsonObject("data").getInt("enc"))) {
		MessageBoxA(0, "请检查电脑的时间是否与北京时间相差超过一分钟", "时间校验失败", 0);
		return 0;
	}
	//设置到期时间
	time_t vip = json.getJsonObject("data").getInt("vip");
	strftime(times, sizeof(times), "%Y-%m-%d %H:%M:%S", localtime(&vip));
	lua_setvalue(L, 0, "YSM.outtime.Caption", times);

	lua_function<int>(L, 0, "bef.hide");
	lua_function<int>(L, 0, "YSM.show");
	SUCCESS = 0x327fc942;//判断成功过的标志
	return 0;
}
