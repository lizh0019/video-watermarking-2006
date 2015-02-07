// EncodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "视频编解码器.h"
#include "EncodeDlg.h"
#include "Global.h"
#include <MATH.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int *codeword[MAXBOOKSIZE];
int *trainvector[MAXIMAGESIZE];
int **Low_codeword,**Mid_codeword,**Hig_codeword;

/////////////////////////////////////////////////////////////////////////////
// CEncodeDlg property page

IMPLEMENT_DYNCREATE(CEncodeDlg, CPropertyPage)

CEncodeDlg::CEncodeDlg() : CPropertyPage(CEncodeDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncodeDlg)
	m_ifPsnr = FALSE;
	m_InitDir = _T("");
	m_InType = _T("yuv件");
	m_MaxFrame = 120;
	m_Pbetween = 19;
	m_QP = 4;
	m_QPI = 8;
	m_ifVQ = FALSE;
	m_bookpath = _T("");
	m_ifgenerate = FALSE;
	m_VisualVQ = FALSE;
	m_visualbook = FALSE;
	m_ifrecord = FALSE;
	m_ifVQ=false;
	m_DCTbookpath = _T("");
	//}}AFX_DATA_INIT
	typeindex=2;
}

CEncodeDlg::~CEncodeDlg()
{
}

void CEncodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncodeDlg)
	DDX_Check(pDX, IDC_CHECK1, m_ifPsnr);
	DDX_Text(pDX, IDC_InitDir, m_InitDir);
	DDX_CBString(pDX, IDC_InType, m_InType);
	DDX_Text(pDX, IDC_MaxFrame, m_MaxFrame);
	DDV_MinMaxInt(pDX, m_MaxFrame, 0, 500);
	DDX_Text(pDX, IDC_Pnum, m_Pbetween);
	DDV_MinMaxInt(pDX, m_Pbetween, 0, 29);
	DDX_Text(pDX, IDC_QP, m_QP);
	DDV_MinMaxInt(pDX, m_QP, 2, 31);
	DDX_Text(pDX, IDC_QPI, m_QPI);
	DDV_MinMaxInt(pDX, m_QPI, 2, 31);
	DDX_Check(pDX, IDC_VQ, m_ifVQ);
	DDX_Text(pDX, IDC_BOOKNAME, m_bookpath);
	DDX_Check(pDX, IDC_BOOKGENERATE, m_ifgenerate);
	DDX_Check(pDX, IDC_VQ2, m_VisualVQ);
	DDX_Check(pDX, IDC_Visualbook, m_visualbook);
	DDX_Check(pDX, IDC_Recoed, m_ifrecord);
	DDX_Text(pDX, IDC_EDIT1, m_DCTbookpath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEncodeDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CEncodeDlg)
	ON_BN_CLICKED(IDC_Browse, OnBrowse)
	ON_BN_CLICKED(IDC_OPENBOOK, Openbook)
	ON_BN_CLICKED(IDC_BUTTON3, DCTBOOK_Open)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncodeDlg message handlers

void CEncodeDlg::OnBrowse() 
{
	UpdateData(true);

    static char szFilter[] = "BMP Files(*.bmp)|*.bmp|YUV File(*.yuv)|*.yuv|RAW Files(*.raw)|*.raw|SIF Files(*.sif)|*.sif|";

	CFileDialog FileDlg( TRUE, NULL,NULL,OFN_HIDEREADONLY, szFilter);
	//FileDlg.m_ofn.lpstrInitialDir="F:\\standard_pictures\\MISSUSA_raw";
    FileDlg.m_ofn.lStructSize=88;
	if(m_InType=="bmp序列")
        typeindex=1;
	else if(m_InType=="YUV文件")
	    typeindex=2;
	else if(m_InType=="raw序列")
		typeindex=3;
	else if(m_InType=="sif 序列")
		typeindex=4;


	FileDlg.m_ofn.nFilterIndex=typeindex;

    if( FileDlg.DoModal() == IDOK ) 
	{
		m_InitDir=FileDlg.GetPathName();
  	    CString dd=FileDlg.GetFileExt();
		CString szFileExt = "bmpyuvrawsif";
		dd.MakeLower();
		int type= szFileExt.Find(dd);
		if(typeindex!=type/3+1)
		{  
			AfxMessageBox("文件类型错误!");
            m_InitDir = "F:\\standard_pictures\\MISSUSA_raw\\mobile.yuv";
		    m_InType=="yuv文件";
		    typeindex=2;
		}
	}
	else
	{  
		 m_InitDir = "F:\\standard_pictures\\MISSUSA_raw\\mobile.yuv";
		 m_InType=="yuv文件";
		 typeindex=2;
	}
    UpdateData(false);
}



