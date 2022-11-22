#pragma once
#include <xstring>

//进程创建时初始化
void processInit();
extern char temp_path[0x400];
extern std::string TOKEN;
extern const char* accountPath;
extern int SUCCESS;
extern PINT SHARED;
#define XXXTXXX if (SUCCESS != 0x327fc942) *((volatile PBYTE) 0) = 0;
bool initControl(lua_State * L);
extern bool hasinit;
struct code_phy {
	std::string code;
	std::string original;
	INT32 mod;
	INT64 phy;
	INT64 vir;
};