// lab2_sp.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "lab2_sp.h"
#include <stdio.h>
#include <strsafe.h>
#include <tlhelp32.h>
#include <unordered_map>
#include <vector>

#define MAX_LOADSTRING 100
#define BUFFER_LEN 300
#define IDB_ButtonNext 1
#define IDB_ButtonPrev 2
#define ID_WinModules 3
#define ID_WinThreads 4

WCHAR messages[50000][300];
static const WCHAR* StringsProcess[] = {L"Process name:         %ls\n", L"\tProcess ID            = 0x%08X\n", L"\tParent process ID = 0x%08X\n", L"\tPriority                  = %d\n", L"\tThread count         = %d\n", L"\n"};
static const WCHAR* StringsModule[] = { L"Module name:    %ls\n", L"\tPath                 = %ls\n", L"\tProcess ID      = 0x%08X\n", L"\tBase size       = %d\n", L"\n"};
static const WCHAR* StringsThread[] = { L"Thread ID       = 0x%08X\n", L"\tBase priority  = %d\n", L"\tDelta priority  = %d\n", L"\n" };
static DWORD lines = 0;

#define PROC_INFO_LENGTH 6
struct ProcessString {
    WCHAR procInfo[PROC_INFO_LENGTH][BUFFER_LEN];
    std::vector<WCHAR*> modulesInfo;
    std::vector<WCHAR*> threadsInfo;
};

ProcessString* GetProcessInfoString(LONG index);
std::vector<WCHAR*> GetModules(DWORD dwPID);
std::vector<WCHAR*> GetThreads(DWORD dwPID);
BOOL PrepareAllThreads();

struct ThreadInfo {
    DWORD ID;
    DWORD BasePri;
    DWORD DeltaPri;

    ThreadInfo(DWORD id, DWORD basePri, DWORD deltaPri) : ID(id), BasePri(basePri), DeltaPri(deltaPri) {}
};

std::unordered_multimap<DWORD, ThreadInfo> threads;
std::vector<ProcessString> processesString;
UINT cur_proc = 0;
ProcessString procStr;
HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

ProcessString* GetProcessInfoString(LONG index) {
    if (index >= (LONG)processesString.size() && processSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry = { 0 };
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        ProcessString ps = { 0 };
        if (Process32Next(processSnap, &processEntry)) {
            swprintf(ps.procInfo[0], BUFFER_LEN, L"Process Number:     %d", index);
            swprintf(ps.procInfo[1], BUFFER_LEN, StringsProcess[0], processEntry.szExeFile);
            swprintf(ps.procInfo[2], BUFFER_LEN, StringsProcess[1], processEntry.th32ProcessID);
            swprintf(ps.procInfo[3], BUFFER_LEN, StringsProcess[2], processEntry.th32ParentProcessID);
            swprintf(ps.procInfo[4], BUFFER_LEN, StringsProcess[3], processEntry.pcPriClassBase);
            swprintf(ps.procInfo[5], BUFFER_LEN, StringsProcess[4], processEntry.cntThreads);

            ps.threadsInfo = GetThreads(processEntry.th32ProcessID);
            ps.modulesInfo = GetModules(processEntry.th32ProcessID);
            processesString.push_back(ps);
            return &processesString.back();
        }
        else {
            return NULL;
        }
    }
    if (index >= 0 && index < (LONG)processesString.size()) {
        return &processesString[index];
    }
    return NULL;
}

BOOL GetProcessesInfo()
{
    HANDLE processSnap = INVALID_HANDLE_VALUE;
    processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 processEntry = { 0 };
    processEntry.dwSize = sizeof(PROCESSENTRY32);
   
    if (!Process32First(processSnap, &processEntry))
    {
        CloseHandle(processSnap);
        return FALSE;
    }
    do
    {
        swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[0], processEntry.szExeFile);
        swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[1], processEntry.th32ProcessID);
        swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[2], processEntry.th32ParentProcessID);
        swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[3], processEntry.pcPriClassBase);
        swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[4], processEntry.cntThreads);
        swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[5]);


        HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processEntry.th32ProcessID);
        MODULEENTRY32 moduleEntry = { 0 };
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (moduleSnap != INVALID_HANDLE_VALUE && Module32First(moduleSnap, &moduleEntry)) {
            do
            {
                swprintf(&messages[lines++][0], BUFFER_LEN, StringsModule[0], moduleEntry.szModule);
                swprintf(&messages[lines++][0], BUFFER_LEN, StringsModule[1], moduleEntry.szExePath);
                swprintf(&messages[lines++][0], BUFFER_LEN, StringsModule[2], moduleEntry.th32ProcessID);
                swprintf(&messages[lines++][0], BUFFER_LEN, StringsModule[3], moduleEntry.modBaseSize);
            } while (Module32Next(moduleSnap, &moduleEntry));
            swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[5]);
        }
        CloseHandle(moduleSnap);
        
        auto iters = threads.equal_range(processEntry.th32ProcessID);
        for (auto it = iters.first; it != iters.second; ++it) {
            swprintf(&messages[lines++][0], BUFFER_LEN, StringsProcess[0], processEntry.szExeFile);
			swprintf(&messages[lines++][0], BUFFER_LEN, StringsThread[0], it->second.ID);
			swprintf(&messages[lines++][0], BUFFER_LEN, StringsThread[1], it->second.BasePri);
			swprintf(&messages[lines++][0], BUFFER_LEN, StringsThread[2], it->second.DeltaPri);
			swprintf(&messages[lines++][0], BUFFER_LEN, StringsThread[3]);
        }

    } while (Process32Next(processSnap, &processEntry));

    CloseHandle(processSnap);
    return TRUE;
}

