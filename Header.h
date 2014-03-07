#pragma once
// һЩ��������
#define MAINWINDOW_WIDTH    400      // �����ڿ��
#define MAINWINDOW_HEIGHT   200      // �����ڸ߶�
#define TEXTBOX_WIDTH       300      // �ı�����
#define TEXTBOX_HEIGHT      20       // �ı���߶�
#define TEXTBOX_MAXLENGTH   1024 // �ı������ı�����󳤶�

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

