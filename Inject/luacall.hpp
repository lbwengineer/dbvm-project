#pragma once
#include <Windows.h>
#include <locale>
#include <xstring>

/*		10	--->>>	|||||||||||||ջ��|||||||||||||||		<<<---	0			ջ��λ����lua_gettop��lua_settop����
		9	--->>>		   �������ص�1�����				<<<---  -1			<<<-------lua_gettop(L)ָ������
		8	--->>>	       �������ص�2�����				<<<---  -2
					��������������������������������		
		5	--->>>		   ������ջ��3������				<<<---  -5
		4	--->>>		   ������ջ��2������				<<<---  -6
		3	--->>>		   ������ջ��1������				<<<---  -7			������lua_getfield��ȡ
		2	--->>>		������ַ��Lua�ռ���Ķ���			<<<---  -8			
					��������������������������������
		0	--->>>	|||||||||||||ջ��|||||||||||||||
*/

/*
ִ��lua��ĺ���
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾfunction_name������ȫ�ֱ����������Ǿֲ�����
function_name	�������ƣ���һ��������ȫ�ֱ�������global.local.function
args			�����Ĳ���
*/
template <class Result, class... Args>
Result lua_function(lua_State* L, int local_stack, const char* function_name, const Args& ...args);
/*
ִ��lua��ĺ����������ĺ����ķ���ֵ
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾfunction_name������ȫ�ֱ����������Ǿֲ�����
function_name	�������ƣ���һ��������ȫ�ֱ�������global.local.function
args			�����Ĳ���
*/
template <class... Args>
void lua_vfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args);
/*
ִ��C++��ĺ���
lua_State*		Lua��ǰ�����пռ�
function		C++������ʵ������һ����ַ
args			�����Ĳ���
*/
template <class Result, class... Args>
Result lua_cfunction(lua_State* L, lua_CFunction function, const Args& ...args);
/*
ִ��C++��ĺ����������ĺ����ķ���ֵ
lua_State*		Lua��ǰ�����пռ�
function		C++������ʵ������һ����ַ
args			�����Ĳ���
*/
template <class... Args>
void lua_cvfunction(lua_State* L, lua_CFunction function, const Args& ...args);
/*
ִ��lua��ĺ���������ֱֵ�ӷŵ�ջ��
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾ������ȫ�ֱ����������Ǿֲ�����
function_name	�������ƣ���һ��������ȫ�ֱ�������global.local.function
args			�����Ĳ���
*/
template <class... Args>
void lua_lfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args);
/*
��ȡ���ر�����ָ��ֵ
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾvalue_name��ȫ�ֱ����������Ǿֲ�����
value_name		�������ƣ���һ��������ȫ�ֱ���������table�е�index����ʹ��ʮ���ƣ���global.[12].function��global.[12]���� global[12]
*/
template<class Result>
Result lua_getvalue(lua_State* L, int local_stack, const char* value_name);
/*
���ö��ر���Ϊָ��ֵ
lua_State*		Lua��ǰ�����пռ�
local_stack		ΪNULLʱ��ʾvalue_name��ȫ�ֱ����������Ǿֲ�����
value_name		�������ƣ���һ��������ȫ�ֱ���������table�е�index����ʹ��ʮ���ƣ���global.[12].function��global.[12]���� global[12]
value			Ҫ���õı�����ֵ
*/
template<class valtype>
void lua_setvalue(lua_State* L, int local_stack, const char* value_name, const valtype value);

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
static inline void arg_push_all(lua_State* L, const unsigned int& arg) {
	lua_pushinteger(L, arg);
}
static inline void arg_push_all(lua_State* L, const long& arg) {
	lua_pushinteger(L, arg);
}
static inline void arg_push_all(lua_State* L, const unsigned long& arg) {
	lua_pushinteger(L, arg);
}
static inline void arg_push_all(lua_State* L, const long long& arg) {
	lua_pushinteger(L, arg);
}
static inline void arg_push_all(lua_State* L, const unsigned long long& arg) {
	lua_pushinteger(L, arg);
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
static inline void arg_push_all(lua_State* L, lua_CFunction arg) {
	lua_pushcfunction(L, arg);
}
static inline void arg_push_all(lua_State* L, const wchar_t* arg) {
	auto len = wcslen(arg) + 1;
	char* cstring = new char[4 * len]{};
	WideCharToMultiByte(CP_UTF8, 0, arg, len, cstring, 4 * len, 0, 0);
	lua_pushlstring(L, cstring, 4 * len);
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
long luam_get_result<long>(lua_State* L) {
	return lua_tointeger(L, -1);
}
template<> static inline
long long luam_get_result<long long>(lua_State* L) {
	return lua_tointeger(L, -1);
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

static inline bool isNumber(const char* key) {
	bool isnumber = false;
	if (key[0] == '[' && key[strlen(key) - 1] == ']') {
		isnumber = true;
		for (size_t i = 1, length = strlen(key); i < length - 1; i++)
		{
			if (key[i] > '9' || (i == 1 && key[1] < '1') || (i != 1 && key[i] < '0'))
			{
				isnumber = false;
				break;
			}
		}
	}
	return isnumber;
}

static inline void luam_set_value(lua_State* L, const char* key, const bool value) {
	lua_pushboolean(L, value);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const int value) {
	lua_pushinteger(L, value);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const long value) {
	lua_pushinteger(L, value);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const long long value) {
	lua_pushinteger(L, value);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const float value) {
	lua_pushnumber(L, value);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const double value) {
	lua_pushnumber(L, value);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const char* value) {
	lua_pushlstring(L, value, strlen(value) + 1);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const std::string& value) {
	lua_pushlstring(L, value.c_str(), value.size() + 1);
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}
static inline void luam_set_value(lua_State* L, const char* key, const wchar_t* value) {
	auto len = wcslen(value) + 1;
	char* cstring = new char[4 * len]{};
	WideCharToMultiByte(CP_UTF8, 0, value, len, cstring, 4 * len, 0, 0);
	lua_pushlstring(L, cstring, 4 * len);
	delete[] cstring;
	if (isNumber(key))
		lua_rawseti(L, -2, strtoll(key + 1, 0, 16));
	else
		lua_setfield(L, -2, key);
}

static inline std::string push_global_stack(lua_State* L, int beforeset, const char* point_name, bool isAllPush)//�������һ�������Ƿ�������
{
	auto strsize = strlen(point_name) + 1;
	char* allstr = new char[strsize];
	strcpy_s(allstr, strsize, point_name);
	char* endptr = 0, *last = 0;
	char* point = strtok_s(allstr, ".", &endptr);
	char line[0x100] = "�˱�������Ϊ��";
	if (beforeset == 0) {
		if (!lua_getglobal(L, point))
			ErrorMessage("�޷��ҵ�ȫ�ֱ���%s", point);
		strcat_s(line, 0x100, point);
		if ((point = strtok_s(0, ".", &endptr)) == NULL)
			return "";
	}
	do 
	{
		last = strtok_s(0, ".", &endptr);
		if (!isAllPush && last == NULL)
			return point;
		strcat_s(line, 0x100, "->");
		strcat_s(line, 0x100, point);
		bool isnumber = isNumber(point);
		if ((isnumber && !lua_rawgeti(L, -1, strtoll(point + 1, 0, 10))) || (!isnumber && !lua_getfield(L, -1, point))) {
			ErrorMessage(line);
		}
		point = last;
	} while (point);
	delete[] allstr;
	return point == NULL ? "" : point;
}

template <class Result, class... Args>
Result lua_function(lua_State* L, int local_stack, const char* function_name, const Args& ...args)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, function_name, true);
	if (!lua_isfunction(L, -1))
		ErrorMessage("����%s����Lua����", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("����Lua����%s����", function_name);
	auto get = luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}

template <class... Args>
void lua_vfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, function_name, true);
	if (!lua_isfunction(L, -1))
		ErrorMessage("����%s����Lua����", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("����Lua����%s����", function_name);
	lua_settop(L, stack);
}

template <class... Args>
void lua_lfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	push_global_stack(L, local_stack, function_name, true);
	if (!lua_isfunction(L, -1))
		ErrorMessage("����%s����Lua����", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("����Lua����%s����", function_name);
}

template <class Result>
Result lua_getvalue(lua_State* L, int local_stack, const char* value_name)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, value_name, true);
	auto get =  luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}

template<class valtype>
void lua_setvalue(lua_State* L, int local_stack, const char* value_name, const valtype value)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	auto key = push_global_stack(L, local_stack, value_name, false);
	luam_set_value(L, key.c_str(), value);
	lua_settop(L, stack);
}

template <class Result, class... Args>
Result lua_cfunction(lua_State* L, lua_CFunction function, const Args& ...args)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	lua_pushcfunction(L, function);
	if (!lua_iscfunction(L, -1))
		ErrorMessage("��ַ%p����C++����", function);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("����C++����%p����", function);
	auto get = luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}

template <class... Args>
void lua_cvfunction(lua_State* L, lua_CFunction function, const Args& ...args)
{
	if (!L)
		ErrorMessage("δ��ʼ��Luaȫ��״̬");
	auto stack = lua_gettop(L);
	lua_pushcfunction(L, function);
	if (!lua_iscfunction(L, -1))
		ErrorMessage("��ַ%p����C++����", function);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("����C++����%p����", function);
	lua_settop(L, stack);
}