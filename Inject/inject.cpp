#include "inject.h"
#include "distorm.h"

HOOK::HOOK(DWORD target_pid)
{
    if (target_pid == 0)
        printf("Process id is 0. Hook may cause failure.\n");
    HOOK::target_pid = target_pid;
	myself = target_pid == _getpid();
    target_handle = OpenProcess(PROCESS_ALL_ACCESS, false, target_pid);
    if (target_handle == NULL)
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("dllinject : OpenProcess(%d) failed: %s\n", target_pid, error);
    }
}

HOOK::~HOOK()
{
    CloseHandle(target_handle);
}

LPVOID HOOK::malloc(size_t size)
{
	LPVOID remoteM;
	if (myself)
		remoteM = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
	else
		remoteM = VirtualAllocEx(target_handle, 0, size, MEM_COMMIT, PAGE_READWRITE);
    if (remoteM == NULL)
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("hook : malloc(%zx) failed: %s\n", size, error);
    }
    return remoteM;
}
std::vector< REGION> HOOK::getRegions()
{
	HPSS pt;
	auto  hsnapshot = PssCaptureSnapshot(target_handle, PSS_CAPTURE_VA_SPACE, 0, &pt);
	HPSSWALK wk;
	PSS_VA_SPACE_ENTRY entry;
	std::vector< REGION> regions;
	if (PssWalkMarkerCreate(0, &wk))
		return regions;
	while (!PssWalkSnapshot(pt, PSS_WALK_VA_SPACE, wk, &entry, sizeof(entry)))
	{
		REGION rg;
		rg.BaseAddress = (PBYTE) entry.BaseAddress;
		rg.Protect = entry.Protect;
		rg.AllocationBase = (PBYTE) entry.AllocationBase;
		rg.AllocationProtect = entry.AllocationProtect;
		rg.State = entry.State;
		rg.Type = entry.Type;
		rg.RegionSize = entry.RegionSize;
		regions.emplace_back(rg);
	}
	PssWalkMarkerFree(wk);
	return regions;
}
LPVOID HOOK::mmapexec(LPVOID baseAddr, size_t size)
{
	LPVOID remoteM;
	if (myself)
		remoteM = VirtualAlloc(baseAddr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	else
		remoteM = VirtualAllocEx(target_handle, baseAddr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (remoteM == NULL)
    {
		auto regions = getRegions();
		int mid = regions.size() - 1;
		for (; mid >= 0; mid--)
			if (baseAddr >= regions[mid].BaseAddress && baseAddr < regions[mid].BaseAddress + regions[mid].RegionSize)
				break;
		LPVOID left = 0, right = 0, reset;
		for (int i = mid; i < regions.size(); i++) {
			if (regions[i].State == MEM_FREE) {
				auto base = (uint64_t)regions[i].BaseAddress & ~(uint64_t)0xffff;
				if (base != (uint64_t)regions[i].BaseAddress)
					base += 0x10000;
				if (base + size <= (uint64_t)regions[i].BaseAddress + regions.size()) {
					right = (LPVOID)base;
					break;
				}
			}
		}
		for (int i = mid; i >= 0; i--) {
			if (regions[i].State == MEM_FREE) {
				auto base = ((uint64_t)regions[i].BaseAddress + regions[i].RegionSize - size) & ~(uint64_t)0xffff;
				if (base >= (uint64_t)regions[i].BaseAddress) {
					left = (LPVOID)base;
					break;
				}
			}
		}
		if (left == 0 && right == 0) {
			printf("Can't find enough free space to alloc memory.\n");
			return 0;
		}
		else if (left == 0)
			reset = right;
		else if (right == 0)
			reset = left;
		else
			reset = (uint64_t)baseAddr - (uint64_t)left < (uint64_t)right - (uint64_t)baseAddr ? left : right;
		if (myself)
			remoteM = VirtualAlloc(reset, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		else
			remoteM = VirtualAllocEx(target_handle, reset, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }
    if (remoteM == NULL)
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("hook : mmapexec(%p, %zx) failed: %s\n", baseAddr, size, error);
    }
    return remoteM;
}

BOOL HOOK::free(LPVOID freeAddr)
{
	if (myself)
		return VirtualFree(freeAddr, 0, MEM_RELEASE);
	else 
		return VirtualFreeEx(target_handle, freeAddr, 0, MEM_RELEASE);
}

DWORD HOOK::call(LPVOID funcAddr, LPVOID params)
{
    auto createThread = CreateRemoteThread(target_handle, NULL, 0, (LPTHREAD_START_ROUTINE)funcAddr, params, 0, NULL);
    if (createThread == NULL)//执行远程线程
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("hook : CreateRemoteThread() failed: %s\n", error);
        return -1;
    }
    return WaitForSingleObject(createThread, INFINITE);
}

LPVOID HOOK::dllinject(LPCWSTR dllPath)
{
    size_t dwBufSize = (wcslen(dllPath) + 1) * sizeof(TCHAR);//开辟的内存的大小
    LPVOID pRemoteBuf = malloc(dwBufSize);//在目标进程空间开辟一块内存
    if (pRemoteBuf == NULL)
        return 0;
    if (!writedata(pRemoteBuf, dllPath, dwBufSize))//向开辟的内存复制dll的路径
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("dllinject : writedata(%p) failed: %s\n", pRemoteBuf, error);
        free(pRemoteBuf);
        return 0;
    }
    DWORD exitC = call(LoadLibraryW, pRemoteBuf);//执行远程LoadLibraryW函数
    free(pRemoteBuf);
    if (exitC != 0)//远程线程异常结束
        return 0;
    return get_module_addr(target_pid, dllPath);//确认结果
}

BOOL HOOK::dlleject(LPCWSTR dllNamePath)
{
    auto all_module = get_module_aaddr(target_pid);
    MODULEENTRY32W me { 0 };
    for (auto& get : all_module)
    {
        if (!wcscmp(dllNamePath, get.szModule) || !wcscmp(dllNamePath, get.szExePath))
        {
            me = get;
            break;
        }
    }
    if (me.dwSize == 0)
    {
        printf("hook : dlleject(\"%S\") not found.\n", dllNamePath);
        return false;
    }
    if (call(FreeLibrary, me.modBaseAddr) != 0)//远程线程异常结束
        return false;
    return !get_module_addr(target_pid, dllNamePath);//确认结果
}

BOOL HOOK::readdata(LPVOID address, LPVOID readdata, size_t readBytes)
{
	if (myself) {
		MEMORY_BASIC_INFORMATION info{};
		VirtualQuery(address, &info, sizeof(info));
		DWORD flag = 0;
		if ((info.Protect & PAGE_READONLY) == 0) {
			if (!VirtualProtect(address, readBytes, PAGE_EXECUTE_READWRITE, &flag))
				return false;
		}
		memcpy(readdata, address, readBytes);
		if (flag != 0)
			return VirtualProtect(address, readBytes, flag, &flag);
		return true;
	}
	if (!ReadProcessMemory(target_handle, address, readdata, readBytes, 0))
	{
		DWORD flag;
		if (!VirtualProtectEx(target_handle, address, readBytes, PAGE_EXECUTE_READWRITE, &flag))
			return false;
		if (!ReadProcessMemory(target_handle, address, readdata, readBytes, 0))
			return false;
		return VirtualProtectEx(target_handle, address, readBytes, flag, &flag);
	}
	return true;
}

BOOL HOOK::writedata(LPVOID address, LPCVOID writedata, size_t writeBytes)
{
	if (myself) {
		MEMORY_BASIC_INFORMATION info{};
		VirtualQuery(address, &info, sizeof(info));
		DWORD flag = 0;
		if ((info.Protect & PAGE_WRITECOPY) == 0) {
			if (!VirtualProtect(address, writeBytes, PAGE_EXECUTE_READWRITE, &flag))
				return false;
		}
		memcpy(address, writedata, writeBytes);
		if (flag != 0)
			return VirtualProtect(address, writeBytes, flag, &flag);
		return true;
	}
	if (!WriteProcessMemory(target_handle, address, writedata, writeBytes, 0))
	{
		DWORD flag;
		if (!VirtualProtectEx(target_handle, address, writeBytes, PAGE_EXECUTE_READWRITE, &flag))
			return false;
		if (!WriteProcessMemory(target_handle, address, writedata, writeBytes, 0))
			return false;
		VirtualProtectEx(target_handle, address, writeBytes, flag, &flag);
	}
	return true;
}

Eile::Eile(const char* path)
{
	if (fopen_s(&fw, path, "a+") != 0)
		fw = 0;
}
Eile::~Eile()
{
	if (fw != 0)
		fclose(fw);
}
int Eile::write(const char* format, ...)
{
	if (fw == 0)
		return -1;
	va_list list;
	va_start(list, format);
	auto s = vfprintf(fw, format, list);
	va_end(list);
	fflush(fw);
	return s;
}


DWORD getpid(LPCSTR name) {
	auto len = strlen(name) + 1;
	auto newname = new WCHAR[len];
	MultiByteToWideChar(CP_ACP, 0, name, len, newname, len);
	auto pid = getpid(newname);
	delete[] newname;
	return pid;
}
DWORD getpid(LPCWSTR name)
{
    HANDLE  hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hsnapshot == INVALID_HANDLE_VALUE)
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("getpid : CreateToolhelp32Snapshot failed: %s\n", error);
        return 0;
    }
    PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };
    int flag = Process32First(hsnapshot, &pe);
    while (flag != 0)
    {
        if (wcscmp(pe.szExeFile, name) == 0)
        {
            CloseHandle(hsnapshot);
            return pe.th32ProcessID;
        }
        flag = Process32Next(hsnapshot, &pe);
    }
    CloseHandle(hsnapshot);
    return 0;
}

