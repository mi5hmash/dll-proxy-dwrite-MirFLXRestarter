#define WIN32_LEAN_AND_MEAN
#ifdef _WIN64
#pragma comment(linker,"/export:DWriteCreateFactory=C:\\Windows\\System32\\DWrite.DWriteCreateFactory,@1")
#elif defined(_WIN32)
#pragma comment(linker,"/export:DWriteCreateFactory=C:\\Windows\\SysWOW64\\DWrite.DWriteCreateFactory,@1")
#endif

#include <windows.h>
#include <psapi.h>
#include <cstdio>

namespace
{
    /// <summary>
    /// The current status of the MirFLX application
    /// </summary>
    int mirflx_current_status = 1;

    /// <summary>
    /// Restarts the current process
    /// </summary>
    void restart_process()
    {
        char sz_file_name[MAX_PATH];
        GetModuleFileNameA(nullptr, sz_file_name, MAX_PATH);

        STARTUPINFOA si = {};
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi;

        // Start a new instance of the current process
        if (!CreateProcessA(sz_file_name, GetCommandLineA(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
        {
            MessageBoxA(nullptr, "Failed to restart the process!", "Error", MB_ICONERROR);
            return;
        }

        // Close process and thread handles for the new instance
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Exit the current process
        ExitProcess(0);
    }

    /// <summary>
    /// Reads an integer from a specific address
    /// </summary>
    /// <param name="h_process"></param>
    /// <param name="address"></param>
    /// <returns></returns>
    int get_status(const HANDLE h_process, const LPCVOID address) {
        int status = 0;
        if (!ReadProcessMemory(h_process, address, &status, sizeof(status), nullptr)) {
            MessageBoxA(nullptr, "Failed to read memory!", "Error", MB_ICONERROR);
        }
        return status;
    }

    /// <summary>
    /// Monitors the MirFlx status
    /// </summary>
    /// <param name="module_name"></param>
    /// <param name="offset"></param>
    /// <param name="interval"></param>
    /// <returns></returns>
    BOOL WINAPI mir_flx_status_monitor(const LPCSTR module_name, const DWORD offset, const DWORD interval)
    {
        MODULEINFO module_info = {};

        // Get a handle to the current process
        const HANDLE h_process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
        if (h_process == nullptr) {
            MessageBoxA(nullptr, "Failed to open process!", "Error", MB_ICONERROR);
            return 0;
        }

        // Get the module handle
        const HMODULE h_module = GetModuleHandleA(module_name);
        if (h_module == nullptr) {
            char message[64] = {};
            sprintf_s(message, sizeof(message), "'%s' module not found!", module_name);
            MessageBoxA(nullptr, message, "Error", MB_ICONERROR);

            CloseHandle(h_process);
            return 0;
        }

        // Get the module information
        if (!GetModuleInformation(h_process, h_module, &module_info, sizeof(module_info))) {
            MessageBoxA(nullptr, "Failed to get module information!", "Error", MB_ICONERROR);
            CloseHandle(h_process);
            return 0;
        }

        // Calculate the address within the module
        const auto status_address = static_cast<const BYTE*>(module_info.lpBaseOfDll) + offset;

        // Check the application status at regular intervals
        while (mirflx_current_status == 1)
        {
            Sleep(interval);
            mirflx_current_status = get_status(h_process, status_address);
        }

        // Close the process handle
        CloseHandle(h_process);

        // Restart the current process
        restart_process();

        return 1;
    }

    DWORD WINAPI payload(LPVOID)
    {
#ifdef _WIN64
        // MirFLX v2.3
        // SHA256: BB2B9E99D1B91044C18036CA15074689DC0785A1F2DB0A5D6515A9FA0060BE1D
		// Offset for x64: clr.dll+912E54
        constexpr DWORD offset = 0x912E54;
#elif defined(_WIN32)
        // MirFLX v2.3
    	// SHA256: 60B17B1536032D78904032CC3D882633B0074847101E01303C9FA4E837217651
        // Offset for x86: clr.dll+749204
        constexpr DWORD offset = 0x749204; 
#endif
        return mir_flx_status_monitor("clr.dll", offset, 180000);
    }
}

/// <summary>
/// Main Function
/// </summary>
/// <param name="hinst_dll"></param>
/// <param name="fdw_reason"></param>
/// <param name="lp_reserved"></param>
/// <returns></returns>
BOOL WINAPI DllMain(HINSTANCE hinst_dll, const DWORD fdw_reason, LPVOID lp_reserved)
{
    switch (fdw_reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, payload, nullptr, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
    default:
        break;
    }
    return TRUE;
}