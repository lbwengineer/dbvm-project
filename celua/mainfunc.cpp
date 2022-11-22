#include "../lua53/src/lua.hpp"
#include "../inject/luacall.hpp"
#include "globalvalue.h"
#include "luaglobal.h"
#include "../tool/HttpConnect.h"
#include "../tool/JsonObject.h"
#include "connect.h"
#include <atlstr.h>
#include <thread>

BOOL APIENTRY DllMain(HMODULE hModule,      //指向自身的句柄
    DWORD  ul_reason_for_call,              //调用原因
    PVOID lpReserved                       //静态加载为非NULL，动态加载为NULL
)
{
    setlocale(0, "");
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:  //进程创建的时候调用
        processInit();
        break;
    case DLL_THREAD_ATTACH:   //线程创建的时候调用
        break;
    case DLL_THREAD_DETACH:   //线程结束的时候调用
        break;
    case DLL_PROCESS_DETACH:  //进程结束的时候调用
        break;
    }
    return TRUE;
}

void luaInit(lua_State * L)
{
    //初始化版本号
    //现在的版本号
    LPCSTR version = "4.5";
    lua_setvalue(L, 0, "bef.nowbanben.Caption", version);
    auto response = encPost("https://www.baidu.com", 0, 0);
    auto json = det::JsonObject(response);
    if (json.getInt("code") != 200) {
        if (json.getInt("code") == 0)
            MessageBoxA(0, "网络连接失败，请重新打开程序。", "获取版本信息", MB_ICONERROR);
        else
            MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "获取版本信息", MB_ICONERROR);
        lua_setvalue(L, 0, "bef.newbanben.Caption", "网络连接失败");
        lua_setvalue(L, 0, "bef.gonggao.Caption", "网络连接失败");
    }
    else {
        auto banben = json.getJsonObject("data").getString("version");
        if (banben != version) {
            std::string error = "科技已更新，请前往网盘下载。\n当前版本：";
            error += version;
            error += "\n最新版本：" + banben;
            error += "\n公告：" + json.getJsonObject("data").getString("notice");
            MessageBoxA(0, error.c_str(), "检查版本", MB_ICONERROR);
            exit(102);
        }
        lua_setvalue(L, 0, "bef.newbanben.Caption", banben);
        lua_setvalue(L, 0, "bef.gonggao.Caption", json.getJsonObject("data").getString("notice", CP_UTF8));
    }
    std::string ppath(temp_path);
    ppath += accountPath;
    FILE* fp;
    if (fopen_s(&fp, ppath.c_str(), "r") == 0) {
        char buff[0x200] = {};
        fread_s(buff, 0x200, 1, 0x200, fp);
        char account[0x20] = {}, password[0x20] = {};
        if (2 == sscanf(buff, "account=%s password=%s ", account, password)) {
            lua_setvalue(L, 0, "bef.account.Text", account);
            lua_setvalue(L, 0, "bef.password.Text", password);
        }
        fclose(fp);
    }
    //开始显示登陆界面
    lua_function<int>(L, 0, "bef.show");
}

static CString convert(CString path, int count) {//path = L"....../XXX"
    for (size_t i = 0; i < count; i++) {
        path = path.Left(path.ReverseFind(L'\\'));
    }
    return path;
}
#include "../Inject/inject.h"

void processInit()
{
    //获取当前的可执行文件的目录
    setlocale(0, "");
    CString strCurPath;
    GetModuleFileNameW(NULL, strCurPath.GetBuffer(MAX_PATH), MAX_PATH);
    strCurPath.ReleaseBuffer();
    strCurPath = strCurPath.Left(strCurPath.ReverseFind(L'\\'));
    //strCurPath = L".*/Temp/cetrainers/CEXXX.tmp/extracted"
    if (-1 == strCurPath.Find(L"Cheat Engine"))
        strCurPath = convert(strCurPath, 3);
    //strCurPath = L".*/Temp"
    //判断用户名是否包括中文
    //auto left = strCurPath.Find(L"\\Users\\");
    //auto right = strCurPath.Find(L"\\AppData\\Local\\Temp\\cetrainers\\");
    //if ((left == -1 || right == -1)) {
    //    if (strCurPath.Find(L"Cheat Engine") == -1) {
    //        MessageBoxW(0, (L"运行路径可能有问题，请联系群主解决问题：\n" + strCurPath).GetString(), L"执行错误", MB_ICONERROR);
    //        exit(102);
    //    }
    //}
    //else {//清理垃圾并截取temp目录
    //    auto back = strCurPath.Left((strCurPath = strCurPath.Left(strCurPath.ReverseFind(L'\\'))).ReverseFind(L'\\'));
    //    auto ownname = strCurPath.Right(strCurPath.GetLength() - 1 - strCurPath.ReverseFind(L'\\'));
    //    /*WIN32_FIND_DATAW info;
    //    HANDLE file;
    //    if (INVALID_HANDLE_VALUE != (file = FindFirstFileW((back + "\\*").GetString(), &info))) {
    //        SHFILEOPSTRUCTA point{ 0, FO_DELETE ,0,0, FOF_NO_UI };
    //        do {
    //            if (wcscmp(info.cFileName, L".") && wcscmp(info.cFileName, L"..") && wcscmp(info.cFileName, ownname)) {
    //                char* del = new char[(back.GetLength() + wcslen(info.cFileName)) * 3 + 10]{ 0 };
    //                sprintf(del, "%S\\%S", back.GetString(), info.cFileName);
    //                point.pFrom = del;
    //                SHFileOperationA(&point);
    //                delete[] del;
    //            }
    //        } while (0 != FindNextFileW(file, &info));
    //        FindClose(file);
    //    }*/
    //    strCurPath = back.Left(back.ReverseFind(L'\\'));
    //}
    //auto mid = strCurPath.Mid(left + 7, right - left - 7);
    for (int i = 0, len = strCurPath.GetLength(); i < len; i++)
    {
        if (strCurPath[i] < 0x20 || strCurPath[i] > 0x7E) {
            MessageBoxW(0, (L"运行路径含有中文，请查看教程创建新用户名，当前运行路径：\n" + strCurPath).GetString(), L"执行错误", MB_ICONERROR);
            exit(102);
        }
    }
    WideCharToMultiByte(CP_ACP, 0, strCurPath.GetString(), strCurPath.GetLength() + 1, temp_path, 0x400, 0, 0);
}

char temp_path[0x400];
