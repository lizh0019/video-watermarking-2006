#if !defined(AFX_MULTINC_H__E87B1BAD_75F5_4E08_9487_A2AA9B995649__INCLUDED_)
#define AFX_MULTINC_H__E87B1BAD_75F5_4E08_9487_A2AA9B995649__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiNC.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MultiNC dialog

class MultiNC : public CDialog
{
// Construction
public:
	MultiNC(CWnd* pParent = NULL);   // standard constructor
    int Bytesoff;
	int height;
	int width;
// Dialog Data
	//{{AFX_DATA(MultiNC)
	enum { IDD = IDD_CALCULATENC2 };
	CString	m_name1;
	CString	m_name2;
	int		corect1;
	int		corect2;
	int		wrg1;
	double	nc2;
	int		total;
	double	nc1;
	int		wrg2;
	int		total2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MultiNC)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MultiNC)
	afx_msg void OpenW1();
	afx_msg void OpenW2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTINC_H__E87B1BAD_75F5_4E08_9487_A2AA9B995649__INCLUDED_)
