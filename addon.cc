#include <napi.h>
#include <Windows.h>
#include <winrt/Windows.ApplicationModel.h>

using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;

std::string make_string(const std::wstring& wstring)
{
  auto wideData = wstring.c_str();
  int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideData, -1, nullptr, 0, NULL, NULL);
  auto utf8 = std::make_unique<char[]>(bufferSize);
  if (0 == WideCharToMultiByte(CP_UTF8, 0, wideData, -1, utf8.get(), bufferSize, NULL, NULL))
    throw std::exception("Can't convert string to UTF8");

  return std::string(utf8.get());
}

Napi::String GetURI(const Napi::CallbackInfo& info) {
  try {
    Package^ package = Package::Current;
    if (package) {
      AppInstallerInfo^ app = package->GetAppInstallerInfo();
      Platform::String^ platformString = app->Uri->ToString();
      std::wstring wstr(platformString->Data());
      Napi::String uriJs = Napi::String::New(info.Env(), make_string(wstr));

      return uriJs;
    } else {
      return Napi::String::New(info.Env(), "");
    }
  } catch (...) {
    return Napi::String::New(info.Env(), "");
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Napi::Function::New(env, GetURI, "getAppInstallerUri");
}

NODE_API_MODULE(addon, Init)