PBYTE get_module_addr(DWORD target_pid, LPCWSTR dllNamePath)
{
    auto all = get_module_aaddr(target_pid);
    for (auto& get : all)
    {
        if (!wcscmp(dllNamePath, get.szModule) || !wcscmp(dllNamePath, get.szExePath))
            return get.modBaseAddr;
    }
    return 0;
}
std::vector<MODULEENTRY32> get_module_aaddr(DWORD target_pid)
{
    HANDLE                  hSnapshot = INVALID_HANDLE_VALUE;
    MODULEENTRY32           mt{ sizeof(MODULEENTRY32) };
    std::vector<MODULEENTRY32> actor;
    if (INVALID_HANDLE_VALUE == (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, target_pid)))//获得进程的快照
    {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
        printf("get_module_addr : CreateToolhelp32Snapshot(%d) failed: %s\n", target_pid, error);
        return actor;
    }
    BOOL bMore = Module32First(hSnapshot, &mt);//遍历进程内得的所有模块
    for (; bMore; bMore = Module32Next(hSnapshot, &mt))
    {
        actor.emplace_back(mt);
    }
    CloseHandle(hSnapshot);
    return actor;
}
BOOL getSpecDll(DWORD target_pid, LPCVOID funcAddr, LPWSTR dllName, size_t cDllSize)
{
    auto all = get_module_aaddr(target_pid);
    for (auto& get : all)
    {
        if (funcAddr >= get.modBaseAddr && funcAddr < get.modBaseAddr + get.modBaseSize)
        {
            wcscpy_s(dllName, cDllSize, get.szModule);
            return true;
        }
    }
    return false;
}

