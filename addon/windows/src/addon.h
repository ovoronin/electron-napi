#include <napi.h>

using namespace std;
namespace addon
{
  void initHook(const Napi::CallbackInfo &info);
  void destroyHook(const Napi::CallbackInfo &info);
  // Export API
  Napi::Object Init(Napi::Env env, Napi::Object exports);
  NODE_API_MODULE(addon, Init)
}