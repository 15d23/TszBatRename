#include "stdafx.h"
#include "afxdialogex.h"
#include "TszRename.h"
#include "FileMetaData.h"
#include "TszRenameDlg.h"
#include "SelectTextRangeDlg.h"
#include "Settings.h"

using namespace std;
using namespace String;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct
{
	LVITEM* plvi;
	BOOL bCheck;
	CString sCol1;
	CString sCol2;
	CString sCol3;
	CString sCol4;
	CString sCol5;
} lvItem, *plvItem;

// CTszRenameDlg �Ի���

IMPLEMENT_DYNAMIC(CTszRenameDlg, CDialogEx)

CTszRenameDlg::CTszRenameDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TSZRENAME_DIALOG, pParent)
	, m_counter_start(1)
	, m_counter_step(1)
	, m_counter_max_len(0)
	, m_replace_word_case(0)
	, m_file_name_pat(_T("<N>"))
	, m_ext_name_pat(_T("<E>"))
	, m_sort_column(0)
	, m_sort_inc(true)
	, m_renaming(false)
	, m_n_file_name_pos(0)
	, m_pDragList(0)
	, m_pDropList(0)
	, m_pDragImage(0)
	, m_bDragging(0)
	, m_nDragIndex(0)
	, m_nDropIndex(0)
	, m_pDropWnd(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTszRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COUNTER_START, m_counter_start);
	DDX_Text(pDX, IDC_EDIT_COUNTER_STEP, m_counter_step);
	DDX_CBIndex(pDX, IDC_COMBO_COUNTER_LEN, m_counter_max_len);
	DDV_MinMaxInt(pDX, m_counter_max_len, 0, 9);
	DDX_CBIndex(pDX, IDC_COMBO_WORD_CASE, m_replace_word_case);
	DDX_CBString(pDX, IDC_COMBO_FILENAME_PAT, m_file_name_pat);
	DDX_Control(pDX, IDC_COMBO_OLD_STRING, m_cmb_str_find);
	DDX_Control(pDX, IDC_COMBO_NEW_STRING, m_cmb_str_replace);
	DDX_Text(pDX, IDC_EDIT_EXT_NAME_PAT, m_ext_name_pat);
	DDX_Control(pDX, IDC_LIST_FILE, m_file_list_ctrl);
	DDX_Control(pDX, IDC_COMBO_FILENAME_PAT, m_cmbFilename);
	DDX_Control(pDX, IDC_EDIT_EXT_NAME_PAT, m_edEXT);
}

void CTszRenameDlg::InitFileListCtrl()
{
	DWORD style = m_file_list_ctrl.GetExtendedStyle();
	style |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES;
	m_file_list_ctrl.SetExtendedStyle(style);
	CRect rect;
	m_file_list_ctrl.GetWindowRect(&rect);
	int w = rect.right - rect.left;
	m_file_list_ctrl.InsertColumn(0, _T("�����ļ���"), LVCFMT_LEFT, int(0.25*w));
	m_file_list_ctrl.InsertColumn(1, _T("��չ��"), LVCFMT_LEFT, int(0.10*w));
	m_file_list_ctrl.InsertColumn(2, _T("���ļ���"), LVCFMT_LEFT, int(0.25*w));
	m_file_list_ctrl.InsertColumn(3, _T("��С"), LVCFMT_LEFT, int(0.10*w));
	m_file_list_ctrl.InsertColumn(4, _T("����"), LVCFMT_LEFT, int(0.20*w));
	m_file_list_ctrl.InsertColumn(5, _T("λ��"), LVCFMT_LEFT, int(0.30*w));
}

void CTszRenameDlg::AddToolTips()
{
	EnableToolTips(TRUE);
	m_tooltip_ctrl.Create(this);
	m_tooltip_ctrl.Activate(TRUE);
	SetToolTips(IDC_BUTTON_UNDO, _T("����������"));
	SetToolTips(IDC_CHECK1, _T("���ִ�Сд"));
	SetToolTips(IDC_CHECK2, _T("ÿ���ļ�ֻ�滻һ��"));
	SetToolTips(IDC_CHECK3, _T("ͬʱ�滻�ļ���չ��"));
	SetToolTips(IDC_COMBO_OLD_STRING, _T("֧��������ʽ��������ʽ��֧�ֻ��ַ�|��ͨ���*��?"));
	SetToolTips(IDC_COMBO_NEW_STRING, _T("֧��������ʽ��������ʽ��֧�ֻ��ַ�|��ͨ���*��?"));
}

void CTszRenameDlg::SetToolTips(UINT uId, CString tips)
{
	CWnd* pW = GetDlgItem(uId);
	m_tooltip_ctrl.AddTool(pW, tips);
}

void CTszRenameDlg::AppendToFileList(CString filePath)
{
	if (m_file_meta_data_map.find(filePath) != m_file_meta_data_map.end())
		return;
	CFileMetaData meta(filePath);
	if (!meta.Status())
		return;
	m_file_meta_data_map[filePath] = meta;

	int n = m_file_list_ctrl.GetItemCount();
	int row = m_file_list_ctrl.InsertItem(n, meta.m_file_name);
	meta.m_sort_idx = n + 1;
	m_file_list_ctrl.SetItemText(row, 1, meta.m_file_ext);
	m_file_list_ctrl.SetItemText(row, 2, _T(""));
	m_file_list_ctrl.SetItemText(row, 3, meta.m_file_size);
	m_file_list_ctrl.SetItemText(row, 4, meta.m_file_time);
	m_file_list_ctrl.SetItemText(row, 5, meta.m_file_dir);
	m_file_list_ctrl.SetCheck(row, TRUE);
}

void CTszRenameDlg::InsertStringToNamePat(CString mark)
{
	int sel = m_n_file_name_pos >> 16;
	if (sel >= 0) {
		m_file_name_pat = m_file_name_pat.Left(sel) + mark + m_file_name_pat.Right(m_file_name_pat.GetLength() - sel);
		m_cmbFilename.GetFocus();
		m_cmbFilename.SetEditSel(sel, sel);
	} else {
		m_file_name_pat += mark;
	}
}

void CTszRenameDlg::InsertStringToEXTPat(CString mark)
{
	CPoint pt = m_edEXT.GetCaretPos();
	int sel = m_edEXT.CharFromPos(pt);
	if (sel >= 0) {
		m_ext_name_pat = m_ext_name_pat.Left(sel) + mark + m_ext_name_pat.Right(m_ext_name_pat.GetLength() - sel);
		m_edEXT.GetFocus();
		CPoint pt = m_edEXT.PosFromChar(sel);
		m_edEXT.SetCaretPos(pt);
	} else {
		m_ext_name_pat += mark;
	}
}