BOOL CEncodeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    browse.AutoLoad(IDC_Browse,this);
	browse.SizeToContent();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEncodeDlg::Openbook2() 
{
  // TODO: Add your control notification handler code here
  int i;
  unsigned short int  bw,bh;
  bool  m_BookSelected;
//  char codeword[1024][64];
  //int    *Scodeword[512];
  void *memr;
  
   OPENFILENAME ofn;
  FILE *stream;
   char FileName[_MAX_PATH],FileTitle[_MAX_PATH];
   FileName[0]='\0';
   FileTitle[0]='\0';
   BOOKFILEHEADER  bfh;

   CDialog dlg;
   CWnd cd;
    //multiselected=false;
	//testselected=false;
   /////Initialize the FileOpenDialog structure begins////
   //The structure size
   ofn.lStructSize=sizeof(OPENFILENAME); 
   //The Owner window handle
   ofn.hwndOwner=NULL; 
   ofn.hInstance=NULL; 
   //The filter
   ofn.lpstrFilter=TEXT("矢量量化码书文件 *.bok\0*.bok\0All File *.*\0*.*\0\0"); 
    //None user filter
   ofn.lpstrCustomFilter=NULL; 
   ofn.nMaxCustFilter=0; 
   //Select the item '矢量量化码书文件 *.bok'
   ofn.nFilterIndex=1; 
   //Initial the full name without path
   //then save the selected filename with full path
   ofn.lpstrFile=FileName; 
   ofn.nMaxFile=_MAX_PATH;    
   //The filename without path
   ofn.lpstrFileTitle=FileTitle; 
   ofn.nMaxFileTitle=_MAX_PATH; 
   //The initial directory is the next book directory of current directory
   ofn.lpstrInitialDir=NULL; 
   //The title
   ofn.lpstrTitle="选择码书"; 
   //The flags
   ofn.Flags=OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR; 
   //Default extension
   ofn.lpstrDefExt="bok"; 
   ofn.lCustData=NULL; 
   ofn.lpfnHook=NULL; 
   //User defined dialog
   ofn.lpTemplateName=NULL; 
   /////Initialize the FileOpenDialog structure ends////
   //If cancle or error,then return
   
   memr=GlobalAlloc(GMEM_FIXED,MAXBOOKSIZE*64*sizeof(int));
   // GlobalFree(memS);
   //  memS=GlobalAlloc(GMEM_FIXED,512*16*sizeof(int));
	/*if(memr==NULL)
	{
	  AfxMessageBox("内存不够!");
	  GlobalFree(memr);
	  return;
	}*/
    for(i=0;i<MAXBOOKSIZE;i++)
    codeword[i]=(int *)memr+64*i;
   // for(i=0;i<512;i++)
   // Scodeword[i]=(int *)memS+16*i;
if(GetOpenFileName(&ofn)==0)
   {
	   AfxMessageBox("如果选择VQ模式编码，请选择一个码书!");
	   m_BookSelected=false;
   }
   //Search the filename
CFileFind cff;
if(cff.FindFile(FileName,0))
{cff.FindNextFile();
 if(cff.GetFilePath().CompareNoCase(FileName)==0)
   { stream=fopen(FileName,"r") ;
     if(stream==NULL )
	 {MessageBox("码书读取过程失败","码书读取",MB_ICONSTOP|MB_OK);}
 if(fread(&bfh,sizeof(BOOKFILEHEADER),1,stream)==NULL)
   {
	  MessageBox("码书读取过程失败","码书读取",MB_ICONSTOP|MB_OK);
	  fclose(stream);
	  return;
   }
 if((strcmp(bfh.FileFlag,"256GRAY"))||(bfh.PixelBits!=8))
   {
	  MessageBox("码书格式不正确，请选择合适的码书","读取码书",MB_ICONSTOP|MB_OK);
	  fclose(stream);
	  return;
   }
   bw=bfh.ImageBlockWidth;
   bh=bfh.ImageBlockHeight;
 if((bw!=bh)||(bw!=8))
   { 
	 MessageBox("码书格式不正确，请选择合适的码书","读取码书",MB_ICONSTOP|MB_OK);
	  fclose(stream);
	  return;
   }
	 fseek(stream,20,SEEK_SET);
	 if(fread(&codeword[0][0],64*sizeof(int),bfh.CodeBookSize,stream)==NULL)//!!!!!!!!!!!!读取码字->&codeword[0][0]           
      {MessageBox("码书格式不正确，请选择合适的码书","读取码书",MB_ICONSTOP|MB_OK);
       fclose(stream);
         return ;}  
	  else
	  { //memcpy(&p,&codeword,1024*64*sizeof(char));
	    // UpdateData(true);
		MessageBox("You have successfully selected a codebook!","Open codebook",MB_ICONINFORMATION|MB_OK);
		fclose(stream);
		m_bookpath=cff.GetFilePath();
        m_BookSelected=TRUE; 
		// UpdateData(false);
	    // strcpy(book,bfh.flag);
		return ;
	  }    	 
	 
}
}

}

