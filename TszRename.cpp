#include "stdafx.h"
#include "TszRename.h"
#include "TszRenameDlg.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTszRenameApp, CWinApp)
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CTszRenameApp::CTszRenameApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

// Ψһ��һ�� CTszRenameApp ����
CTszRenameApp theApp;

// CTszRenameApp ��ʼ��
BOOL CTszRenameApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager* pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("TszBatRename"));

	CWinApp* pApp = AfxGetApp();
	CString cmdLine = pApp->m_lpCmdLine;
	vector<CString> items;
	int n = cmdLine.Find(_T("-f"));
	if (n >= 0) {
		cmdLine = cmdLine.Right(cmdLine.GetLength() - n - 3);
		bool queto = false;
		CString item;
		for (int i = 0; i < cmdLine.GetLength(); ++i) {
			TCHAR c = cmdLine[i];
			if (c == _T('"')) {
				if (queto) {
					item += c;
					items.push_back(item);
					item = _T("");
					queto = false;
				} else {
					queto = true;
					item += c;
				}
			} else if (_istspace(c)) {
				if (queto) {
					item += c;
				} else {
					if (!item.IsEmpty()) {
						items.push_back(item);
						item = _T("");
					}
				}
			} else {
				item += c;
			}
		}
		if (!item.IsEmpty()) {
			items.push_back(item);
			item = _T("");
		}
	}

	CTszRenameDlg dlg;
	dlg.set_file_list(items);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	} else if (nResponse == IDCANCEL) {
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	} else if (nResponse == -1) {
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL) {
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

int CTszRenameApp::ExitInstance()
{
	DestorySetting();
	return CWinApp::ExitInstance();
}