PBYTE get_remote_addr(DWORD target_pid, LPCVOID localAddr)
{
    wchar_t find_module[0x50];
    DWORD myself = _getpid();
    if (!getSpecDll(myself, localAddr, find_module, sizeof(find_module) / 2))
    {
        printf("get_remote_addr(%d, %p) : getSpecDll failed.\n", target_pid, localAddr);
        return 0;
    }
    auto local = get_module_addr(myself, find_module);
    auto remote = get_module_addr(target_pid, find_module);
    if (remote == 0)
    {
        printf("get_remote_addr(%d, %p) : remote module \"%S\" not found.\n", target_pid, localAddr, find_module);
        return 0;
    }
    return remote + ((PBYTE)localAddr - local);
}

#define PTROFF(base, off) (uint64_t)&((base*) 0)->off

size_t HOOK::writecode(LPVOID address, LPCSTR code, ...)
{
	va_list args;
	va_start(args, code);
	std::vector<std::string> opline;
	auto strsize = strlen(code) + 1;
	auto line = new char[strsize];
	strcpy_s(line, strsize, code);
	char* point = 0;
	char* hex = strtok_s(line, " ", &point);
	if (hex == NULL) {
		printf("Writecode for address %p has NULL code.\n", address);
		delete[] line;
		return -1;
	}
	opline.emplace_back(hex);
	while ((hex = strtok_s(NULL, " ", &point)))
		opline.emplace_back(hex);
	delete[] line;
	char* writein = (char*)address;
	for (auto& op : opline)
	{
		if (op == "int")
		{
			int form = va_arg(args, int);
			writedata(writein, &form, 4);
			writein += 4;
		}
		else if (op == "llong")
		{
			long long form = va_arg(args, long long);
			writedata(writein, &form, 8);
			writein += 8;
		}
		else if (op == "jmp")
		{
			char* target = va_arg(args, char*);
			long long test_jmp = target - writein - 2;
			if (test_jmp >= (char)1 << 7 && test_jmp <= 0xffu >> 1)
			{
				char jmps[2] = { 0xEB, test_jmp };
				writedata(writein, &jmps, 2);
				writein += 2;
			}
			else if (test_jmp -= 3, test_jmp >= (int)1 << 31 && test_jmp <= 0xffffffffu >> 1)
			{
				char jmps[5] = { 0xE9 };
				memcpy(jmps + 1, &test_jmp, 4);
				writedata(writein, &jmps, 5);
				writein += 5;
			}
			else
			{
				printf("jmp too far from %p to %p.\n", writein, target);
				return -1;
			}
		}
		else if (op == "call")
		{
			char* target = va_arg(args, char*);
			long long test_call = target - writein - 5;
			if (test_call >= (int)1 << 31 && test_call <= 0xffffffffu >> 1)
			{
				char calls[5] = { 0xE8, test_call };
				memcpy(calls + 1, &test_call, 4);
				writedata(writein, &calls, 5);
				writein += 5;
			}
			else
			{
				printf("call too far from %p to %p.\n", writein, target);
				return -1;
			}
		}
		else
		{
			if (op.size() != 2)
			{
				printf("Op code for \"%s\" is unknown.\n", op.c_str());
				return -1;
			}
			char form = strtol(op.c_str(), 0, 16);
			writedata(writein, &form, 1);
			writein++;
		}
	}
	va_end(args);
	return (size_t)(writein - address);
}