void CEncodeDlg::Openbook()
{
  
  int i;
  unsigned short int  bw,bh;
  bool  m_BookSelected;
//  char codeword[1024][64];
  //int    *Scodeword[512];
  void  *memr;
  
  OPENFILENAME ofn;
  FILE *stream;
   char FileName[_MAX_PATH],FileTitle[_MAX_PATH];
   FileName[0]='\0';
   FileTitle[0]='\0';
   BOOKFILEHEADER  bfh;
  
   CDialog dlg;
   CWnd cd;
    //multiselected=false;
	//testselected=false;
   /////Initialize the FileOpenDialog structure begins////
   //The structure size
   ofn.lStructSize=sizeof(OPENFILENAME); 
   //The Owner window handle
   ofn.hwndOwner=NULL; 
   ofn.hInstance=NULL; 
   //The filter
   ofn.lpstrFilter=TEXT("矢量量化码书文件 *.bok\0*.bok\0All File *.*\0*.*\0\0"); 
    //None user filter
   ofn.lpstrCustomFilter=NULL; 
   ofn.nMaxCustFilter=0; 
   //Select the item '矢量量化码书文件 *.bok'
   ofn.nFilterIndex=1; 
   //Initial the full name without path
   //then save the selected filename with full path
   ofn.lpstrFile=FileName; 
   ofn.nMaxFile=_MAX_PATH;    
   //The filename without path
   ofn.lpstrFileTitle=FileTitle; 
   ofn.nMaxFileTitle=_MAX_PATH; 
   //The initial directory is the next book directory of current directory
   ofn.lpstrInitialDir=NULL; 
   //The title
   ofn.lpstrTitle="选择码书"; 
   //The flags
   ofn.Flags=OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR; 
   //Default extension
   ofn.lpstrDefExt="bok"; 
   ofn.lCustData=NULL; 
   ofn.lpfnHook=NULL; 
   //User defined dialog
   ofn.lpTemplateName=NULL; 
   /////Initialize the FileOpenDialog structure ends////
   //If cancle or error,then return
   
     memr=GlobalAlloc(GMEM_FIXED,MAXBOOKSIZE*64*sizeof(int));
   // GlobalFree(memS);
   //  memS=GlobalAlloc(GMEM_FIXED,512*16*sizeof(int));
	/*if(memr==NULL)
	{
	  AfxMessageBox("内存不够!");
	  GlobalFree(memr);
	  return;
	}*/
   for(i=0;i<MAXBOOKSIZE;i++)
	   codeword[i]=(int *)memr+64*i;
  
if(GetOpenFileName(&ofn)==0)
   {
	   AfxMessageBox("如果选择VQ模式编码，请选择一个码书!");
	   m_BookSelected=false;
   }
   //Search the filename
CFileFind cff;
if(cff.FindFile(FileName,0))
{cff.FindNextFile();
 if(cff.GetFilePath().CompareNoCase(FileName)==0)
   { stream=fopen(FileName,"r") ;
     if(stream==NULL )
	 {MessageBox("码书读取过程失败","码书读取",MB_ICONSTOP|MB_OK);}
 if(fread(&bfh,sizeof(BOOKFILEHEADER),1,stream)==NULL)
   {
	  MessageBox("码书读取过程失败","码书读取",MB_ICONSTOP|MB_OK);
	  fclose(stream);
	  return;
   }
 if((strcmp(bfh.FileFlag,"256GRAY"))||(bfh.PixelBits!=8))
   {
	  MessageBox("码书格式不正确，请选择合适的码书","读取码书",MB_ICONSTOP|MB_OK);
	  fclose(stream);
	  return;
   }
   bw=bfh.ImageBlockWidth;
   bh=bfh.ImageBlockHeight;
 if((bw!=bh)||(bw!=8))
   { 
	 MessageBox("码书格式不正确，请选择合适的码书","读取码书",MB_ICONSTOP|MB_OK);
	  fclose(stream);
	  return;
   }
	 fseek(stream,20,SEEK_SET);
	 if(fread(&codeword[0][0],bw*bh*sizeof(int),bfh.CodeBookSize,stream)==NULL)//!!!!!!!!!!!读取码字->&codeword[0][0]           
      {
	   MessageBox("无法读取码书","读取码书",MB_ICONSTOP|MB_OK);
       fclose(stream);
       return ;
	 }  
	  else
	  { //memcpy(&p,&codeword,1024*64*sizeof(char));
	     UpdateData(true);
		MessageBox("码书选择成功!","读取码书",MB_ICONINFORMATION|MB_OK);
		fclose(stream);
		m_bookpath=cff.GetFilePath();
        m_BookSelected=TRUE; 
	    UpdateData(false);
	    //strcpy(book,bfh.flag);
		return ;
	  }    	 
}
}

}

