// TszRename.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

// CTszRenameApp:
// �йش����ʵ�֣������ TszRename.cpp
//

class CTszRenameApp : public CWinApp
{
public:
	CTszRenameApp();

	// ��д
public:
	virtual BOOL InitInstance();

	// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTszRenameApp theApp;