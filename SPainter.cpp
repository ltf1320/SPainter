// SPainter.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Header.h"
#include "SPainter.h"
#include "Painter.h"


// ȫ�ֱ���: 


HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

HWND mainWND;
HWND hTextBoxWND;

TCHAR _TextBoxClass[] = _T("MySimpleTextBox"); // �ı��������
ATOM _RegisterTextBoxClass();                      // ע���ı������
HWND _CreateTextBoxWindow(HWND hParentWnd);        // �����ı���
LRESULT CALLBACK _TextBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); // �ı��򴰿���Ϣ������
void _DrawText(HDC hDC);                           // �����ı�
void _SetCaretPos(HWND hWnd);                      // ���ù��λ��
void _UpdateWindow(HWND hWnd);                     // ���´���


TCHAR _String[TEXTBOX_MAXLENGTH + 1] = _T("");     // �ı�
int    _StringPosition = ::_tcslen(_String);        // ����������ڵ�λ��

TCHAR stateText[100];
ButtonState cButtonState,hButtonState;
const TCHAR *cButtonTexts[3] = { { _T("Connect") }, { _T("Disconnect") }, { _T("Disabled") } };
const TCHAR *hButtonTexts[3] = { { _T("Host") }, { _T("End") }, { _T("Disabled") } };

RECT cButtonRect;
RECT hButtonRect;
bool mouseInCButton = 0;//�����cbutton����
bool mouseInHButton = 0;//�����hbutton����

RECT drawButtonRect, eraseButtonRect, clearButtonRect,aboutButtonRect;
bool mouseInDrawButton, mouseInEraseButton, mouseInClearButton,mouseInAboutButton;

ID2D1Factory* pDF = NULL;
ID2D1HwndRenderTarget* pRT = NULL;
RECT winrc;
IDWriteFactory* pDWriteFactory = NULL;
IWICImagingFactory* WICIFactory = NULL;
bool dialog_Show = 0;

Painter *SPainter = NULL;
PaintType pType = P_Draw;
BOOL mousePressed = 0;


VOID initD2D1()
{
	HRESULT hr;
	if (pDF == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pDF);
	}
	if (pDWriteFactory == NULL)
	{
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast < IUnknown**>(&pDWriteFactory)
			);
	}
	if (SPainter == NULL)
	{
		SPainter = new Painter();
	}
}

VOID initRT(HWND hWnd)
{
	if (pRT == NULL)
	{
		GetClientRect(hWnd, &winrc);
		HRESULT hr = pDF->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
			hWnd, D2D1::SizeU(winrc.right - winrc.left, winrc.bottom - winrc.top)
			),
			&pRT);
		cButtonRect.left = 100 + TEXTBOX_WIDTH+10;
		cButtonRect.top = 0;
		cButtonRect.right = 100 + TEXTBOX_WIDTH + 70+10;
		cButtonRect.bottom=TEXTBOX_HEIGHT;

		hButtonRect.left = 100 + TEXTBOX_WIDTH+70+20;
		hButtonRect.top = 0;
		hButtonRect.right = 100 + TEXTBOX_WIDTH + 70+20+60;
		hButtonRect.bottom = TEXTBOX_HEIGHT;

		drawButtonRect.left = 3;
		drawButtonRect.top = TEXTBOX_HEIGHT + 20;
		drawButtonRect.right = drawButtonRect.left + 20;
		drawButtonRect.bottom = drawButtonRect.top + 20;

		eraseButtonRect.left = drawButtonRect.right+3;
		eraseButtonRect.top = TEXTBOX_HEIGHT + 20;
		eraseButtonRect.right = eraseButtonRect.left + 20;
		eraseButtonRect.bottom = eraseButtonRect.top + 20;

		clearButtonRect.left = eraseButtonRect.right+3;
		clearButtonRect.top = TEXTBOX_HEIGHT + 20;
		clearButtonRect.right = clearButtonRect.left + 20;
		clearButtonRect.bottom = clearButtonRect.top + 20;

		aboutButtonRect.left = winrc.right - 23;
		aboutButtonRect.top = winrc.bottom - 23;
		aboutButtonRect.right = winrc.right - 3;
		aboutButtonRect.bottom = winrc.bottom - 3;
	}
}