BEGIN_MESSAGE_MAP(CTszRenameDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CTszRenameDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON10, &CTszRenameDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CTszRenameDlg::OnBnClickedButton_SelectFold)
	ON_BN_CLICKED(IDC_BUTTON12, &CTszRenameDlg::OnBnClickedButton_CleanFiles)
	ON_BN_CLICKED(IDC_BUTTON13, &CTszRenameDlg::OnBnClickedButton_RemoveFiles)
	ON_BN_CLICKED(IDC_BUTTON14, &CTszRenameDlg::OnBnClickedButton_Rename_UNDO)
	ON_BN_CLICKED(IDC_BUTTON2, &CTszRenameDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CTszRenameDlg::OnBnClickedButton_Name_Range)
	ON_BN_CLICKED(IDC_BUTTON4, &CTszRenameDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CTszRenameDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CTszRenameDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CTszRenameDlg::OnBnClickedButton_RenameNow)
	ON_BN_CLICKED(IDC_BUTTON8, &CTszRenameDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CTszRenameDlg::OnBnClickedButton_SelectFiles)
	ON_BN_CLICKED(IDC_CHECK1, &CTszRenameDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CTszRenameDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CTszRenameDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CTszRenameDlg::OnBnClickedCheck4)
	ON_CBN_EDITCHANGE(IDC_COMBO_FILENAME_PAT, &CTszRenameDlg::OnCbnEditchangeComboFilenamePat)
	ON_CBN_EDITCHANGE(IDC_COMBO_NEW_STRING, &CTszRenameDlg::OnCbnEditchangeComboNewString)
	ON_CBN_EDITCHANGE(IDC_COMBO_OLD_STRING, &CTszRenameDlg::OnCbnEditchangeComboOldString)
	ON_CBN_SELCHANGE(IDC_COMBO_COUNTER_LEN, &CTszRenameDlg::OnCbnSelchangeComboCounterLen)
	ON_CBN_SELCHANGE(IDC_COMBO_NEW_STRING, &CTszRenameDlg::OnCbnSelchangeComboNewString)
	ON_CBN_SELCHANGE(IDC_COMBO_OLD_STRING, &CTszRenameDlg::OnCbnSelchangeComboOldString)
	ON_CBN_SELCHANGE(IDC_COMBO_WORD_CASE, &CTszRenameDlg::OnCbnSelchangeComboWordCase)
	ON_EN_CHANGE(IDC_EDIT_COUNTER_START, &CTszRenameDlg::OnEnChangeEditCounterStart)
	ON_EN_CHANGE(IDC_EDIT_COUNTER_STEP, &CTszRenameDlg::OnEnChangeEditCounterStep)
	ON_EN_CHANGE(IDC_EDIT_EXT_NAME_PAT, &CTszRenameDlg::OnEnChangeEditExtNamePat)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_FILE, &CTszRenameDlg::OnBeginDrag)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_FILE, &CTszRenameDlg::OnLvnColumnclickListFile)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILE, &CTszRenameDlg::OnLvnItemchangedListFile)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CTszRenameDlg::OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &CTszRenameDlg::OnDeltaposSpin2)
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

// CTszRenameDlg ��Ϣ�������

BOOL CTszRenameDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	DragAcceptFiles(TRUE);

	((CButton*)GetDlgItem(IDC_CHECK3))->SetCheck(TRUE);
	InitFileListCtrl();
	AddToolTips();

	/////////////////////////////////////////////
//// Set up initial variables
	m_bDragging = false;
	m_nDragIndex = -1;
	m_nDropIndex = -1;
	m_pDragImage = NULL;

	//
	for (size_t i = 0; i < m_input_file_list.size(); ++i) {
		AppendToFileList(m_input_file_list[i]);
	}
	rename_file_list();
	//restore window position
	int left = 0, bottom = 0, right = 0, top = 0;
	BOOL isZoomed = FALSE, isIconic = FALSE;
	LoadSetting(_T("CTszRenameDlg/wndLeft"), left);
	LoadSetting(_T("CTszRenameDlg/wndBottom"), bottom);
	LoadSetting(_T("CTszRenameDlg/wndRight"), right);
	LoadSetting(_T("CTszRenameDlg/wndTop"), top);
	LoadSetting(_T("CTszRenameDlg/IsZoomed"), isZoomed);
	LoadSetting(_T("CTszRenameDlg/IsIconic"), isIconic);
	if (!isIconic && !isZoomed) {
		if (abs(right - left) > 0 && abs(top - bottom) > 0) {
			SetWindowPos(NULL, left, top, abs(right - left), abs(bottom - top), 0);
		}
	} else if (isZoomed) {
		ShowWindow(SW_SHOWMAXIMIZED);
	}

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�
void CTszRenameDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}

