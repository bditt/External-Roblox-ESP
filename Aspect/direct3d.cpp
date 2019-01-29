#include <direct3d.h>

//Direct3D Resources
IDirect3DDevice9Ex* pDevice;
IDirect3D9Ex* pInstance;
D3DPRESENT_PARAMETERS pParams;

ID3DXFont* pFontVisualsLarge;
ID3DXFont* pFontVisualsSmall;

ID3DXFont* pFontUILarge;
ID3DXFont* pFontUISmall;

ID3DXLine* pLine;

char buf[128];

//Overlay FPS
float flCurTick, flOldTick;
int iTempFPS, iRenderFPS;

BOOL D3DInitialize(HWND hWnd) {

	//Direct3D
	Direct3DCreate9Ex(D3D_SDK_VERSION, &pInstance);

	pParams.Windowed = TRUE;
	pParams.BackBufferFormat = D3DFMT_A8R8G8B8;
	pParams.BackBufferHeight = Globals::rHeight;
	pParams.BackBufferWidth = Globals::rWidth;
	pParams.MultiSampleQuality = DEFAULT_QUALITY;
	pParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pParams.EnableAutoDepthStencil = TRUE;
	pParams.AutoDepthStencilFormat = D3DFMT_D16;
	pParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	pInstance->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pParams, 0, &pDevice);

	if (pDevice == NULL) { return FALSE; }

	D3DXCreateFont(pDevice, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Tahoma", &pFontVisualsLarge);
	D3DXCreateFont(pDevice, 12, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Tahoma", &pFontVisualsSmall);

	D3DXCreateFont(pDevice, 14, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Tahoma", &pFontUILarge);
	D3DXCreateFont(pDevice, 12, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Tahoma", &pFontUISmall);

	D3DXCreateLine(pDevice, &pLine);

	//ImGui
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig fontCfg = ImFontConfig();
	io.DeltaTime = 1.0f / 60.0f;

	fontCfg.RasterizerFlags = 0x01;
	fontCfg.OversampleH = fontCfg.OversampleV = 1;
	fontCfg.PixelSnapH = true;

	ImGui_ImplDX9_Init(hWnd, pDevice);

	return TRUE;
}

void D3DRender() {
	pParams.BackBufferHeight = Globals::rHeight;
	pParams.BackBufferWidth = Globals::rWidth;
	pDevice->Reset(&pParams);
	pDevice->Clear(NULL, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0, 0), 1.0f, NULL);//->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

	pDevice->BeginScene();

	if (Globals::tWnd == GetForegroundWindow() || GetActiveWindow() == GetForegroundWindow()) {
		//Get FPS
		iGetFPS(&Globals::iFPS);

		//Render
		Render::DrawVisuals(pDevice);
		Render::DrawInfo(pDevice);

		if (Globals::bShowMenu)
			Render::DrawMenu(pDevice);
	}


	pDevice->EndScene();
	pDevice->PresentEx(0, 0, 0, 0, 0);
}

void DrawCrosshair(float r, float g, float b, float size) {
	//DrawLine(((swidth / 2) - 10.0f * size) - 1.0f, (sheight / 2) + 0.006f, ((swidth / 2) + 10.5f * size) + 1.0f, (sheight / 2) + 0.006f, 1, 1, 1, 255, 3.0f);
	//DrawLine((swidth / 2) - 0.006f, ((sheight / 2) - 10.0f * size) - 1.0f, (swidth / 2) - 0.006f, ((sheight / 2) + 11.0f * size) + 0.006f, 1, 1, 1, 255, 3.0f);
	//DrawLine(((swidth / 2) - 10 * size), sheight / 2, ((swidth / 2) + 10.5f * size), sheight / 2, r, g, b, 255, 1.0f);
	//DrawLine(swidth / 2, ((sheight / 2) - 10.0f * size), swidth / 2, ((sheight / 2) + 11.0f * size), r, g, b, 255, 1.0f);
}

void DrawString(const char* string, float x, float y, int r, int g, int b, int a, ID3DXFont* pFont) {
	RECT rPosition;
	rPosition.left = x;
	rPosition.top = y;
	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(r, g, b, a));
}

void DrawStringOutline(const char* string, float x, float y, int r, int g, int b, int a, ID3DXFont* pFont) {
	RECT rPosition;

	rPosition.left = x + 1.0f;
	rPosition.top = y;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x - 1.0f;
	rPosition.top = y;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x;
	rPosition.top = y + 1.0f;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x;
	rPosition.top = y - 1.0f;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x;
	rPosition.top = y;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(r, g, b, a));
}

