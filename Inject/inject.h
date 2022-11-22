#pragma once
#include <Windows.h> 
#include <string>
#include <vector>
#include <iostream>
#include <process.h>
#include <processsnapshot.h>
#include <tlhelp32.h>


struct REGION {
	PBYTE BaseAddress;
	PBYTE AllocationBase;
	DWORD Protect;
	DWORD AllocationProtect;
	DWORD State;
	DWORD Type;
	size_t RegionSize;
};

class HOOK
{
private:
	DWORD target_pid;
	BOOL myself;
	HANDLE target_handle;
public:
	HOOK(DWORD pid);
	~HOOK();
	/*
	远程分配读写内存
	size	分配的内存大小
	*/
	PVOID malloc(size_t size = 0x1000);
	/*
	获取内存的分配信息
	*/
	std::vector< REGION> getRegions();
	/*
	远程分配读写执行内存
	baseAddr	要分配的地址应该尽可能靠近此地址
	size		分配的内存大小
	*/
	PVOID mmapexec(PVOID baseAddr = 0, size_t size = 0x1000);
	/*
	释放分配的内存
	freeAddr	要释放的内存的地址
	*/
	BOOL free(PVOID freeAddr);
	/*
	远程线程执行远程函数
	funcAddr	远程函数地址
	params		远程函数的参数，注意参数如果只有一个只需要强制转换成PVOID即可，
	如果参数有多个就要使用结构体把结构体写到远程内存然后把远程地址传进去即可
	*/
	DWORD call(PVOID funcAddr, PVOID params);
	/*
	向指定的进程注入相应的模块
    dllPath		被注入的dll的完整路径
	*/
	PVOID dllinject(LPCWSTR dllPath);
	/*
	从进程中移除模块，注入几次就要移除几次
	dllPath		被要移除的dll的模块名称或完整路径
	*/
	BOOL dlleject(LPCWSTR dllNamePath);
	/*
	读取远程进程的内存地址的数值
	address		远程进程的内存地址
	readdata	本地进程的内存地址
	readBytes	要读取的数据的字节数
	*/
	BOOL readdata(PVOID address, PVOID readdata, size_t readBytes);
	/*
	写入远程进程的内存地址的数值
	address		远程进程的内存地址
	writedata	本地进程的内存地址
	writeBytes	要写入的数据的字节数
	*/
	BOOL writedata(PVOID address, LPCVOID writedata, size_t writeBytes);
	/*
	只能解析int, llong, jmp，call，两位的十六进制
	*/
	size_t writecode(PVOID address, LPCSTR code, ...);
	/*
	把函数复制到新的地址，并返回原始函数
	*/
	PVOID copyfunc(PVOID from_address, PVOID to_address, int copy_lens = 5, PVOID ret_address = 0);
	/*
	对一个函数使用inline hook
	new_address		把原始函数hook后执行的地址
	save_address	保存原始函数的地址，可以设置为0表示不需要保存原始函数
	hook_address	需要hook的原始函数
	*/
	BOOL inlifhook(PVOID new_address, PVOID save_address, PVOID hook_address);
	/*
	获取一个模块的import table address
	module_handle	要获取的模块所在的地址
	func_module		要获取的函数所在的模块名称，区分大小写，输入0表示任何模块都可以
	func_name		要获取的函数的名称
	*/
	PBYTE getiat(PVOID module_handle, LPCWSTR func_module, LPCSTR func_name);
	/*
	hook指定模块的import table address
	module_handle	要hook的模块所在的地址
	func_module		要hook的函数所在的模块名称，区分大小写，输入0表示任何模块都可以
	func_name		要hook的函数的名称
	new_address		把func_name处的导入地址改成new_address，使得module_handle模块的func_name调用变成new_address调用
	*/
	BOOL iathook(PVOID module_handle, LPCWSTR func_module, LPCSTR func_name, PVOID new_address);
	/*
	获取指定dll文件的symbol信息
	module		dll的首地址，也就是句柄
	func_name	dll中指定symbol的名称
	*/
	PVOID dllsym(PVOID module_handle, LPCSTR func_name);
};

class Eile
{
private:
	FILE* fw;
public:
	Eile(const char* path);
	~Eile();
	int write(const char* format, ...);
};

DWORD getpid(LPCWSTR name);
DWORD getpid(LPCSTR name);
PBYTE get_module_addr(DWORD target_pid, LPCWSTR dllNamePath);
std::vector< MODULEENTRY32> get_module_aaddr(DWORD target_pid);
BOOL getSpecDll(DWORD target_pid, LPCVOID funcAddr, LPWSTR dllName, size_t cDllSize);
PBYTE get_remote_addr(DWORD target_pid, LPCVOID localAddr);
PBYTE createSharedMemory(LPCSTR name, size_t size);
PBYTE createSharedMemory(LPCWSTR name, size_t size);
PBYTE openSharedMemory(LPCSTR name, size_t size);
PBYTE openSharedMemory(LPCWSTR name, size_t size);
/*
查找本地进程的内存 [start, end)，返回找到的地址的首地址，如果查找到两个就返回NULL
start		初始地址，包括
end			结束地址，不包括
code		字节型字符串，如 02 43 78 F2 e3 Ba ? ???? 09，但是 ?2 / 9 / F00 这种类型是不可以的
*/
PVOID searchLocalBytes(PVOID start, size_t range_size, LPCSTR code);