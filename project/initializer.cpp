#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <ctime>
#include <string>
#include <thread>
#include <map>
#include <fstream>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_TRAYMENU_RELOAD 2
#define ID_TRAYMENU_KEYBIND 3
#define ID_TRAYMENU_EXIT 4
#define ID_TRAYMENU_STATUS 5

HANDLE HMutex;

HWND ConsoleHWND = nullptr;
HWND RobloxHWND = nullptr;

bool ConsoleVisible = false;
bool CursorLockEnabled = false;

int ActivationKeybind = VK_INSERT;

HMENU HPopupMenu;
NOTIFYICONDATA NId;

std::wstring ConfigurationFileName = L"Keybind.txt";

std::wstring GetCurrentTimeString() {
    time_t Now = time(nullptr);

    struct tm LocalTime;
    localtime_s(&LocalTime, &Now);

    wchar_t TimeBuffer[9];
    wcsftime(TimeBuffer, sizeof(TimeBuffer) / sizeof(wchar_t), L"%H:%M:%S", &LocalTime);

    return std::wstring(TimeBuffer);
}

std::map<int, std::wstring> KeybindNames = {
    { VK_BACK, L"Backspace" },
    { VK_TAB, L"Tab" },
    { VK_RETURN, L"Enter" },
    { VK_SHIFT, L"Shift" },
    { VK_CONTROL, L"Control" },
    { VK_MENU, L"Alt" },
    { VK_CAPITAL, L"Caps Lock" },
    { VK_ESCAPE, L"Escape" },
    { VK_SPACE, L"Spacebar" },
    { VK_PRIOR, L"Page Up" },
    { VK_NEXT, L"Page Down" },
    { VK_END, L"End" },
    { VK_HOME, L"Home" },
    { VK_INSERT, L"Insert" },
    { VK_F1, L"F1" },
    { VK_F2, L"F2" },
    { VK_F3, L"F3" },
    { VK_F4, L"F4" },
    { VK_F5, L"F5" },
    { VK_F6, L"F6" },
    { VK_F7, L"F7" },
    { VK_F8, L"F8" },
    { VK_F9, L"F9" },
    { VK_F10, L"F10" },
    { VK_F11, L"F11" },
    { VK_F12, L"F12" }
};

HWND GetRobloxWindow() {
    return FindWindowA(nullptr, "Roblox");
}

void PositionCursor(HWND Handle) {
    if (GetForegroundWindow() == Handle) {
        POINT CursorPosition;
        if (GetCursorPos(&CursorPosition) && ScreenToClient(Handle, &CursorPosition)) {
            RECT WindowRect;
            if (GetWindowRect(Handle, &WindowRect)) {
                int SizeX = WindowRect.right - WindowRect.left;
                int SizeY = WindowRect.bottom - WindowRect.top;

                int CenterX = SizeX / 2;
                int CenterY = SizeY / 2;

                POINT RealCursorPosition;
                if (GetCursorPos(&RealCursorPosition)) {
                    int MarginX = 30;
                    int MarginY = 70;

                    if (CursorPosition.x < MarginX) {
                        SetCursorPos(WindowRect.left + 100, RealCursorPosition.y);
                    }
                    else if (CursorPosition.x > SizeX - MarginX) {
                        SetCursorPos(WindowRect.right - 100, RealCursorPosition.y);
                    };

                    if (CursorPosition.y < MarginY) {
                        SetCursorPos(RealCursorPosition.x, WindowRect.top + 100);
                    }
                    else if (CursorPosition.y > SizeY - MarginY) {
                        SetCursorPos(RealCursorPosition.x, WindowRect.bottom - 100);
                    };
                };
            };
        };
    };
};

void GetCurrentStatistics() {
    system("cls");
    std::wcout << L"" << GetCurrentTimeString() << L" [debug] entry: starting RBXUWPCursorLock" << std::endl;
    const wchar_t* Process = L"Windows10Universal.exe";

    HANDLE HSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (HSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(HSnapshot, &pe32)) {
            do {
                if (wcscmp(pe32.szExeFile, Process) == 0) {

                    std::cout << std::endl;
                    std::wcout << L"target process name: " << pe32.szExeFile << std::endl;
                    std::wcout << L"target process id: " << pe32.th32ProcessID << std::endl;
                    std::cout << std::endl;

                }
            } while (Process32NextW(HSnapshot, &pe32));
        }

        CloseHandle(HSnapshot);
    }
    else {
        std::wcerr << L"[error] main: initialization unsuccessful" << std::endl;
    }
    std::cout << "Found RobloxUWP Window" << std::endl;
    std::cout << "  -> handle = " << RobloxHWND << std::endl;
    std::cout << std::endl;
    std::wcout << GetCurrentTimeString() << L" [debug] initializer: initialization successful, press " << KeybindNames[ActivationKeybind] << std::endl;
    std::wcout << GetCurrentTimeString() << L" [debug] main: cursor lock is " << (CursorLockEnabled ? "enabled" : "not enabled") << std::endl;
}

void ToggleCursorLock() {
    while (true) {
        if (GetAsyncKeyState(ActivationKeybind) & 0x8000) {
            CursorLockEnabled = !CursorLockEnabled;
            GetCurrentStatistics();
            Sleep(300);
        }
        Sleep(50);
    }
}

void CheckAndFixCursor() {
    while (true) {
        if (GetForegroundWindow() == RobloxHWND && CursorLockEnabled) {
            PositionCursor(RobloxHWND);
        }
        Sleep(10);
    }
}

void AutoAttachHandler() {
    while (true) {
        RobloxHWND = GetRobloxWindow();
        Sleep(1000);
    }
}

