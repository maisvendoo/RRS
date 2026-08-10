#include    "crash-handler.h"

#include    <cstdio>
#include    <cstring>
#include    <ctime>
#include    <csignal>
#include    <sys/stat.h>
#include    <cstdarg>
#include    <cstdint>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#else
    #include <unistd.h>
    #include <execinfo.h>
    #include <dlfcn.h>
#endif

//------------------------------------------------------------------------------
// Глобальная переменная для хранения имени файла лога
//------------------------------------------------------------------------------
static char log_file_path[256] = "../logs/crash.log";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void removeOldLogFile()
{
#ifdef _WIN32
    DeleteFileA(log_file_path);
#else
    unlink(log_file_path);
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void initLogFile()
{
    // Создаем директорию ../logs если её нет
#ifdef _WIN32
    mkdir("../logs");
#else
    mkdir("../logs", 0777);
#endif

    // Удаляем старый файл
    removeOldLogFile();

    // Создаем новый файл
    FILE* file = fopen(log_file_path, "w");
    if (file)
    {
        time_t now = time(nullptr);
        struct tm* tm_info = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        fprintf(file, "========================================\n");
        fprintf(file, "=== Crash Handler Initialized ===\n");
        fprintf(file, "=== Log file: %s ===\n", log_file_path);
        fprintf(file, "=== Created at: %s ===\n", timestamp);
        fprintf(file, "========================================\n\n");
        fflush(file);
        fclose(file);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void writeToFile(const char* msg)
{
    FILE* file = fopen(log_file_path, "a");
    if (file)
    {
        fputs(msg, file);
        fflush(file);
        fclose(file);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void writeToFileFormatted(const char* format, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    writeToFile(buffer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#ifdef _WIN32
static void writeLoadedModules()
{
    HANDLE hProcess = GetCurrentProcess();
    HMODULE hMods[1024];
    DWORD cbNeeded;

    writeToFile("=== Loaded Modules ===\n");

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        int count = cbNeeded / sizeof(HMODULE);
        for (int i = 0; i < count && i < 50; i++) // Ограничим 50 модулями
        {
            char szModName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName)))
            {
                MODULEINFO modInfo;
                if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo)))
                {
                    // Берем только имя файла
                    const char* name = strrchr(szModName, '\\');
                    if (!name) name = strrchr(szModName, '/');
                    if (!name) name = szModName;
                    else name++;

                    writeToFileFormatted("  %-20s Base: 0x%016llX, Size: 0x%08X\n",
                                         name,
                                         (unsigned long long)modInfo.lpBaseOfDll,
                                         modInfo.SizeOfImage);
                }
            }
        }
    }
    writeToFile("\n");
}
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#ifdef _WIN32
static void writeStackTraceWithModules(void* stack[], unsigned short frames)
{
    HANDLE hProcess = GetCurrentProcess();
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        // Если не удалось получить модули, выводим голые адреса
        for (unsigned short i = 0; i < frames; ++i)
        {
            writeToFileFormatted("  #%03d: 0x%016llX\n",
                                 i, (unsigned long long)(uintptr_t)stack[i]);
        }
        return;
    }

    int moduleCount = cbNeeded / sizeof(HMODULE);

    for (unsigned short i = 0; i < frames; ++i)
    {
        uintptr_t addr = (uintptr_t)stack[i];
        const char* module_name = "unknown";
        uintptr_t offset = 0;
        int found = 0;

        // Ищем в каком модуле находится адрес
        for (int m = 0; m < moduleCount && !found; m++)
        {
            MODULEINFO modInfo;
            if (GetModuleInformation(hProcess, hMods[m], &modInfo, sizeof(modInfo)))
            {
                uintptr_t base = (uintptr_t)modInfo.lpBaseOfDll;
                if (addr >= base && addr < base + modInfo.SizeOfImage)
                {
                    offset = addr - base;
                    found = 1;

                    char szModName[MAX_PATH];
                    if (GetModuleFileNameExA(hProcess, hMods[m], szModName, sizeof(szModName)))
                    {
                        const char* name = strrchr(szModName, '\\');
                        if (!name) name = strrchr(szModName, '/');
                        if (!name) name = szModName;
                        else name++;
                        module_name = name;
                    }
                }
            }
        }

        if (found)
        {
            writeToFileFormatted("  #%03d: 0x%016llX (%s+0x%llX)\n",
                                 i, (unsigned long long)addr, module_name, (unsigned long long)offset);
        }
        else
        {
            writeToFileFormatted("  #%03d: 0x%016llX\n", i, (unsigned long long)addr);
        }
    }
}
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#ifdef __linux__
static void writeStackTraceWithModules()
{
    void* buffer[32];
    int nptrs = backtrace(buffer, 32);

    for (int i = 0; i < nptrs; ++i)
    {
        Dl_info info;
        const char* module_name = "unknown";
        uintptr_t offset = 0;

        if (dladdr(buffer[i], &info) && info.dli_fname)
        {
            const char* name = strrchr(info.dli_fname, '/');
            module_name = name ? name + 1 : info.dli_fname;
            offset = (uintptr_t)buffer[i] - (uintptr_t)info.dli_fbase;
        }

        writeToFileFormatted("  #%03d: 0x%016llX (%s+0x%llX)\n",
                             i, (unsigned long long)(uintptr_t)buffer[i],
                             module_name, (unsigned long long)offset);
    }
}
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void writeCrashReport(const char* reason)
{
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    writeToFile("========================================\n");
    writeToFile("=== FATAL ERROR ===\n");
    writeToFile("Time: ");
    writeToFile(timestamp);
    writeToFile("\n");
    writeToFile("Reason: ");
    writeToFile(reason);
    writeToFile("\n\n");

    // Информация о системе
    writeToFile("=== System Information ===\n");

#ifdef _WIN32
    writeToFile("  OS: Windows\n");

    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((LPOSVERSIONINFOW)&osvi);
    writeToFileFormatted("  Version: %d.%d Build %d\n",
                         osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);

    SYSTEM_INFO sysinfo = {};
    GetSystemInfo(&sysinfo);
    writeToFileFormatted("  Processors: %d\n", sysinfo.dwNumberOfProcessors);

#elif __linux__
    writeToFile("  OS: Linux\n");

    // Пробуем получить версию ядра
    FILE* proc = fopen("/proc/version", "r");
    if (proc)
    {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), proc))
        {
            // Убираем лишние пробелы в конце
            char* end = buffer + strlen(buffer) - 1;
            while (end > buffer && (*end == '\n' || *end == ' ')) *end-- = '\0';
            writeToFile("  Kernel: ");
            writeToFile(buffer);
            writeToFile("\n");
        }
        fclose(proc);
    }

