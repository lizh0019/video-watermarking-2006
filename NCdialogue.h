#if !defined(AFX_NCDIALOGUE_H__DEE20779_507E_4A79_A9EA_49627CE84CE8__INCLUDED_)
#define AFX_NCDIALOGUE_H__DEE20779_507E_4A79_A9EA_49627CE84CE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NCdialogue.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NCdialogue dialog
class NCdialogue : public CDialog
{
// Construction
public:
	int Bytesoff;
	int height;
	int width;
	NCdialogue(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NCdialogue)
	enum { IDD = IDD_CALCULATENC };
	CString	m_name;
	int		m_correct;
	int		m_wrong;
	int		m_total;
	double	m_nc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NCdialogue)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NCdialogue)
	afx_msg void OpenOriginal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NCDIALOGUE_H__DEE20779_507E_4A79_A9EA_49627CE84CE8__INCLUDED_)