std::vector<WCHAR*> GetThreads(DWORD dwPID)
{
    std::vector<WCHAR*> vec;
    //UINT entries = 0;

    auto iters = threads.equal_range(dwPID);
    for (auto it = iters.first; it != iters.second; ++it) {
        vec.insert(vec.end(), {new WCHAR[BUFFER_LEN], new WCHAR[BUFFER_LEN], new WCHAR[BUFFER_LEN], new WCHAR[2]});
        swprintf(*(vec.end() - 4), BUFFER_LEN, StringsThread[0], it->second.ID);
        swprintf(*(vec.end() - 3), BUFFER_LEN, StringsThread[1], it->second.BasePri);
        swprintf(*(vec.end() - 2), BUFFER_LEN, StringsThread[2], it->second.DeltaPri);
        wcscpy_s(vec.back(), 2, L"\n");
    }

    return vec;
}

std::vector<WCHAR*> GetModules(DWORD dwPID)
{
    std::vector<WCHAR*> vec;
    UINT entries = 0;
    HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (moduleSnap == INVALID_HANDLE_VALUE)
        return vec;

    MODULEENTRY32 moduleEntry = { 0 };
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (!Module32First(moduleSnap, &moduleEntry))
    {
        CloseHandle(moduleSnap);
        return vec;
    }

    do
    {
        vec.insert(vec.end(), { new WCHAR[BUFFER_LEN], new WCHAR[BUFFER_LEN], new WCHAR[BUFFER_LEN], new WCHAR[BUFFER_LEN], new WCHAR[2] });

        swprintf(*(vec.end() - 5), BUFFER_LEN, StringsModule[0], moduleEntry.szModule);
        swprintf(*(vec.end() - 4), BUFFER_LEN, StringsModule[1], moduleEntry.szExePath);
        swprintf(*(vec.end() - 3), BUFFER_LEN, StringsModule[2], moduleEntry.th32ProcessID);
        swprintf(*(vec.end() - 2), BUFFER_LEN, StringsModule[3], moduleEntry.modBaseSize);
        wcscpy_s(vec.back(), 2, L"\n");
    } while (Module32Next(moduleSnap, &moduleEntry));

    CloseHandle(moduleSnap);
    return vec;
}

BOOL PrepareAllThreads()
{
	HANDLE threadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (threadSnap == INVALID_HANDLE_VALUE)
        return FALSE;
	THREADENTRY32 threadEntry = { 0 };
	threadEntry.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(threadSnap, &threadEntry))
    {
        CloseHandle(threadSnap);
        return FALSE;
    }

	do
	{
		threads.emplace(threadEntry.th32OwnerProcessID, ThreadInfo(threadEntry.th32ThreadID, threadEntry.tpBasePri, threadEntry.tpDeltaPri));
	} while (Thread32Next(threadSnap, &threadEntry));

	CloseHandle(threadSnap);
    return TRUE;
}

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
WCHAR szModulesClass[] = L"MODULES_CLASS";      
WCHAR szThreadsClass[] = L"THREADS_CLASS";      

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                RegisterMainWindow(HINSTANCE hInstance);
ATOM                RegisterChildWindow(HINSTANCE hInstance, LPCWSTR className, WNDPROC wndProc);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ModulesProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ThreadsProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.
    PrepareAllThreads();
    procStr = *GetProcessInfoString(cur_proc);
    //GetProcessInfo();

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2SP, szWindowClass, MAX_LOADSTRING);
    RegisterMainWindow(hInstance);
    RegisterChildWindow(hInstance, szModulesClass, ModulesProc);
    RegisterChildWindow(hInstance, szThreadsClass, ThreadsProc);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2SP));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM RegisterMainWindow(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2SP));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB2SP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM RegisterChildWindow(HINSTANCE hInstance, LPCWSTR className, WNDPROC wndProc)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2SP));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB2SP);
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hwnd = CreateWindowW(szWindowClass,
     szTitle,
     WS_OVERLAPPEDWINDOW,
     CW_USEDEFAULT,
     0,
     CW_USEDEFAULT,
     0,
     NULL,
     NULL,
     hInstance,
     NULL);

   HWND hwndButNext = CreateWindow(
       L"BUTTON",  // Predefined class; Unicode assumed 
       L"NEXT",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       0, 0, 0, 0,
       hwnd,     // Parent window
       (HMENU) IDB_ButtonNext,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
       NULL);

   HWND hwndButPrev = CreateWindow(
       L"BUTTON",  // Predefined class; Unicode assumed 
       L"PREV",      // Button text 
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
       0, 0, 0, 0,
       hwnd,     // Parent window
       (HMENU)IDB_ButtonPrev,       // No menu.
       (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
       NULL);

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   return TRUE;
}

BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
    LPRECT rcParent = (LPRECT)lParam;
    int i, idChild;
    idChild = GetWindowLong(hwndChild, GWL_ID);

    static const int navBarSize = 100;

	if (idChild == ID_WinModules) {
		MoveWindow(hwndChild,
			0,
            navBarSize,
			rcParent->right / 2,
			rcParent->bottom - navBarSize,
			TRUE);
	}
	else if (idChild == ID_WinThreads) {
		MoveWindow(hwndChild,
			rcParent->right / 2,
            navBarSize,
			rcParent->right / 2,
			rcParent->bottom - navBarSize,
			TRUE);
	}
	else if (idChild == IDB_ButtonPrev) {
		MoveWindow(hwndChild,
			0,
			0,
            100,
			70,
			TRUE);
	}
	else if (idChild == IDB_ButtonNext) {
		MoveWindow(hwndChild,
			rcParent->right - 100,
			0,
            100,
			70,
			TRUE);
	}
    ShowWindow(hwndChild, SW_SHOW);

    return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;
    
    // These variables are required to display text. 
    static int xClient;     // width of client area 
    static int yClient;     // height of client area 
    
    static int xChar;       // horizontal scrolling unit 
    static int yChar;       // vertical scrolling unit 
   
    int i;                  // loop counter 
    int x, y;               // horizontal and vertical coordinates 
    ProcessString* pStr;
    RECT rcClient;


    switch (message)
    {
    case WM_CREATE: {

        HWND hwndModules = CreateWindowW(szModulesClass,
            L"Modules",
            WS_CHILD | WS_BORDER | WS_HSCROLL | WS_VSCROLL,
            0, 0, 0, 0,
            hwnd,
            (HMENU)ID_WinModules,
            hInst,
            NULL);

        HWND hwndThreads = CreateWindowW(szThreadsClass,
            L"Threads",
            WS_CHILD | WS_BORDER | WS_HSCROLL | WS_VSCROLL,
            0, 0, 0, 0,
            hwnd,
            (HMENU)ID_WinThreads,
            hInst,
            NULL);

        // Get the handle to the client area's device context. 
        hdc = GetDC(hwnd);

        // Extract font dimensions from the text metrics. 
        GetTextMetrics(hdc, &tm);
        xChar = tm.tmAveCharWidth;
        yChar = tm.tmHeight + tm.tmExternalLeading;

        // Free the device context. 
        ReleaseDC(hwnd, hdc);
        return 0;
    }

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDB_ButtonNext:          
                if ((pStr = GetProcessInfoString(cur_proc + 1)) != NULL) {
                    ++cur_proc;
                    procStr = *pStr;
                    InvalidateRect(hwnd, NULL, TRUE);
                    UpdateWindow(hwnd);
                }
                else {
                    MessageBox(NULL, L"This process is the last one.", L"There is no next process", MB_OK);
                }
                break;
            case IDB_ButtonPrev:
                if ((pStr = GetProcessInfoString(cur_proc - 1)) != NULL) {
                    --cur_proc;
                    procStr = *pStr;
                    InvalidateRect(hwnd, NULL, TRUE);
                    UpdateWindow(hwnd);
                }
                else {
                    MessageBox(NULL, L"This process is the first one.", L"There is no prev process", MB_OK);
                }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hwnd);
                break;
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        break;

    case WM_SIZE:

        // Retrieve the dimensions of the client area. 
        yClient = HIWORD(lParam);
        xClient = LOWORD(lParam);

        GetClientRect(hwnd, &rcClient);
        EnumChildWindows(hwnd, EnumChildProc, (LPARAM)&rcClient);
        return 0;

    case WM_PAINT: {
        hdc = BeginPaint(hwnd, &ps);

        int max = 0;
        size_t lengths[PROC_INFO_LENGTH];
        for (i = 0; i < PROC_INFO_LENGTH; ++i) {
            lengths[i] = wcslen(procStr.procInfo[i]);

            if (max < lengths[i]) {
                max = lengths[i];
            }
        }

        for (i = 0; i < PROC_INFO_LENGTH; ++i) {
            TextOutW(hdc, (xClient - max * xChar) / 2, yChar * i, procStr.procInfo[i], lengths[i]);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void SetScrollSizeInner(SCROLLINFO* si, int clientSize, int charSize, int maxSize) {
    si->cbSize = sizeof(SCROLLINFO);
    si->fMask = SIF_RANGE | SIF_PAGE;
    si->nMin = 0;
    si->nMax = maxSize;
    si->nPage = clientSize / charSize;
}

void SetScrollPos(SCROLLINFO* si, WORD param) {
    switch (param)
    {
        // User clicked the HOME keyboard key.
	case SB_TOP:
		si->nPos = si->nMin;
		break;
		// User clicked the END keyboard key.
	case SB_BOTTOM:
		si->nPos = si->nMax;
		break;

		// User clicked the top/left arrow.
	case SB_LINEUP: //SB_LINELEFT
		si->nPos -= 1;
		break;
		// User clicked the bottom/right arrow.
	case SB_LINEDOWN: //SB_LINERIGHT
		si->nPos += 1;
		break;

		// User clicked the scroll bar shaft above the scroll box.
	case SB_PAGEUP://SB_PAGELEFT
		si->nPos -= si->nPage;
		break;
		// User clicked the scroll bar shaft below the scroll box.
	case SB_PAGEDOWN: //SB_PAGERIGHT
		si->nPos += si->nPage;
		break;

        // User dragged the scroll box.
    case SB_THUMBTRACK:
        si->nPos = si->nTrackPos;
        break;

    default:
        break;
    }
}

LRESULT CALLBACK ModulesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;
    SCROLLINFO si;

    // These variables are required to display text. 
    static int xClient;     // width of client area 
    static int yClient;     // height of client area 
    static int xClientMax;  // maximum width of client area 

    static int xChar;       // horizontal scrolling unit 
    static int yChar;       // vertical scrolling unit 
    static int xUpper;      // average width of uppercase letters 

    static int xPos;        // current horizontal scrolling position 
    static int yPos;        // current vertical scrolling position 

    int i;                  // loop counter 
    int x, y;               // horizontal and vertical coordinates

    int FirstLine;          // first line in the invalidated area 
    int LastLine;           // last line in the invalidated area 
    HRESULT hr;
    size_t Length;        // length of an abc[] item 
    ProcessString* pStr;


    switch (message)
    {
    case WM_CREATE:
        // Get the handle to the client area's device context. 
        hdc = GetDC(hwnd);

        // Extract font dimensions from the text metrics. 
        GetTextMetrics(hdc, &tm);
        xChar = tm.tmAveCharWidth;
        xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar / 2;
        yChar = tm.tmHeight + tm.tmExternalLeading;

        // Free the device context. 
        ReleaseDC(hwnd, hdc);
        xClientMax = 48 * xChar + 12 * xUpper;

        return 0;

    case WM_COMMAND:
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    break;

    case WM_SIZE:

        // Retrieve the dimensions of the client area. 
        yClient = HIWORD(lParam);
        xClient = LOWORD(lParam);

        // Set the vertical scrolling range and page size
        SetScrollSizeInner(&si, yClient, yChar, procStr.modulesInfo.size());
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        // Set the horizontal scrolling range and page size. 
        SetScrollSizeInner(&si, xClient, xChar, 2 + xClientMax / xChar);
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

        return 0;
    case WM_HSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;

        // Save the position for comparison later on.
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;
        SetScrollPos(&si, LOWORD(wParam));
        
        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
        GetScrollInfo(hwnd, SB_HORZ, &si);

        // If the position has changed, scroll the window.
        if (si.nPos != xPos)
        {
            ScrollWindow(hwnd, xChar * (xPos - si.nPos), 0, NULL, NULL);
        }
        return 0;

    case WM_VSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);

        // Save the position for comparison later on.
        yPos = si.nPos;
        SetScrollPos(&si, LOWORD(wParam));

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hwnd, SB_VERT, &si);

        // If the position has changed, scroll window and update it.
        if (si.nPos != yPos)
        {
            ScrollWindow(hwnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
            UpdateWindow(hwnd);
        }

        return 0;

    case WM_PAINT: {
        // Prepare the window for painting.
        hdc = BeginPaint(hwnd, &ps);

        SetScrollSizeInner(&si, yClient, yChar, procStr.modulesInfo.size());
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        // Get vertical scroll bar position.
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        GetScrollInfo(hwnd, SB_VERT, &si);
        yPos = si.nPos;

        // Get horizontal scroll bar position.
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;

        // Find painting limits.
        FirstLine = max(0, yPos + ps.rcPaint.top / yChar);
        LastLine = min(lines - 1, yPos + ps.rcPaint.bottom / yChar);

        for (i = FirstLine; i <= LastLine; i++)
        {
            x = xChar * (1 - xPos);
            y = yChar * (i - yPos);
            
            if (i >= 0 && i < procStr.modulesInfo.size()) {
                Length = wcslen(procStr.modulesInfo[i]);
                TextOutW(hdc, x, y, procStr.modulesInfo[i], Length);
            }
        }

        // Indicate that painting is finished.
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK ThreadsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;
    SCROLLINFO si;

    // These variables are required to display text. 
    static int xClient;     // width of client area 
    static int yClient;     // height of client area 
    static int xClientMax;  // maximum width of client area 

    static int xChar;       // horizontal scrolling unit 
    static int yChar;       // vertical scrolling unit 
    static int xUpper;      // average width of uppercase letters 

    static int xPos;        // current horizontal scrolling position 
    static int yPos;        // current vertical scrolling position 

    int i;                  // loop counter 
    int x, y;               // horizontal and vertical coordinates

    int FirstLine;          // first line in the invalidated area 
    int LastLine;           // last line in the invalidated area 
    HRESULT hr;
    size_t Length;        // length of an abc[] item 


    switch (message)
    {
    case WM_CREATE:
        // Get the handle to the client area's device context. 
        hdc = GetDC(hwnd);

        // Extract font dimensions from the text metrics. 
        GetTextMetrics(hdc, &tm);
        xChar = tm.tmAveCharWidth;
        xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar / 2;
        yChar = tm.tmHeight + tm.tmExternalLeading;

        // Free the device context. 
        ReleaseDC(hwnd, hdc);
        xClientMax = 48 * xChar + 12 * xUpper;
        return 0;

    case WM_COMMAND:
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    break;

    case WM_SIZE:
        // Retrieve the dimensions of the client area. 
        yClient = HIWORD(lParam);
        xClient = LOWORD(lParam);

        SetScrollSizeInner(&si, yClient, yChar, procStr.threadsInfo.size());
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        SetScrollSizeInner(&si, xClient, xChar, 2 + xClientMax / xChar);
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

        return 0;
    case WM_HSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;

        // Save the position for comparison later on.
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;
        SetScrollPos(&si, LOWORD(wParam));

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
        GetScrollInfo(hwnd, SB_HORZ, &si);

        // If the position has changed, scroll the window.
        if (si.nPos != xPos)
        {
            ScrollWindow(hwnd, xChar * (xPos - si.nPos), 0, NULL, NULL);
        }

        return 0;

    case WM_VSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);

        // Save the position for comparison later on.
        yPos = si.nPos;
        SetScrollPos(&si, LOWORD(wParam));

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hwnd, SB_VERT, &si);

        // If the position has changed, scroll window and update it.
        if (si.nPos != yPos)
        {
            ScrollWindow(hwnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
            UpdateWindow(hwnd);
        }

        return 0;

    case WM_PAINT: {
        // Prepare the window for painting.
        hdc = BeginPaint(hwnd, &ps);

        SetScrollSizeInner(&si, yClient, yChar, procStr.threadsInfo.size());
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        // Get vertical scroll bar position.
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        GetScrollInfo(hwnd, SB_VERT, &si);
        yPos = si.nPos;

        // Get horizontal scroll bar position.
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;

        // Find painting limits.
        FirstLine = max(0, yPos + ps.rcPaint.top / yChar);
        LastLine = min(lines - 1, yPos + ps.rcPaint.bottom / yChar);

        for (i = FirstLine; i <= LastLine; i++)
        {
            x = xChar * (1 - xPos);
            y = yChar * (i - yPos);

            if (i >= 0 && i < procStr.threadsInfo.size()) {
                Length = wcslen(procStr.threadsInfo[i]);
                TextOutW(hdc, x, y, procStr.threadsInfo[i], Length);
            }
        }

        // Indicate that painting is finished.
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
