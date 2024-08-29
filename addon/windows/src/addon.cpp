#define _HAS_STD_BYTE 0
#include <windows.h>
#include "addon.h"
#include "hook.h"

using namespace std;

void addon::initHook(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  std::string processName = info[0].As<Napi::String>();

  locationCallback = Napi::ThreadSafeFunction::New(
      env,
      info[1].As<Napi::Function>(),
      "Callback",
      0,
      1);

  InitializeHook(processName);
}

void addon::destroyHook(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  ShutdownHook();

  if (locationCallback)
  {
    locationCallback.Release();
    locationCallback.Unref(env);
  }

}

Napi::Object addon::Init(Napi::Env env, Napi::Object exports)
{
  // export Napi function
  exports.Set("initHook", Napi::Function::New(env, addon::initHook));
  exports.Set("destroyHook", Napi::Function::New(env, addon::destroyHook));
  return exports;
}