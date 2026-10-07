#include "Zuma_Prefix.pch"

#include "CircleShootApp.h"
#include <SDL2/SDL.h>
#include <filesystem>

static bool IsGameDirectory(std::filesystem::path p)
{
    std::filesystem::path rpath = p / "properties" / "resources.xml";
    std::filesystem::path ppath = p / "main.pak";
    return Sexy::FileExists(rpath.string()) || Sexy::FileExists(ppath.string());
}

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
void PlatformInit()
{
    SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, "1");
    NSString *pathWithTilde = @"~/Library/Application Support/ZumaPortable";
    NSString *fullPath = [pathWithTilde stringByExpandingTildeInPath];
    const char *path = [fullPath UTF8String];
    Sexy::MkDir(path);
    std::string filename = Sexy::StrFormat("%s/gamefolder.txt", path);

    // check for known game data locations
    if (!Sexy::FileExists(filename))
    {
        // check for steam install
        NSString *steamTilde = @"~/Library/Application Support/Steam/steamapps/common/Zuma Deluxe/Zuma Deluxe.app/Contents/Resources";
        std::string steam = [[steamTilde stringByExpandingTildeInPath] UTF8String];
        if (IsGameDirectory(steam))
        {
            std::ofstream outFile(filename, std::ios::binary);
            if (outFile)
            {
                outFile.write(steam.c_str(), steam.size());
                outFile.close();
            }
        }
        
        // check directory containing app
        NSString *appPath = [[NSBundle mainBundle] bundlePath];
        NSString *appDir = [appPath stringByDeletingLastPathComponent];
        std::string app = [appDir UTF8String];
        if (IsGameDirectory(app))
        {
            std::ofstream outFile(filename, std::ios::binary);
            if (outFile)
            {
                outFile.write(app.c_str(), app.size());
                outFile.close();
            }
        }
    }
    
    for (;;)
    {
        // read zuma file location
        std::string rpath;
        std::ifstream file(filename, std::ios::binary); // open in binary mode
        if (file)
        {
            std::ostringstream contents;
            contents << file.rdbuf(); // read entire file into stream
            rpath = contents.str();
            
            // remove line endings
            while (rpath.size() && (rpath.back() == '\n' || rpath.back() == '\r'))
                rpath.pop_back();
        }

        if (IsGameDirectory(rpath))
        {
            Sexy::SetResourceFolder(rpath);
            Sexy::ChDir(rpath);
            break;
        }
        else
        {
            // THIS WILL NOT WORK AS CONSOLE APP, NEEDS TO BE APP BUNDLE
            NSOpenPanel* openDlg = [NSOpenPanel openPanel];
            [openDlg setMessage:@"Select Zuma Deluxe Folder"];
            [openDlg setCanChooseDirectories:YES];
            [openDlg setCanChooseFiles:NO];
            [openDlg setAllowsMultipleSelection:NO];
            [openDlg setShowsHiddenFiles:YES];
            [openDlg setDirectoryURL:[NSURL URLWithString:[NSString stringWithUTF8String:"."] ] ];
            [openDlg setTreatsFilePackagesAsDirectories:YES];
            auto result = [openDlg runModal];
            if (result == NSModalResponseOK)
            {
                std::string path = [[[[openDlg URLs] objectAtIndex:0] path] UTF8String];
                
                // fixup path for macOS demo version
                std::string mac_demo_path = Sexy::StrFormat("%s/Contents/Resources/Zuma Deluxe.app/Contents/Resources", path.c_str());
                if (Sexy::FileExists(mac_demo_path))
                {
                    path = mac_demo_path;
                }
                
                // fixup path for macOS full version
                std::string mac_full_path = Sexy::StrFormat("%s/Contents/Resources", path.c_str());
                if (Sexy::FileExists(mac_full_path))
                {
                    path = mac_full_path;
                }
                
                std::ofstream outFile(filename, std::ios::binary);
                if (outFile)
                {
                    outFile.write(path.c_str(), path.size());
                    outFile.close();
                }
            }
            else
            {
                exit(0);
            }
        }
    }
}
#elif defined(_WIN32)
#include <windows.h>
#include <shlobj.h>         // SHGetFolderPathA, SHBrowseForFolder
#include <shellapi.h>       // SHFileOperationW
#include <shobjidl.h>
#include <shlwapi.h>        // PathIsDirectoryEmptyW
#if !defined(__MINGW32__)
#include <shlobj_core.h>    // SHGetKnownFolderPath
#endif
#include <KnownFolders.h>   // KNOWNFOLDERID
#include <string>
#include <fstream>
#include <sstream>

