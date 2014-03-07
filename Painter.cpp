#include "stdafx.h"
#include "Painter.h"
#include <cmath>
#include <cstdio>
#include "TCPConnecter.h"
extern TCHAR stateText[100];
extern TCHAR _String[TEXTBOX_MAXLENGTH + 1];
extern int    _StringPosition;

Painter::Painter()
{
	memset(rec, 0, sizeof(rec));
	nowType = P_Ended;
	connected = 0;
	connecter = NULL;
}

Painter::~Painter()
{
}

void Painter::moveTo(UINT x, UINT y)
{
	
	FLOAT nx=lastPoint.x, ny=lastPoint.y, dx, dy,d;
	if (fabs(x - nx) < fabs(y-ny))
	{
		d = abs(y - ny);
		dy = (FLOAT)(y>ny ? 1 : -1);
		dx = dy*(x - nx) / (y - ny);
	}
	else
	{
		d = fabs(x - nx);
		dx = (FLOAT)(x>nx ? 1 : -1);
		dy = dx*(y - ny) / (x - nx);
	}
//	wsprintf(stateText, _T("%d"), (int)d);
	for (int i = 0; i < d;i++)
	{
		if (nx<0 || nx>WINDOW_WIDTH)
			break;
		if (ny<TEXTBOX_HEIGHT || ny>WINDOW_HEIGHT)
		{
			break;
		}
		switch (nowType)
		{
		case P_Draw:rec[(int)floor(nx + 0.5)][(int)floor(ny + 0.5)-TEXTBOX_HEIGHT] = 1;
			if (connected)
				connecter->sendChange(nowType, (int)floor(nx + 0.5), (int)floor(ny + 0.5) - TEXTBOX_HEIGHT);
			break;
		case P_Erase:
			for (int i = 0; i < 10; i++)
				for (int j = 0; j < 10; j++)
				{
					rec[(int)floor(nx + 0.5)-5+i][(int)floor(ny + 0.5) - TEXTBOX_HEIGHT-5+j] = 0;
					if (connected)
						connecter->sendChange(nowType, (int)floor(nx + 0.5)-5+i, (int)floor(ny + 0.5)-5+j - TEXTBOX_HEIGHT);
				}
			break;
		case P_Ended:return;
		}
		nx += dx;
		ny += dy;
	}
	lastPoint.x = (FLOAT)x;
	lastPoint.y = (FLOAT)y;
}


void Painter::doDraw(ID2D1HwndRenderTarget* pRT, ID2D1SolidColorBrush *pBrush)
{

	for (UINT i = 0; i < WINDOW_WIDTH; i++)
		for (UINT j = 0; j < WINDOW_HEIGHT - TEXTBOX_HEIGHT; j++)
		{
			if (rec[i][j])
			{
//				pRT->DrawLine(D2D1::Point2F((FLOAT)i + TEXTBOX_HEIGHT, (FLOAT)j), D2D1::Point2F((FLOAT)i + 1 + TEXTBOX_HEIGHT, (FLOAT)j + 1), pBrush);
				pRT->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((FLOAT)i, (FLOAT)j + TEXTBOX_HEIGHT), 0.7f, 0.7f), pBrush);
			}
		}
}

void Painter::beginDraw(UINT x, UINT y, PaintType type)
{
	nowType = type;
	lastPoint.x = (FLOAT)x;
	lastPoint.y = (FLOAT)y;
}

void Painter::endDraw(UINT x, UINT y)
{
	if (nowType == P_Ended)
		return;
	if ((~x)&&(~y))
		moveTo(x, y);
	nowType = P_Ended;
}

void Painter::doCmd(char *cmd)
{
	PaintType pType;
	int x, y;
	if (sscanf_s(cmd, "%d:%d:%d", &pType, &x, &y) != 3)
		return;
	switch (pType)
	{
	case P_Draw:
		rec[x][y] = 1;
		break;
	case P_Erase:
		rec[x][y] = 0;
		break;
	case P_Clear:
		memset(rec, 0, sizeof(rec));
		break;
	case P_Ended:
		break;
	default:
		break;
	}
}

int Painter::conTo(char *ip)
{
	if (connecter != NULL)
		delete connecter;
	connecter = new TCPConnecter(this,TCPConnecter::Client);
	if (connecter->conTo(ip) == TERROR)
	{
		wsprintf(stateText, _T("connect failed"));
		return TERROR;
	}
	wsprintf(stateText, _T("synchronizing"));
	if (connecter->synData() == TERROR)
	{
		wsprintf(stateText, _T("connect failed"));
		return TERROR;
	}
	connecter->listenStart();
	connected = true;
	wsprintf(stateText, _T("connect succeeded!"));
	return 0;
}

void Painter::endCon()
{
	if (connected)
	{
		wsprintf(stateText, _T("connect ended"));
		connected = false;
		delete connecter;
		connecter = NULL;
	}
}

int Painter::beHost(char *localIp)
{
	if (connecter != NULL)
		delete connecter;
	connecter = new TCPConnecter(this, TCPConnecter::Host);
	if (connecter->getLocalIp(localIp) == TERROR)
	{
		return TERROR;
	}
	connecter->listenStart();
	return 0;
}

void Painter::hostIsConnected()
{
	connected = true;
	wsprintf(stateText, _T("synchronizing"));
	if (connecter->synData() == TERROR)
	{
		wsprintf(stateText, _T("synchronizing failed"));
		return ;
	}
	MultiByteToWideChar(CP_ACP, 0, "connected", -1, stateText, 100);
}

void Painter::clear()
{
	memset(rec, 0, sizeof(rec));
	if (connected)
		connecter->sendChange(P_Clear, 0, 0);
}