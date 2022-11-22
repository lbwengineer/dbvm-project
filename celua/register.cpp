#include "../lua53/src/lua.hpp"
#include "../Inject/luacall.hpp"
#include "../tool/md5.h"
#include "../tool/JsonObject.h"
#include "connect.h"
#include "globalvalue.h"
#include "luaglobal.h"

using namespace std;

int register_getcrcClick(lua_State* L)
{
	std::string data[4]{ "username", lua_getvalue<std::string>(L, 0, "register.email.Text"),"type","reg"};
	auto response = encPost("https://www.baidu.com", data, 2);
	auto json = det::JsonObject(response);
	if (json.getInt("code") == 200)
		MessageBoxA(0, "��֤���Ѿ����͵��������", "������֤��", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "��������ʧ�ܣ���������������ӡ�", "������֤��", 0);
	else 
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "������֤��", 0);
	return 0;
}

int register_registerClick(lua_State* L)
{
	std::string data[8]{ "username", lua_getvalue<std::string>(L, 0, "register.email.Text"),
		"password", lua_getvalue<std::string>(L, 0, "register.password.Text") ,
		"markcode",lua_getvalue<std::string>(L, 0, "register.crc.Text"),
		"machine_code","1" };
	auto response = encPost("https://www.baidu.com", data, 4);
	auto json = det::JsonObject(response);
	if (json.getInt("code") == 200)
		MessageBoxA(0, "ע��ɹ�", "ע������", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "��������ʧ�ܣ���������������ӡ�", "ע������", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "ע������", 0);
	return 0;
}