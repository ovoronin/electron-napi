#include <napi.h>

Napi::ThreadSafeFunction locationCallback;

boolean InitializeHook(std::string processName);
void ShutdownHook();
