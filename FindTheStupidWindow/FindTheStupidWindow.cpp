//
// Copyright 2019 David Roller 
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "stdafx.h"
#include <Windows.h>
#include <fstream>
#include <time.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>

#ifdef WIN32
#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#endif

BOOL QueryWindowFullProcessImageName(
    HWND hwnd,
    DWORD dwFlags,
    PTSTR lpExeName,
    DWORD dwSize)
{
    DWORD pid = 0;
    BOOL fRc = FALSE;
    if (GetWindowThreadProcessId(hwnd, &pid))
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (hProcess)
        {
            fRc = QueryFullProcessImageName(hProcess, dwFlags, lpExeName, &dwSize);
            CloseHandle(hProcess);
        }
    }
    return fRc;
}

VOID CALLBACK WinEventProcCallback(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND &&
        idObject == OBJID_WINDOW &&
        idChild == CHILDID_SELF)
    {
        PCTSTR pszMsg;
        TCHAR szBuf[MAX_PATH];
        if (hwnd)
        {
            DWORD cch = ARRAYSIZE(szBuf);
            if (QueryWindowFullProcessImageName(hwnd, 0, szBuf, ARRAYSIZE(szBuf)))
            {
                pszMsg = szBuf;
            }
            else
            {
                pszMsg = TEXT("<unknown>");
            }
        }
        else
        {
            pszMsg = TEXT("<none>");
        }

        std::wofstream outfile;
        outfile.open("test.log", std::ios_base::app);

        tm localTime;
        std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
        time_t now = std::chrono::system_clock::to_time_t(t);
        localtime_r(&now, &localTime);

        const std::chrono::duration<double> tse = t.time_since_epoch();
        std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(tse).count() % 1000;

        std::wcout << (1900 + localTime.tm_year) << L'-'
            << std::setfill(L'0') << std::setw(2) << (localTime.tm_mon + 1) << L'-'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_mday << L' '
            << std::setfill(L'0') << std::setw(2) << localTime.tm_hour << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_min << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_sec << L'.'
            << std::setfill(L'0') << std::setw(3) << milliseconds
            << L" - " << pszMsg << std::endl << std::flush;

        outfile << (1900 + localTime.tm_year) << L'-'
            << std::setfill(L'0') << std::setw(2) << (localTime.tm_mon + 1) << L'-'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_mday << L' '
            << std::setfill(L'0') << std::setw(2) << localTime.tm_hour << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_min << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_sec << L'.'
            << std::setfill(L'0') << std::setw(3) << milliseconds
            << L" - " << pszMsg << std::endl << std::flush;
    }
}

int main()
{
    CoInitialize(NULL);

    auto hEvent = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND, NULL,
        WinEventProcCallback, 0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWinEvent(hEvent);
    CoUninitialize();
    return 0;
}