static bool PickFolder(std::string& outPath)
{
    bool result = false;
    IFileDialog* pDialog = nullptr;

    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
        return false;

    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDialog)
    );

    if (FAILED(hr))
        return false;

    DWORD options = 0;
    pDialog->GetOptions(&options);

    // Folder picker mode
    pDialog->SetOptions(options |
        FOS_PICKFOLDERS |
        FOS_FORCEFILESYSTEM |
        FOS_PATHMUSTEXIST |
        FOS_NOCHANGEDIR);

    pDialog->SetTitle(L"Select Zuma Deluxe Folder");

    hr = pDialog->Show(nullptr);

    if (SUCCEEDED(hr))
    {
        IShellItem* item = nullptr;

        if (SUCCEEDED(pDialog->GetResult(&item)))
        {
            PWSTR path = nullptr;

            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
            {
                outPath = Sexy::PathToU8(path);
                CoTaskMemFree(path);
                result = true;
            }

            item->Release();
        }
    }

    pDialog->Release();
    return result;
}

static std::wstring GetFolder(KNOWNFOLDERID folder_id)
{
    std::wstring result;
    PWSTR path = NULL;
    HRESULT hr = SHGetKnownFolderPath(folder_id, 0, NULL, &path);
    if (SUCCEEDED(hr))
    {
        result = path;
        CoTaskMemFree(path);
    }
    return result;
}

std::wstring GetUTF16(const std::string& utf8)
{
    std::wstring result;
    if (utf8.size() != 0)
    {
        int len = MultiByteToWideChar(CP_UTF8,
                                      MB_ERR_INVALID_CHARS,
                                      utf8.data(),
                                      (int)utf8.size(),
                                      nullptr,
                                      0);

        if (len > 0)
        {
            result.resize(len);
            MultiByteToWideChar(CP_UTF8,
                                MB_ERR_INVALID_CHARS,
                                utf8.data(),
                                (int)utf8.size(),
                                result.data(),
                                len);
        }
    }
    return result;
}

