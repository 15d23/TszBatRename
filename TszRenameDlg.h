#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "FileMetaData.h"

bool PathLess(CString p1, CString p2);
struct PathLessPred : std::binary_function<CString, CString, bool>
{
	bool operator()(CString p1, CString p2) const {
		return PathLess(p1, p2);
	}
};

class CTszRenameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTszRenameDlg)
public:
	CTszRenameDlg(CWnd* pParent = NULL);	// ��׼���캯��

	enum { IDD = IDD_TSZRENAME_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
	void InitFileListCtrl();
	void AddToolTips();
	void SetToolTips(UINT uId, CString tips);
	void AppendToFileList(CString filePath);
	void InsertStringToNamePat(CString mark);
	void InsertStringToEXTPat(CString mark);

	// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CToolTipCtrl m_tooltip_ctrl;
	int m_counter_start;		// ��������ʼֵ
	int m_counter_step;			// ����������
	int m_counter_max_len;		// ������λ��
	int m_replace_word_case;	// �ļ�����Сд����
	DWORD m_n_file_name_pos;
	CString m_file_name_pat;	// �ļ�������
	CString m_ext_name_pat;		// ��չ������
	CComboBox m_cmb_str_find;	// ����
	CComboBox m_cmb_str_replace;// �滻Ϊ
	CListCtrl m_file_list_ctrl;	// �ļ��б�
	int m_sort_column;			//������
	bool m_sort_inc;			//����
	bool m_renaming;			//����������
	map<int, CString> m_file_idx_to_name_map;
	std::map<CString, CFileMetaData, PathLessPred> m_file_meta_data_map;
	map<CString, CString> m_new_name_map;
	vector<CString> m_input_file_list;

public:
	void set_file_list(vector<CString> input_file_list);
	void sort_file_list();
	void rename_file_list();
	void set_word_case();
	void replace_str_by_pat();
	CString replace_str_by_pat(const CFileMetaData& meta, vector<CString> pats, bool ext = false);
	vector<CString> split_name_pattern(CString name_pat);

	afx_msg void OnBnClickedButton_SelectFiles();
	afx_msg void OnBnClickedButton_SelectFold();
	afx_msg void OnBnClickedButton_CleanFiles();
	afx_msg void OnBnClickedButton_RemoveFiles();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnEditchangeComboFilenamePat();
	afx_msg void OnCbnSelchangeComboWordCase();
	afx_msg void OnEnChangeEditExtNamePat();
	afx_msg void OnLvnColumnclickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton_RenameNow();
	afx_msg void OnBnClickedButton_Rename_UNDO();
	afx_msg void OnBnClickedButton_Name_Range();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnCbnSelchangeComboOldString();
	afx_msg void OnCbnEditchangeComboOldString();
	afx_msg void OnCbnSelchangeComboNewString();
	afx_msg void OnCbnEditchangeComboNewString();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnEnChangeEditCounterStart();
	afx_msg void OnEnChangeEditCounterStep();
	afx_msg void OnCbnSelchangeComboCounterLen();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	CComboBox m_cmbFilename;
	CEdit m_edEXT;
};

bool StringSearch(CString src, CString pat, int& begin, int& end);

bool StringMatch(CString src, CString pat, int begin, int& end);