static LPVOID fix_call_func(HOOK* thiz, byte code[], LPVOID from_address, LPVOID to_address, int copy_lens, LPVOID ret_address) {
	if (code[0] == 0xE8) {//call lable
		struct {
			char OP;
			int R;
		} push{ 0xE8 };
		int data = 0;
		memcpy(&data, code + 1, 4);
		auto rva = data + (uint64_t)from_address - (uint64_t)to_address;
		if ((rva & ~(uint64_t)0xffffffff) != 0)
		{
			printf("Fix call too far, from_address = %p, to_address = %p, rva = %016llx.\n", from_address, to_address, rva);
			return 0;
		}
		push.R = rva;
		if (!thiz->writedata(to_address, &push, 5)) {
			char error[0x100];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
			printf("writedata for copyfunc to %p failed: %s\n", to_address, error);
			return 0;
		}
		return thiz->copyfunc((PBYTE)from_address + 5, (PBYTE)to_address + 5, copy_lens - 5, (PBYTE)ret_address);
	}
	return 0;
}

static LPVOID fix_jmp_func(HOOK* thiz, byte code[], LPVOID from_address, LPVOID to_address, int copy_lens, LPVOID ret_address) {
	auto putJmp = [&](int R) -> LPVOID
	{// R = 偏移量 + 原始jmp指令长度 - 5
		struct {
			char OP;
			uint32_t R;
		} push{ 0xE9 };
		auto rva = R + (uint64_t)from_address - (uint64_t)to_address;
		if ((rva & ~(uint64_t)0xffffffff) != 0)
		{
			printf("Fix jmp too far, from_address = %p, to_address = %p, rva = %016llx.\n", from_address, to_address, rva);
			return 0;
		}
		push.R = rva;
		if (!thiz->writedata(to_address, &push, 5)) {
			char error[0x100];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
			printf("writedata for copyfunc to %p failed: %s\n", to_address, error);
			return 0;
		}
		return thiz->copyfunc((PBYTE)from_address + 5, (PBYTE)to_address + 5, copy_lens - 5, (PBYTE)ret_address);
	};
	if (code[0] == 0xEB) {//jmp short
		char r = 0;
		memcpy(&r, code + 1, 1);
		return putJmp(r + 2 - 5);
	}
	if (code[0] == 0xE9) {//jmp local
		int r = 0;
		memcpy(&r, code + 1, 4);
		return putJmp(r + 5 - 5);
	}
	if (code[0] == 0xFF && code[1] == 0x25) {//jmp far1
		int r = 0;
		memcpy(&r, code + 2, 4);
		return putJmp(r + 6 - 5);
	}
	if (code[0] == 0x48 && code[1] == 0xFF && code[2] == 0x25) {//jmp far2
		int r = 0;
		memcpy(&r, code + 3, 4);
		return putJmp(r + 7 - 5);
	}
	return 0;
}
static BOOL cquals(const char* cstr, ...) //最后一个参数必须是0才能终止函数的运行
{
	va_list all;
	va_start(all, cstr);
	const char* more;
	while (more = va_arg(all, char*))
	{
		if (strcmp(cstr, more) != 0)
			return FALSE;
	}
	return TRUE;
}
static LPVOID fix_copy_func(HOOK* thiz, byte code[], LPVOID from_address, LPVOID to_address, int copy_lens, LPVOID ret_address) {
#ifdef _M_X64
	_DecodeType dt = Decode64Bits;
#else
	_DecodeType dt = Decode32Bits;
#endif
	_DecodedInst decodedInstructions[0x10];
	uint32_t insCount = 0;
	_DecodeResult res = distorm_decode((_OffsetType)from_address, code, 15, dt, decodedInstructions, 0x10, &insCount);
	const char* opcode = (char* )decodedInstructions[0].operands.p;
	if (cquals(opcode, "MOV", "MOVZX", "PUSH", "POP"))
	{
		auto copy = decodedInstructions[0].size;
		if (!thiz->writedata(to_address, code, copy)) {
			char error[0x100];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
			printf("writedata for copyfunc to %p failed: %s\n", to_address, error);
			return 0;
		}
		return thiz->copyfunc((PBYTE)from_address + copy, (PBYTE)to_address + copy, copy_lens - copy, (PBYTE)ret_address);
	}
	else
	{
		printf("Hex %s for code %s in %p cannot convert to known code.\n", decodedInstructions[0].instructionHex.p,
			decodedInstructions[0].mnemonic.p, from_address);
		return 0;
	}
}

