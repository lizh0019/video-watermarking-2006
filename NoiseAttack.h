#if !defined(AFX_NOISEATTACK_H__16A3DF78_4C31_4740_9108_35F5AFBF7D39__INCLUDED_)
#define AFX_NOISEATTACK_H__16A3DF78_4C31_4740_9108_35F5AFBF7D39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NoiseAttack.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NoiseAttack dialog

class NoiseAttack : public CDialog
{
// Construction
public:
	unsigned char * bitstream;
	bool ifopen;
	long videosize;
	NoiseAttack(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NoiseAttack)
	enum { IDD = IDD_ATTACK };
	CString	m_videoname;
	int		rateid;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NoiseAttack)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NoiseAttack)
	afx_msg void OnAttack();
	afx_msg void OnAttackopen();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOISEATTACK_H__16A3DF78_4C31_4740_9108_35F5AFBF7D39__INCLUDED_)