ID2D1SolidColorBrush* pEraseBrush=NULL;
void drawEraser(HWND hWnd)
{
	if (pType != P_Erase)
		return;
	initRT(hWnd);
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);
	HRESULT hr;
	if (!pEraseBrush)
		hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pEraseBrush);
	pRT->DrawRectangle(D2D1::RectF(pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5), pEraseBrush, 0.5);
}

void drawText(HWND hWnd)
{
	HRESULT hr;
	initRT(hWnd);
	IDWriteTextFormat* pTFormat = NULL;
	ID2D1SolidColorBrush* pBrush = NULL;

	IWICBitmap* WICBitmap=NULL;

	hr = pDWriteFactory->CreateTextFormat(
		L"Gabriola",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		18.f,
		L"en-us",
		&pTFormat);
	if (SUCCEEDED(hr))
	{
		hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &pBrush);
	}

	
	D2D1_RECT_F IpAddrTextRect = D2D1::RectF(
		10,
		0,
		100,
		TEXTBOX_HEIGHT
		);

	D2D1_RECT_F stateTextRect = D2D1::RectF(
		100.f + TEXTBOX_WIDTH +150,
		0.f,
		(FLOAT)winrc.right,
		100.f
		);

	if (SUCCEEDED(hr))
	{
		pRT->Clear(D2D1::ColorF(D2D1::ColorF::White,0));
		pRT->DrawText(
			L"IP Address:",
			wcslen(L"IP Address:"),
			pTFormat,
			IpAddrTextRect,
			pBrush);
		pRT->DrawText(
			stateText,
			wcslen(stateText),
			pTFormat,
			stateTextRect,
			pBrush);
	}
	Safe_Release(pTFormat);
	Safe_Release(pBrush);
}