void CTszRenameDlg::OnDestroy()
{
	CRect rect;
	GetWindowRect(&rect);
	SaveSetting(_T("CTszRenameDlg/wndLeft"), rect.left);
	SaveSetting(_T("CTszRenameDlg/wndBottom"), rect.bottom);
	SaveSetting(_T("CTszRenameDlg/wndRight"), rect.right);
	SaveSetting(_T("CTszRenameDlg/wndTop"), rect.top);
	SaveSetting(_T("CTszRenameDlg/IsZoomed"), IsZoomed());
	SaveSetting(_T("CTszRenameDlg/IsIconic"), IsIconic());
	CDialogEx::OnDestroy();
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTszRenameDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int  comare_method = 0;
bool compare_reverse = false;
bool PathLess(CString p1, CString p2)
{
	p1 = p1.MakeLower();
	p2 = p2.MakeLower();
	return StrCmpLogicalW(p1, p2) < 0;
}

bool FileMetaLessFunc(const CFileMetaData & meta1, const CFileMetaData & meta2)
{
	if (meta1.m_checked && !meta2.m_checked)
		return true;
	if (!meta1.m_checked && meta2.m_checked)
		return false;
	if (!meta1.m_checked && !meta2.m_checked) {
		return PathLess(meta1.m_file_name, meta2.m_file_name);
	}
	bool b = false;
	if (comare_method == 0) {
		int bn = meta1.m_file_name.CompareNoCase(meta2.m_file_name);
		if (bn != 0) {
			b = bn < 0;
		} else {
			return false;
		}
		b = PathLess(meta1.m_file_name, meta2.m_file_name);
	} else if (comare_method == 1) {
		int bn = meta1.m_file_ext.CompareNoCase(meta2.m_file_ext);
		if (bn != 0) {
			b = bn < 0;
		} else {
			return false;
		}
		b = PathLess(meta1.m_file_ext, meta2.m_file_ext);
	} else if (comare_method == 3) {
		int n1 = ToInt(ToString(meta1.m_file_size));
		int n2 = ToInt(ToString(meta2.m_file_size));
		int bn = n1 - n2;
		if (bn != 0) {
			b = bn < 0;
		} else {
			return false;
		}
	} else if (comare_method == 4) {
		int bn = meta1.m_file_time.CompareNoCase(meta2.m_file_time);
		if (bn != 0) {
			b = bn < 0;
		} else {
			return false;
		}
	} else if (comare_method == 5) {
		int bn = meta1.m_file_dir.CompareNoCase(meta2.m_file_dir);
		if (bn != 0) {
			b = bn < 0;
		} else {
			return false;
		}
		b = PathLess(meta1.m_file_dir, meta2.m_file_dir);
	}

	return compare_reverse ? !b : b;
}

bool FileMetaLessFunc2(const CFileMetaData & meta1, const CFileMetaData & meta2)
{
	return meta1.m_sort_idx < meta2.m_sort_idx;
}

void CTszRenameDlg::set_file_list(vector<CString> input_file_list)
{
	m_input_file_list.clear();
	for (size_t i = 0; i < input_file_list.size(); ++i) {
		CString filePath = input_file_list[i];
		if (filePath.IsEmpty())
			continue;
		if (filePath[0] == _T('"')) {
			filePath = filePath.Right(filePath.GetLength() - 1);
		}
		if (filePath.IsEmpty())
			continue;
		if (filePath[filePath.GetLength() - 1] == _T('"')) {
			filePath = filePath.Left(filePath.GetLength() - 1);
		}
		if (filePath.IsEmpty())
			continue;
		m_input_file_list.push_back(filePath);
	}
}

void CTszRenameDlg::sort_file_list(bool reSort)
{
	int n = m_file_list_ctrl.GetItemCount();
	if (n == 0)
		return;
	for (int i = 0; i < n; ++i) {
		CString name = m_file_list_ctrl.GetItemText(i, 0);
		CString ext = m_file_list_ctrl.GetItemText(i, 1);
		CString path = m_file_list_ctrl.GetItemText(i, 5);
		CString fullPath = JoinPath(path, name + _T(".") + ext);
		bool b = m_file_list_ctrl.GetCheck(i);
		auto itr = m_file_meta_data_map.find(fullPath);
		if (itr != m_file_meta_data_map.end()) {
			itr->second.m_checked = b;
		}
	}
	if (reSort) {
		vector<CFileMetaData> file_list;
		auto itr = m_file_meta_data_map.begin();
		for (; itr != m_file_meta_data_map.end(); ++itr) {
			file_list.push_back(itr->second);
		}
		std::sort(file_list.begin(), file_list.end(), FileMetaLessFunc);
		for (size_t i = 0; i < file_list.size(); ++i) {
			m_file_meta_data_map[file_list[i].m_file_path].m_sort_idx = i + 1;
		}
	}
}

void CTszRenameDlg::auto_rename(bool reSort)
{
	m_file_idx_to_name_map.clear();
	vector<CString> name_pats = split_name_pattern(m_file_name_pat);
	vector<CString> ext_pats = split_name_pattern(m_ext_name_pat);
	if (reSort) {
		auto itr = m_file_meta_data_map.begin();
		int  count = 0;
		for (; itr != m_file_meta_data_map.end(); ++itr) {
			if (itr->second.m_checked)
				++count;
			CFileMetaData & meta = itr->second;
			CString name = replace_str_by_pat(meta, name_pats, false, count);
			CString ext = replace_str_by_pat(meta, ext_pats, true, count);
			meta.m_file_new = name + _T(".") + ext;
			int idx = meta.m_sort_idx - 1;
			m_file_list_ctrl.SetItemText(idx, 0, meta.m_file_name);
			m_file_list_ctrl.SetItemText(idx, 1, meta.m_file_ext);
			m_file_list_ctrl.SetItemText(idx, 2, meta.m_file_new);
			m_file_list_ctrl.SetItemText(idx, 3, meta.m_file_size);
			m_file_list_ctrl.SetItemText(idx, 4, meta.m_file_time);
			m_file_list_ctrl.SetItemText(idx, 5, meta.m_file_dir);
			m_file_list_ctrl.SetCheck(idx, meta.m_checked);
			m_file_idx_to_name_map[idx] = itr->first;
		}
	} else {
		int n = m_file_list_ctrl.GetItemCount();
		if (n == 0)
			return;
		int  count = 0;
		for (int idx = 0; idx < n; ++idx) {
			CString name = m_file_list_ctrl.GetItemText(idx, 0);
			CString ext = m_file_list_ctrl.GetItemText(idx, 1);
			CString path = m_file_list_ctrl.GetItemText(idx, 5);
			CString fullPath = JoinPath(path, name + _T(".") + ext);
			bool b = m_file_list_ctrl.GetCheck(idx);
			auto itr = m_file_meta_data_map.find(fullPath);
			if (itr != m_file_meta_data_map.end()) {
				if (itr->second.m_checked)
					++count;
				CFileMetaData & meta = itr->second;
				CString name = replace_str_by_pat(meta, name_pats, false, count);
				CString ext = replace_str_by_pat(meta, ext_pats, true, count);
				meta.m_file_new = name + _T(".") + ext;
				m_file_list_ctrl.SetItemText(idx, 0, meta.m_file_name);
				m_file_list_ctrl.SetItemText(idx, 1, meta.m_file_ext);
				m_file_list_ctrl.SetItemText(idx, 2, meta.m_file_new);
				m_file_list_ctrl.SetItemText(idx, 3, meta.m_file_size);
				m_file_list_ctrl.SetItemText(idx, 4, meta.m_file_time);
				m_file_list_ctrl.SetItemText(idx, 5, meta.m_file_dir);
				m_file_list_ctrl.SetCheck(idx, meta.m_checked);
				m_file_idx_to_name_map[idx] = itr->first;
			}
		}
	}
	replace_str_by_pat();
}

void CTszRenameDlg::rename_file_list(bool reSort)
{
	if (!m_renaming) {
		m_renaming = true;
		sort_file_list(reSort);
		auto_rename(reSort);
		m_renaming = false;
	}
}

CString formatNumber(int no, int length)
{
	CString ans;
	ans.Format(_T("%d"), no);
	const int len = ans.GetLength();
	for (int i = 0; i < length - len; ++i) {
		ans = _T("0") + ans;
	}
	return ans;
}

CString CTszRenameDlg::replace_str_by_pat(const CFileMetaData & meta, vector<CString> pats, bool ext, int idx)
{
	CTime now = CTime::GetCurrentTime();
	CString ymd = YYMMDD(now);
	CString hms = hhmmss(now);

	CString tmp_str = ext ? meta.m_file_ext : meta.m_file_name;
	if (!meta.m_checked)
		return tmp_str;
	CString ans = _T("");
	for (size_t i = 0; i < pats.size(); ++i) {
		CString pat = pats[i];
		if (pat == _T("<N>") || pat == _T("<E>")) {
			ans += tmp_str;
		} else if (pat == _T("<C>")) {
			//int idx = meta.m_sort_idx;
			idx = m_counter_start + (idx - 1) * m_counter_step;
			//ans.Format(_T("%s%d"), ans, idx);
			ans += formatNumber(idx, m_counter_max_len + 1);
		} else if (pat == _T("<YMD>")) {
			ans += ymd;
		} else if (pat == _T("<hms>")) {
			ans += hms;
		} else if (pat[0] != '<') {
			ans += pat;
		} else if (pat.Find(_T("<N")) == 0 || pat.Find(_T("<E")) == 0) {
			pat = pat.Right(pat.GetLength() - 2);
			if (pat.Find(_T(">")) > 0) {
				pat = pat.Left(pat.GetLength() - 1);
				int nS, nE, len;
				if (!pat.IsEmpty()) {
					nS = _tstoi(pat.GetBuffer());
					pat.ReleaseBuffer();
					if (nS < 0) {
						nS = tmp_str.GetLength() + nS + 1;
					}
					if (pat[0] == _T('-')) {
						pat = pat.Right(pat.GetLength() - 1);
					}
					int n = pat.Find(_T(","));
					if (n > 0) {
						pat = pat.Right(pat.GetLength() - n - 1);
						len = _tstoi(pat.GetBuffer());
						pat.ReleaseBuffer();
					} else {
						n = pat.Find(_T("-"));
						if (n > 0) {
							pat = pat.Right(pat.GetLength() - n - 1);
							nE = _tstoi(pat.GetBuffer());
							pat.ReleaseBuffer();
							if (nE < 0) {
								nE = tmp_str.GetLength() + nE + 1;
							}
							len = nE - nS + 1;
						} else {
							len = 1;
						}
					}
				}
				if (len < 0)
					len = 0;
				if (len) {
					ans += tmp_str.Mid(nS - 1, len);
				}
			}
		}
	}
	if (ans.IsEmpty()) {
		return ans;
	}
	if (ext && m_replace_word_case > 2) {
	} else {
		ans = change_word_case(ans, m_replace_word_case);
	}
	return ans;
}

std::vector<CString> CTszRenameDlg::split_name_pattern(CString name_pat)
{
	std::vector<CString> ans;
	CString pat;
	for (int i = 0; i < name_pat.GetLength(); ++i) {
		TCHAR c = name_pat[i];
		if (c == L'<') {
			if (!pat.IsEmpty()) {
				ans.push_back(pat);
				pat = _T("");
			}
			pat += c;
			while (i < name_pat.GetLength()) {
				++i;
				c = name_pat[i];
				if (c == L'>') {
					pat += c;
					ans.push_back(pat);
					pat = _T("");
					break;
				} else {
					pat += c;
				}
			}
		} else {
			pat += c;
		}
	}
	if (!pat.IsEmpty()) {
		ans.push_back(pat);
		pat = _T("");
	}
	return ans;
}

void CTszRenameDlg::DropItemOnList(CListCtrl* pDragList, CListCtrl* pDropList)
{
	//This routine performs the actual drop of the item dragged.
	//It simply grabs the info from the Drag list (pDragList)
	// and puts that info into the list dropped on (pDropList).
	//Send:	pDragList = pointer to CListCtrl we dragged from,
	//		pDropList = pointer to CListCtrl we are dropping on.
	//Return: nothing.

	////Variables
	ASSERT(m_nDragIndex != -1);

	// Unhilight the drop target
	pDropList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED);

	//Set up the LV_ITEM for retrieving item from pDragList and adding the new item to the pDropList
	TCHAR szLabel[256];
	LVITEM lviT;
	LVITEM* plvitem;

	ZeroMemory(&lviT, sizeof(LVITEM)); //allocate and clear memory space for LV_ITEM
	lviT.iItem = m_nDragIndex;
	lviT.mask = LVIF_TEXT;
	lviT.pszText = szLabel;
	lviT.cchTextMax = 255;

	lvItem* pItem;
	lvItem lvi;
	lvi.plvi = &lviT;
	lvi.plvi->iItem = m_nDragIndex;
	lvi.plvi->mask = LVIF_TEXT;
	lvi.plvi->pszText = szLabel;
	lvi.plvi->cchTextMax = 255;

	if (pDragList->GetSelectedCount() == 1) {
		// Get item that was dragged
		pDragList->GetItem(lvi.plvi);
		lvi.bCheck = pDragList->GetCheck(lvi.plvi->iItem);
		lvi.sCol1 = pDragList->GetItemText(lvi.plvi->iItem, 1);
		lvi.sCol2 = pDragList->GetItemText(lvi.plvi->iItem, 2);
		lvi.sCol3 = pDragList->GetItemText(lvi.plvi->iItem, 3);
		lvi.sCol4 = pDragList->GetItemText(lvi.plvi->iItem, 4);
		lvi.sCol5 = pDragList->GetItemText(lvi.plvi->iItem, 5);

		// Delete the original item (for Move operation)
		// This is optional. If you want to implement a Copy operation, don't delete.
		// This works very well though for re-arranging items within a CListCtrl.
		// It is written at present such that when dragging from one list to the other
		//  the item is copied, but if dragging within one list, the item is moved.
		if (pDragList == pDropList) {
			pDragList->DeleteItem(m_nDragIndex);
			if (m_nDragIndex < m_nDropIndex) m_nDropIndex--; //decrement drop index to account for item
															 //being deleted above it
		}

		// Insert item into pDropList
		// if m_nDropIndex == -1, iItem = GetItemCount() (inserts at end of list), else iItem = m_nDropIndex
		lvi.plvi->iItem = (m_nDropIndex == -1) ? pDropList->GetItemCount() : m_nDropIndex;
		pDropList->InsertItem(lvi.plvi);
		pDropList->SetCheck(lvi.plvi->iItem, lvi.bCheck);
		pDropList->SetItemText(lvi.plvi->iItem, 1, (LPCTSTR)lvi.sCol1);
		pDropList->SetItemText(lvi.plvi->iItem, 2, (LPCTSTR)lvi.sCol2);
		pDropList->SetItemText(lvi.plvi->iItem, 3, (LPCTSTR)lvi.sCol3);
		pDropList->SetItemText(lvi.plvi->iItem, 4, (LPCTSTR)lvi.sCol4);
		pDropList->SetItemText(lvi.plvi->iItem, 5, (LPCTSTR)lvi.sCol5);

		// Select the new item we just inserted
		pDropList->SetItemState(lvi.plvi->iItem, LVIS_SELECTED, LVIS_SELECTED);
	} else //more than 1 item is being dropped
	{
		//We have to parse through all of the selected items from the DragList
		//1) Retrieve the info for the items and store them in memory
		//2) If we are reordering, delete the items from the list
		//3) Insert the items into the list (either same list or different list)

		CList<lvItem*, lvItem*> listItems;
		POSITION listPos;

		//Retrieve the selected items
		POSITION pos = pDragList->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
		while (pos) //so long as we have a valid POSITION, we keep iterating
		{
			plvitem = new LVITEM;
			ZeroMemory(plvitem, sizeof(LVITEM));
			pItem = new lvItem;
			//ZeroMemory(pItem, sizeof(lvItem)); //If you use ZeroMemory on the lvItem struct, it creates an error when you try to set sCol2
			pItem->plvi = plvitem;
			pItem->plvi->iItem = m_nDragIndex;
			pItem->plvi->mask = LVIF_TEXT;
			pItem->plvi->pszText = new TCHAR[MAX_PATH]; //since this is a pointer to the string, we need a new pointer to a new string on the heap
			pItem->plvi->cchTextMax = 255;

			m_nDragIndex = pDragList->GetNextSelectedItem(pos);

			//Get the item
			pItem->plvi->iItem = m_nDragIndex; //set the index in the drag list to the selected item
			pDragList->GetItem(pItem->plvi); //retrieve the information
			pItem->bCheck = pDragList->GetCheck(pItem->plvi->iItem);
			pItem->sCol1 = pDragList->GetItemText(pItem->plvi->iItem, 1);
			pItem->sCol2 = pDragList->GetItemText(pItem->plvi->iItem, 2);
			pItem->sCol3 = pDragList->GetItemText(pItem->plvi->iItem, 3);
			pItem->sCol4 = pDragList->GetItemText(pItem->plvi->iItem, 4);
			pItem->sCol5 = pDragList->GetItemText(pItem->plvi->iItem, 5);

			//Save the pointer to the new item in our CList
			listItems.AddTail(pItem);
		} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

		if (pDragList == pDropList) //we are reordering the list (moving)
		{
			//Delete the selected items
			pos = pDragList->GetFirstSelectedItemPosition();
			while (pos) {
				pos = pDragList->GetFirstSelectedItemPosition();
				m_nDragIndex = pDragList->GetNextSelectedItem(pos);

				pDragList->DeleteItem(m_nDragIndex); //since we are MOVING, delete the item
				if (m_nDragIndex < m_nDropIndex) m_nDropIndex--; //must decrement the drop index to account
																 //for the deleted items
			} //EO while(pos)
		} //EO if(pDragList...

		  //Iterate through the items stored in memory and add them back into the CListCtrl at the drop index
		listPos = listItems.GetHeadPosition();
		while (listPos) {
			pItem = listItems.GetNext(listPos);

			m_nDropIndex = (m_nDropIndex == -1) ? pDropList->GetItemCount() : m_nDropIndex;
			pItem->plvi->iItem = m_nDropIndex;
			pDropList->InsertItem(pItem->plvi); //add the item
			pDropList->SetCheck(pItem->plvi->iItem);
			pDropList->SetItemText(pItem->plvi->iItem, 1, pItem->sCol1);
			pDropList->SetItemText(pItem->plvi->iItem, 2, pItem->sCol2);
			pDropList->SetItemText(pItem->plvi->iItem, 3, pItem->sCol3);
			pDropList->SetItemText(pItem->plvi->iItem, 4, pItem->sCol4);
			pDropList->SetItemText(pItem->plvi->iItem, 5, pItem->sCol5);

			pDropList->SetItemState(pItem->plvi->iItem, LVIS_SELECTED, LVIS_SELECTED); //highlight/select the item we just added

			m_nDropIndex++; //increment the index we are dropping at to keep the dropped items in the same order they were in in the Drag List
							//If we dont' increment this, the items are added in reverse order
		} //EO while(listPos)
		//free
		listPos = listItems.GetHeadPosition();
		while (listPos) {
			pItem = listItems.GetNext(listPos);

			delete[] pItem->plvi->pszText;
			delete pItem->plvi;
			delete pItem;
		} //EO while(listPos)
	}
}

