#include <main.h>
#include <iostream>
#include <string>

char WINNAME[19] = " ";
char TARGETNAME[64] = "Roblox";
DWORD procID;
MARGINS MARGIN = { 0, 0, Globals::rWidth, Globals::rHeight };

BOOL UNLOADING = FALSE;
BOOL NOTFOUND = FALSE;
BOOL INITIALIZED = FALSE;


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	srand((unsigned)time(NULL));
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	SetConsoleTitle("External ESP Example");
	//Get the Roblox Handle.
	Globals::rbx.Proc = Globals::rbx.LoadRobloxProcess();
	//Set Roblox as the memory process to read from.
	Globals::rbx.Mem.SetProcess(Globals::rbx.Proc);
	//Get the base addresses.
	Globals::rbx.LoadAddresses();
	//Set the view matrix.
	Globals::rbx.SetViewMatrix();

	//Create the update threads that update the overlay window.
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Cheat::Update, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Cheat::LateUpdate, 0, 0, 0);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)UpdateWinPosition, 0, 0, 0);

	//Get Roblox's window size.
	while (Globals::rWidth < 640 && Globals::rHeight < 480)
	{
		Globals::tWnd = FindWindow(NULL, TARGETNAME);
		RECT wSize;
		GetWindowRect(Globals::tWnd, &wSize);
		Globals::rWidth = wSize.right - wSize.left;
		Globals::rHeight = wSize.bottom - wSize.top;
	}

	//Create the overlay to draw in.
	Globals::hWnd = InitializeWin((HINSTANCE)hInst);
	MSG uMessage;

	//Check if Roblox is opened.
	if (Globals::hWnd == NULL) { exit(1); }

	//Show the overlay window.
	ShowWindow(Globals::hWnd, SW_SHOW);

	INITIALIZED = TRUE;

	while (!UNLOADING)
	{
		if (!FindWindow(NULL, "ROBLOX"))
			ExitProcess(0);

		if (PeekMessage(&uMessage, Globals::hWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&uMessage);
			TranslateMessage(&uMessage);
		}
	}

	DestroyWindow(Globals::hWnd);
	UnregisterClass(WINNAME, (HINSTANCE)hInst);
	FreeLibraryAndExitThread((HMODULE)hInst, 0);
	ExitProcess(0);
	return 0;
}

HWND WINAPI InitializeWin(HINSTANCE hInst)
{
	WNDCLASSEX wndClass;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.cbClsExtra = NULL;
	wndClass.cbWndExtra = NULL;
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wndClass.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	wndClass.hInstance = hInst; 
	wndClass.lpfnWndProc = WindowProc;
	wndClass.lpszClassName = WINNAME;
	wndClass.lpszMenuName = WINNAME;
	wndClass.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wndClass))
	{
		exit(1);
	}

	Globals::hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, WINNAME, WINNAME, WS_POPUP, 1, 1, Globals::rWidth, Globals::rHeight, 0, 0, 0, 0);
	SetLayeredWindowAttributes(Globals::hWnd, 0, 0.1f, LWA_ALPHA);
	SetLayeredWindowAttributes(Globals::hWnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(Globals::hWnd, &margins);
	D3DInitialize(Globals::hWnd);

	return Globals::hWnd;
}

void UpdateWinPosition()
{
	while (!UNLOADING)
	{
		UpdateSurface(Globals::hWnd);

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	return;
}

void WINAPI UpdateSurface(HWND hWnd)
{
		RECT wSize;
		HWND tWnd;

		tWnd = FindWindow(NULL, TARGETNAME);

		if (!tWnd && hWnd && !UNLOADING && !NOTFOUND)
		{

		}

		if (tWnd)
		{
			GetWindowRect(tWnd, &wSize);
			Globals::rWidth = wSize.right - wSize.left;
			Globals::rHeight = wSize.bottom - wSize.top;

			DWORD dwStyle = GetWindowLong(tWnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				wSize.top += 23; Globals::rHeight -= 23;
				//wSize.left += 10; rWidth -= 10;
			}

			if (hWnd)
			{
				MoveWindow(hWnd, wSize.left, wSize.top, Globals::rWidth, Globals::rHeight, true);
			}
		}

	return;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage) {
		case WM_CREATE:
			DwmExtendFrameIntoClientArea(hWnd, &MARGIN);
			break;

		case WM_PAINT:
			D3DRender();
			break;

		case WM_DESTROY:
			PostQuitMessage(1);
			break;

		default:
			ImGui_ImplWin32_WndProcHandler(hWnd, uMessage, wParam, lParam);
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
			break;
	}

	return 0;
}