VOID drawBtn(HWND hWnd)
{
	HRESULT hr;
	initRT(hWnd);
	IDWriteTextFormat* pCBFormat = NULL;
	IDWriteTextFormat* pHBFormat = NULL;
	ID2D1SolidColorBrush* pBrush = NULL;

	IWICBitmap* WICBitmap = NULL;

	hr = pDWriteFactory->CreateTextFormat(
		L"Gabriola",
		NULL,
		mouseInCButton?DWRITE_FONT_WEIGHT_BLACK: DWRITE_FONT_WEIGHT_REGULAR,
	//	DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STYLE_ITALIC,
		DWRITE_FONT_STRETCH_NORMAL,
		18.f,
		L"en-us",
		&pCBFormat);
	if (SUCCEEDED(hr))
	{
		hr = pDWriteFactory->CreateTextFormat(
			L"Gabriola",
			NULL,
			mouseInHButton ? DWRITE_FONT_WEIGHT_BLACK : DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_ITALIC,
		//	DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			18.f,
			L"en-us",
			&pHBFormat);
	}
	if (SUCCEEDED(hr))
	{
		hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &pBrush);
	}

	D2D1_RECT_F CBRect = D2D1::RectF(
		cButtonRect.left,
		cButtonRect.top,
		cButtonRect.right,
		cButtonRect.bottom
		);

	D2D1_RECT_F HBRect = D2D1::RectF(
		hButtonRect.left,
		hButtonRect.top,
		hButtonRect.right,
		hButtonRect.bottom
		);

	if (SUCCEEDED(hr))
	{
		pRT->DrawText(
			cButtonTexts[cButtonState],
			wcslen(cButtonTexts[cButtonState]),
			pCBFormat,
			CBRect,
			pBrush);
		pRT->DrawText(
			hButtonTexts[hButtonState],
			wcslen(hButtonTexts[hButtonState]),
			pHBFormat,
			HBRect,
			pBrush);
	}
	Safe_Release(pCBFormat);
	Safe_Release(pHBFormat);
	Safe_Release(pBrush);
}
ID2D1Bitmap *pDrawBitmap = NULL, *pEraseBitmap = NULL, *pClearBitmap = NULL, *pAboutBitmap = NULL;
ID2D1SolidColorBrush *pEdgeBrush = NULL, *pChosedBrush = NULL;
void drawPBtn(HWND hWnd)
{
	initRT(hWnd);
	HRESULT hr = S_OK;
	if(!pEdgeBrush)
		hr=pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),&pEdgeBrush);
	if (SUCCEEDED(hr) && !pChosedBrush)
		hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 0.3), &pChosedBrush);
	if (SUCCEEDED(hr) && !pDrawBitmap)
	{
		hr = LoadBitmapFromFile(_T("pen.png"), drawButtonRect.right - drawButtonRect.left, drawButtonRect.bottom - drawButtonRect.top, &pDrawBitmap);
		if (FAILED(hr))
			hr = pRT->CreateBitmap(
			     D2D1::SizeU(drawButtonRect.right - drawButtonRect.left, drawButtonRect.bottom - drawButtonRect.top),
				 D2D1::BitmapProperties(
				       D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
				 &pDrawBitmap);
	}
	if (SUCCEEDED(hr) && !pEraseBitmap)
	{
		hr = LoadBitmapFromFile(_T("eraser.png"), eraseButtonRect.right - eraseButtonRect.left, eraseButtonRect.bottom - eraseButtonRect.top, &pEraseBitmap);
		if (FAILED(hr))
			hr = pRT->CreateBitmap(
			D2D1::SizeU(eraseButtonRect.right - eraseButtonRect.left, eraseButtonRect.bottom - eraseButtonRect.top),
			D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&pEraseBitmap);
	}
	if (SUCCEEDED(hr) && !pClearBitmap)
	{
		hr = LoadBitmapFromFile(_T("clear.png"), clearButtonRect.right - clearButtonRect.left, clearButtonRect.bottom - clearButtonRect.top, &pClearBitmap);
		if (FAILED(hr))
			hr = pRT->CreateBitmap(
			D2D1::SizeU(clearButtonRect.right - clearButtonRect.left, clearButtonRect.bottom - clearButtonRect.top),
			D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&pClearBitmap);
	}
	if (SUCCEEDED(hr) && !pAboutBitmap)
	{
		hr = LoadBitmapFromFile(_T("about.png"), aboutButtonRect.right - aboutButtonRect.left, aboutButtonRect.bottom - aboutButtonRect.top, &pAboutBitmap);
		if (FAILED(hr))
			hr = pRT->CreateBitmap(
			D2D1::SizeU(aboutButtonRect.right - aboutButtonRect.left, aboutButtonRect.bottom - aboutButtonRect.top),
			D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&pAboutBitmap);
	}

	pRT->DrawRectangle(rectToD2D1(drawButtonRect),pEdgeBrush,0.3);
	pRT->DrawRectangle(rectToD2D1(eraseButtonRect), pEdgeBrush, 0.3);
	pRT->DrawRectangle(rectToD2D1(clearButtonRect), pEdgeBrush, 0.3);
	pRT->DrawRectangle(rectToD2D1(aboutButtonRect), pEdgeBrush, 0.3);

	pRT->DrawBitmap(pDrawBitmap, rectToD2D1(drawButtonRect), mouseInDrawButton?1.0:0.5);
	pRT->DrawBitmap(pEraseBitmap, rectToD2D1(eraseButtonRect), mouseInEraseButton ? 1.0 : 0.5);
	pRT->DrawBitmap(pClearBitmap, rectToD2D1(clearButtonRect), mouseInClearButton ? 1.0 : 0.5);
	pRT->DrawBitmap(pAboutBitmap, rectToD2D1(aboutButtonRect), mouseInAboutButton ? 1.0 : 0.5);

	if (mouseInDrawButton)
		pRT->FillRectangle(rectToD2D1(drawButtonRect), pChosedBrush);
	if (mouseInEraseButton)
		pRT->FillRectangle(rectToD2D1(eraseButtonRect), pChosedBrush);
	if (mouseInClearButton)
		pRT->FillRectangle(rectToD2D1(clearButtonRect), pChosedBrush);
	if (mouseInAboutButton)
		pRT->FillRectangle(rectToD2D1(aboutButtonRect), pChosedBrush);
}