#elif __APPLE__
    writeToFile("  OS: macOS\n");
#endif

    writeToFileFormatted("  Architecture: %d-bit\n", (int)(sizeof(void*) == 8 ? 64 : 32));
    writeToFile("\n");

// Загруженные модули (только Windows)
#ifdef _WIN32
    writeLoadedModules();
#endif

    // Стек вызовов
    writeToFile("=== Stack Trace ===\n");

#ifdef _WIN32
    void* stack[32];
    unsigned short frames = 0;

    typedef void (__cdecl *RtlCaptureStackBackTrace_t)(unsigned long, unsigned long, void**, unsigned long*);

    HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
    if (!hNtDll)
    {
        hNtDll = LoadLibraryA("ntdll.dll");
    }

    if (hNtDll)
    {
        RtlCaptureStackBackTrace_t pFunc =
            (RtlCaptureStackBackTrace_t)GetProcAddress(hNtDll, "RtlCaptureStackBackTrace");

        if (pFunc)
        {
            unsigned long hash = 0;
            pFunc(1, 32, stack, &hash);
            frames = (unsigned short)hash;
        }
        FreeLibrary(hNtDll);
    }

    if (frames > 0)
    {
        writeStackTraceWithModules(stack, frames);
    }
    else
    {
        writeToFile("  Stack trace not available\n");
    }

#elif defined(__linux__) || defined(__APPLE__)
    writeStackTraceWithModules();