void PlatformInit()
{
    // check if running on wine
    {
        HMODULE hntdll = GetModuleHandle("ntdll.dll");
        if(hntdll)
        {
            FARPROC wine_get_version = GetProcAddress(hntdll, "wine_get_version");
            if (wine_get_version)
            {
                // wasapi causes static/crackle sounds, force directsound
                SDL_SetHint(SDL_HINT_AUDIODRIVER, "directsound");
            }
        }
    }

    // 1. Create settings folder in AppData\Roaming\ZumaPortable
    wchar_t appDataPath[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appDataPath)))
    {
        std::string path = Sexy::StrFormat("%ls\\ZumaPortable", appDataPath);
        Sexy::MkDir(path.c_str());

        std::string gameFolderFile = path + "\\gamefolder.txt";
        std::string rpath;

        // check for known game data locations
        if (!Sexy::FileExists(gameFolderFile))
        {
            const std::string FOLDERS[] = {
                ".",
                Sexy::StrFormat("%ls\\Steam\\SteamApps\\Common\\Zuma Deluxe", GetFolder(FOLDERID_ProgramFilesX86).c_str()),
                Sexy::StrFormat("%ls\\Steam\\SteamApps\\Common\\Zuma Deluxe", GetFolder(FOLDERID_ProgramFiles).c_str()),
                Sexy::StrFormat("%ls\\PopCap Games\\Zuma Deluxe", GetFolder(FOLDERID_ProgramFilesX86).c_str()),
                "C:\\Zylom Games\\Zuma",
            };
            for (std::string folder : FOLDERS)
            {
                if (IsGameDirectory(folder))
                {
                    std::ofstream outFile(gameFolderFile, std::ios::binary);
                    if (outFile)
                    {
                        outFile.write(folder.c_str(), folder.size());
                        outFile.close();
                    }
                    break;
                }
            }
        }

        // check for known save locations
        {
            wchar_t appdata[MAX_PATH] = {};
            SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, appdata);

            std::string datapath;
            std::ifstream file(gameFolderFile, std::ios::binary);
            if (file)
            {
                std::ostringstream contents;
                contents << file.rdbuf();
                datapath = contents.str();
                while (!datapath.empty() && (datapath.back() == '\r' || datapath.back() == '\n'))
                    datapath.pop_back();
            }

            const std::string FOLDERS[] = {
                Sexy::StrFormat("%ls\\PopCap Games\\Zuma Deluxe", appdata),
                Sexy::StrFormat("%ls\\Steam\\Zuma", GetFolder(FOLDERID_ProgramData).c_str()),
                "C:\\Zylom Games\\Zuma",
                datapath,
            };
            
            std::string dest = path + "\\userdata";
            for (std::string folder : FOLDERS)
            {
                std::string src = Sexy::StrFormat("%s\\userdata", folder.c_str());
                if (Sexy::FileExists(src) && !Sexy::FileExists(dest))
                {
                    std::wstring wsrc = GetUTF16(src);
                    std::wstring wdest = GetUTF16(dest + "\\..");
                    if (!PathIsDirectoryEmptyW(wsrc.c_str()))
                    {
                        // double null terminated: PCZZWSTR
                        wsrc.push_back(L'\0');
                        wdest.push_back(L'\0');

                        SHFILEOPSTRUCTW s = {};
                        s.wFunc = FO_COPY;
                        s.fFlags = FOF_NO_UI;
                        s.pTo = wdest.c_str();
                        s.pFrom = wsrc.c_str();
                        SHFileOperationW(&s);
                        break;
                    }
                }
            }
        }

        for (;;)
        {
            // 2. Try reading saved path
            std::ifstream file(gameFolderFile, std::ios::binary);
            if (file)
            {
                std::ostringstream contents;
                contents << file.rdbuf();
                rpath = contents.str();
                while (!rpath.empty() && (rpath.back() == '\r' || rpath.back() == '\n'))
                    rpath.pop_back();
            }

            // 3. Check if the game folder is valid
            if (IsGameDirectory(rpath))
            {
                Sexy::SetResourceFolder(rpath);
                Sexy::ChDir(rpath);
                break;
            }
            else
            {
                // 4. Show legacy folder selection dialog
                if (!PickFolder(rpath))
                {
                    exit(0); // user canceled
                }

                // Save path to file
                std::ofstream outFile(gameFolderFile, std::ios::binary);
                if (outFile)
                {
                    outFile.write(rpath.c_str(), rpath.size());
                    outFile.close();
                }
            }
        }
    }
    else
    {
        MessageBoxA(nullptr, "Failed to get AppData folder.", "Error", MB_OK | MB_ICONERROR);
        exit(0);
    }
}

#elif defined(__ANDROID__)
#include <jni.h>
#include <unistd.h>
#include <string>
std::string android_data_path;
extern "C"
JNIEXPORT void JNICALL
Java_io_itch_ksylvestre_zumaportable_ZumaPortableActivity_nativeSetWorkingDir(
        JNIEnv* env,
        jobject /* this */,
        jstring path_) {

    const char *path = env->GetStringUTFChars(path_, nullptr);
    if (path)
    {
        android_data_path = path;
        env->ReleaseStringUTFChars(path_, path);
    }
}
void PlatformInit() 
{
    SDL_Log("ANDROID_DATA_PATH:%s", android_data_path.c_str());
    Sexy::SetResourceFolder(android_data_path);
    Sexy::ChDir(android_data_path);
}