VOID drawImg(HWND hWnd)
{
	initRT(hWnd);
	HRESULT hr;
	ID2D1SolidColorBrush *pBrush;
	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush);
	if (SUCCEEDED(hr))
	{
		SPainter->doDraw(pRT, pBrush);
	}
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		dialog_Show = 1;
//		UpdateWindow(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			dialog_Show = 0;
//			SendMessage(mainWND, WM_PAINT, 0, 0);
//			PostMessage(mainWND, WM_PAINT, 0, 0);
			_UpdateWindow(mainWND);
//			RedrawWindow(mainWND, 0, 0, 0);
//			RedrawWindow(mainWND, NULL, NULL, RDW_UPDATENOW | RDW_VALIDATE | RDW_INVALIDATE | RDW_ALLCHILDREN); //this also works
//			ShowWindow(mainWND, 0);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
//	FILE *fp;
//	freopen_s(&fp,"log.txt", "w", stdout);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;
	initD2D1();
//	InitializeCriticalSection(&cs);

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPAINTER, szWindowClass, MAX_LOADSTRING);
	lstrcpy(stateText, _T("Initializing"));
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPAINTER));

	lstrcpy(stateText, _T("Ready"));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
//		EnterCriticalSection(&cs);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
//		LeaveCriticalSection(&cs);
	}

	return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX|WS_SYSMENU,
      300,50, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   mainWND = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int wmId, wmEvent;
	int x, y;
	D2D1_POINT_2F pt;
	RECT drawRect;
	drawRect.left = winrc.left;
	drawRect.top = winrc.top + TEXTBOX_HEIGHT+10;
	drawRect.right = winrc.right;
	drawRect.bottom = winrc.bottom;
	
	switch (message)
	{
	case WM_CREATE:
		_RegisterTextBoxClass();
		hTextBoxWND=_CreateTextBoxWindow(hWnd);
		break;
	case WM_PAINT:
		if (dialog_Show)
		{
			hdc = BeginPaint(hWnd, &ps);
			// TODO:  �ڴ���������ͼ����...
			EndPaint(hWnd, &ps);
		}
		initRT(hWnd);
		pRT->BeginDraw();
		drawText(hWnd);
		drawImg(hWnd);
		drawBtn(hWnd);
		drawPBtn(hWnd);
		drawEraser(hWnd);
		pRT->EndDraw();
//		UpdateWindow(hWnd);
		
		_UpdateWindow(hTextBoxWND);
		break;
	case WM_ACTIVATE:                // �����ڱ�����ʱ���������������ı�����
		::SetFocus(hTextBoxWND);

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam), y = HIWORD(lParam);
		pt = D2D1::Point2F((FLOAT)x, (FLOAT)y);
		if (isPointInRect(pt, moveRect(cButtonRect,0,10)))
		{
			if (cButtonState == DisConnected)
			{
				if (_tcslen(_String) > 0)
				{
					char* tmp = ConvertLPWSTRToLPSTR(_String);
					wsprintf(stateText, _T("connecting"));
					UpdateWindow(hWnd);
					if (SPainter->conTo(tmp) == TERROR)
					{
						break;
					}
					hButtonState = Disabled;
					cButtonState = Connected;
					delete tmp;
					UpdateWindow(hWnd);
				}
				
			}
			else if (cButtonState == Connected)
			{
				SPainter->endCon();
				hButtonState = DisConnected;
				cButtonState = DisConnected;
			}
		}
		else if (isPointInRect(pt, moveRect(hButtonRect,0,10)))
		{
			if (hButtonState == DisConnected)
			{
				MultiByteToWideChar(CP_ACP, NULL, "establishing", -1, stateText, 100);
				char localIp[100];
				UpdateWindow(hWnd);
				if (SPainter->beHost(localIp) == TERROR)
				{
					MultiByteToWideChar(CP_ACP, 0, "establishing failed", -1, stateText, 100);
					break;
				}
				MultiByteToWideChar(CP_ACP, 0, localIp, -1, _String, 100);
				MultiByteToWideChar(CP_ACP, 0, "waiting for connect", -1, stateText, 100);
				hButtonState = Connected;
				cButtonState = Disabled;
			}
			else if(hButtonState == Connected)
			{
				SPainter->endCon();
				hButtonState = DisConnected;
				cButtonState = DisConnected;
				MultiByteToWideChar(CP_ACP, 0, "host ended", -1, stateText, 100);
			}
		}
		else if (isPointInRect(pt,drawButtonRect))
		{
			pType = P_Draw;
		}
		else if (isPointInRect(pt, eraseButtonRect))
		{
			pType = P_Erase;
		}
		else if (isPointInRect(pt, clearButtonRect))
			SPainter->clear();
		else if (isPointInRect(pt, aboutButtonRect))
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//			MessageBox(NULL,_T("asd"),_T("S"),MB_OK);
		}
		else if (isPointInRect(pt, drawRect))
		{
			mousePressed = true;
			SPainter->beginDraw(x, y, pType);
		}
		UpdateWindow(hWnd);
		break;
	case WM_LBUTTONUP:
	case WM_MOUSELEAVE:
		SPainter->endDraw(-1,-1);
		mousePressed = false;
		break;

	case WM_MOUSEMOVE:
		x = LOWORD(lParam), y = HIWORD(lParam);
		pt = D2D1::Point2F((FLOAT)x, (FLOAT)y);
		mouseInCButton = false;
		mouseInHButton = false;
		mouseInDrawButton = false;
		mouseInEraseButton = false;
		mouseInClearButton = false;
		mouseInAboutButton = false;
		if (mousePressed&&isPointInRect(pt, drawRect))
		{
			SPainter->moveTo(x, y);
		}
		else
		{
			SPainter->endDraw(x,y);
			mousePressed = false;
			if (isPointInRect(pt, moveRect(cButtonRect,0,10)))
				mouseInCButton = true;
			if (isPointInRect(pt, moveRect(hButtonRect,0,10)))
				mouseInHButton = true;
			if (isPointInRect(pt, moveRect(drawButtonRect, 0, 10)))
				mouseInDrawButton = true;
			if (isPointInRect(pt, moveRect(eraseButtonRect, 0, 10)))
				mouseInEraseButton = true;
			if (isPointInRect(pt, moveRect(clearButtonRect, 0, 10)))
				mouseInClearButton = true;
			if (isPointInRect(pt, moveRect(aboutButtonRect, 0, 10)))
				mouseInAboutButton = true;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



// ע���ı������
ATOM _RegisterTextBoxClass()
{
	WNDCLASSEX wc;
	::ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(wc);
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;   // ָ�������ڳߴ緢���仯ʱ�ػ洰�ڣ�������Ӧ���˫���¼�
	wc.hInstance = hInst;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // ָ�����ڱ�����ɫΪϵͳ��ɫ�����ڱ�����
	wc.lpszClassName = _TextBoxClass;                  // ָ��Ҫע��Ĵ�����������������ʱҪ�Դ�����Ϊ��ʶ��
	wc.lpfnWndProc = _TextBoxWndProc;               // ��������Ϣ�ĺ���
	return ::RegisterClassEx(&wc);                     // ����API����ע���ı��򴰿�
}


// �����ı���
HWND _CreateTextBoxWindow(HWND hParentWnd)
{
	// ֮�´�����Ϊ�����ı�����ʾ�ڸ��������룬������λ��
	RECT parentWndRect;
	::GetClientRect(hParentWnd, &parentWndRect);  // ��ȡ�����ڿͻ�����λ��
	int left = parentWndRect.left+100, top = parentWndRect.top+10 ;

	// �����ı���
	HWND hWnd = ::CreateWindow(_TextBoxClass, NULL, WS_CHILDWINDOW | WS_VISIBLE,
		left, top, TEXTBOX_WIDTH, TEXTBOX_HEIGHT,
		hParentWnd, NULL, hInst, NULL);
	return hWnd;
}

// �ı�����Ϣ�Ĵ������
LRESULT CALLBACK _TextBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT: {  // ��������֮���Լ�һ�Դ����ţ���Ϊ����֮�¶���ı����ֲ���
		static PAINTSTRUCT ps;
		static RECT rect;
		HDC hDC = ::BeginPaint(hWnd, &ps);  // ��ʼ���Ʋ���

		::GetClientRect(hWnd, &rect);        // ��ȡ�ͻ����ĳߴ�
		::DrawEdge(hDC, &rect,EDGE_ETCHED, BF_BOTTOM);  // ���Ʊ߿�EDGE_SUNKEN��ʾ������ʽΪ��Ƕ��ʽ��BF_RECT��ʾ���ƾ��α߿�
		_DrawText(hDC);                      // �����ı�
		::EndPaint(hWnd, &ps);               // �������Ʋ���

	} break;

	case WM_SETFOCUS: {    // ��ý���
		::CreateCaret(hWnd, (HBITMAP)NULL, 1, TEXTBOX_HEIGHT - 5);     // �������
		_SetCaretPos(hWnd);                            // ���ù��λ��
		::ShowCaret(hWnd);                   // ��ʾ���
	} break;

	case WM_KILLFOCUS: // ʧȥ����
		::HideCaret(hWnd);                   // ���ع��
		::DestroyCaret();                    // ���ٹ��
		break;

	case WM_SETCURSOR: {  // ���ù����״
		static HCURSOR hCursor = ::LoadCursor(NULL, IDC_IBEAM);
		::SetCursor(hCursor);
	} break;

	case WM_CHAR: {    // �ַ���Ϣ
		TCHAR code = (TCHAR)wParam;
		int len = ::_tcslen(_String);
		if (code < (TCHAR)' ' || len >= TEXTBOX_MAXLENGTH)
			return 0;

		::MoveMemory(_String + _StringPosition + 1, _String + _StringPosition, (len - _StringPosition + 1) * sizeof(TCHAR));
		_String[_StringPosition++] = code;

		_UpdateWindow(hWnd);
		_SetCaretPos(hWnd);

	} break;

	case WM_KEYDOWN: {  // ��������Ϣ
		TCHAR code = (TCHAR)wParam;

		switch (code)
		{
		case VK_LEFT: // �����
			if (_StringPosition > 0)
				_StringPosition--;
			break;

		case VK_RIGHT:     // �ҹ���
			if (_StringPosition < (int)::_tcslen(_String))
				_StringPosition++;
			break;

		case VK_HOME: // HOME ��
			_StringPosition = 0;
			break;

		case VK_END:  // END ��
			_StringPosition = ::_tcslen(_String);
			break;

		case VK_BACK: // �˸��
			if (_StringPosition > 0)
			{
				::MoveMemory(_String + _StringPosition - 1, _String + _StringPosition, (::_tcslen(_String) - _StringPosition + 1) * sizeof(TCHAR));
				_StringPosition--;
				_UpdateWindow(hWnd);
			}
			break;

		case VK_DELETE: {  // ɾ����
			int len = ::_tcslen(_String);
			if (_StringPosition < len)
			{
				::MoveMemory(_String + _StringPosition, _String + _StringPosition + 1, (::_tcslen(_String) - _StringPosition + 1) * sizeof(TCHAR));
				_UpdateWindow(hWnd);
			}

		} break;

		}

		_SetCaretPos(hWnd);

	} break;

	case WM_LBUTTONDOWN: {  // ��굥�������ù��λ��
		int x = LOWORD(lParam);
		HDC hDc = ::GetDC(hWnd);

		int strLen = ::_tcslen(_String), strPos = 0;
		SIZE size;
		SetFocus(hWnd);
		for (strPos = 0; strPos<strLen; strPos++)
		{
			::GetTextExtentPoint(hDc, _String, strPos, &size);

			if (size.cx + 4 >= x)
				break;
		}

		_StringPosition = strPos;
		::GetTextExtentPoint(hDc, _String, strPos, &size);
		::SetCaretPos(size.cx + 4, 3);

		::ReleaseDC(hWnd, hDc);

	} break;

	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}

	return (LRESULT)0;
}

// ���´���
void _UpdateWindow(HWND hWnd)
{
	RECT rect;
	::GetClientRect(hWnd, &rect);
	::InvalidateRect(hWnd, &rect, TRUE);
	::UpdateWindow(hWnd);
}

// �����ı�
void _DrawText(HDC hDC)
{
	int len = ::_tcslen(_String);
	::TextOut(hDC, 4, 2, _String, len);
}

// ���ù��λ��
void _SetCaretPos(HWND hWnd)
{
	HDC hDC = ::GetDC(hWnd);

	SIZE size;
	::GetTextExtentPoint(hDC, _String, _StringPosition, &size);
	::SetCaretPos(4 + size.cx, 3);

	::ReleaseDC(hWnd, hDC);

}