LPVOID HOOK::copyfunc(LPVOID from_address, LPVOID to_address, int copy_lens, LPVOID ret_address)
{
	if (copy_lens <= 0) {
		if (-1 == writecode(to_address, "jmp", from_address))
			return 0;
		return ret_address;
	}
	if (ret_address == 0)
		ret_address = from_address;
	byte code[15] = {};
	if (!readdata(from_address, code, 15)) {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
		printf("readdata for copyfunc from %p failed: %s\n", from_address, error);
		return 0;
	}
	if ((code[0] == 0xC3 && copy_lens > 1) || (code[0] == 0xC2 && copy_lens > 3)) {//无法inline hook短函数
		printf("function %p end too early left %d bytes\n", to_address, copy_lens);
		return 0;
	}
	//修复指令
	LPVOID fix;
	fix = fix_call_func(this, code, from_address, to_address, copy_lens, ret_address);
	if (fix)
		return fix;
	fix = fix_jmp_func(this, code, from_address, to_address, copy_lens, ret_address);
	if (fix)
		return fix;
	return fix_copy_func(this, code, from_address, to_address, copy_lens, ret_address);
}

BOOL HOOK::inlifhook(LPVOID new_address, LPVOID save_address, LPVOID hook_address)
{
	if (hook_address == 0)
		return false;
	auto mmap_original = mmapexec(hook_address);
	if (mmap_original == 0) {
		return false;
	}
	auto fixedAddress = copyfunc(hook_address, mmap_original);
	if (fixedAddress == 0)
		return false;
	if (save_address != 0) {
		if (!writedata(save_address, &mmap_original, sizeof(mmap_original))) {
			char error[0x100];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
			printf("Write save address %p failed: %s\n", save_address, error);
			return false;
		}
	}
	else {
		free(mmap_original);
	}
	return -1 != writecode(fixedAddress, "jmp", new_address);
}
BOOL HOOK::iathook(LPVOID module_handle, LPCWSTR func_module, LPCSTR func_name, LPVOID new_address)
{
	if (module_handle == 0) {
		printf("module_handle is NULL.\n");
		return false;
	}
	auto iatr = getiat(module_handle, func_module, func_name);
	if (iatr == 0) {
		printf("Can't find function \"%s\".\n", func_name);
		if (func_module)
			printf("Function in module \"%S\".\n", func_module);
		return false;
	}
	if (!writedata(iatr, &new_address, sizeof(LPVOID))) {
		char error[0x100];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 0, error, 0x100, 0);
		printf("Writedata for iat address %p failed: %s\n", iatr, error);
		return false;
	}
	return true;
}
PBYTE HOOK::getiat(LPVOID module_handle, LPCWSTR func_module, LPCSTR func_name)
{
	auto base = (PBYTE)module_handle;
	LONG e_lfanew = 0;
	readdata(base + PTROFF(IMAGE_DOS_HEADER, e_lfanew), &e_lfanew, sizeof(e_lfanew));
	auto optionalHeader = e_lfanew + PTROFF(IMAGE_NT_HEADERS64, OptionalHeader);
	auto importHeader = PTROFF(IMAGE_OPTIONAL_HEADER, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_DIRECTORY_ENTRY_IMPORT;
	IMAGE_IMPORT_DESCRIPTOR import_image{};
	IMAGE_DATA_DIRECTORY image_directory{};
	readdata(base + optionalHeader + importHeader, &image_directory, sizeof(image_directory));
	auto funcFit = [&]() -> PBYTE
	{//单独一个模块里搜索函数名
		auto func_len = strlen(func_name);
		auto funcn = new char[func_len + 1]{ 0 };
		for (size_t i = 0; ; i++)//准备遍历
		{
			PBYTE rva_str = 0;//保存指向字符串的偏移量
			readdata(base + import_image.Characteristics + i * sizeof(rva_str), &rva_str, sizeof(rva_str));
			if (rva_str == NULL)
				break;
			readdata(base + (uint64_t)rva_str + 2, funcn, func_len);//获取字符串，前面有两个无关字节
			if (strcmp(funcn, func_name) == 0)
			{
				delete[] funcn;
				return base + import_image.FirstThunk + i * sizeof(rva_str);
			}
		}
		delete[] funcn;
		return NULL;
	};
	size_t module_len = func_module ? (wcslen(func_module) + 1) * 2 : 0;
	char* get_module = 0, * turn_module = 0;
	if (func_module)
	{
		get_module = new char[module_len] { 0 };
		turn_module = new char[module_len] { 0 };
		BOOL turn_ok = FALSE;
		WideCharToMultiByte(CP_ACP, 0, func_module, module_len / 2, turn_module, module_len, 0, &turn_ok);
		if (turn_ok)
		{
			delete[] get_module;
			delete[] turn_module;
			printf("Module of function can't convert from wchar_t \"%S\"to char*.\n", func_module);
			return 0;
		}
	}
	for (size_t i = 0; i < image_directory.Size; i += sizeof(IMAGE_IMPORT_DESCRIPTOR))
	{
		readdata(base + image_directory.VirtualAddress + i, &import_image, sizeof(import_image));
		if (func_module == 0)
		{
			auto addr = funcFit();
			if (addr != NULL)
				return addr;
		}
		else
		{
			readdata(base + import_image.Name, get_module, module_len);
			if (strcmp(get_module, turn_module) == 0)//发现有模块相同便停止寻找，无论最后是否找到了也要返回结果
			{
				delete[] get_module;
				delete[] turn_module;
				return funcFit();
			}
		}
	}
	delete[] get_module;
	delete[] turn_module;
	return 0;
}

LPVOID HOOK::dllsym(LPVOID module_handle, LPCSTR func_name)
{
	auto base = (PBYTE)module_handle;
	LONG e_lfanew = 0;
	readdata(base + PTROFF(IMAGE_DOS_HEADER, e_lfanew), &e_lfanew, sizeof(e_lfanew));
	auto optionalHeader = e_lfanew + PTROFF(IMAGE_NT_HEADERS64, OptionalHeader);
	auto exportHeader = PTROFF(IMAGE_OPTIONAL_HEADER, DataDirectory) + sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_DIRECTORY_ENTRY_EXPORT;
	IMAGE_EXPORT_DIRECTORY export_image{};
	IMAGE_DATA_DIRECTORY image_directory{};
	readdata(base + optionalHeader + exportHeader, &image_directory, sizeof(image_directory));
	readdata(base + image_directory.VirtualAddress, &export_image, sizeof(export_image));
	size_t count = (export_image.AddressOfNames - export_image.AddressOfFunctions) / 4;
	auto funclen = strlen(func_name) + 1;
	char* matchName = new char[funclen];
	for (size_t i = 0; i < count; i++)
	{
		DWORD stroff;
		readdata(base + export_image.AddressOfNames + 4 * i, &stroff, 4);
		readdata(base + stroff, matchName, funclen);
		if (strcmp(matchName, func_name) == 0)
		{
			delete[] matchName;
			DWORD func;
			readdata(base + export_image.AddressOfFunctions + i * 4, &func, 4);
			return base + func;
		}
	}
	printf("undefined symbol: %s\n", func_name);
	return 0;
}

PBYTE createSharedMemory(LPCSTR name, size_t size)
{
	auto create = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, size&~(uint64_t)0xffffffff, size&0xffffffff, name);
	if (create == 0)
		return 0;
	auto map = MapViewOfFile(create, FILE_READ_ACCESS | FILE_WRITE_ACCESS, 0, 0, size);
	return (PBYTE)map;
}
PBYTE createSharedMemory(LPCWSTR name, size_t size)
{
	auto create = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, size & ~(uint64_t)0xffffffff, size & 0xffffffff, name);
	if (create == 0)
		return 0;
	auto map = MapViewOfFile(create, FILE_READ_ACCESS | FILE_WRITE_ACCESS, 0, 0, size);
	return (PBYTE)map;
}
PBYTE openSharedMemory(LPCSTR name, size_t size)
{
	auto op = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
	if (op == 0)
		return 0;
	auto map = MapViewOfFile(op, FILE_READ_ACCESS | FILE_WRITE_ACCESS, 0, 0, size);
	return (PBYTE)map;
}
PBYTE openSharedMemory(LPCWSTR name, size_t size)
{
	auto op = OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
	if (op == 0)
		return 0;
	auto map = MapViewOfFile(op, FILE_READ_ACCESS | FILE_WRITE_ACCESS, 0, 0, size);
	return (PBYTE)map;
}

