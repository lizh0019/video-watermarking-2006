// ViewNC.cpp : implementation file
//

#include "stdafx.h"
#include "视频编解码器.h"
#include "ViewNC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern double  Fp_NC[20],Ro_NC[20];
extern int  I_num_toshow;
/////////////////////////////////////////////////////////////////////////////
// ViewNC dialog


ViewNC::ViewNC(CWnd* pParent /*=NULL*/)
	: CDialog(ViewNC::IDD, pParent)
{
	//{{AFX_DATA_INIT(ViewNC)
	m_total = 0;
	m_wms = 0;
	//}}AFX_DATA_INIT
}


void ViewNC::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ViewNC)
	DDX_Control(pDX, IDC_NCLIST1, m_list);
	DDX_Text(pDX, IDC_TotalFrame, m_total);
	DDX_Text(pDX, IDC_WMS, m_wms);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ViewNC, CDialog)
	//{{AFX_MSG_MAP(ViewNC)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ViewNC message handlers

BOOL ViewNC::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	 m_list.InsertColumn(0,"WMS No");
     m_list.SetColumnWidth(0,80);
     m_list.InsertColumn(1,"Robust");
     m_list.SetColumnWidth(1,150);
	 m_list.InsertColumn(2,"Fingerprint");
     m_list.SetColumnWidth(2,150);
     int i=0;
     CString str_no,str_ro,str_fp;
	 for(i=0;i<I_num_toshow;i++)
	 { 
       str_no.Format("%d",i);
	   m_list.InsertItem(i,str_no);//插入行
	   str_ro.Format("%4f",Ro_NC[i+1]);
	   m_list.SetItemText(i,1,str_ro);
	   str_fp.Format("%4f",Fp_NC[i+1]);
	   m_list.SetItemText(i,2,str_fp);
	 }
	 return TRUE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}

