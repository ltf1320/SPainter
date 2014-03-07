#pragma once
#include "Header.h"
#include "stdafx.h"

//yiikou is bool


class Painter
{
public:
	Painter();
	~Painter();
	void beginDraw(UINT x, UINT y,PaintType type);
	void endDraw(UINT x, UINT y);
	void moveTo(UINT x, UINT y);
	FLOAT radius;
	void doDraw(ID2D1HwndRenderTarget* pRT, ID2D1SolidColorBrush *brush);

	int conTo(char *ip);
	int beHost(char *localIp);
	void hostIsConnected();

	void clear();
	void endCon();
	void doCmd(char *cmd);
private:
	char rec[WINDOW_WIDTH][WINDOW_HEIGHT - TEXTBOX_HEIGHT];
	D2D1_POINT_2F lastPoint;
	PaintType nowType;
	bool connected;
	TCPConnecter* connecter;
	friend class TCPConnecter;
};

