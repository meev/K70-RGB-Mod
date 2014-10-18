#include "Main.h"
#include "LightControl.h"
#include <stdio.h>
#include <usb.h>
#include <math.h>

#define MAXCOL 8

HANDLE				g_Device;
bool				g_ConsoleQuit = false;
int					g_iInterval = 100;
HHOOK				hHook = 0;
LightControl		*pLC;

// 144 = Possible keys, 3 = RGB (0-255 range)
unsigned char		g_LEDState[144][3], g_PrevLEDState[144][3];

// Keyboard layout with numbers - NORIDC LAYOUT
unsigned char		g_XYa[] = {
	137, 8, 20, 255,
	0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 6, 18, 30, 42, 32, 44, 56, 68, 255,
	1, 13, 25, 37, 49, 61, 73, 85, 97, 109, 121, 133, 7, 31, 54, 66, 78, 80, 92, 104, 116, 255,
	2, 14, 26, 38, 50, 62, 74, 86, 98, 110, 122, 134, 90, 126, 43, 55, 67, 9, 21, 33, 128, 255,
	3, 15, 27, 39, 51, 63, 75, 87, 99, 111, 123, 135, 114, 126, 57, 69, 81, 128, 255,
	4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 79, 103, 93, 105, 117, 140, 255,
	5, 17, 29, 53, 89, 101, 113, 91, 115, 127, 139, 129, 141, 140, 255,
};

// Put Virtual key code into array to get its led number - NORIDC LAYOUT
unsigned char		g_vkToled[] = {
	0, 
	0, 0, 0, 0, 0, 0, 0, 31, 2, 0,	
	0, 0, 126, 0, 0, 0, 0, 0, 42, 3,	//20 // ENTER IN RIGHT CORNER = 140
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
	0, 53, 78, 67, 55, 66, 115, 103, 139, 127,	//40
	0, 0, 0, 18, 54, 43, 0, 121, 13, 25,	
	37, 49, 61, 73, 85, 97, 109, 39, 38, 0,	//60
	0, 0, 0, 0, 15, 76, 52, 39, 38, 51,	
	63, 75, 98, 87, 99, 111, 100, 88, 110, 122,	//80
	14, 50, 27, 62, 86, 64, 26, 40, 74, 28,	
	17, 101, 113, 0, 0, 129, 93, 105, 117, 57,	//100
	69, 81, 9, 21, 33, 104, 128, 0, 116, 141,
	92, 12, 24, 36, 48, 60, 72, 84, 96, 108,	//120
	120, 132, 6, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	//140
	0, 0, 0, 80, 30, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 4,	//160
	79, 5, 91, 0, 89, 29, 0, 0, 0, 0,
	0, 0, 179, 0, 0, 68, 44, 32, 56, 0,	//180
	0, 0, 0, 0, 0, 90, 133, 112, 136, 124,
	114, 123, 0, 0, 0, 0, 0, 0, 0, 0,	//200
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 7, 1,	//220
	134, 135, 0, 0, 0, 16, 0, 0, 0, 0,
};
// Lookup table for virtual keycodes - NORIDC LAYOUT
unsigned char		g_XYv[] = {
	0x0, 0x0, 0xAD, 255,
	0x1B, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x2C, 0x91, 0x13, 0xB2, 0xB1, 0xB3, 0xB0, 255,
	0xDC, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0xBB, 0xDB, 0x8, 0x2D, 0x24, 0x21, 0x90, 0x6F, 0x6A, 0x6D, 255,
	0x9, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50, 0xDD, 0xBA, 0xD, 0x2E, 0x23, 0x22, 0x67, 0x68, 0x69, 0x6B, 255,
	0x14, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0xC0, 0xDE, 0xBF, 0xD, 0x64, 0x65, 0x66, 0x6B, 255,
	0xA0, 0xE2, 0x5A, 0x58, 0x43, 0x56, 0x42, 0x4E, 0x4D, 0xBC, 0xBE, 0xBD, 0xA1, 0x26, 0x61, 0x62, 0x63, 0xD, 255,
	0xA2, 0x5B, 0xA4, 0x20, 0xA5, 0x5C, 0x5D, 0xA3, 0x25, 0x28, 0x27, 0x60, 0x6E, 0xD, 255,
};
// Key lenghts
float				g_XYb[] = {
	-15.5, 1, 1, -2.5, 1, -2, 0,
	1, -.5, 1, 1, 1, 1, -.75, 1, 1, 1, 1, -.75, 1, 1, 1, 1, -.5, 1, 1, 1, -.5, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, -.5, 1, 1, 1, -.5, 1, 1, 1, 1, 0,
	1.5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1.5, -.5, 1, 1, 1, -.5, 1, 1, 1, 1, 0,
	1.75, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1.25, -4, 1, 1, 1, 1, 0,
	1.25, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2.75, -1.5, 1, -1.5, 1, 1, 1, 1, 0,
	1.5, 1, 1.25, 4.25, 3, 1.25, 1, 1, 1.5, -.5, 1, 1, 1, -.5, 2, 1, 1, 0,
};

