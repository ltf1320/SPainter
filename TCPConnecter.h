#pragma once
#include "stdafx.h"
#include "Header.h"
#include "Painter.h"
class TCPConnecter
{
public:
	enum TType
	{
		Host,
		Client
	};
	TType type;
	TCPConnecter(Painter *pter,TType);
	~TCPConnecter();
	int conTo(char *ip);
	int synData();
	int listenStart();
	int sendChange(PaintType pType,UINT x,UINT y);
	void ConEnd();
	int getLocalIp(char *ip);
private:
	SOCKET sock;
	Painter* painter;
	WSADATA wsaData;
	static DWORD WINAPI HostListenThread(LPVOID pArg);
	static DWORD WINAPI ClientListenThread(LPVOID pArg);
	HANDLE thread;
};