void HandleKeybind(int Keybind) {
    ActivationKeybind = Keybind;
    std::wofstream ConfigurationFile(ConfigurationFileName);
    if (ConfigurationFile.is_open()) {
        ConfigurationFile << Keybind;
        ConfigurationFile.close();
    }
    GetCurrentStatistics();
}

void LoadConfiguration() {
    std::wifstream ConfigurationFile(ConfigurationFileName);
    if (ConfigurationFile.is_open()) {
        int Keybind;
        ConfigurationFile >> Keybind;
        if (KeybindNames.find(Keybind) != KeybindNames.end()) {
            ActivationKeybind = Keybind;
        }
        ConfigurationFile.close();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HMENU HKeybindSubMenu = nullptr;

    switch (msg) {
    case WM_TRAYICON:
        if (LOWORD(lParam) == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(HPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
            PostMessage(hWnd, WM_NULL, 0, 0);
        }
        else if (LOWORD(lParam) == WM_LBUTTONUP) {
            ConsoleVisible = !ConsoleVisible;
            ShowWindow(ConsoleHWND, ConsoleVisible ? SW_SHOW : SW_HIDE);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAYMENU_RELOAD:
            ShellExecute(nullptr, L"open", L"https://vanitydev.lol", nullptr, nullptr, SW_SHOWNORMAL);
            break;
        case ID_TRAYMENU_KEYBIND:
            break;
        case ID_TRAYMENU_EXIT:
            ReleaseMutex(HMutex);
            CloseHandle(HMutex);
            DestroyWindow(hWnd);
            exit(0);
            break;
        default:
            if (LOWORD(wParam) >= VK_BACK && LOWORD(wParam) <= VK_F12) {
                HandleKeybind(LOWORD(wParam));
            }
            break;
        }
        break;
    case WM_INITMENUPOPUP:
        if (wParam == (WPARAM)HPopupMenu) {
            if (!HKeybindSubMenu) {
                HKeybindSubMenu = CreatePopupMenu();
                for (auto& pair : KeybindNames) {
                    AppendMenu(HKeybindSubMenu, MF_STRING, pair.first, pair.second.c_str());
                }
            }

            for (int i = VK_BACK; i <= VK_F12; ++i) {
                CheckMenuItem(HKeybindSubMenu, i, MF_BYCOMMAND | ((ActivationKeybind == i) ? MF_CHECKED : MF_UNCHECKED));
            }

            if (DeleteMenu(HPopupMenu, ID_TRAYMENU_KEYBIND, MF_BYCOMMAND)) {
                AppendMenu(HPopupMenu, MF_POPUP, (UINT_PTR)HKeybindSubMenu, L"Keybind");
            }

            ModifyMenu(HPopupMenu, ID_TRAYMENU_STATUS, MF_BYCOMMAND | MF_STRING | (CursorLockEnabled ? MF_CHECKED : 0),
                ID_TRAYMENU_STATUS, CursorLockEnabled ? L"Enabled" : L"Disabled");

            EnableMenuItem(HPopupMenu, ID_TRAYMENU_STATUS, MF_BYCOMMAND | MF_DISABLED | (CursorLockEnabled ? MF_GRAYED : 0));
        }
        break;
    case WM_CLOSE:
        ShowWindow(hWnd, SW_MINIMIZE);
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &NId);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void HideConsoleWindow() {
    ConsoleHWND = GetConsoleWindow();
    ShowWindow(ConsoleHWND, SW_HIDE);
    ConsoleVisible = false;
}

int main() {
    HMutex = CreateMutex(nullptr, TRUE, L"UniqueMutexName");

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "this program is already running" << std::endl;
        CloseHandle(HMutex);
        return 0;
    }

    LoadConfiguration();

    RobloxHWND = GetRobloxWindow();

    if (RobloxHWND == nullptr) {
        std::cout << "Failed to find an open Roblox UWP window." << std::endl;
        return 1;
    }

    GetCurrentStatistics();

    WNDCLASSEX Wc = { sizeof(WNDCLASSEX) };
    Wc.lpfnWndProc = WndProc;
    Wc.hInstance = GetModuleHandle(nullptr);
    Wc.lpszClassName = L"RBXUWPCursorLock";
    RegisterClassEx(&Wc);

    HWND hWnd = CreateWindow(Wc.lpszClassName, L"RBXUWPCursorLock", 0, 0, 0, 0, 0, 0, 0, Wc.hInstance, 0);

    NId.cbSize = sizeof(NOTIFYICONDATA);
    NId.hWnd = hWnd;
    NId.uID = ID_TRAYICON;
    NId.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    NId.uCallbackMessage = WM_TRAYICON;
    wcscpy_s(NId.szTip, L"RBXUWPCursorLock");
    Shell_NotifyIcon(NIM_ADD, &NId);

    HPopupMenu = CreatePopupMenu();
    AppendMenu(HPopupMenu, MF_STRING, ID_TRAYMENU_STATUS, CursorLockEnabled ? L"Enabled" : L"Disabled");
    AppendMenu(HPopupMenu, MF_STRING, ID_TRAYMENU_RELOAD, L"Releases");
    AppendMenu(HPopupMenu, MF_STRING | (CursorLockEnabled ? MFS_CHECKED : 0), ID_TRAYMENU_KEYBIND, L"Keybind");
    AppendMenu(HPopupMenu, MF_STRING, ID_TRAYMENU_EXIT, L"Exit");

    std::thread CursorToggleThread(ToggleCursorLock);
    std::thread CursorCheckThread(CheckAndFixCursor);
    std::thread AutoAttachThread(AutoAttachHandler);

    HideConsoleWindow();

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) && msg.message != WM_QUIT) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseMutex(HMutex);
    CloseHandle(HMutex);

    CursorToggleThread.join();
    CursorCheckThread.join();
    AutoAttachThread.join();

    return 0;
}