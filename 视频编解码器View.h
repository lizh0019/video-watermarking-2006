// ÊÓÆµ±à½âÂëÆ÷View.h : interface of the CMyView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEW_H__1247F38D_CA2A_4348_AB3D_72E1DC1137BF__INCLUDED_)
#define AFX_VIEW_H__1247F38D_CA2A_4348_AB3D_72E1DC1137BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "EncodeDlg.h"
#include "ÊÓÆµ±à½âÂëÆ÷Doc.h"
#include <stdio.h>
#include<string.h> 
#include <io.h>
#include<fcntl.h>

typedef struct _iobuf FILE;

class CMyView : public CView
{
protected: // create from serialization only
	CMyView();
	DECLARE_DYNCREATE(CMyView)

// Attributes
public:
	CMyDoc* GetDocument();

// Operations
public:
	//ÂÊ¿ØÖÆ
	int ratecontrol;
	int  targetrate;
	float ref_frame_rate;
    //encoding parameters
	int bFlag;
	CString csTimeElapse;
	CString conclusion;
	CString number;

	CString	m_szFilePathName;
	CString m_szFileName;

	CString	m_codebookPathName;
	CString m_codebookName;

	int MaxFrame;
	int m_orgWidth;
	int m_orgHeight;
	unsigned char * m_pImageData;
	BITMAPINFO* image;
	int m_Type;
	int QP,QPI;
	int Pbetween;
	BOOL ifPsnr;
	BOOL ifVQ,VisualVQ,ifvisualbook,h263encode;
	BOOL rdoptimize;
	int ifgenerate;
	double psnrs[3];
    //decoding parameters
    CString DecfileName;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	bool rofgfp_finished;
	bool SaveMark(int width, int height, unsigned char *image_data,BITMAPINFO* image);
	bool RoFrFp;
	int  *Lowf_Trainvector[MAXIMAGESIZE], *Midf_Trainvector[MAXIMAGESIZE], *Hghf_Trainvector[MAXIMAGESIZE];
	bool DCT_codebook;
	bool ifopen;
	void ConvertStrToChar(CString &embedsite );
	CString website;
	BOOL ifbridge;
	CFile yuvfile;
	bool decode_record;
	long int speed;
	void DPSEUDO(unsigned char *pmwmdata,unsigned char *rewmdata,int N1,int N2,int number,int seed);
	bool multifinished;
	bool markfinish;
	void Record(PictImage *recon, int width,int height, CFile file);
	BOOL ifrecord;
	bool keymark;
	bool robandfrg;
	bool rob_semf;
	bool multipurpose;
	void clearmark();
	bool mvwatermark;	
	bool vqwatermark;
	bool watercodebook;
	unsigned char *Markimage,*SndMarkimage;	
    int m_width,m_height,m_width2,m_height2;
	BOOL PeekAndPump();
	void CodeBmps();
	void CodeYUV();
	virtual ~CMyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CFont m_font,hit_font,dsp_font;
// Generated message map functions
protected:
	//{{AFX_MSG(CMyView)
	afx_msg void OnEncode();
	afx_msg void OnDecode();
	afx_msg void OnFileOpen();
	afx_msg void OnDecodevq();
	afx_msg void OnUpdateDecode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDecodevq(CCmdUI* pCmdUI);
	afx_msg void OnVisualdec();
	afx_msg void OnUpdateVisualdec(CCmdUI* pCmdUI);
	afx_msg void ExtractWatermark();
	afx_msg void OnMvDM();
	afx_msg void OnMultiExtract();
	afx_msg void OnUpdateMultiExtract(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMvdm(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuitem32783(CCmdUI* pCmdUI);
	afx_msg void OnCalNC();
	afx_msg void OnDctparity();
	afx_msg void OnUpdateDctparity(CCmdUI* pCmdUI);
	afx_msg void Savwmark();
	afx_msg void OnUpdateSavwmark(CCmdUI* pCmdUI);
	afx_msg void Savemultimark();
	afx_msg void OnUpdateSavemultimark(CCmdUI* pCmdUI);
	afx_msg void OnRfwm();
	afx_msg void OnUpdateRfwm(CCmdUI* pCmdUI);
	afx_msg void On2nc();
	afx_msg void OnUpdate2nc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuitem32786(CCmdUI* pCmdUI);
	afx_msg void OnBiterror();
	afx_msg void OnNormalplay();
	afx_msg void OnSlowplay();
	afx_msg void OnFastplay();
	afx_msg void OnRecord();
	afx_msg void OnFileNew();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBridge_extract();
	afx_msg void OnUpdateNormalplay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFastplay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSlowplay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecord(CCmdUI* pCmdUI);
	afx_msg void OnRobsemifrgwm();
	afx_msg void OnTrimark();
	afx_msg void OnRfview();
	afx_msg void OnAttackMethod();
	afx_msg void OnUpdateTrimark(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRfview(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ÊÓÆµ±à½âÂëÆ÷View.cpp
inline CMyDoc* CMyView::GetDocument()
   { return (CMyDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEW_H__1247F38D_CA2A_4348_AB3D_72E1DC1137BF__INCLUDED_)