#endif

    writeToFile("\n");
    writeToFile("========================================\n\n");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void signalHandler(int sig)
{
    const char* sig_name = "UNKNOWN";
    switch(sig)
    {
    case SIGSEGV: sig_name = "SIGSEGV"; break;
    case SIGABRT: sig_name = "SIGABRT"; break;
    case SIGFPE:  sig_name = "SIGFPE"; break;
    case SIGILL:  sig_name = "SIGILL"; break;
    case SIGTERM: sig_name = "SIGTERM"; break;
    }

    char reason[128];
    snprintf(reason, sizeof(reason), "Signal %d (%s)", sig, sig_name);
    writeCrashReport(reason);

    signal(sig, SIG_DFL);
    raise(sig);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#ifdef _WIN32
static LONG WINAPI windowsExceptionHandler(_EXCEPTION_POINTERS* ex)
{
    char reason[512];

    // Расшифровка кода исключения
    const char* exception_name = "UNKNOWN";
    switch(ex->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        exception_name = "ACCESS_VIOLATION";
        if (ex->ExceptionRecord->NumberParameters >= 2)
        {
            const char* op = "write";
            if (ex->ExceptionRecord->ExceptionInformation[0] == 0)
                op = "read";
            else if (ex->ExceptionRecord->ExceptionInformation[0] == 8)
                op = "DEP violation";

            snprintf(reason, sizeof(reason),
                     "EXCEPTION: %s - %s at 0x%016llX, address 0x%016llX",
                     exception_name, op,
                     (unsigned long long)ex->ExceptionRecord->ExceptionAddress,
                     (unsigned long long)ex->ExceptionRecord->ExceptionInformation[1]);
        }
        break;

    case EXCEPTION_STACK_OVERFLOW:
        exception_name = "STACK_OVERFLOW";
        snprintf(reason, sizeof(reason),
                 "EXCEPTION: %s at 0x%016llX",
                 exception_name, (unsigned long long)ex->ExceptionRecord->ExceptionAddress);
        break;

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        exception_name = "DIVIDE_BY_ZERO";
        snprintf(reason, sizeof(reason),
                 "EXCEPTION: %s at 0x%016llX",
                 exception_name, (unsigned long long)ex->ExceptionRecord->ExceptionAddress);
        break;

    default:
        snprintf(reason, sizeof(reason),
                 "EXCEPTION: 0x%08X at 0x%016llX",
                 ex->ExceptionRecord->ExceptionCode,
                 (unsigned long long)ex->ExceptionRecord->ExceptionAddress);
        break;
    }

    // Добавляем информацию о регистрах
    writeToFile("=== Registers ===\n");

#ifdef _M_AMD64
    writeToFileFormatted(
        "RAX: 0x%016llX  RBX: 0x%016llX\n"
        "RCX: 0x%016llX  RDX: 0x%016llX\n"
        "RSI: 0x%016llX  RDI: 0x%016llX\n"
        "RBP: 0x%016llX  RSP: 0x%016llX\n"
        "RIP: 0x%016llX  EFLAGS: 0x%08X\n\n",
        (unsigned long long)ex->ContextRecord->Rax,
        (unsigned long long)ex->ContextRecord->Rbx,
        (unsigned long long)ex->ContextRecord->Rcx,
        (unsigned long long)ex->ContextRecord->Rdx,
        (unsigned long long)ex->ContextRecord->Rsi,
        (unsigned long long)ex->ContextRecord->Rdi,
        (unsigned long long)ex->ContextRecord->Rbp,
        (unsigned long long)ex->ContextRecord->Rsp,
        (unsigned long long)ex->ContextRecord->Rip,
        (unsigned int)ex->ContextRecord->EFlags);
#else
    writeToFileFormatted(
        "EAX: 0x%08X  EBX: 0x%08X\n"
        "ECX: 0x%08X  EDX: 0x%08X\n"
        "ESI: 0x%08X  EDI: 0x%08X\n"
        "EBP: 0x%08X  ESP: 0x%08X\n"
        "EIP: 0x%08X  EFLAGS: 0x%08X\n\n",
        (unsigned int)ex->ContextRecord->Eax,
        (unsigned int)ex->ContextRecord->Ebx,
        (unsigned int)ex->ContextRecord->Ecx,
        (unsigned int)ex->ContextRecord->Edx,
        (unsigned int)ex->ContextRecord->Esi,
        (unsigned int)ex->ContextRecord->Edi,
        (unsigned int)ex->ContextRecord->Ebp,
        (unsigned int)ex->ContextRecord->Esp,
        (unsigned int)ex->ContextRecord->Eip,
        (unsigned int)ex->ContextRecord->EFlags);
#endif

    writeCrashReport(reason);

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void setup_crash_handler(const char* log_filename)
{
    // Сохраняем имя файла лога
    if (log_filename != nullptr && strlen(log_filename) > 0)
    {
        strncpy(log_file_path, log_filename, sizeof(log_file_path) - 1);
        log_file_path[sizeof(log_file_path) - 1] = '\0';
    }

    // Удаляем старый лог и создаем новый
    initLogFile();

    // Теперь writeToFile будет дописывать в созданный файл
    writeToFile("Crash handler ready, waiting for signals...\n\n");

    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGTERM, signalHandler);

#ifdef _WIN32
    SetUnhandledExceptionFilter(windowsExceptionHandler);
#endif
}