void CTszRenameDlg::set_word_case()
{
	if (m_replace_word_case == 0)
		return;
}

bool StringSearch(CString src, CString pat, int & begin, int & end)
{
	int p = pat.Find(_T("**"));
	while (p >= 0) {
		pat.Delete(p, 1);
		p = pat.Find(_T("**"));
	}
	p = pat.Find(_T("*"));
	if (p == 0) {
		pat = pat.Right(pat.GetLength() - 1);
	}
	if (begin < 0)
		begin = 0;
	if (end < 0)
		end = 0;
	int sidx = begin;
	while (sidx < src.GetLength()) {
		bool b = StringMatch(src, pat, sidx, end);
		if (b) {
			begin = sidx;
			return true;
		}
		++sidx;
	}
	return false;
}

static bool g_cmp_ignore_case = true;

bool StringMatch(CString src, CString pat, int begin, int & end)
{
	int p = pat.Find(_T("**"));
	while (p >= 0) {
		pat.Delete(p, 1);
		p = pat.Find(_T("**"));
	}
	if (begin < 0)
		begin = 0;
	if (end < 0)
		end = 0;
	int sidx = begin, pidx = 0;
	while (sidx < src.GetLength() && pidx < pat.GetLength()) {
		if (_T('*') == pat[pidx]) {
			CString patNext = pat.Right(pat.GetLength() - pidx - 1);
			bool b = StringSearch(src, patNext, sidx, end);
			return b;
		}
		bool b = g_cmp_ignore_case ? (_tolower(src[sidx]) == _tolower(pat[pidx])) : (src[sidx] == pat[pidx]);
		if (_T('?') == pat[pidx] || b) {
			++sidx;
			++pidx;
			continue;
		}
		return false;
	}
	if (pidx >= pat.GetLength()) {
		end = sidx;
		return true;
	}
	return false;
}