// Defines 7 Rows, 92 columns (4 columns = 1 normal key)
unsigned char g_XY[7][92];
unsigned char g_XYvk[7][92];

bool g_KP[255];

//==================================================================================================
// Code by http://www.reddit.com/user/chrisgzy
//==================================================================================================
bool IsMatchingDevice(wchar_t *pDeviceID, unsigned int uiVID, unsigned int uiPID, unsigned int uiMI)
{
	unsigned int uiLocalVID = 0, uiLocalPID = 0, uiLocalMI = 0;

	LPWSTR pszNextToken = 0;
	LPWSTR pszToken = wcstok_s(pDeviceID, L"\\#&", &pszNextToken);
	while (pszToken)
	{
		std::wstring tokenStr(pszToken);
		if (tokenStr.find(L"VID_", 0, 4) != std::wstring::npos)
		{
			std::wistringstream iss(tokenStr.substr(4));
			iss >> std::hex >> uiLocalVID;
		}
		else if (tokenStr.find(L"PID_", 0, 4) != std::wstring::npos)
		{
			std::wistringstream iss(tokenStr.substr(4));
			iss >> std::hex >> uiLocalPID;
		}
		else if (tokenStr.find(L"MI_", 0, 3) != std::wstring::npos)
		{
			std::wistringstream iss(tokenStr.substr(3));
			iss >> std::hex >> uiLocalMI;
		}

		pszToken = wcstok_s(0, L"\\#&", &pszNextToken);
	}

	if (uiVID != uiLocalVID || uiPID != uiLocalPID || uiMI != uiLocalMI)
		return false;

	return true;
}

//==================================================================================================
// Code by http://www.reddit.com/user/chrisgzy
//==================================================================================================
HANDLE GetDeviceHandle(unsigned int uiVID, unsigned int uiPID, unsigned int uiMI)
{
	const GUID GUID_DEVINTERFACE_HID = { 0x4D1E55B2L, 0xF16F, 0x11CF, 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 };
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_HID, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return 0;

	HANDLE hReturn = 0;

	SP_DEVINFO_DATA deviceData = { 0 };
	deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (unsigned int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceData); ++i)
	{
		wchar_t wszDeviceID[MAX_DEVICE_ID_LEN];
		if (CM_Get_Device_IDW(deviceData.DevInst, wszDeviceID, MAX_DEVICE_ID_LEN, 0))
			continue;

		if (!IsMatchingDevice(wszDeviceID, uiVID, uiPID, uiMI))
			continue;

		SP_INTERFACE_DEVICE_DATA interfaceData = { 0 };
		interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

		if (!SetupDiEnumDeviceInterfaces(hDevInfo, &deviceData, &GUID_DEVINTERFACE_HID, 0, &interfaceData))
			break;

		DWORD dwRequiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, 0, 0, &dwRequiredSize, 0);

		SP_INTERFACE_DEVICE_DETAIL_DATA *pData = (SP_INTERFACE_DEVICE_DETAIL_DATA *)new unsigned char[dwRequiredSize];
		pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, pData, dwRequiredSize, 0, 0))
		{
			delete pData;
			break;
		}

		HANDLE hDevice = CreateFile(pData->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			delete pData;
			break;
		}

		hReturn = hDevice;
		break;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return hReturn;
}

//==================================================================================================
// Send the current LED state to the keyboard.
// Code by http://www.reddit.com/user/fly-hard
//==================================================================================================
void SendLEDState()
{
	// Check for changes since last time we sent the state
	if (!memcmp(g_PrevLEDState, g_LEDState, sizeof(g_LEDState)))
		return;
	memcpy(g_PrevLEDState, g_LEDState, sizeof(g_LEDState));

	unsigned char pkt[65] = { 0 };

	int p = 0;
	int cc = 0;
	auto getrgb = [&] {
		unsigned char b = g_LEDState[p][cc] | (g_LEDState[p + 1][cc] << 4);
		p += 2;
		if (p >= 144) {
			p = 0;
			cc++;
		}
		return b;
	};

	pkt[1] = 0x7F;
	pkt[2] = 0x01;
	pkt[3] = 0x3C;

	for (int i = 0; i < 0x3C; i++)
		pkt[i + 5] = getrgb();
	HidD_SetFeature(g_Device, pkt, 65);

	pkt[2]++;
	for (int i = 0; i < 0x3C; i++)
		pkt[i + 5] = getrgb();
	HidD_SetFeature(g_Device, pkt, 65);

	pkt[2]++;
	for (int i = 0; i < 0x3C; i++)
		pkt[i + 5] = getrgb();
	HidD_SetFeature(g_Device, pkt, 65);

	pkt[2]++;
	pkt[3] = 0x24;
	for (int i = 0; i < 0x24; i++)
		pkt[i + 5] = getrgb();
	memset(pkt + 0x24 + 5, 0, 65 - 0x24 - 5);
	HidD_SetFeature(g_Device, pkt, 65);

	memset(pkt, 0, 65);
	pkt[1] = 0x07;
	pkt[2] = 0x27;
	pkt[3] = 0;
	pkt[5] = 0xD8;
	HidD_SetFeature(g_Device, pkt, 65);
}

