#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

class CTszRenameApp : public CWinApp
{
public:
	CTszRenameApp();

	// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTszRenameApp theApp;