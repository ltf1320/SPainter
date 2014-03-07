#include "stdafx.h"
#include "TCPConnecter.h"

#define THREADEND {TConner->painter->endCon();return TERROR;}


TCPConnecter::TCPConnecter(Painter* pter,TType tType)
{
	painter = pter;
	type = tType;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
}


TCPConnecter::~TCPConnecter()
{
	ConEnd();
	WSACleanup();
}

int TCPConnecter::conTo(char *ip)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(7777);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return TERROR;
	/*
	//check
	char buf[20];
	sprintf_s(buf, "who is bool");
	if (send(sock, buf, 20, 0) == SOCKET_ERROR)
		return TERROR;
	if (recv(sock, buf, 20, 0) == SOCKET_ERROR)
		return TERROR;
	if (strcmp(buf, "sbmomo"))
		return TERROR;
		*/
	return 0;
}

int TCPConnecter::synData()
{
	char buf[10];
	switch (type)
	{
	case Client:
		memset(painter->rec, 0, sizeof(painter->rec));
		int pp;
		for (int i = 0; i < WINDOW_WIDTH; i++)
		{
			if ((pp = recv(sock,painter->rec[i], WINDOW_HEIGHT - TEXTBOX_HEIGHT, 0)) != WINDOW_HEIGHT - TEXTBOX_HEIGHT)
				return TERROR;
			if (send(sock, buf, 1, 0) ==SOCKET_ERROR)
				return TERROR;

		}
		break;
	case Host:
		int op = sizeof(painter->rec);
		for (int i = 0; i < WINDOW_WIDTH; i++)
		{
			op = sizeof(painter->rec[i]);
			if (op != 580)
			{
				op++;
				op--;
			}
			if (send(sock, painter->rec[i], WINDOW_HEIGHT - TEXTBOX_HEIGHT, 0) == SOCKET_ERROR)
				return TERROR;
			if (recv(sock, buf, 1, 0) == SOCKET_ERROR)
				return TERROR;
		}
		break;

	}
	return 0;
}

int TCPConnecter::sendChange(PaintType pType, UINT x, UINT y)
{
	char buf[100];
	sprintf_s(buf, "%d:%d:%d\0", pType, x, y);
	if (send(sock, buf, sizeof(buf),0) == SOCKET_ERROR)
	{
		return TERROR;
	}
	return 0;
}

int TCPConnecter::listenStart()
{
	switch (type)
	{
		case Host:
			thread = CreateThread(NULL, 0, HostListenThread, this, 0, NULL);
			break;
		case Client:
			thread = CreateThread(NULL, 0, ClientListenThread, this, 0, NULL);
			break;
	}
	return 0;
}

DWORD TCPConnecter::HostListenThread(LPVOID pArg)
{
	TCPConnecter *TConner = reinterpret_cast<TCPConnecter*>(pArg);
	if (TConner == NULL)
		return false;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(7777);
	SOCKET mysock;
	mysock = socket(AF_INET, SOCK_STREAM, 0);
	bind(mysock, (sockaddr*)&addr, sizeof(addr));
	if (listen(mysock, 5) == SOCKET_ERROR)
		THREADEND
	TConner->sock = accept(mysock, (sockaddr*)&addr, NULL);
	if (TConner->sock == INVALID_SOCKET)
		THREADEND
	char buf[100];
	/*
	//check
	if (recv(TConner->sock, buf, 20, 0) == SOCKET_ERROR)
		THREADEND;
	if (strcmp(buf, "who is bool"))
		THREADEND;
	if (send(TConner->sock, "sbmomo", 20, 0) == SOCKET_ERROR)
		THREADEND;
	*/
	TConner->painter->hostIsConnected();
	//listen cmd
	while (true)
	{
		if (recv(TConner->sock, buf, 100, 0) == SOCKET_ERROR)
			THREADEND;
		TConner->painter->doCmd(buf);
	}
}

DWORD TCPConnecter::ClientListenThread(LPVOID pArg)
{
	TCPConnecter *TConner = reinterpret_cast<TCPConnecter*>(pArg);
	if (TConner->sock==INVALID_SOCKET)
		THREADEND
	char buf[100];
	//listen cmd
	while (true)
	{
		if (recv(TConner->sock, buf, 100, 0) == SOCKET_ERROR)
			THREADEND
		TConner->painter->doCmd(buf);
	}
}



void TCPConnecter::ConEnd()
{
	DWORD pt;
	GetExitCodeThread(thread, &pt);
	if (pt == STILL_ACTIVE)
		TerminateThread(thread, 0);
	GetExitCodeThread(thread, &pt);
	int err=closesocket(sock);
}

int TCPConnecter::getLocalIp(char *ip)
{
	char tmp[100];
	PHOSTENT hostinfo;
	char *ipAddress;

	if ((gethostname(tmp, sizeof(tmp))) == 0) {
		if ((hostinfo = gethostbyname(tmp)) != NULL) {
			ipAddress = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
		}
		else
		{
			return TERROR;
		}
	}
	else return TERROR;
	strcpy_s(ip,100, ipAddress);
	return 0;
}