LPVOID searchLocalBytes(LPVOID start, size_t range_size, LPCSTR code)
{
	struct CP {
		byte number;
		byte nop;
	};
	if (code[0] == 0)
		return 0;
	std::vector<CP> cp;
	char* get, * next = 0;
	char* newline = new char[strlen(code) + 1];
	strcpy_s(newline, strlen(code) + 1, code);
	get = strtok_s(newline, " ", &next);
	do
	{
		if (strstr(get, "?")) {
			auto len = strlen(get);
			for (size_t i = 0; i < len; i++)
				cp.emplace_back(CP{ 0,1 });
		}
		else if (get[2] != 0) {
			WCHAR error[0x200];
			wsprintf(error, L"本地内存查找时输入了不正确的字符串\"%hs\"", code);
			MessageBoxW(0, error, L"searchLocalBytes error", MB_ICONERROR);
			exit(-1);
		}
		else
			cp.emplace_back(CP{ (byte)strtoull(get, 0, 16),0 });
	} while ((get = strtok_s(0, " ", &next)) != 0);
	delete[] newline;
	auto cplen = cp.size();
	CP* mcp = new CP[cplen]{};
	for (size_t i = 0; i < cp.size(); i++)
		mcp[i] = cp[i];
	for (size_t i = 0; i <= range_size - cplen; i++)
	{
		bool allok = true;
		for (size_t j = 0; j < cplen; j++)
		{
			if (!mcp[j].nop && mcp[j].number != *((PBYTE)start + i + j)) {
				allok = false;
				break;
			}
		}
		if (allok) {
			delete[] mcp;
			return (PBYTE)start + i;
		}
	}
	delete[] mcp;
	return 0;
}

