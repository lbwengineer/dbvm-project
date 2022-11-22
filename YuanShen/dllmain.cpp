#include <Windows.h>
#include <fstream>
#include <map>
#include <set>
#include "../tool/md5.h"
#include "../il2cpp/il2cpp_dump.h"
#include "../Inject/inject.h"


using namespace std;

void doMain();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       PVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)doMain, 0, 0, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

PINT speedDialog_off;
BOOL hook_QueryPerformanceCounter(LARGE_INTEGER* ipPerformanceCount) {
    auto ret = QueryPerformanceCounter(ipPerformanceCount);
    static UINT64 last = ipPerformanceCount->QuadPart;
    static UINT64 biglast = last;
    UINT64 add = ipPerformanceCount->QuadPart - last;
    last = ipPerformanceCount->QuadPart;
    if (*speedDialog_off) {
        add *= 8;
    }
    biglast += add;
    ipPerformanceCount->QuadPart = biglast;
    return ret;
}
#include <sstream>
BOOL skills(IL2CPP& cpp, PVOID& full, PVOID& cd)
{
    vector<IL2CPP::CLASS> clazz;
    cpp.get_all_class(clazz);
    
    for (auto& clz : clazz) {
        if (clz.modifer == "public sealed class" && clz.space == "" && clz.extends.size() == 1)
        {
            vector<IL2CPP::FIELD> field;
            cpp.get_all_filed(clz.tClass, field);
            if (field.size() >= 10 && field[3].type == "Boolean" && field[4].type == "Single"
                && field[5].type == "Int32" && field[6].type == "Int32" && field[8].type == "Dictionary`2") {
                //这里就是找到的类
                vector<IL2CPP::METHOD> method;
                cpp.get_all_method(clz.tClass, method);
                for (auto& med : method) {
                    if (med.modifier == "public" && med.type == "Boolean")
                    {
                        if (med.args.empty())//无限大招
                        {
                            char line[] = { 0x40 ,0x53 ,0x48 ,0x83,0xEC ,0x40 ,0x48 ,0x8B };
                            if (memcmp(med.address, line, sizeof(line)) == 0)
                            {
                                full = med.address;
                            }
                        }
                        else if (med.args.size() == 1 && med.args[0].type != "UInt32")//技能无cd
                        {
                            cd = med.address;
                        }
                        if (full && cd)
                            return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

PVOID runnocd(IL2CPP& cpp)
{
    vector<IL2CPP::CLASS> clazz;
    cpp.get_all_class(clazz);
    for (auto& clz : clazz) {
        if (clz.modifer == "public abstract class" && clz.space == "" && clz.extends.empty())
        {
            vector<IL2CPP::FIELD> field;
            cpp.get_all_filed(clz.tClass, field);
            if (field.size() >= 10 && field[0].type == "UInt32" && field[1].type == "Dictionary`2"
                && field[2].type == "Dictionary`2" && field[3].type == "Boolean" && field[4].type == "Boolean"
                && field[6].type == "UInt32") {
                //这里就是找到的类
                vector<IL2CPP::METHOD> method;
                cpp.get_all_method(clz.tClass, method);
                for (auto& med : method) {
                    if (med.modifier == "public virtual" && med.type == "Void" && med.args.size() == 3 
                        && med.args[0].type == "UInt32" && med.args[1].type == "Int64")
                    {
                        return med.address;
                    }
                }
            }
        }
    }
    return 0;
}

PVOID wudi(IL2CPP& cpp)
{
    vector<IL2CPP::CLASS> clazz;
    cpp.get_all_class(clazz);
    for (auto& clz : clazz) {
        if (clz.modifer == "public class" && clz.space == "" && clz.extends.size() == 1 && clz.extends[0] == "LCBase")
        {
            vector<IL2CPP::FIELD> field;
            cpp.get_all_filed(clz.tClass, field);
            if (field.size() >= 10 && field[2].type == "Action`1" && field[3].type == "Boolean"
                && field[6].type == "TargetType" && field[9].type == "Dictionary`2") {
                //这里就是找到的类
                vector<IL2CPP::METHOD> method;
                cpp.get_all_method(clz.tClass, method);
                for (auto& med : method) {
                    if (med.modifier == "public" && med.type == "Void" && med.args.size() == 2 && med.args[0].type == "UInt32")
                    {
                        return med.address;
                    }
                }
            }
        }
    }
    return 0;
}

PVOID runspeed()
{
    HOOK hook(_getpid());
    auto reg = hook.getRegions();
    auto mod = get_module_addr(_getpid(), L"UnityPlayer.dll");
    if (mod == 0)
        return 0;
    for (size_t i = 0; i < reg.size(); i++) {
        if (reg[i].BaseAddress == mod + 0x1000)
            return searchLocalBytes(reg[i].BaseAddress, reg[i].RegionSize,
                "F3 42 0F 10 74 39 38 F3 42 0F 11 34 33 F3 0F 10 45 50 48 8B 5C 24 50 48 8B 6C 24 58 0F 5A C0");
    }
    return 0;
}

BOOL xiguai(PVOID& initaddr, PVOID& xyzaddr)
{
    HOOK hook(_getpid());
    auto reg = hook.getRegions();
    auto mod = get_module_addr(_getpid(), L"UnityPlayer.dll");
    if (mod == 0)
        return 0;
    for (size_t i = 0; i < reg.size(); i++)
        if (reg[i].BaseAddress == mod + 0x1000) {
            initaddr = searchLocalBytes(reg[i].BaseAddress, reg[i].RegionSize, 
                "F3 44 0F 10 4B 0C 45 0F 28 D0 F3 0F 10 3B 44 0F 57 D0");
            xyzaddr = searchLocalBytes(reg[i].BaseAddress, reg[i].RegionSize, 
                "F3 0F 11 55 10 F3 0F 11 5D 14 F3 0F 11 65 18");
            if (initaddr && xyzaddr)
                return TRUE;
            break;
        }
    return FALSE;
}



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


bool (*skillcd_o)(PVOID thiz, PVOID JIDGBMBKBGL) = 0;
bool skillcd(PVOID thiz, PVOID JIDGBMBKBGL) {
    if (*cd_off)
        return 0;
    return skillcd_o(thiz, JIDGBMBKBGL);
}

bool (*spenough_o)(PVOID thiz) = 0;
bool spenough(PVOID thiz) {
    if (*skill_off)
        return 1;
    return spenough_o(thiz);
}

enum actionType { Reset, Change };
void (*Runnocd_o)(PVOID thiz, UINT32 id, INT64 actionBar, actionType type) = 0;
void Runnocd(PVOID thiz, UINT32 id, INT64 actionBar, actionType type) {
    if (*runcd_off && id == 0x271B && type == Change) {
        static int64_t maxVal = actionBar;
        if (maxVal < actionBar)
            maxVal = actionBar;
        Runnocd_o(thiz, id, maxVal, type);
        return;
    }
    Runnocd_o(thiz, id, actionBar, type);
}


void (*wudimiaosha_o)(PVOID thiz, UINT32 id, PVOID arg2) = 0;
void wudimiaosha(PVOID thiz, UINT32 id, PVOID arg2) {
    if (*wudimiaosha_off == 0) {
        wudimiaosha_o(thiz, id, arg2);
        return;
    }
    if (id > 0x1000000 && id < 0x2000000) {//我方人员受到的伤害
        return;
    }
    else {//其余物体受到的伤害
        for (int i = 0; i < *wudimiaosha_ratio; ++i) {
            wudimiaosha_o(thiz, id, arg2);
        }
    }
}

PVOID speedHack_inline;
void speedHack(UINT64 rcx, PFLOAT rdx)
{
    if (*rdx < 1.0f || *rdx > 2.0f)
        return;
    if (rcx == 0x108) {//这是移速
        if (*movespeed_off)
            *rdx = *movespeed_ratio;
    }
    else if (rcx > 0x108 && rcx < 0x1000) {//这是攻速
        if (*attackspeed_off)
            *rdx = *attackspeed_ratio;
    }
}

UINT64 SAVE_RBX;//这个地址是我方角色的矩阵首地址
PVOID saveXYZrbx_inline;

std::map<UINT64, UINT> saverbx;
void saveXYZrbx(UINT64 rcx, UINT64 rdx) {
    if (rcx != 0x83000000 || *(int*)(rdx - 4) != 0x01060006)
        return;
    saverbx[rdx - 0x20]++;
    static ULONGLONG lasttime = GetTickCount64();
    ULONGLONG nowtime = GetTickCount64();
    if (nowtime - lasttime > 5000) {
        auto i = saverbx.begin();
        UINT64 max_rbx = 0;
        UINT max_count = 0;
        for (; i != saverbx.end(); ++i) {
            if ((*i).second > max_count) {
                max_rbx = (*i).first;
                max_count = (*i).second;
            }
        }
        if (max_rbx) {
            SAVE_RBX = max_rbx;
        }
        saverbx.clear();
        lasttime = nowtime;
    }
}

CRITICAL_SECTION oneInOne;
#include <set>
std::set< UINT64> role;
PVOID xyzHack_inline;
//rsi = 除主角外矩阵首地址
void xyzHack(UINT64 rsi) { //[x, y, z] = [rsi + 0x10, rsi + 0x14, rsi + 0x18]，y轴向上
    EnterCriticalSection(&oneInOne);
    role.emplace(rsi);
    if (*gaotiao_click && rsi == SAVE_RBX) {
        *(PFLOAT)(SAVE_RBX + 0x14) += 50;
        *gaotiao_click = 0;
    }
    static ULONGLONG lasttime = GetTickCount64();
    ULONGLONG nowtime = GetTickCount64();
    if (nowtime - lasttime > 2000) {
        for (auto i = role.begin(); i != role.end(); ) {
            if ((*(PINT32)(*i + 0x1C) & ~(INT32)0xf) != 0x01060000 || *i == SAVE_RBX)
                i = role.erase(i);
            else
                i++;
        }
        lasttime = nowtime;
    }
    LeaveCriticalSection(&oneInOne);
}


bool threadalive;
void getAllmyPlace() {
    while (threadalive) {
        EnterCriticalSection(&oneInOne);
        for (size_t j = 0; j < 100; j++)
            for (auto i = role.begin(); i != role.end(); ) {
                if ((*(PINT32)(*i + 0x1C) & ~(INT32)0xf) != 0x01060000 || *i == SAVE_RBX)
                    i = role.erase(i);
                else {
                    *(PFLOAT)(*i + 0x10) = *(PFLOAT)(SAVE_RBX + 0x10);
                    *(PFLOAT)(*i + 0x14) = *(PFLOAT)(SAVE_RBX + 0x14);
                    *(PFLOAT)(*i + 0x18) = *(PFLOAT)(SAVE_RBX + 0x18);
                    i++;
                }
            }
        LeaveCriticalSection(&oneInOne);
    }
}

void xyzThread() {
    for (;;) {
        Sleep(100);
        if (!*yidongxiguai_total)
            continue;
        if (*xiguai_click && SAVE_RBX != 0) {//点击了吸怪且我方人物地址已经初始化
            threadalive = true;
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)getAllmyPlace, 0, 0, 0);
            Sleep(100);
            threadalive = false;
            *xiguai_click = 0;
        }
    }
}

#define YANZHENG {\
char all_md5line[0x50]{};\
auto llongg = GetTickCount64() / 1000;\
bool can = false;\
for (int i = 0; -20 <= i && i <= 20; i = i >= 0 ? -i - 1 : -i) {\
    sprintf(all_md5line, "87f8d729a4b3f7a5%016llx2a9c0821b5e28a0f", llongg + i);\
    auto line = MD5(all_md5line).toString();\
    if (0 == memcmp(global_message, line.c_str(), 32)) {\
        can = true;\
        break;\
    }\
}\
if (!can)\
*(PINT)0 = 0;\
}\

#define initdef \
char* global_message = (char* )openSharedMemory(L"ALLMESSAGEENCRYPTION", 0x1000);\
YANZHENG;\
SHARED = (PINT)createSharedMemory(L"AllGoOutOfMyHome", 0x1000);\
PWCHAR MESSAGE = (PWCHAR)createSharedMemory(L"SharedMessageDialog", 0x1000);\
setlocale(0, "");\
if (SHARED == 0 || MESSAGE == 0)\
return;\
memset(MESSAGE, 0, 0x1000);


std::string dump_il2cpp(IL2CPP* cpp) {
    char* base = (char* )get_module_addr(_getpid(), L"UserAssembly.dll");
    std::vector<IL2CPP::CLASS> clazz;
    cpp->get_all_class(clazz);
    std::stringstream ss;
    for (auto& get : clazz) {
        std::vector<IL2CPP::FIELD> field;
        std::vector<IL2CPP::METHOD> method;
        cpp->get_all_filed(get.tClass, field);
        cpp->get_all_method(get.tClass, method);
        ss << "// Namespace: " << get.space << "\n";
        ss << get.modifer << " " << get.name << " ";
        if (!get.extends.empty()) {
            ss << ": ";
            for (int i = 0; i < get.extends.size(); ++i) {
                ss << get.extends[i];
                if (i != get.extends.size() - 1)
                    ss << ",";
                ss << " ";
            }
        }
        ss << "{";
        if (!field.empty())
            ss << "\n\t// Fields:";
        bool isEnum = false;
        if (get.modifer.find("enum") != -1)
            isEnum = true;
        bool isEnum_first = true;
        for (auto& fld : field) {
            ss << "\n\t" << fld.modifier << " " << fld.type << " " << fld.name;
            if (isEnum && !isEnum_first)
                ss << " = " << std::dec << fld.value;
            isEnum_first = false;
            ss << "; //0x" << std::hex << fld.offset;
        }
        if (!method.empty()) {
            if (!field.empty())
                ss << "\n";
            ss << "\n\t// Methods";
        }
        for (auto& med : method) {
            // RVA是内存地址相对模块首地址的偏移量，PVA是崩坏三特殊处理的相对地址
            ss << "\n\t// RVA: " << (med.address == 0 ? 0 : med.address - base);
            ss << "\n\t" << med.modifier << " " << med.type << " " << med.name << "(";
            if (!med.args.empty()) {
                ss << med.args[0].type << " " << med.args[0].name;
                for (int i = 1; i < med.args.size(); ++i) {
                    ss << ", " << med.args[i].type << " " << med.args[i].name;
                }
            }
            ss << ") { };";
        }
        ss << "\n}\n\n";
    }
    return ss.str();
}


void doMain()
{
#ifndef DEBUGUNITY
    initdef;
#endif
    //个别游戏的特定区域
    while (get_module_addr(_getpid(), L"UserAssembly.dll") == 0 || get_module_addr(_getpid(), L"UnityPlayer.dll") == 0)
        Sleep(1000);

    IL2CPP cpp;
#ifdef DEBUGUNITY
    auto fp = fopen("D:\\dump.cs", "w");
    if (fp) {
        fprintf(fp, "aaaaa");
        auto ft = dump_il2cpp(&cpp);
        fwrite(ft.c_str(), ft.size(), 1, fp);
        fflush(fp);
        fclose(fp);
    }
    exit(0);
#else
    HOOK hook(_getpid());

    PVOID full = 0, cd = 0;
    auto k1 = skills(cpp, full, cd);
    if (k1 == FALSE) {
        wcscpy(MESSAGE, L"技能无cd和无限大招初始化失败");
        *(SHARED + 0x200) = 1;
        return;
    }

    skillcd_o = (bool(*)(PVOID, PVOID))hook.mmapexec(cd);
    spenough_o = (bool(*)(PVOID))hook.mmapexec(full);

    auto k2 = runnocd(cpp);
    if (k2 == 0) {
        wcscpy(MESSAGE, L"无限耐力初始化失败");
        *(SHARED + 0x200) = 2;
        return;
    }
    Runnocd_o = (void (*)(PVOID, UINT32, INT64, actionType))hook.mmapexec(k2);

    auto k3 = wudi(cpp);
    if (k3 == 0) {
        wcscpy(MESSAGE, L"无敌初始化失败");
        *(SHARED + 0x200) = 3;
        return;
    }
    wudimiaosha_o = (void (*)(PVOID, UINT32, PVOID))hook.mmapexec(k3);

    YANZHENG;

    auto k4 = runspeed();
    if (k4 == 0) {
        wcscpy(MESSAGE, L"攻击速度和移动速度初始化失败");
        *(SHARED + 0x200) = 4;
        return;
    }
    speedHack_inline = hook.mmapexec(k4);
   
    PVOID initaddr = 0, xyzaddr = 0;
    auto k5 = xiguai(initaddr, xyzaddr);
    if (k5 == FALSE) {
        wcscpy(MESSAGE, L"吸怪初始化失败");
        *(SHARED + 0x200) = 5;
        return;
    }
    saveXYZrbx_inline = hook.mmapexec(initaddr);
    xyzHack_inline = hook.mmapexec(xyzaddr);

    YANZHENG;

    PVOID line[] = { cd , full, k2, k3, k4, 
        initaddr, xyzaddr, skillcd, skillcd_o ,spenough ,//前面7个才是需要设置内存属性的地址
        spenough_o, Runnocd ,Runnocd_o,wudimiaosha, wudimiaosha_o,
        speedHack_inline, speedHack, saveXYZrbx_inline, saveXYZrbx, xyzHack_inline, 
        xyzHack };
    for (int i = 0; i < sizeof(line) / sizeof(PVOID); i++) {
        if (line[i] == 0) {
            wsprintf(MESSAGE, L"发现至少一个地址分配内存失败了，序号%d", i);
            *(SHARED + 0x200) = 6;
            return;
        }
    }
#ifndef USEDBVM
    for (int i = 0; i < 7; i++) {
        DWORD flag;
        if (!VirtualProtect(line[i], 5, PAGE_EXECUTE_READWRITE, &flag)) {
            wsprintf(MESSAGE, L"地址%p设置内存属性失败了", line[i]);
            *(SHARED + 0x200) = 7;
            return;
        }
    }
#endif
    memcpy(SHARED, line, sizeof(line));

    YANZHENG;

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

    *movespeed_ratio = *attackspeed_ratio = 1.0;

    speedDialog_off = (PINT)(SHARED + 0x120);

    {//设置帧率为120帧
        /*if (cpp.setClass("UnityEngine", "public sealed class Application"))
        {
            auto method = (void(*)(int))cpp.get_method("public static Void set_targetFrameRate");
            if (method != 0) {
                method(120);
            } 
        }*/
        //对UnityPlayer.dll进行hook
        hook.iathook(get_module_addr(_getpid(), L"UnityPlayer.dll"), 0, "QueryPerformanceCounter", hook_QueryPerformanceCounter);
    }
 
    YANZHENG;

    *(SHARED + 0x200) = 100;
    InitializeCriticalSection(&oneInOne);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)xyzThread, 0, 0, 0);

    YANZHENG;
#endif
}
