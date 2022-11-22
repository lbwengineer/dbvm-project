#pragma once
#include <Windows.h>
#include <locale>
#include <xstring>

/*		10	--->>>	|||||||||||||栈顶|||||||||||||||		<<<---	0			栈顶位置用lua_gettop、lua_settop设置
		9	--->>>		   函数返回第1个结果				<<<---  -1			<<<-------lua_gettop(L)指向这里
		8	--->>>	       函数返回第2个结果				<<<---  -2
					················		
		5	--->>>		   函数入栈第3个参数				<<<---  -5
		4	--->>>		   函数入栈第2个参数				<<<---  -6
		3	--->>>		   函数入栈第1个参数				<<<---  -7			对象用lua_getfield获取
		2	--->>>		函数地址在Lua空间里的对象			<<<---  -8			
					················
		0	--->>>	|||||||||||||栈底|||||||||||||||
*/

/*
执行lua里的函数
lua_State*		Lua当前的运行空间
local_stack		为NULL时表示function_name函数是全局变量，否则是局部变量
function_name	函数名称，第一级必须是全局变量，如global.local.function
args			函数的参数
*/
template <class Result, class... Args>
Result lua_function(lua_State* L, int local_stack, const char* function_name, const Args& ...args);
/*
执行lua里的函数，不关心函数的返回值
lua_State*		Lua当前的运行空间
local_stack		为NULL时表示function_name函数是全局变量，否则是局部变量
function_name	函数名称，第一级必须是全局变量，如global.local.function
args			函数的参数
*/
template <class... Args>
void lua_vfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args);
/*
执行C++里的函数
lua_State*		Lua当前的运行空间
function		C++函数，实际上是一个地址
args			函数的参数
*/
template <class Result, class... Args>
Result lua_cfunction(lua_State* L, lua_CFunction function, const Args& ...args);
/*
执行C++里的函数，不关心函数的返回值
lua_State*		Lua当前的运行空间
function		C++函数，实际上是一个地址
args			函数的参数
*/
template <class... Args>
void lua_cvfunction(lua_State* L, lua_CFunction function, const Args& ...args);
/*
执行lua里的函数，返回值直接放到栈顶
lua_State*		Lua当前的运行空间
local_stack		为NULL时表示函数是全局变量，否则是局部变量
function_name	函数名称，第一级必须是全局变量，如global.local.function
args			函数的参数
*/
template <class... Args>
void lua_lfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args);
/*
获取多重变量的指定值
lua_State*		Lua当前的运行空间
local_stack		为NULL时表示value_name是全局变量，否则是局部变量
value_name		函数名称，第一级必须是全局变量，对于table中的index必须使用十进制，如global.[12].function，global.[12]代表 global[12]
*/
template<class Result>
Result lua_getvalue(lua_State* L, int local_stack, const char* value_name);
/*
设置多重变量为指定值
lua_State*		Lua当前的运行空间
local_stack		为NULL时表示value_name是全局变量，否则是局部变量
value_name		变量名称，第一级必须是全局变量，对于table中的index必须使用十进制，如global.[12].function，global.[12]代表 global[12]
value			要设置的变量的值
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
	MessageBoxA(0, message, "Lua语言的错误", MB_OK);
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

static inline std::string push_global_stack(lua_State* L, int beforeset, const char* point_name, bool isAllPush)//返回最后一个变量是否是数字
{
	auto strsize = strlen(point_name) + 1;
	char* allstr = new char[strsize];
	strcpy_s(allstr, strsize, point_name);
	char* endptr = 0, *last = 0;
	char* point = strtok_s(allstr, ".", &endptr);
	char line[0x100] = "此变量调用为空";
	if (beforeset == 0) {
		if (!lua_getglobal(L, point))
			ErrorMessage("无法找到全局变量%s", point);
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
		ErrorMessage("未初始化Lua全局状态");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, function_name, true);
	if (!lua_isfunction(L, -1))
		ErrorMessage("变量%s不是Lua函数", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("调用Lua函数%s出错", function_name);
	auto get = luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}

template <class... Args>
void lua_vfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args)
{
	if (!L)
		ErrorMessage("未初始化Lua全局状态");
	auto stack = lua_gettop(L);
	push_global_stack(L, local_stack, function_name, true);
	if (!lua_isfunction(L, -1))
		ErrorMessage("变量%s不是Lua函数", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("调用Lua函数%s出错", function_name);
	lua_settop(L, stack);
}

template <class... Args>
void lua_lfunction(lua_State* L, int local_stack, const char* function_name, const Args& ...args)
{
	if (!L)
		ErrorMessage("未初始化Lua全局状态");
	push_global_stack(L, local_stack, function_name, true);
	if (!lua_isfunction(L, -1))
		ErrorMessage("变量%s不是Lua函数", function_name);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("调用Lua函数%s出错", function_name);
}

template <class Result>
Result lua_getvalue(lua_State* L, int local_stack, const char* value_name)
{
	if (!L)
		ErrorMessage("未初始化Lua全局状态");
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
		ErrorMessage("未初始化Lua全局状态");
	auto stack = lua_gettop(L);
	auto key = push_global_stack(L, local_stack, value_name, false);
	luam_set_value(L, key.c_str(), value);
	lua_settop(L, stack);
}

template <class Result, class... Args>
Result lua_cfunction(lua_State* L, lua_CFunction function, const Args& ...args)
{
	if (!L)
		ErrorMessage("未初始化Lua全局状态");
	auto stack = lua_gettop(L);
	lua_pushcfunction(L, function);
	if (!lua_iscfunction(L, -1))
		ErrorMessage("地址%p不是C++函数", function);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("调用C++函数%p出错", function);
	auto get = luam_get_result<Result>(L);
	lua_settop(L, stack);
	return get;
}

template <class... Args>
void lua_cvfunction(lua_State* L, lua_CFunction function, const Args& ...args)
{
	if (!L)
		ErrorMessage("未初始化Lua全局状态");
	auto stack = lua_gettop(L);
	lua_pushcfunction(L, function);
	if (!lua_iscfunction(L, -1))
		ErrorMessage("地址%p不是C++函数", function);
	arg_push_all(L, args...);
	if (lua_pcall(L, sizeof...(args), 1, 0))
		ErrorMessage("调用C++函数%p出错", function);
	lua_settop(L, stack);
}