void CTszRenameDlg::replace_str_by_pat()
{
	BOOL chk1 = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	BOOL chk2 = ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck();
	BOOL chk3 = ((CButton*)GetDlgItem(IDC_CHECK3))->GetCheck();
	BOOL chk4 = ((CButton*)GetDlgItem(IDC_CHECK4))->GetCheck();
	CString findStr, replaceStr;
	GetDlgItemText(IDC_COMBO_OLD_STRING, findStr);
	GetDlgItemText(IDC_COMBO_NEW_STRING, replaceStr);
	if (findStr.IsEmpty())
		return;

	g_cmp_ignore_case = chk1 == FALSE;

	if (!chk4) {	//������
		vector<CString> findGroups;
		vector<CString> replaceGroups;
		CString item = _T("");
		for (int i = 0; i < findStr.GetLength(); ++i) {
			TCHAR c = findStr[i];
			if (c == _T('|')) {
				if (!item.IsEmpty()) {
					findGroups.push_back(item);
					item = _T("");
				}
			} else {
				item += c;
			}
		}
		if (!item.IsEmpty()) {
			findGroups.push_back(item);
			item = _T("");
		}
		for (int i = 0; i < replaceStr.GetLength(); ++i) {
			TCHAR c = replaceStr[i];
			if (c == _T('|')) {
				if (!item.IsEmpty()) {
					replaceGroups.push_back(item);
					item = _T("");
				}
			} else {
				item += c;
			}
		}
		if (!item.IsEmpty()) {
			replaceGroups.push_back(item);
			item = _T("");
		}
		for (size_t i = 0; i < findGroups.size(); ++i) {
			CString pat = findGroups[i];
			CString replace = _T("");
			if (replaceGroups.size() == 1) {
				replace = replaceGroups[0];
			} else if (i < replaceGroups.size()) {
				replace = replaceGroups[i];
			}
			int n = m_file_list_ctrl.GetItemCount();
			for (int j = 0; j < n; ++j) {
				CString oldName = m_file_idx_to_name_map[j];
				CString str = m_file_meta_data_map[oldName].m_file_new;
				CString ext = _T("");
				//�ļ���׺���滻
				if (!chk3) {
					int n = str.ReverseFind(_T('.'));
					if (n >= 0) {
						ext = str.Right(str.GetLength() - n);
						str = str.Left(n);
					}
				}
				CString tmp = _T("");
				int begin = 0;
				int end = 0;
				int last = 0;
				bool b = StringSearch(str, pat, begin, end);
				while (b) {
					tmp += str.Mid(last, begin - last);
					tmp += (replace == _T("<���>") || replace == _T("")) ? _T("") : replace;
					if (chk2)
						break;
					begin = end;
					last = end;
					b = StringSearch(str, pat, begin, end);
				}
				tmp += str.Right(str.GetLength() - end) + ext;
				m_file_meta_data_map[oldName].m_file_new = tmp;
				m_file_list_ctrl.SetItemText(j, 2, tmp);
			}
		}
	} else {
		//������ʽ
		auto p = ToString(findStr);
		auto r = ToString(replaceStr);
		if (r == "<���>")
			r = "";
		try {
			regex rgx(p, regex::icase);
			if (chk1) {
				rgx.assign(p);
			}
			int n = m_file_list_ctrl.GetItemCount();
			for (int j = 0; j < n; ++j) {
				CString oldName = m_file_idx_to_name_map[j];
				CString str = m_file_meta_data_map[oldName].m_file_new;
				CString ext = _T("");
				//�ļ���׺���滻
				if (!chk3) {
					int n = str.ReverseFind(_T('.'));
					if (n >= 0) {
						ext = str.Right(str.GetLength() - n);
						str = str.Left(n);
					}
				}
				auto ss = ToString(str);
				if (chk2) {
					ss = regex_replace(ss, rgx, r, regex_constants::format_first_only);
				} else {
					ss = regex_replace(ss, rgx, r);
				}
				str = ToCString(ss) + ext;
				m_file_meta_data_map[oldName].m_file_new = str;
				m_file_list_ctrl.SetItemText(j, 2, str);
			}
		} catch (...) {
		}
	}
}

