// NoiseAttack.cpp : implementation file
//

#include "stdafx.h"
#include "视频编解码器.h"
#include "NoiseAttack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NoiseAttack dialog


NoiseAttack::NoiseAttack(CWnd* pParent /*=NULL*/)
	: CDialog(NoiseAttack::IDD, pParent)
{
	//{{AFX_DATA_INIT(NoiseAttack)
	m_videoname = _T("");
	rateid = -1;
	//}}AFX_DATA_INIT
}


void NoiseAttack::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NoiseAttack)
	DDX_Text(pDX, IDC_EDIT1, m_videoname);
	DDX_Radio(pDX, IDC_MINERROR, rateid);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NoiseAttack, CDialog)
	//{{AFX_MSG_MAP(NoiseAttack)
	ON_BN_CLICKED(IDOK, OnAttack)
	ON_BN_CLICKED(IDC_ATTACKOPEN, OnAttackopen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NoiseAttack message handlers

void NoiseAttack::OnAttack() 
{
	// TODO: Add your control notification handler code here
	char temp[300];
	double rate,par;
	int errorbitnum;
	int k;
    CString tempname; 

	if(ifopen)
	{
      UpdateData(true);
	 
	  switch(rateid) {
	  case 0:
		  rate=0.0001;
	  	break;
	  case 1:
		  rate=0.0005;
	  	break;
	  case 2:
		  rate=0.001;
		  break;
	  default:
		   rate=0.0001;
	  }
      
	  errorbitnum=(int)videosize*rate;

	  for(int i=0;i<errorbitnum;i++)
	  {
		  par=(double)(videosize-1)/(double)0x7fff;//产生随即数
		  srand((unsigned)time(NULL)); 
		  k=(int)((double)rand()*par);
		  *(bitstream+k)=*(bitstream+k)+8;
	  }
	  sprintf(temp,"Attack finished! %d bits have been modified.",errorbitnum);
      MessageBox(temp,"Noisy Attack",MB_ICONINFORMATION|MB_OK);
	  
	  CFileDialog openbits(FALSE,NULL,NULL,OFN_HIDEREADONLY,"263 Files(*.263)|*.263|");
      openbits.m_ofn.lpstrTitle="保存攻击后的263文件";
	  openbits.m_ofn.lStructSize=88;
	  if(openbits.DoModal()==IDOK)
	  {
		  tempname=openbits.GetPathName();
		  tempname+=".263";
		  FILE *fp;
		  if(fp=fopen(tempname,"wb"))
		  {   
			  fseek(fp,0,SEEK_SET);
			  if(fwrite(bitstream,sizeof(unsigned char),videosize,fp)==NULL)
				 MessageBox("Sorry! Unable to write attacked bitstream.","Noisy Attack",MB_ICONSTOP|MB_OK);
		        else
				{
				MessageBox("Successfully Saved!","Noisy Attack",MB_ICONINFORMATION|MB_OK);
				fclose(fp);
				}
		}
		  else
			      MessageBox("Sorry! Unable to save key file.","Noisy Attack",MB_ICONSTOP|MB_OK);
	  }
	}
    else
	{
	  sprintf(temp,"You havn't opened a videofile,\nPlease open a H.263 file first!");
      MessageBox(temp,"Noisy Attack",MB_ICONSTOP|MB_OK);
	  return;
	}
}

void NoiseAttack::OnAttackopen() 
{
	// TODO: Add your control notification handler code here
	CFileDialog openbits(TRUE,NULL,NULL,OFN_HIDEREADONLY,"263 Files(*.263)|*.263|");
	openbits.m_ofn.lpstrTitle="打开263文件";
	openbits.m_ofn.lStructSize=88;
	ifopen=false;
	CFile videofile;
	CString m_videopath;
	char temp[50];
	if(openbits.DoModal()==IDOK)
	{
	 m_videoname=openbits.GetFileName();
	 m_videopath=openbits.GetPathName();
	 UpdateData(false);
     if(videofile.Open(m_videopath,CFile::modeRead))
	 {
	   videosize=videofile.GetLength();
	   bitstream=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*videosize);
	   videofile.Read(bitstream,videosize);
	   ifopen=true;
	   sprintf(temp,"You have successfully opened a H.263 file!");
	   MessageBox(temp,"Noisy Attack",MB_ICONINFORMATION|MB_OK);
	   return;
	 }
	 else
     {
	   sprintf(temp,"Can not open the selcted compressed video file!");
	   MessageBox(temp,"Noisy Attack",MB_ICONSTOP|MB_OK);
	   return;
	 }
	  
	}
}

BOOL NoiseAttack::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	ifopen=false;
	rateid=0;
	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