void CEncodeDlg::OpenDCTbook()
{
  DCTBOOK_Open(true);
}

int CEncodeDlg::DCTBOOK_Open(bool indicator) 
{
	// TODO: Add your control notification handler code here
	CFileDialog openbook(TRUE,NULL,NULL,OFN_HIDEREADONLY,"DCT codebook (*.DCTBOK)|*.DCTBOK|All Files (*.*)|*.*",NULL);
	openbook.m_ofn.lpstrTitle="Open DCT codebook";
	if(openbook.DoModal()==IDOK)
	{
        CFile bookfile;
		m_DCTbookpath=openbook.GetPathName();
		CString m_DCTbookname;
		m_DCTbookname=openbook.GetFileName();
		if(indicator)
		UpdateData(false);
		if(!bookfile.Open(m_DCTbookname,CFile::modeRead,NULL))
		{
		  MessageBox("Unable to open codebook","System Information",MB_ICONSTOP|MB_OK);
		  return 0;
		}
		bookfile.SeekToBegin();
        DCTBOOKFILEHEADER h;
		bookfile.Read(&h,sizeof(DCTBOOKFILEHEADER));
		
		if(strcmp(h.FileFlag,"DCTBOOK"))
		{
	     MessageBox("CodeBook format error!","System Information",MB_ICONSTOP|MB_OK);
	     bookfile.Close();
		 return 0;
		}
		
		Low_codeword=(int **)GlobalAlloc(GMEM_FIXED,sizeof(int)*h.CodebookSize[0]*h.dimension[0]);
		if(Low_codeword==NULL)
		{
		  MessageBox("Not enough memory for Low_codeword","System Information",MB_ICONSTOP|MB_OK);
		  bookfile.Close();
		  return 0;
		}
		Mid_codeword=(int **)GlobalAlloc(GMEM_FIXED,sizeof(int)*h.CodebookSize[1]*h.dimension[1]);
		if(Mid_codeword==NULL)
		{
		  MessageBox("Not enough memory for Low_codeword","System Information",MB_ICONSTOP|MB_OK);
		  bookfile.Close();
		  return 0;
		}
		Hig_codeword=(int **)GlobalAlloc(GMEM_FIXED,sizeof(int)*h.CodebookSize[2]*h.dimension[2]);
		if(Hig_codeword==NULL)
		{
		  MessageBox("Not enough memory for Low_codeword","System Information",MB_ICONSTOP|MB_OK);
		  bookfile.Close();
		  return 0;
		}
		
		for(int i=0;i<h.CodebookSize[0];i++)
		 Low_codeword[i]=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int)*h.dimension[0]);
        for(i=0;i<h.CodebookSize[1];i++)
		 Mid_codeword[i]=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int)*h.dimension[1]);
		for(i=0;i<h.CodebookSize[2];i++)
		 Hig_codeword[i]=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int)*h.dimension[2]);

		bookfile.Seek(20,SEEK_SET);
		
		for(i=0;i<h.CodebookSize[0];i++)
		bookfile.Read(Low_codeword[i],sizeof(int)*h.dimension[0]);
        for(i=0;i<h.CodebookSize[1];i++)
		bookfile.Read(Mid_codeword[i],sizeof(int)*h.dimension[1]);
        for(i=0;i<h.CodebookSize[2];i++)
		bookfile.Read(Hig_codeword[i],sizeof(int)*h.dimension[2]);
        
        int tt=0,ty=0,ty1=0;
        CString test,show;
        for(int k=0;k<h.CodebookSize[1]/2;k++)
        {
          tt=0;
          for(int j=0;j<h.dimension[1];j++)
          {
            if(fabs(Mid_codeword[k][j])==0)
             tt++;
          }
          if(tt==h.dimension[1])
            ty++;
        }
        for( k=0;k<h.CodebookSize[1];k++)
        {
          tt=0;
          for(int j=0;j<h.dimension[1];j++)
          {
            if(fabs(Mid_codeword[k][j])==0)
             tt++;
          }
          if(tt==h.dimension[1])
            ty1++;
        }
       
       show.Format("%d+%d=%d",ty,ty1,ty+ty1);
       AfxMessageBox(show);
        
		bookfile.Close();
		MessageBox("You have successfully selected a DCT codebook!","Open DCT codebook",MB_ICONINFORMATION|MB_OK);
		return 1;
	}
	else
	return 0;
}
