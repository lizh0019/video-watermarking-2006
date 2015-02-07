#if !defined(AFX_ENCODEDLG_H__D11EF289_28DE_45D8_9A51_C7329D05E8A1__INCLUDED_)
#define AFX_ENCODEDLG_H__D11EF289_28DE_45D8_9A51_C7329D05E8A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EncodeDlg.h : header file
//
 #include "Global.h"

   
/////////////////////////////////////////////////////////////////////////////
// CEncodeDlg dialog

class CEncodeDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CEncodeDlg)

// Construction
public:
	void Openbook();
	CEncodeDlg();
	~CEncodeDlg();
    afx_msg void Openbook2();
	afx_msg void OpenDCTbook();
    int DCTBOOK_Open(bool indicator);
	int typeindex;
	CBitmapButton browse;

// Dialog Data
	//{{AFX_DATA(CEncodeDlg)
	enum { IDD = IDD_EncodeDlg };
	BOOL	m_ifPsnr;
	CString	m_InitDir;
	CString	m_InType;
	int		m_MaxFrame;
	int		m_Pbetween;
	int		m_QP;
	int		m_QPI;
	BOOL	m_ifVQ;
	CString	m_bookpath;
	BOOL	m_ifgenerate;
	BOOL	m_VisualVQ;
	BOOL	m_visualbook;
	BOOL	m_ifrecord;
	CString	m_DCTbookpath;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEncodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
 protected:
	// Generated message map functions
	//{{AFX_MSG(CEncodeDlg)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	//afx_msg void DCTBOOK_Open();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENCODEDLG_H__D11EF289_28DE_45D8_9A51_C7329D05E8A1__INCLUDED_)
