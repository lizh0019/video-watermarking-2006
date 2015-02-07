#if !defined(AFX_VIEWNC_H__995EA189_C5AB_4393_9878_D080E86393F2__INCLUDED_)
#define AFX_VIEWNC_H__995EA189_C5AB_4393_9878_D080E86393F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewNC.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ViewNC dialog

class ViewNC : public CDialog
{
// Construction
public:
	ViewNC(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ViewNC)
	enum { IDD = IDD_NCVIEW };
	CListCtrl	m_list;
	int		m_total;
	int		m_wms;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewNC)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ViewNC)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWNC_H__995EA189_C5AB_4393_9878_D080E86393F2__INCLUDED_)
