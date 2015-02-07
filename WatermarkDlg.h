#if !defined(AFX_WATERMARKDLG_H__9C0F767F_0B87_48A9_A439_CCCD0A736AD2__INCLUDED_)
#define AFX_WATERMARKDLG_H__9C0F767F_0B87_48A9_A439_CCCD0A736AD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WatermarkDlg.h : header file
//
#include <io.h>
#include<fcntl.h>
/////////////////////////////////////////////////////////////////////////////
// CWatermarkDlg dialog

class CWatermarkDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CWatermarkDlg)

// Construction
public:
	void PSEUDO(unsigned char *origwmdata,unsigned char *pmwmdata,int N1,int N2,int number,int seed);
	BOOL if2ndopen;
	BOOL ifopen;
	BOOL fingeropen;
	int Bytesoff;
	int height;
	int width;
	int basic_click,bridge_click,gen_click;
	CWatermarkDlg();
	~CWatermarkDlg();

// Dialog Data
	//{{AFX_DATA(CWatermarkDlg)
	enum { IDD = IDD_WatmarDlg };
	CString	m_keyname;
	CString	m_2ndkeyname;
	CString	m_algorithm;
	CString	m_web;
	int		m_basic;
	CString	m_wmcodebook;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWatermarkDlg)
	public:
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWatermarkDlg)
	afx_msg void OnOpenkey();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void On2ndwatermark();
	afx_msg void OnBasic();
	afx_msg void OnGen();
	afx_msg void OnMb();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WATERMARKDLG_H__9C0F767F_0B87_48A9_A439_CCCD0A736AD2__INCLUDED_)
