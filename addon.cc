#include <Windows.h>
#include <dwmapi.h>
#include <napi.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.Metadata.h>

#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"
#include "shobjidl.h"

using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comctl32.lib")

typedef enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
} WINDOWCOMPOSITIONATTRIB;

typedef enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
} ACCENT_STATE;

typedef struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef BOOL(WINAPI *SetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);

typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

namespace
{
HMODULE hmodule_win32;
}

std::wstring make_wstring(std::string &&string)
{
    auto utf8Data = string.c_str();
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, utf8Data, -1, nullptr, 0);
    auto wide = std::make_unique<wchar_t[]>(bufferSize);
    if (0 == MultiByteToWideChar(CP_UTF8, 0, utf8Data, -1, wide.get(), bufferSize))
        throw std::exception("Can't convert string to Unicode");

    return std::wstring(wide.get());
}

std::string make_string(const std::wstring &wstring)
{
    auto wideData = wstring.c_str();
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideData, -1, nullptr, 0, NULL, NULL);
    auto utf8 = std::make_unique<char[]>(bufferSize);
    if (0 == WideCharToMultiByte(CP_UTF8, 0, wideData, -1, utf8.get(), bufferSize, NULL, NULL))
        throw std::exception("Can't convert string to UTF8");

    return std::string(utf8.get());
}

Napi::String GetURI(const Napi::CallbackInfo &info)
{
    if (winrt::Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent(
            L"Windows.Foundation.UniversalApiContract", 7))
    {
        try
        {
            Package ^ package = Package::Current;
            if (package)
            {
                AppInstallerInfo ^ app = package->GetAppInstallerInfo();
                if (app)
                {
                    Platform::String ^ platformString = app->Uri->ToString();
                    std::wstring wstr(platformString->Data());
                    return Napi::String::New(info.Env(), make_string(wstr));
                }
            }
            return Napi::String::New(info.Env(), "");
        }
        catch (...)
        {
            return Napi::String::New(info.Env(), "");
        }
    }
    return Napi::String::New(info.Env(), "");
}

Napi::String GetPackageFamilyName(const Napi::CallbackInfo &info)
{
    if (winrt::Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent(
            L"Windows.Foundation.UniversalApiContract", 1))
    {
        try
        {
            Package ^ package = Package::Current;
            Platform::String ^ name = package->Id->FamilyName;
            std::wstring wstr(name->Data());
            return Napi::String::New(info.Env(), make_string(wstr));
        }
        catch (...)
        {
            return Napi::String::New(info.Env(), "");
        }
    }
    return Napi::String::New(info.Env(), "");
}

Napi::Boolean CheckUpdateAvailabilityAsync(const Napi::CallbackInfo &info)
{
    if (winrt::Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent(
            L"Windows.Foundation.UniversalApiContract", 7))
    {
        try
        {
            Package ^ package = Package::Current;

            Napi::Function cb = info[0].As<Napi::Function>();
            auto *context = new Napi::Reference<Napi::Value>(Napi::Persistent(info.This()));
            Napi::ThreadSafeFunction threadSafeCall =
                Napi::ThreadSafeFunction::New(info.Env(), cb,
                                              "", // resource name
                                              0,  // Unlimited queue
                                              1   // Only one thread will use this initially
                );

            IAsyncOperation<PackageUpdateAvailabilityResult ^> ^ op = package->CheckUpdateAvailabilityAsync();
            op->Completed = ref new AsyncOperationCompletedHandler<PackageUpdateAvailabilityResult ^>(
                [&threadSafeCall](IAsyncOperation<PackageUpdateAvailabilityResult ^> ^ sender,
                                  AsyncStatus const /* asyncStatus */) {
                    PackageUpdateAvailabilityResult ^ result = sender->GetResults();
                    int *availability = new int(static_cast<int>(result->Availability));
                    threadSafeCall.BlockingCall(availability, [](Napi::Env env, Napi::Function func, int *data) {
                        func.Call({Napi::Number::New(env, *data)});
                        delete data;
                    });
                    threadSafeCall.Release();
                });
            return Napi::Boolean::New(info.Env(), true);
        }
        catch (...)
        {
            return Napi::Boolean::New(info.Env(), false);
        }
    }
    return Napi::Boolean::New(info.Env(), false);
}

Napi::Object GetWindowsVersion(const Napi::CallbackInfo &info)
{
    Napi::Object result = Napi::Object::New(info.Env());

    HMODULE hmodule = ::GetModuleHandleW(L"ntdll.dll");
    if (hmodule)
    {
        RtlGetVersionPtr rtl_get_version_ptr = (RtlGetVersionPtr)::GetProcAddress(hmodule, "RtlGetVersion");
        if (rtl_get_version_ptr != nullptr)
        {
            RTL_OSVERSIONINFOW rovi = {0};
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (STATUS_SUCCESS == rtl_get_version_ptr(&rovi))
            {
                result.Set("major", rovi.dwMajorVersion);
                result.Set("minor", rovi.dwMinorVersion);
                result.Set("build", rovi.dwBuildNumber);
                FreeLibrary(hmodule);
                return result;
            }
        }
        FreeLibrary(hmodule);
    }
    result.Set("major", 0);
    result.Set("minor", 0);
    result.Set("build", 0);

    return result;
}

