// MultiNC.cpp : implementation file
//

#include "stdafx.h"
#include "ÊÓÆµ±à½âÂëÆ÷.h"
#include "MultiNC.h"
#include "MainFrm.h"
#include "ÊÓÆµ±à½âÂëÆ÷Doc.h"
#include "ÊÓÆµ±à½âÂëÆ÷View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern unsigned char *ImageBytes;
extern unsigned char *SndImageBytes;
extern unsigned char *TrdImageBytes;
extern unsigned char singalbit[4096];
extern unsigned char secondsingalbit[4096];
extern unsigned char markbit[5000];
extern unsigned char secondmarkbit[5000];
extern unsigned char tribit[1500];
extern unsigned char secondtribit[1500];
extern unsigned char key[44096];
extern unsigned char bittoshow[4096],sndbittoshow[4096];
/////////////////////////////////////////////////////////////////////////////
// MultiNC dialog


MultiNC::MultiNC(CWnd* pParent /*=NULL*/)
	: CDialog(MultiNC::IDD, pParent)
{
	//{{AFX_DATA_INIT(MultiNC)
	m_name1 = _T("");
	m_name2 = _T("");
	corect1 = 0;
	corect2 = 0;
	wrg1 = 0;
	nc2 = 0.0;
	total = 0;
	nc1 = 0.0;
	wrg2 = 0;
	total2 = 0;
	//}}AFX_DATA_INIT
}


void MultiNC::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MultiNC)
	DDX_Text(pDX, IDC_EDIT1, m_name1);
	DDX_Text(pDX, IDC_EDIT6, m_name2);
	DDX_Text(pDX, IDC_EDIT2, corect1);
	DDX_Text(pDX, IDC_EDIT7, corect2);
	DDX_Text(pDX, IDC_EDIT3, wrg1);
	DDX_Text(pDX, IDC_EDIT10, nc2);
	DDX_Text(pDX, IDC_EDIT4, total);
	DDX_Text(pDX, IDC_EDIT5, nc1);
	DDX_Text(pDX, IDC_EDIT8, wrg2);
	DDX_Text(pDX, IDC_EDIT9, total2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MultiNC, CDialog)
	//{{AFX_MSG_MAP(MultiNC)
	ON_BN_CLICKED(IDC_BUTTON1, OpenW1)
	ON_BN_CLICKED(IDC_BUTTON2, OpenW2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MultiNC message handlers

void MultiNC::OpenW1() 
{
    // TODO: Add your control notification handler code here    
    CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
    CMyView *pView=(CMyView *)pMain->GetActiveView();
    NCDATA nc_result;
    unsigned char *orgbit;
    CString &wm_name=m_name1;
    orgbit=new unsigned char [pView->m_width*pView->m_height];
    OpenWm(pView->m_width,pView->m_height,orgbit,wm_name);
    nc_result=GetNC(pView->m_width,pView->m_height,orgbit,bittoshow);
    corect1=nc_result.Corrct;
    wrg1=nc_result.Wrong;
    nc1=nc_result.NC;
    total=corect1+wrg1;
    delete [] orgbit; 
    UpdateData(false);
}

void MultiNC::OpenW2() 
{
  // TODO: Add your control notification handler code here    
    CMainFrame *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd;
    CMyView *pView=(CMyView *)pMain->GetActiveView();
    NCDATA nc_result;
    unsigned char *orgbit;
    CString &wm_name=m_name2;
    orgbit=new unsigned char [pView->m_width*pView->m_height];
    OpenWm(pView->m_width,pView->m_height,orgbit,wm_name);
    nc_result=GetNC(pView->m_width,pView->m_height,orgbit,sndbittoshow);
    corect2=nc_result.Corrct;
    wrg2=nc_result.Wrong;
    nc2=nc_result.NC;
    total2=corect2+wrg2;
    delete [] orgbit; 
    UpdateData(false);
}