void CTszRenameDlg::OnBnClickedButton_SelectFiles()
{
	UpdateDataWnd upd(this);
	CFileDialog fdlg(TRUE, _T("*.*"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
		_T("�����ļ�(*.*)|*.*|PDF�ļ�(*.pdf)|*.pdf|ͼֽ(*.dwg;*.dwf)|*.dwg;*.dwf||"));
	const DWORD numberOfFileNames = 100;
	const DWORD fileNameMaxLength = MAX_PATH + 1;
	const DWORD bufferSize = numberOfFileNames * fileNameMaxLength + 1;
	TCHAR * buffer = new TCHAR[bufferSize];
	buffer[0] = NULL;
	buffer[bufferSize - 1] = NULL;
	fdlg.m_ofn.lpstrFile = buffer;
	fdlg.m_ofn.nMaxFile = bufferSize;

	if (fdlg.DoModal() != IDOK) {
		delete[] buffer;
		return;
	}

	POSITION p = fdlg.GetStartPosition();
	while (p) {
		CString str = fdlg.GetNextPathName(p);
		AppendToFileList(str);
	}
	delete[] buffer;

	rename_file_list();
}

void CTszRenameDlg::OnBnClickedButton_SelectFold()
{
	static bool selecting = false;
	if (!selecting) {
		selecting = true;
		vector<CString> folders = select_path_dlg(GetSafeHwnd());
		for (size_t i = 0; i < folders.size(); ++i) {
			CString path = folders[i];
			if (!path.IsEmpty()) {
				vector<CString> files = get_files_in_path(path);
				for (size_t j = 0; j < files.size(); ++j) {
					AppendToFileList(files[j]);
				}
			}
		}
		selecting = false;
	}
}

void CTszRenameDlg::OnBnClickedButton_CleanFiles()
{
	m_file_list_ctrl.DeleteAllItems();
	m_file_meta_data_map.clear();
}

void CTszRenameDlg::OnBnClickedButton_RemoveFiles()
{
	set<int> selected_items;
	POSITION pos = m_file_list_ctrl.GetFirstSelectedItemPosition();
	while (pos) {
		int n = m_file_list_ctrl.GetNextSelectedItem(pos);
		if (n >= 0) {
			selected_items.insert(n);
		}
	}
	if (selected_items.empty())
		return;
	auto itr = selected_items.rbegin();
	for (; itr != selected_items.rend(); ++itr) {
		int idx = *itr;
		CString name = m_file_list_ctrl.GetItemText(idx, 0);
		CString ext = m_file_list_ctrl.GetItemText(idx, 1);
		CString dir = m_file_list_ctrl.GetItemText(idx, 5);
		CString path = JoinPath(dir, name + _T(".") + ext);
		m_file_meta_data_map.erase(path);
		m_file_idx_to_name_map.erase(idx);
		m_file_list_ctrl.DeleteItem(idx);
	}
	rename_file_list();
	int knt = m_file_list_ctrl.GetItemCount();
	if (knt == 0)
		return;
	if (*selected_items.begin() < knt)
		m_file_list_ctrl.SetItemState(*selected_items.begin(), LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
	else
		m_file_list_ctrl.SetItemState(knt - 1, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
	m_file_list_ctrl.SetFocus();
}

void CTszRenameDlg::OnBnClickedButton1()
{
	UpdateDataWnd upd(this);
	InsertStringToNamePat(_T("<N>"));
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedButton2()
{
	UpdateDataWnd upd(this);
	InsertStringToNamePat(_T("<YMD>"));
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedButton4()
{
	UpdateDataWnd upd(this);
	InsertStringToNamePat(_T("<hms>"));
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedButton5()
{
	UpdateDataWnd upd(this);
	InsertStringToNamePat(_T("<C>"));
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedButton6()
{
	UpdateDataWnd upd(this);
	InsertStringToEXTPat(_T("<E>"));
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedButton10()
{
	UpdateDataWnd upd(this);
	InsertStringToEXTPat(_T("<C>"));
	rename_file_list();
}

void CTszRenameDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateDataWnd upd(this);
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	if (pNMUpDown->iDelta < 0)
		m_counter_start += 1;
	else
		m_counter_start -= 1;
	rename_file_list();
}

void CTszRenameDlg::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateDataWnd upd(this);
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	if (pNMUpDown->iDelta < 0)
		m_counter_step += 1;
	else
		m_counter_step -= 1;
	rename_file_list();
}

void CTszRenameDlg::OnCbnEditchangeComboFilenamePat()
{
	UpdateData(TRUE);
	rename_file_list();
}

void CTszRenameDlg::OnCbnSelchangeComboWordCase()
{
	UpdateDataWnd upd(this);
	rename_file_list();
}

void CTszRenameDlg::OnEnChangeEditExtNamePat()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
	UpdateData(TRUE);
	rename_file_list();
}

void CTszRenameDlg::OnLvnColumnclickListFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	int sort_column = pNMLV->iSubItem;
	comare_method = sort_column;
	if (sort_column == 2)
		return;
	TCHAR buffer[256];
	LVCOLUMN  pColumn;
	pColumn.mask = LVCF_TEXT;
	pColumn.pszText = buffer;
	pColumn.cchTextMax = 256;
	memset(buffer, 0, sizeof(TCHAR) * 256);
	m_file_list_ctrl.GetColumn(sort_column, &pColumn);
	CString str = pColumn.pszText;
	if (str.Find(_T("��")) >= 0) {
		str.SetAt(0, _T('��'));
		m_sort_inc = false;
		compare_reverse = true;
	} else if (str.Find(_T("��")) >= 0) {
		str.SetAt(0, _T('��'));
		m_sort_inc = true;
		compare_reverse = false;
	} else {
		str.Insert(0, _T('��'));
		m_sort_inc = true;
		compare_reverse = false;
	}
	pColumn.pszText = str.GetBuffer();
	str.ReleaseBuffer();
	m_file_list_ctrl.SetColumn(sort_column, &pColumn);
	for (int i = 0; i < 6; ++i) {
		if (sort_column == i)
			continue;
		LVCOLUMN  pColumn;
		pColumn.mask = LVCF_TEXT;
		pColumn.pszText = buffer;
		pColumn.cchTextMax = 256;
		memset(buffer, 0, sizeof(TCHAR) * 256);
		m_file_list_ctrl.GetColumn(i, &pColumn);
		CString str = pColumn.pszText;
		str.Remove(_T('��'));
		str.Remove(_T('��'));
		pColumn.pszText = str.GetBuffer();
		str.ReleaseBuffer();
		m_file_list_ctrl.SetColumn(i, &pColumn);
	}
	rename_file_list();
}

void CTszRenameDlg::OnLvnItemchangedListFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
	if (pNMLV->uNewState == 0x1000 && pNMLV->uOldState == 0x2000 || pNMLV->uNewState == 0x2000 && pNMLV->uOldState == 0x1000) {
		if (!m_renaming) {
			rename_file_list(false);
		}
	}
}

void CTszRenameDlg::OnBnClickedButton_RenameNow()
{
	//�ȼ����û��������
	set<CString> tmp_name_set;
	auto itr = m_file_meta_data_map.begin();
	for (; itr != m_file_meta_data_map.end(); ++itr) {
		const CFileMetaData & meta = itr->second;
		if (meta.m_checked) {
			if (tmp_name_set.find(meta.m_file_new) != tmp_name_set.end()) {
				break;
			}
			tmp_name_set.insert(meta.m_file_new);
		}
	}
	if (itr != m_file_meta_data_map.end()) {
		MessageBox(_T("�����������ļ����Ƿ����ִ�У�"), _T("����������"));
	}

	m_new_name_map.clear();
	itr = m_file_meta_data_map.begin();
	vector<CFileMetaData> file_list;
	for (; itr != m_file_meta_data_map.end(); ++itr) {
		const CFileMetaData & meta = itr->second;
		if (meta.m_checked) {
			CString str = meta.m_file_dir + meta.m_file_new;
			if (str != meta.m_file_path) {
				file_list.push_back(meta);
				m_new_name_map[str] = meta.m_file_path;
			}
		}
	}
	sort(file_list.begin(), file_list.end(), FileMetaLessFunc2);
	for (size_t i = 0; i < file_list.size(); ++i) {
		const CFileMetaData & meta = file_list[i];
		CString str = meta.m_file_dir + meta.m_file_new + _T(".tmp");
		MoveFile(meta.m_file_path, str);
	}
	int count = 0;
	for (size_t i = 0; i < file_list.size(); ++i) {
		const CFileMetaData & meta = file_list[i];
		CString str1 = meta.m_file_dir + meta.m_file_new + _T(".tmp");
		CString str2 = meta.m_file_dir + meta.m_file_new;
		BOOL b = MoveFile(str1, str2);
		count = b ? count + 1 : count;
	}
	if (count > 0) {
		GetDlgItem(IDC_BUTTON_UNDO)->EnableWindow(TRUE);
		CString str;
		str.Format(_T("��������������ɣ��ɹ� %d ����"), count);
		MessageBox(str, _T("����������"));
	}
}

void CTszRenameDlg::OnBnClickedButton_Rename_UNDO()
{
	map<CString, CString>::iterator itr = m_new_name_map.begin();
	for (; itr != m_new_name_map.end(); ++itr) {
		MoveFile(itr->first, itr->first + _T(".tmp"));
	}
	itr = m_new_name_map.begin();
	for (; itr != m_new_name_map.end(); ++itr) {
		MoveFile(itr->first + _T(".tmp"), itr->second);
	}
	m_new_name_map.clear();
	GetDlgItem(IDC_BUTTON_UNDO)->EnableWindow(FALSE);
	CString str;
	str.Format(_T("�����������ѳ�����"));
	MessageBox(str, _T("����������"));
}

BOOL CTszRenameDlg::PreTranslateMessage(MSG* pMsg)
{
	if (NULL != m_tooltip_ctrl.GetSafeHwnd()) {
		m_tooltip_ctrl.RelayEvent(pMsg);
	}
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_KEYUP) {
		if (m_cmbFilename.IsChild(this->GetFocus())) {
			m_n_file_name_pos = m_cmbFilename.GetEditSel();
		}
	}
	if (pMsg->message == WM_KEYDOWN) {
		if (GetFocus() == GetDlgItem(IDC_LIST_FILE)) {
			switch (pMsg->wParam) {
			case VK_DELETE:
				OnBnClickedButton_RemoveFiles();
				break;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CTszRenameDlg::OnBnClickedButton_Name_Range()
{
	UpdateDataWnd(this);
	set<CString> name_set;
	set<CString> ext_set;
	POSITION pos = m_file_list_ctrl.GetFirstSelectedItemPosition();
	while (pos) {
		int n = m_file_list_ctrl.GetNextSelectedItem(pos);
		if (n >= 0) {
			name_set.insert(m_file_list_ctrl.GetItemText(n, 0));
			ext_set.insert(m_file_list_ctrl.GetItemText(n, 1));
		}
	}
	if (name_set.empty()) {
		int n = m_file_list_ctrl.GetItemCount();
		for (int i = 0; i < n; ++i) {
			name_set.insert(m_file_list_ctrl.GetItemText(i, 0));
			ext_set.insert(m_file_list_ctrl.GetItemText(i, 1));
		}
	}
	CString str;
	auto itr = name_set.begin();
	for (; itr != name_set.end(); ++itr) {
		if (itr->GetLength() > str.GetLength())
			str = *itr;
	}
	if (str.IsEmpty()) {
		MessageBox(_T("�ļ��б�Ϊ�գ��޷�ѡ���ı���Χ"), _T("����������"));
	} else {
		CSelectTextRangeDlg dlg;
		dlg.SetOriginText(str);
		int rt = dlg.DoModal();
		if (rt == IDOK) {
			CString str = dlg.GetTextRange();
			if (!str.IsEmpty()) {
				m_file_name_pat += _T("<N") + str + _T(">");
				rename_file_list();
				UpdateData(FALSE);
			}
		}
	}
}

void CTszRenameDlg::OnBnClickedButton8()
{
	UpdateDataWnd(this);
	set<CString> name_set;
	set<CString> ext_set;
	POSITION pos = m_file_list_ctrl.GetFirstSelectedItemPosition();
	while (pos) {
		int n = m_file_list_ctrl.GetNextSelectedItem(pos);
		if (n >= 0) {
			name_set.insert(m_file_list_ctrl.GetItemText(n, 0));
			ext_set.insert(m_file_list_ctrl.GetItemText(n, 1));
		}
	}
	if (name_set.empty()) {
		int n = m_file_list_ctrl.GetItemCount();
		for (int i = 0; i < n; ++i) {
			name_set.insert(m_file_list_ctrl.GetItemText(i, 0));
			ext_set.insert(m_file_list_ctrl.GetItemText(i, 1));
		}
	}
	CString str;
	set<CString>::iterator itr = ext_set.begin();
	for (; itr != ext_set.end(); ++itr) {
		if (itr->GetLength() > str.GetLength())
			str = *itr;
	}
	if (str.IsEmpty()) {
		MessageBox(_T("�ļ��б�Ϊ�գ��޷�ѡ���ı���Χ"), _T("����������"));
	} else {
		CSelectTextRangeDlg dlg;
		dlg.SetOriginText(str);
		int rt = dlg.DoModal();
		if (rt == IDOK) {
			CString str = dlg.GetTextRange();
			if (!str.IsEmpty()) {
				InsertStringToEXTPat(_T("<E") + str + _T(">"));
				rename_file_list();
				UpdateData(FALSE);
			}
		}
	}
}

void CTszRenameDlg::OnCbnSelchangeComboOldString()
{
	UpdateDataWnd(this);
	CString findStr, replaceStr;
	GetDlgItemText(IDC_COMBO_OLD_STRING, findStr);
	GetDlgItemText(IDC_COMBO_NEW_STRING, replaceStr);
	if (replaceStr.IsEmpty()) {
		SetDlgItemText(IDC_COMBO_NEW_STRING, _T("<���>"));
	}
	rename_file_list();
}

void CTszRenameDlg::OnCbnEditchangeComboOldString()
{
	UpdateDataWnd(this);
	CString findStr, replaceStr;
	GetDlgItemText(IDC_COMBO_OLD_STRING, findStr);
	GetDlgItemText(IDC_COMBO_NEW_STRING, replaceStr);
	if (replaceStr.IsEmpty()) {
		SetDlgItemText(IDC_COMBO_NEW_STRING, _T("<���>"));
	}
	rename_file_list();
}

void CTszRenameDlg::OnCbnSelchangeComboNewString()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnCbnEditchangeComboNewString()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedCheck1()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedCheck2()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedCheck3()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnBnClickedCheck4()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnEnChangeEditCounterStart()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnEnChangeEditCounterStep()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnCbnSelchangeComboCounterLen()
{
	UpdateDataWnd(this);
	rename_file_list();
}

void CTszRenameDlg::OnDropFiles(HDROP hDropInfo)
{
	int count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < count; ++i) {
		TCHAR buffer[MAX_PATH] = { 0 };
		if (DragQueryFile(hDropInfo, i, buffer, MAX_PATH) > 0) {
			AppendToFileList(buffer);
		}
	}
	rename_file_list(true);
	CDialogEx::OnDropFiles(hDropInfo);
}

void CTszRenameDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//While the mouse is moving, this routine is called.
	//This routine will redraw the drag image at the present
	// mouse location to display the dragging.
	//Also, while over a CListCtrl, this routine will highlight
	// the item we are hovering over.

	//// If we are in a drag/drop procedure (m_bDragging is true)
	if (m_bDragging) {
		//// Move the drag image
		CPoint pt(point);	//get our current mouse coordinates
		ClientToScreen(&pt); //convert to screen coordinates
		BOOL b = m_pDragImage->DragMove(pt); //move the drag image to those coordinates
									// Unlock window updates (this allows the dragging image to be shown smoothly)
		ASSERT(b);
		b = m_pDragImage->DragShowNolock(false);
		ASSERT(b);

		//// Get the CWnd pointer of the window that is under the mouse cursor
		CWnd* pDropWnd = WindowFromPoint(pt);
		ASSERT(pDropWnd); //make sure we have a window

						  //// If we drag outside current window we need to adjust the highlights displayed
		if (pDropWnd != m_pDropWnd) {
			if (m_nDropIndex != -1) //If we drag over the CListCtrl header, turn off the hover highlight
			{
				TRACE("m_nDropIndex is not -1\n");
				CListCtrl* pList = (CListCtrl*)m_pDropWnd;
				VERIFY(pList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED));
				// redraw item
				VERIFY(pList->RedrawItems(m_nDropIndex, m_nDropIndex));
				pList->UpdateWindow();
				m_nDropIndex = -1;
			} else //If we drag out of the CListCtrl altogether
			{
				TRACE("m_nDropIndex is -1\n");
				CListCtrl* pList = (CListCtrl*)m_pDropWnd;
				int i = 0;
				int nCount = pList->GetItemCount();
				for (i = 0; i < nCount; i++) {
					pList->SetItemState(i, 0, LVIS_DROPHILITED);
				}
				pList->RedrawItems(0, nCount);
				pList->UpdateWindow();
			}
		}

		// Save current window pointer as the CListCtrl we are dropping onto
		m_pDropWnd = pDropWnd;

		// Convert from screen coordinates to drop target client coordinates
		pDropWnd->ScreenToClient(&pt);

		//If we are hovering over a CListCtrl we need to adjust the highlights
		if (pDropWnd->IsKindOf(RUNTIME_CLASS(CListCtrl))) {
			//Note that we can drop here
			SetCursor(LoadCursor(NULL, IDC_HAND));
			UINT uFlags;
			CListCtrl* pList = (CListCtrl*)pDropWnd;

			if (m_nDropIndex != -1) {
				// Turn off hilight for previous drop target
				pList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED);
				// Redraw previous item
				pList->RedrawItems(m_nDropIndex, m_nDropIndex);
			}

			// Get the item that is below cursor
			m_nDropIndex = ((CListCtrl*)pDropWnd)->HitTest(pt, &uFlags);
			if (m_nDropIndex != -1) {
				// Highlight it
				pList->SetItemState(m_nDropIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
				// Redraw item
				pList->RedrawItems(m_nDropIndex, m_nDropIndex);
			}
			pList->UpdateWindow();
		} else {
			//If we are not hovering over a CListCtrl, change the cursor
			// to note that we cannot drop here
			SetCursor(LoadCursor(NULL, IDC_NO));
		}
		// Lock window updates
		b = m_pDragImage->DragShowNolock(true);
		ASSERT(b);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CTszRenameDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	//This routine is the end of the drag/drop operation.
	//When the button is released, we are to drop the item.
	//There are a few things we need to do to clean up and
	// finalize the drop:
	//	1) Release the mouse capture
	//	2) Set m_bDragging to false to signify we are not dragging
	//	3) Actually drop the item (we call a separate function to do that)

	bool dragged = false;
	//If we are in a drag and drop operation (otherwise we don't do anything)
	if (m_bDragging) {
		// Release mouse capture, so that other controls can get control/messages
		ReleaseCapture();

		// Note that we are NOT in a drag operation
		m_bDragging = FALSE;

		// End dragging image
		BOOL b = m_pDragImage->DragLeave(GetDesktopWindow());
		ASSERT(b);
		m_pDragImage->EndDrag();
		delete m_pDragImage; //must delete it because it was created at the beginning of the drag

		CPoint pt(point); //Get current mouse coordinates
		ClientToScreen(&pt); //Convert to screen coordinates
							 // Get the CWnd pointer of the window that is under the mouse cursor
		CWnd* pDropWnd = WindowFromPoint(pt);
		ASSERT(pDropWnd); //make sure we have a window pointer
						  // If window is CListCtrl, we perform the drop
		if (pDropWnd && pDropWnd->IsKindOf(RUNTIME_CLASS(CListCtrl))) {
			m_pDropList = (CListCtrl*)pDropWnd; //Set pointer to the list we are dropping on
			DropItemOnList(m_pDragList, m_pDropList); //Call routine to perform the actual drop
			dragged = true;
		}
	}

	if (dragged)
		rename_file_list(false);
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CTszRenameDlg::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	//This routine sets the parameters for a Drag and Drop operation.
	//It sets some variables to track the Drag/Drop as well
	// as creating the drag image to be shown during the drag.

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Save the index of the item being dragged in m_nDragIndex
	//  This will be used later for retrieving the info dragged
	m_nDragIndex = pNMListView->iItem;

	// Create a drag image
	POINT pt;
	pt.x = 0;
	pt.y = 0;
	int nOffset = -2; //offset in pixels for drag image (positive is up and to the left; neg is down and to the right)
	if (m_file_list_ctrl.GetSelectedCount() > 1) //more than one item is selected
		pt.x = nOffset;
	pt.y = nOffset;

	m_pDragImage = m_file_list_ctrl.CreateDragImage(m_nDragIndex, &pt);
	ASSERT(m_pDragImage); //make sure it was created
						  //We will call delete later (in LButtonUp) to clean this up

	// Change the cursor to the drag image
	//	(still must perform DragMove() in OnMouseMove() to show it moving)
	BOOL b = m_pDragImage->BeginDrag(0, CPoint(nOffset, nOffset - 4));
	ASSERT(b);
	b = m_pDragImage->DragEnter(GetDesktopWindow(), pNMListView->ptAction);
	ASSERT(b);

	// Set dragging flag and others
	m_bDragging = TRUE;	//we are in a drag and drop operation
	m_nDropIndex = -1;	//we don't have a drop index yet
	m_pDragList = &m_file_list_ctrl; //make note of which list we are dragging from
	m_pDropWnd = &m_file_list_ctrl;	//at present the drag list is the drop list

	// Capture all mouse messages
	SetCapture();

	*pResult = 0;
}