int SetLedRGB(int led, int r, int g, int b)
{
	return pLC->SetLedRGB(led, r, g, b);
}
int SetXYRGB(int x, int y, int r, int g, int b)
{
	return pLC->SetXYRGB(x, y, r, g, b);
}

//Rainbow with reactive typing
void RainbowRT()
{
	g_iInterval = 50;
	const int TotalTicks = 200;
	static int r = 7, g = 0, b = 0, Tick = 0, Ticker = 0;
	static int aRGB[TotalTicks][3];
	static bool bKeyPressed[92][7] = { 0 };
	static int ikeyTicker[92][7] = { 0 }, iKeyTicks = 7;
	if (Ticker > 6)
	{
		if (r == 7 && b != 7 && g == 0)
			b++;
		else if (b == 7 && r != 0 && g == 0)
			r--;
		else if (b == 7 && g != 7 && r == 0)
			g++;
		else if (g == 7 && b != 0 && r == 0)
			b--;
		else if (g == 7 && r != 7 && b == 0)
			r++;
		else if (r == 7 && g != 0 && b == 0)
			g--;
		else
			printf("FAIL\n");

		if (Tick >= TotalTicks)
			Tick = 0;

		aRGB[Tick][0] = r;
		aRGB[Tick][1] = g;
		aRGB[Tick][2] = b;
		Ticker = 0;
	}
	else
	{
		aRGB[Tick][0] = r;
		aRGB[Tick][1] = g;
		aRGB[Tick][2] = b;
	}

	//Rainbow
	for (int x = 0; x < 91; x++)
	{
		for (int y = 0; y < 7; y++)
		{
			int realTick = Tick - x;

			if (realTick < 0)
				realTick += TotalTicks;
			
			if (g_KP[g_XYvk[y][x]])
				bKeyPressed[x][y] = true;
			else
			{
				bKeyPressed[x][y] = false;
				SetXYRGB(x, y, aRGB[realTick][0], aRGB[realTick][1], aRGB[realTick][2]);
			}
		}
	}
	
	//Reactive Typing
	for (int x = 0; x < 91; x++)
	{
		for (int y = 0; y < 7; y++)
		{
			if (bKeyPressed[x][y])
			{
				static int Pressed;

				if (ikeyTicker[x][y] < MAXCOL)
					ikeyTicker[x][y] += 4;
				for (int x2 = 0; x2 < 91; x2++)
				{
					int realTick = Tick - x2;

					if (realTick < 0)
						realTick += TotalTicks;

					SetXYRGB(x2, y, ikeyTicker[x][y] + aRGB[realTick][0], ikeyTicker[x][y] + aRGB[realTick][1], ikeyTicker[x][y] + aRGB[realTick][2]);
				}
			}
			else
			{
				if (ikeyTicker[x][y] > 0)
				{
					ikeyTicker[x][y]--;
					for (int x2 = 0; x2 < 91; x2++)
					{
						int realTick = Tick - x2;

						if (realTick < 0)
							realTick += TotalTicks;

						SetXYRGB(x2, y, ikeyTicker[x][y] + aRGB[realTick][0], ikeyTicker[x][y] + aRGB[realTick][1], ikeyTicker[x][y] + aRGB[realTick][2]);
					}
				}
				else
					ikeyTicker[x][y] = 0;
			}
		}
	}
	Tick++;
	Ticker++;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
		switch (wParam)
		{
			case WM_KEYDOWN:
				g_KP[p->vkCode] = true;
				break;
			case WM_KEYUP:
				g_KP[p->vkCode] = false;
				break;
			default:
				break;
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void LedController()
{
	pLC->BuildMatrix();
	printf("Keyboard matrix setup.\n");
	pLC->BuildMatrixVK();
	printf("Virtual keyboard matrix setup.\n");

	g_iInterval = 10;
	int iDelta, iLastTick = GetTickCount(), iKey = 0;

	while (!g_ConsoleQuit)
	{
		iDelta = GetTickCount() - iLastTick;
		if (iDelta < g_iInterval)
		{
			Sleep(1);
			continue;
		}
		iLastTick = GetTickCount();

		RainbowRT();

		SendLEDState();
	}

	CloseHandle(g_Device);
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, &LowLevelKeyboardProc, hThisInstance, NULL);
	if (hHook == NULL)
	{
		printf("Error\n");
		return 1;
	}

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	MSG messages;

	srand(time_t(NULL));
	g_Device = GetDeviceHandle(0x1B1C, 0x1B13, 0x3);
	if (!g_Device) {
		printf("Device not found\n");
		return 1;
	}
	else
		printf("Device found.\n");

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)LedController, NULL, NULL, NULL);

	while (GetMessage(&messages, NULL, 0, 0))
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	printf("Ending.\n");
	return messages.wParam;
}