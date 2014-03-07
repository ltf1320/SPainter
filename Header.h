#pragma once
// 一些常量定义
#define MAINWINDOW_WIDTH    400      // 主窗口宽度
#define MAINWINDOW_HEIGHT   200      // 主窗口高度
#define TEXTBOX_WIDTH       300      // 文本框宽度
#define TEXTBOX_HEIGHT      20       // 文本框高度
#define TEXTBOX_MAXLENGTH   1024 // 文本框中文本的最大长度

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_LOADSTRING 100

#define TERROR -1

class TCPConnecter;
class Painter;

#define Safe_Release(P) if(P) {P->Release();P=NULL;}


enum PaintType
{
	P_Draw = 0, P_Erase, P_Ended,P_Clear
};

enum ButtonState
{
	DisConnected = 0,
	Connected = 1,
	Disabled
};


//methods:
HRESULT LoadBitmapFromFile(
	PCWSTR uri,
	UINT dweight,
	UINT dheight,
	ID2D1Bitmap **bitmap
	);

RECT moveRect(RECT rc, int dx, int dy);
char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn);
BOOL isPointInRect(D2D1_POINT_2F point, RECT rec);
D2D1_RECT_F rectToD2D1(RECT rec);