#if __ANDROID_API__ < 23
extern "C" ssize_t __write_chk(int fd, const void* buf, size_t count, size_t buf_size)
{
    //__check_count("write", "count", count);
    //__check_buffer_access("write", "read from", count, buf_size);
    return write(fd, buf, count);
}
#undef stderr
FILE *stderr = &__sF[2];
#endif

#elif defined(__SWITCH__)
#include <switch.h>
void PlatformInit() 
{
    const char *path = "sdmc:/switch/zumaportable";
    Sexy::SetResourceFolder(path);
    Sexy::SetAppDataFolder(path);
    Sexy::ChDir(path);
    std::string logpath = Sexy::StrFormat("%s/log.txt", path);
    FILE *logfile = fopen(logpath.c_str(), "wb");
    const auto LogFunction = [](void *userdata, int category, SDL_LogPriority priority, const char *message)
    {
        if (userdata && message)
        {
            fprintf((FILE*)userdata, "%s\n", message);
            fflush((FILE*)userdata);
        }
    };
    SDL_LogSetOutputFunction(LogFunction, logfile);

    // don't change button positions to match switch layout
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
}
extern "C" {
	u32 __nx_stack_size = 4 * 1024 * 1024; // 4MB
}

#else
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

static void ResetFramebuffer()
{
    int fb = open("/dev/fb0", O_RDWR);
    if (fb >= 0)
    {
        struct fb_var_screeninfo vinfo;
        if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == 0)
        {
            if (vinfo.yoffset != 0 || vinfo.xoffset != 0)
            {
                vinfo.yoffset = 0;
                vinfo.xoffset = 0;
                ioctl(fb, FBIOPAN_DISPLAY, &vinfo);
            }
        }
        close(fb);
    }
}

static void CrashSignalHandler(int sig, siginfo_t *info, void *context)
{
    ResetFramebuffer();

    const char msg[] = "\n========================================\nFATAL CRASH DETECTED BY SIGNAL HANDLER\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);

    char buf[128];
    int len = snprintf(buf, sizeof(buf), "Signal %d at fault address %p\n", sig, info ? info->si_addr : NULL);
    if (len > 0) write(STDERR_FILENO, buf, len);

    void *array[32];
    int size = backtrace(array, 32);
    len = snprintf(buf, sizeof(buf), "Stack frames (%d):\n", size);
    if (len > 0) write(STDERR_FILENO, buf, len);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    const char endmsg[] = "========================================\n";
    write(STDERR_FILENO, endmsg, sizeof(endmsg) - 1);
    _exit(1);
}

static void TermSignalHandler(int sig)
{
    (void)sig;
    if (Sexy::gSexyAppBase)
    {
        Sexy::gSexyAppBase->mShutdown = true;
    }
}

static void InstallCrashHandler()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = CrashSignalHandler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = TermSignalHandler;
    sigaction(SIGTERM, &sa_term, NULL);
    sigaction(SIGINT, &sa_term, NULL);
}

void PlatformInit() 
{
    InstallCrashHandler();

    char exePath[1024] = {0};
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len > 0)
    {
        exePath[len] = '\0';
        std::string dir = Sexy::GetFileDir(exePath);
        if (!dir.empty())
        {
            chdir(dir.c_str());
        }
    }
}
#endif

int main(int argc, char *argv[])
{
    PlatformInit();
    {
        Sexy::CircleShootApp app;

        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_CRITICAL);
        
        app.Init();
#ifndef _WIN32
        InstallCrashHandler();
#endif
        SDL_Log("Start");
        app.Start();
        SDL_Log("Shutdown");
        app.Shutdown();
    }

    SDL_Log("Quitting SDL");
    SDL_Quit();
#ifndef _WIN32
    ResetFramebuffer();
#endif
    SDL_Log("Return from main");
    return 0;
}
