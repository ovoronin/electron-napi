#include <windows.h>
#include <dwmapi.h>
#include <stdio.h>
#include <string>
#include <tlhelp32.h>
#include <napi.h>

extern Napi::ThreadSafeFunction locationCallback;

HWINEVENTHOOK g_hook;
HWND targetHwnd;

void callLocationCallback(long left, long top) {
  if (locationCallback)
  {
    auto cb = [=](Napi::Env env, Napi::Function jsCallback)
    {
      jsCallback.Call({
        Napi::Number::New(env, left),
        Napi::Number::New(env, top)
      });
    };
    locationCallback.BlockingCall(cb);
  }    
}

// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                             LONG idObject, LONG idChild,
                             DWORD dwEventThread, DWORD dwmsEventTime)
{
  if (hwnd == targetHwnd && event == EVENT_OBJECT_LOCATIONCHANGE && idObject == OBJID_WINDOW)
  {
    RECT rect;
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));

    callLocationCallback(rect.left, rect.top);
  }
}

HWND FindTopWindow(DWORD pid)
{
    std::pair<HWND, DWORD> params = { 0, pid };

    // Enumerate the windows using a lambda to process each window
    BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL 
    {
        auto pParams = (std::pair<HWND, DWORD>*)(lParam);

        DWORD processId;
        if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
        {
            // Stop enumerating
            SetLastError(-1);
            pParams->first = hwnd;
            return FALSE;
        }

        // Continue enumerating
        return TRUE;
    }, (LPARAM)&params);

    if (!bResult && GetLastError() == -1 && params.first)
    {
        return params.first;
    }

    return 0;
}

DWORD findProcess(std::string name)
{
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;

  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(hProcessSnap, &pe32))
  {
    CloseHandle(hProcessSnap);
    return (0);
  }

  do
  {
    // printf("Process %s\n", pe32.szExeFile);
    std::string exe(pe32.szExeFile);
    if (!exe.compare(name))
    {
      return pe32.th32ProcessID;
    }

  } while (Process32Next(hProcessSnap, &pe32));
  return 0;
}

boolean InitializeHook(std::string processName)
{
  int pId = findProcess(processName);
  if (pId) {
    targetHwnd = FindTopWindow(pId);
    if (targetHwnd) {
      // get initial coordinates
      RECT rect;
      BOOL hasCoords = GetWindowRect(targetHwnd, &rect);
      if (hasCoords) {
        callLocationCallback(rect.left, rect.top);
      }

      g_hook = SetWinEventHook(
          EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE,     // Range of events
          NULL,                                             // Handle to DLL.
          HandleWinEvent,                                   // The callback.
          pId, 0,                                           // Process and thread IDs of interest (0 = all)
          WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS); // Flags.
      return (boolean)g_hook;
    }
  }
  return false;
}

void ShutdownHook()
{
  UnhookWinEvent(g_hook);
}
