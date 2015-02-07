#if !defined(AFX_VIDEOATTACK_H__0002E520_A60B_4E8B_AF76_4B6377E3432D__INCLUDED_)
#define AFX_VIDEOATTACK_H__0002E520_A60B_4E8B_AF76_4B6377E3432D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoAttack.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VideoAttack dialog

class VideoAttack : public CDialog
{
// Construction
public:
	double angle;
	int m_pos;
	void Clear();
	bool if_rotate;
	bool if_noise;
	double percent;
	CString video_path;
	bool ifopen;
	long int filesize;
	VideoAttack(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(VideoAttack)
	enum { IDD = IDD_IMPROATTACK };
	CSliderCtrl	m_slider;
	CProgressCtrl	m_process;
	CString	m_name;
	CString	m_InputType;
	int		atk_type;
	CString	m_text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VideoAttack)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VideoAttack)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenyuv();
	afx_msg void OnAttack();
	afx_msg void OnRotate();
	afx_msg void OnCrop();
	afx_msg void OnNoise();
	afx_msg void OnBlur();
	afx_msg void OnFilter();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOATTACK_H__0002E520_A60B_4E8B_AF76_4B6377E3432D__INCLUDED_)
