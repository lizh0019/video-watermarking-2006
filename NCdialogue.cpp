// NCdialogue.cpp : implementation file
//

#include "stdafx.h"
#include "ÊÓÆµ±à½âÂëÆ÷.h"
#include "NCdialogue.h"
#include "MainFrm.h"
#include "ÊÓÆµ±à½âÂëÆ÷View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern unsigned char *ImageBytes;
extern unsigned char *SndImageBytes;
extern unsigned char singalbit[4096];
extern unsigned char secondsingalbit[4096];
extern unsigned char markbit[5000];
extern unsigned char secondmarkbit[5000];
extern unsigned char tribit[1500];
extern unsigned char secondtribit[1500];
extern unsigned char bittoshow[4096],sndbittoshow[4096];
/////////////////////////////////////////////////////////////////////////////
// NCdialogue dialog


NCdialogue::NCdialogue(CWnd* pParent /*=NULL*/)
	: CDialog(NCdialogue::IDD, pParent)
{
	//{{AFX_DATA_INIT(NCdialogue)
	m_name = _T("");
	m_correct = 0;
	m_wrong = 0;
	m_total = 0;
	m_nc = 0.0;
	//}}AFX_DATA_INIT
}


void NCdialogue::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NCdialogue)
	DDX_Text(pDX, IDC_EDIT1, m_name);
	DDX_Text(pDX, IDC_EDIT2, m_correct);
	DDX_Text(pDX, IDC_EDIT3, m_wrong);
	DDX_Text(pDX, IDC_EDIT4, m_total);
	DDX_Text(pDX, IDC_EDIT5, m_nc);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NCdialogue, CDialog)
	//{{AFX_MSG_MAP(NCdialogue)
	ON_BN_CLICKED(IDC_BUTTON1, OpenOriginal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NCdialogue message handlers

void NCdialogue::OpenOriginal() 
{
	// TODO: Add your control notification handler code here    
    CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
    CMyView *pView=(CMyView *)pMain->GetActiveView();
    NCDATA nc_result;
    unsigned char *orgbit;
    CString &wm_name=m_name;
    orgbit=new unsigned char [pView->m_width*pView->m_height];
    OpenWm(pView->m_width,pView->m_height,orgbit,wm_name);
    nc_result=GetNC(pView->m_width,pView->m_height,orgbit,bittoshow);
    m_correct=nc_result.Corrct;
    m_wrong=nc_result.Wrong;
    m_nc=nc_result.NC;
    m_total=m_correct+m_wrong;
    delete [] orgbit; 
    UpdateData(false);
}


