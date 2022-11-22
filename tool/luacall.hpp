#pragma once
#include <Windows.h>
#include <string>

/*
ִ��lua��ĺ���
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾ������ȫ�ֱ����������Ǿֲ�����
function_name	�������ƣ���һ��������ȫ�ֱ�������global.local.function
args			�����Ĳ���*/
template <class Result, class... Args>
Result lua_function(lua_State* L, int local_stack, const char* function_name, const Args& ...args);

/*
��ȡ���ر�����ָ��ֵ
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾ������ȫ�ֱ����������Ǿֲ�����
value_name		�������ƣ���һ��������ȫ�ֱ���������table�е�index����ʹ��ʮ���ƣ���global.12.function
*/
template<class Result>
Result lua_value(lua_State* L, int local_stack, const char* value_name);

static void ErrorMessage(LPCSTR error, ...)
{
	char message[0x100];
	va_list args;
	va_start(args, error);
	vsprintf_s(message, 0x100, error, args);
	va_end(args);
	MessageBoxA(0, message, "Lua���ԵĴ���", MB_OK);
	exit(101);
}

static inline void arg_push_all(lua_State* L, const bool& arg) {
	lua_pushboolean(L, arg);
}
static inline void arg_push_all(lua_State* L, const int& arg) {
	lua_pushinteger(L, arg);
}
static inline void arg_push_all(lua_State* L, const long long& arg) {
	lua_pushnumber(L, arg);
}
static inline void arg_push_all(lua_State* L, const float& arg) {
	lua_pushnumber(L, arg);
}
static inline void arg_push_all(lua_State* L, const double& arg) {
	lua_pushnumber(L, arg);
}
static inline void arg_push_all(lua_State* L, const char* arg) {
	lua_pushlstring(L, arg, strlen(arg) + 1);
}
static inline void arg_push_all(lua_State* L, const wchar_t* arg) {
	auto len = wcslen(arg) + 1;
	char* cstring = new char[2 * len]{};
	WideCharToMultiByte(CP_ACP, 0, arg, len, cstring, 2 * len, 0, 0);
	lua_pushlstring(L, cstring, 2 * len);
	delete[] cstring;
}
static inline void arg_push_all(lua_State* L, const std::string& arg) {
	lua_pushlstring(L, arg.c_str(), arg.size() + 1);
}
static inline void arg_push_all(lua_State*) {
}
template <class type, class... Args>
static inline void arg_push_all(lua_State* L, const type& arg, const Args& ...args) {
	arg_push_all(L, arg);
	arg_push_all(L, args...);
}
template<class Result> inline
Result luam_get_result(lua_State* L);
template<> static inline
bool luam_get_result<bool>(lua_State* L) {
	return lua_toboolean(L, -1);
}
template<> static inline
int luam_get_result<int>(lua_State* L) {
	return lua_tointeger(L, -1);
}
template<> static inline
long long luam_get_result<long long>(lua_State* L) {
	return lua_tonumber(L, -1);
}
template<> static inline
float luam_get_result<float>(lua_State* L) {
	return lua_tonumber(L, -1);
}
template<> static inline
double luam_get_result<double>(lua_State* L) {
	return lua_tonumber(L, -1);
}
template<> static inline
std::string luam_get_result<std::string>(lua_State* L) {
	return lua_tostring(L, -1);
}
template<> static inline
lua_CFunction luam_get_result<lua_CFunction>(lua_State* L) {
	return lua_tocfunction(L, -1);
}

static inline void push_global_stack(lua_State* L, int beforeset, const char* point_name)
{
	auto strsize = strlen(point_name) + 1;
	char* allstr = new char[strsize];
	strcpy_s(allstr, strsize, point_name);
	char* endptr = 0;
	char* point = strtok_s(allstr, ".", &endptr);
	if (beforeset == 0 && !lua_getglobal(L, point))
		ErrorMessage("�޷��ҵ�ȫ�ֱ���%s", point);
	else if (beforeset && !lua_getfield(L, beforeset, point))
		ErrorMessage("�޷��ҵ��ֲ���������һ������%s", point);
	char line[0x100] = "�˱�������Ϊ��";
	strcat_s(line, 0x100, point);
	while (point = strtok_s(0, ".", &endptr))
	{
		bool isnumber = true;
		for (size_t i = 0, length = strlen(point); i <= length; i++)
		{
			if (point[0] > '9' || (i == 0 && point[0] < '1') || point[0] < '0')
			{
				isnumber = false;
				break;
			}
		}
		if ((isnumber && !lua_getfield(L, -1, point)) || (!isnumber && !lua_getfield(L, -1, point))) {
			strcat_s(line, 0x100, "->");
			strcat_s(line, 0x100, point);
			ErrorMessage(line);
		}
	}
	delete[] allstr;
}

template <class Result, class... Args>
Result lua_function(lua_State* L, int local_stack, const char* function_name, const Args& ...args)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, function_name);
	if (!lua_isfunction(L, -1))
		ErrorMessage("����%s����Lua����", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("����Lua����%s����", function_name);
	auto get = luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}


template <class Result>
Result lua_value(lua_State* L, int local_stack, const char* value_name)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, value_name);
	auto get =  luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}