void DrawRect(RECT drawRect, int r, int g, int b, int a) {
	D3DXVECTOR2 points[5];
	pLine->SetWidth(5);

	float x = drawRect.left;
	float y = drawRect.top;
	float x2 = drawRect.right;
	float y2 = drawRect.bottom;

	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x2, y);
	points[2] = D3DXVECTOR2(x2, y2);
	points[3] = D3DXVECTOR2(x, y2);
	points[4] = D3DXVECTOR2(x, y);

	pLine->Begin();
	pLine->Draw(points, 5, D3DCOLOR_RGBA(r, g, b, a));
	pLine->End();
}

void DrawLine(float x, float y, float x2, float y2, int r, int g, int b, int a, float w) {
	D3DXVECTOR2 points[2];
	pLine->SetWidth(w);

	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x2, y2);

	pLine->Begin();
	pLine->Draw(points, 2, D3DCOLOR_RGBA(r, g, b, a));
	pLine->End();
}

void DrawRectOutlined(RECT drawRect, int r, int g, int b, int a) {
	D3DXVECTOR2 points[5];

	float x = drawRect.left;
	float y = drawRect.top;
	float x2 = drawRect.right;
	float y2 = drawRect.bottom;

	points[0] = D3DXVECTOR2(x + 0.25f, y + 0.25f);
	points[1] = D3DXVECTOR2(x2 + 0.25f, y + 0.25f);
	points[2] = D3DXVECTOR2(x2 + 0.25f, y2 + 0.25f);
	points[3] = D3DXVECTOR2(x + 0.25f, y2 + 0.25f);
	points[4] = D3DXVECTOR2((x + 0.25f), y + 0.25f);

	pLine->SetWidth(3.5f);
	pLine->Draw(points, 5, D3DCOLOR_RGBA(1, 1, 1, a));

	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x2, y);
	points[2] = D3DXVECTOR2(x2, y2);
	points[3] = D3DXVECTOR2(x, y2);
	points[4] = D3DXVECTOR2(x, y);

	pLine->SetWidth(1);
	pLine->Begin();
	pLine->Draw(points, 5, D3DCOLOR_RGBA(r, g, b, a));
	pLine->End();
}

void FillRect(RECT drawRect, int r, int g, int b, int a) {
	D3DRECT RectDraw = { drawRect.left, drawRect.top, drawRect.right, drawRect.bottom };

	pDevice->Clear(1, &RectDraw, D3DCLEAR_TARGET | D3DCLEAR_TARGET, D3DCOLOR_RGBA(r, g, b, a), 0, 0);
}

void FillRectXY(float x, float y, float x2, float y2, int r, int g, int b, int a) {
	D3DXVECTOR2 points[2];

	int h = x2 - x;
	int w = y2 - y;

	pLine->SetWidth(h);

	points[0].x = x; points[0].y = y + (h >> 1);
	points[1].x = x + w; points[1].y = y + (h >> 1);

	pLine->Begin();
	pLine->Draw(points, 2, D3DCOLOR_RGBA(r, g, b, a));
	pLine->End();
}

void DrawCircle(int X, int Y, int radius, int numSides, int r, int g, int b, int a)
{
	D3DXVECTOR2 Line[128];
	float Step = M_PI * 2.0 / numSides;
	int Count = 0;
	for (float a = 0; a < M_PI*2.0; a += Step)
	{
		float X1 = radius * cos(a) + X;
		float Y1 = radius * sin(a) + Y;
		float X2 = radius * cos(a + Step) + X;
		float Y2 = radius * sin(a + Step) + Y;
		Line[Count].x = X1;
		Line[Count].y = Y1;
		Line[Count + 1].x = X2;
		Line[Count + 1].y = Y2;
		Count += 2;
	}
	pLine->Begin();
	pLine->Draw(Line, Count, D3DCOLOR_RGBA(r, g, b, a));
	pLine->End();
}

void DrawStringOutlinexxx(char* string, float x, float y, int r, int g, int b, int a, ID3DXFont* pFont) {
	RECT rPosition;

	rPosition.left = x + 1.0f;
	rPosition.top = y;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x - 1.0f;
	rPosition.top = y;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x;
	rPosition.top = y + 1.0f;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x;
	rPosition.top = y - 1.0f;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(1, 1, 1, a));

	rPosition.left = x;
	rPosition.top = y;

	pFont->DrawTextA(0, string, strlen(string), &rPosition, DT_NOCLIP, D3DCOLOR_RGBA(r, g, b, a));
}

void iGetFPS(int * fps) {

	flCurTick = clock() * 0.001f;
	iTempFPS++;
	if ((flCurTick - flOldTick) > 1.0f) {
		iRenderFPS = iTempFPS;

		iTempFPS = 0;
		flOldTick = flCurTick;
	}

	*(int *)fps = iRenderFPS;
}