Napi::Boolean SetMica(const Napi::CallbackInfo &info)
{
    Napi::ArrayBuffer buf = info[0].As<Napi::ArrayBuffer>();
    const HWND *array = reinterpret_cast<HWND *>(buf.Data());
    HWND hwnd(array[0]);

    bool enabled = info[1].As<Napi::Boolean>();
    BOOL enabledBOOL = enabled ? TRUE : FALSE;
    bool succeeded = SUCCEEDED(::DwmSetWindowAttribute(hwnd, 1029 /* mica */, &enabledBOOL, sizeof(enabledBOOL)));

    return Napi::Boolean::New(info.Env(), succeeded);
}

Napi::Boolean SetWindowBlur(const Napi::CallbackInfo &info)
{
    if (!hmodule_win32)
    {
        hmodule_win32 = ::GetModuleHandleW(L"user32.dll");
    }
    if (hmodule_win32)
    {
        SetWindowCompositionAttribute set_window_composition_attribute_ =
            (SetWindowCompositionAttribute)::GetProcAddress(hmodule_win32, "SetWindowCompositionAttribute");
        Napi::ArrayBuffer buf = info[0].As<Napi::ArrayBuffer>();
        int effect = info[1].As<Napi::Number>();

        const HWND *array = reinterpret_cast<HWND *>(buf.Data());
        HWND hwnd(array[0]);

        ACCENT_STATE state = ACCENT_DISABLED;
        switch (effect)
        {
        case 1:
            state = ACCENT_ENABLE_GRADIENT;
            break;
        case 2:
            state = ACCENT_ENABLE_TRANSPARENTGRADIENT;
            break;
        case 3:
            state = ACCENT_ENABLE_BLURBEHIND;
            break;
        case 4:
            state = ACCENT_ENABLE_ACRYLICBLURBEHIND;
            break;
        case 5:
            state = ACCENT_ENABLE_HOSTBACKDROP;
            break;
        }

        ACCENT_POLICY accent = {state, 2, static_cast<DWORD>(0x00FFFFFF), 0};

        WINDOWCOMPOSITIONATTRIBDATA data;
        data.Attrib = WCA_ACCENT_POLICY;
        data.pvData = &accent;
        data.cbData = sizeof(accent);

        set_window_composition_attribute_(hwnd, &data);
        return Napi::Boolean::New(info.Env(), true);
    }
    return Napi::Boolean::New(info.Env(), false);
}

Napi::Boolean CreateShortcut(const Napi::CallbackInfo &info)
{
    std::wstring exePath = make_wstring(info[0].As<Napi::String>());
    std::wstring destPath = make_wstring(info[1].As<Napi::String>());
    std::wstring arguments = make_wstring(info[2].As<Napi::String>());
    std::wstring comment = make_wstring(info[3].As<Napi::String>());
    std::wstring cwd = make_wstring(info[4].As<Napi::String>());
    std::wstring iconPath = make_wstring(info[5].As<Napi::String>());

    HRESULT hr;
    IShellLinkW *psl;

    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
    if (SUCCEEDED(hr))
    {
        IPersistFile *ppf;

        // Set the path to the shortcut target and add the description.
        psl->SetPath(exePath.c_str());
        psl->SetDescription(comment.c_str());
        psl->SetArguments(arguments.c_str());
        psl->SetWorkingDirectory(cwd.c_str());
        psl->SetIconLocation(iconPath.c_str(), 0);

        // Query IShellLink for the IPersistFile interface, used for saving the
        // shortcut in persistent storage.
        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

        if (SUCCEEDED(hr))
        {
            // Save the link by calling IPersistFile::Save.
            hr = ppf->Save(destPath.c_str(), TRUE);
            ppf->Release();
            return Napi::Boolean::New(info.Env(), SUCCEEDED(hr));
        }
        psl->Release();
    }
    return Napi::Boolean::New(info.Env(), SUCCEEDED(hr));
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set("getAppInstallerUri", Napi::Function::New(env, GetURI, "getAppInstallerUri"));
    exports.Set("getWindowsVersion", Napi::Function::New(env, GetWindowsVersion, "getWindowsVersion"));
    exports.Set("setWindowBlur", Napi::Function::New(env, SetWindowBlur, "setWindowBlur"));
    exports.Set("setMica", Napi::Function::New(env, SetMica, "setMica"));
    exports.Set("createShortcut", Napi::Function::New(env, CreateShortcut, "createShortcut"));
    exports.Set("getPackageFamilyName", Napi::Function::New(env, GetPackageFamilyName, "getPackageFamilyName"));
    exports.Set("checkUpdateAvailabilityAsync",
                Napi::Function::New(env, CheckUpdateAvailabilityAsync, "checkUpdateAvailabilityAsync"));
    return exports;
}

NODE_API_MODULE(addon, Init)
