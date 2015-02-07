// 视频编解码器View.cpp : implementation of the CMyView class
//

#include "stdafx.h"
#include <math.h>                   
#include "视频编解码器.h"

#include "视频编解码器Doc.h"
#include "视频编解码器View.h"
#include "OpenFiles.h"
#include "Global.h"
#include "ProgressBar.h"
#include "Display.h"
#include "PropertyDlg.h"
#include "NCdialogue.h"
#include "MultiNC.h"
#include "NoiseAttack.h"
#include "EncodeDlg.h"
#include "ViewNC.h"
#include "AttackMethod.h"
#include "VideoAttack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL h263=false;
BOOL vqdecode=false;
BOOL visualcode=false;
BOOL basicextract=false;
BOOL bridge_extract=false;
BOOL dmmvextract=false;
BOOL multiextract=false;
BOOL robfrgextract=false;
BOOL rob_semifrg_extract=false;
BOOL RoFgFp_extract=false;
BOOL dctparity=false;
bool ifshow=false;
bool ifgo=false;


int pels,lines,mv_outside_frame,long_vectors,cpels;
CFile streamfile,psnrfile,recordfile;
static void SeekPsnr(PictImage *curr,PictImage *recon,int width,int height,double psnr[3]);
extern int firstframe;
extern int  I_num_toshow;

extern int *codeword[MAXBOOKSIZE];
extern unsigned char singalbit[4096];
extern unsigned char markbit[5000];
extern unsigned char trimark[1500];
extern unsigned char secondsingalbit[4096];
extern unsigned char secondtrimark[1500];
extern unsigned char keybit[44096];
extern unsigned char key[444096];
extern unsigned char finalbit[4096];
extern unsigned char frg_bit[444096];
extern unsigned char I_frg_bit[16384];
extern int *trainvector[MAXIMAGESIZE];

unsigned char po[4096]; 
unsigned char bittoshow[4096],sndbittoshow[4096];
unsigned char Robit[20][64],Fpbit[20][64];
extern int **Low_codeword,**Mid_codeword,**Hig_codeword;

int dim_Low,dim_Mid,dim_Hig;

#define OFFLINE_RATE_CONTROL//jwp
/////////////////////////////////////////////////////////////////////////////
// CMyView

IMPLEMENT_DYNCREATE(CMyView, CView)

BEGIN_MESSAGE_MAP(CMyView, CView)
	//{{AFX_MSG_MAP(CMyView)
	ON_COMMAND(ID_Encode, OnEncode)
	ON_COMMAND(ID_Decode, OnDecode)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_DECODEVQ, OnDecodevq)
	ON_UPDATE_COMMAND_UI(ID_Decode, OnUpdateDecode)
	ON_UPDATE_COMMAND_UI(ID_DECODEVQ, OnUpdateDecodevq)
	ON_COMMAND(ID_VISUALDEC, OnVisualdec)
	ON_UPDATE_COMMAND_UI(ID_VISUALDEC, OnUpdateVisualdec)
	ON_COMMAND(ID_MENUITEM32783, ExtractWatermark)
	ON_COMMAND(ID_MVDM, OnMvDM)
	ON_COMMAND(ID_MultiExtract, OnMultiExtract)
	ON_UPDATE_COMMAND_UI(ID_MultiExtract, OnUpdateMultiExtract)
	ON_UPDATE_COMMAND_UI(ID_MVDM, OnUpdateMvdm)
	ON_UPDATE_COMMAND_UI(ID_MENUITEM32783, OnUpdateMenuitem32783)
	ON_COMMAND(ID_MENUITEM32786, OnCalNC)
	ON_COMMAND(ID_DCTPARITY, OnDctparity)
	ON_UPDATE_COMMAND_UI(ID_DCTPARITY, OnUpdateDctparity)
	ON_COMMAND(ID_SAVWMARK, Savwmark)
	ON_UPDATE_COMMAND_UI(ID_SAVWMARK, OnUpdateSavwmark)
	ON_COMMAND(ID_SAVEMULTIMARK, Savemultimark)
	ON_UPDATE_COMMAND_UI(ID_SAVEMULTIMARK, OnUpdateSavemultimark)
	ON_COMMAND(ID_RFWM, OnRfwm)
	ON_UPDATE_COMMAND_UI(ID_RFWM, OnUpdateRfwm)
	ON_COMMAND(ID_2NC, On2nc)
	ON_UPDATE_COMMAND_UI(ID_2NC, OnUpdate2nc)
	ON_UPDATE_COMMAND_UI(ID_MENUITEM32786, OnUpdateMenuitem32786)
	ON_COMMAND(ID_BITERROR, OnBiterror)
	ON_COMMAND(ID_NORMALPLAY, OnNormalplay)
	ON_COMMAND(ID_SLOWPLAY, OnSlowplay)
	ON_COMMAND(ID_FASTPLAY, OnFastplay)
	ON_COMMAND(ID_RECORD, OnRecord)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_WM_SIZE()
	ON_COMMAND(ID_BRIDGEEXTRACT, OnBridge_extract)
	ON_UPDATE_COMMAND_UI(ID_NORMALPLAY, OnUpdateNormalplay)
	ON_UPDATE_COMMAND_UI(ID_FASTPLAY, OnUpdateFastplay)
	ON_UPDATE_COMMAND_UI(ID_SLOWPLAY, OnUpdateSlowplay)
	ON_UPDATE_COMMAND_UI(ID_RECORD, OnUpdateRecord)
	ON_COMMAND(ID_ROBSEMIFRGWM, OnRobsemifrgwm)
	ON_COMMAND(ID_TRIMARK, OnTrimark)
	ON_COMMAND(ID_RFVIEW, OnRfview)
	ON_COMMAND(ID_MENUITEM32818, OnAttackMethod)
	ON_UPDATE_COMMAND_UI(ID_TRIMARK, OnUpdateTrimark)
	ON_UPDATE_COMMAND_UI(ID_RFVIEW, OnUpdateRfview)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_Encode, OnEncode)
	ON_COMMAND(ID_Decode, OnDecode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
// CMyView construction/destruction

CMyView::CMyView()

{
    m_font.CreateFont(
		50,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,                   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		"Arial");   

	hit_font.CreateFont(
		30,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,                   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		"Arial"); 

	dsp_font.CreateFont(
		25,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,                   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		"Arial"); 
	
	bFlag=0;
	vqwatermark=false;
	mvwatermark=false;
	multipurpose=false;
	robandfrg=false;
	rob_semf=false;
	keymark=false;
    h263=false;
	vqdecode=false;
    visualcode=false;
    basicextract=false;
	dmmvextract=false;
	rob_semifrg_extract=false;
	RoFgFp_extract=false;
	markfinish=false;
	multifinished=false;
    rofgfp_finished=false;
	watercodebook=false;
	ifshow=false;
	decode_record=false;
	ifbridge=false;
	ifopen=false;
	DCT_codebook=false;
	RoFrFp=false;
}

CMyView::~CMyView()
{
	m_font.DeleteObject();
	hit_font.DeleteObject();
	dsp_font.DeleteObject();
    clearmark();
	GlobalFree(image);
	GlobalFree(Markimage);
}

BOOL CMyView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMyView drawing

void CMyView::OnDraw(CDC* pDC)
{
	CDC dcMemory;
    dcMemory.CreateCompatibleDC(pDC);	

	if(bFlag==0)
	{  
	   CBrush cb;
	   cb.CreateSolidBrush(RGB(128,128,128));
	   pDC->SelectObject(&cb);
	   pDC->Rectangle(0,0,1400,800);
	   pDC->SelectObject(cb);
      
	   CFont *pOldFont;
	   pOldFont=pDC->SelectObject(&m_font);
	   CString title;
	   pDC->SetBkColor(RGB(128,128,128));
	   pDC->SetTextColor(RGB(255,255,255));
	   title.Format("Video Compression & Watermarking");
       pDC->TextOut(10,140,title);

	   CFont *pOldFonthit;
	   pOldFonthit=pDC->SelectObject(&hit_font);
	   pDC->SetBkColor(RGB(128,128,128));
	   pDC->SetTextColor(RGB(255,255,255));
	   title.Format("Harbin Institute of Technology");
       pDC->TextOut(180,260,title);

	   CFont *pOldFontdsp;
	   pOldFontdsp=pDC->SelectObject(&dsp_font);
       pDC->SetBkColor(RGB(128,128,128));
	   pDC->SetTextColor(RGB(255,255,255));
	   title.Format("Image Processing and Pattern Recognition Research Center");
       pDC->TextOut(80,300,title);
	   
	   pOldFont->DeleteObject();
	   //pOldFonthit->DeleteObject();
	   //pOldFontdsp->DeleteObject();
	}
	
	if(bFlag==1)//编码
	{
	    pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(RGB(255,0,0));
		pDC->StretchBlt(0,0,1024*3/4,768*3/4,&dcMemory,0,0,1024,768,SRCCOPY);
        CString kk, inform;
		kk.Format("Encoding... Totally %d frames",MaxFrame);
		pDC->TextOut(60,60,kk);
		if(ifgenerate)
        kk.Format("Designing VQ codebook...");
		else
		kk.Format("Every %d frames is intra coded.",Pbetween);
        pDC->TextOut(60,100,kk);
		inform.Format("See processing bar to konw encoding process!");
		pDC->TextOut(60,140,inform);
		
	}
    else if(bFlag==2)//解码
	{

	CBrush Brush(RGB(180,180,180));//190,190,190
	CBrush* oldBrush=pDC->SelectObject(&Brush);
    CPen pen(0,2,RGB(180,180,180));
	CPen *oldPen=pDC->SelectObject(&pen);
	pDC->Rectangle(40,40,440,400);//画矩形
	pDC->MoveTo(40,400); // (wide,height)
	pDC->LineTo(650,400);
	pDC->MoveTo(650,40);        
	pDC->LineTo(650,400);
	pDC->MoveTo(40,40);        
	pDC->LineTo(650,40);	
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
   
	  //输出解码结果
     if(bridge_extract)
	 {
		 pDC->SetTextColor(RGB(200,100,150));
		 CString info;
		 info.Format("Media Bridge Technology");
		 pDC->TextOut(460,220,info);
	 }

    	//显示解码图象
	 CDisplay m_display;
	 
	 m_display.play_movie(pDC,DecfileName,speed,decode_record,yuvfile);
	 
	 conclusion=m_display.conclusion;
    
	 pDC->SetTextColor(RGB(200,100,150));
	 int dd=conclusion.Find(",",0);
	 CString kk=conclusion.Right(conclusion.GetLength()-dd-1);
	 int zz=kk.Find(",",0);
	 pDC->TextOut(470,140,conclusion.Left(dd));
	 pDC->TextOut(470,160,kk.Left(zz));
	 pDC->TextOut(500,180,kk.Right(kk.GetLength()-zz-1));
      
	 unsigned char tempnum[512],tempnum2[512];
     int round=0,round2=0;
     int i,j,k,m;
	if((basicextract||dmmvextract||multiextract||dctparity||robfrgextract||rob_semifrg_extract)&&(ifshow))//显示水印
	{   
		pDC->SetTextColor(RGB(200,100,150));
		CString wm,rob,frg;
		wm.Format("Watermark");
		pDC->TextOut(510,220,wm);

		if(dctparity)
		{
		wm.Format("Secret Key");
		pDC->TextOut(510,300,wm);
		}  
        
		if(robfrgextract||rob_semifrg_extract)
		{
		 frg.Format("Fragile");
		 rob.Format("Robust");
         pDC->TextOut(565,280,frg);
		 pDC->TextOut(565,350,rob);
		}

        Markimage=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(64*64/8));
     
		//还原水印信息并加以显示
	   if(basicextract||multiextract||robfrgextract)//基本VQ水印
	   {  
		  m_width=64;
          m_height=64;
          for(m=0;m<m_width*m_height;m++)
		  {
		   singalbit[2*m+1]=markbit[m]%2;
	       markbit[m]=markbit[m]>>1;
	       singalbit[2*m]=markbit[m]%2;
		  }  
		 DPSEUDO(singalbit,bittoshow,64,64,4096,45);
	   }

	   if(rob_semifrg_extract)//码字索引受限
	   {
		 for(i=0;i<m_width*m_height;i++)
		 {
		   bittoshow[i]=frg_bit[i];
		   //if(bittoshow[i]==0)
		   //AfxMessageBox("=0");
		 }
		   //DPSEUDO(singalbit,bittoshow,64,64,4096,45);
           m_width=64;
           m_height=64;
	   }
	   
	   if(dmmvextract)//运动矢量水印
	   {
		  m_width=64;
          m_height=64;
          for(m=0;m<m_width*m_height/3;m++)
		  {
			singalbit[3*m+2]=trimark[m]%2;
		    trimark[m]=trimark[m]>>1;
		    singalbit[3*m+1]=trimark[m]%2;
            trimark[m]=trimark[m]>>1;
			singalbit[3*m]=trimark[m]%2;
		  }  
		  //DPSEUDO(singalbit,bittoshow,64,64,4096,45);
		  for(i=0;i<m_width*m_height;i++)
		  {
		    bittoshow[i]=singalbit[i];//bittoshow[i]反置乱后的二进制数
		  }
	   }
       
 
       for(i=0;i<m_width*m_height/8;i++)
	   {  	
		   unsigned char tempvar=0;
		   for(k=0;k<8;k++)
		   {
			   tempvar=tempvar+bittoshow[i*8+7-k]*(unsigned char)pow(2,k);	
		   }	
           tempnum[i]=tempvar;//tempnum[i]=0,255
	   }
	    
      for(i=0;i<m_height;i++)
	  {
		for(j=0;j<m_width/8;j++)
		{
		 *(Markimage+i*8+j)=tempnum[i*(m_width/8)+j];//*Markimage写入文件和用于显示的数据
		}
	  }

	   if (multiextract||dctparity||robfrgextract||rob_semifrg_extract) //显示多功能水印(DCT极性)
	   {   	
		   SndMarkimage=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(64*48/8));
		   m_width2=48;
           m_height2=48;
           round2=(m_width2%64)?(m_width2/64+1)*64:m_width2;
		   if(multiextract)
		   {
			   for(m=0;m<m_width2*m_width2/3;m++)
			   {
			   secondsingalbit[3*m+2]=secondtrimark[m]%2;
			   secondtrimark[m]=secondtrimark[m]>>1;
			   secondsingalbit[3*m+1]=secondtrimark[m]%2;
			   secondtrimark[m]=secondtrimark[m]>>1;
			   secondsingalbit[3*m]=secondtrimark[m]%2;
			   }  
           }

		   if(dctparity)
		   {
			 for(m=0;m<m_width2*m_width2;m++)
			 *(secondsingalbit+m)=*(keybit+m);
		   }
		   
          DPSEUDO(secondsingalbit,sndbittoshow,64,64,4096,45);

		   for(i=0;i<m_width2*m_width2/8;i++)
		   {  	
			   unsigned char tempvar=0;
			   for(k=0;k<8;k++)
			   {
				   tempvar=tempvar+sndbittoshow[i*8+7-k]*(unsigned char)pow(2,k);	
			   }	
			   tempnum2[i]=tempvar;
		   }

		   for(i=0;i<m_height2;i++)
		   {
			   for(j=0;j<m_width2/8;j++)
			   {
				 *(SndMarkimage+i*8+j)=tempnum2[i*(m_width2/8)+j];
			   }
			   *(SndMarkimage+i*8+round/8-2)=0;
			   *(SndMarkimage+i*8+round/8-1)=0;
		   }

		   multifinished=true;
	   }
        
         image=(BITMAPINFO *)GlobalAlloc(GMEM_FIXED,44);
		 if (image==NULL) 
		  AfxMessageBox("Memory is not enought");

	    //显示第1个基本水印
        image->bmiHeader.biSize=40;
		image->bmiHeader.biBitCount=1;
		image->bmiHeader.biCompression=0;
		image->bmiHeader.biSizeImage=round*m_height/8;
		image->bmiHeader.biHeight=m_height;
		image->bmiHeader.biWidth=m_width;
		image->bmiHeader.biClrUsed=0;
		image->bmiHeader.biClrImportant=0;
		image->bmiHeader.biPlanes=1;
	    image->bmiHeader.biXPelsPerMeter=3780;
		image->bmiHeader.biYPelsPerMeter=3780;
		
		image->bmiColors[0].rgbBlue=0;
		image->bmiColors[0].rgbGreen=0;
		image->bmiColors[0].rgbRed=0;
		image->bmiColors[0].rgbReserved=0;
		image->bmiColors[1].rgbBlue=255;
		image->bmiColors[1].rgbGreen=255;
		image->bmiColors[1].rgbRed=255;
		image->bmiColors[1].rgbReserved=0;
        
		::StretchDIBits(pDC->m_hDC,                                                
		510,
		250,
		m_width,
		m_height,
		0,
		0,
		m_width,
		m_height,
        Markimage,
	    image,
		DIB_RGB_COLORS,
		SRCCOPY);
        
        //显示第2个基本水印
    	if (multiextract||dctparity||robfrgextract||rob_semifrg_extract)
		{     
			::StretchDIBits(pDC->m_hDC,                                                
				510,
				320,
				m_width2,
				m_height2,
				0,
				0,
				m_width2,
				m_height2,
				SndMarkimage,
				image,
				DIB_RGB_COLORS,
				SRCCOPY);
		}
          markfinish=true;
     }//end of !RoFgFp_extract	
        
    if(RoFgFp_extract&&ifgo)//数字水印、数字指纹、脆弱水印
    {
      unsigned char tempnum3[512],tempnum4[512];
      unsigned char tempvar=0,tempvar2=0;
      m_height=64;
      m_width=64;
      m_height2=64;
      m_width2=64;
      round=(m_width%64)?(m_width/64+1)*64:m_width;
      round2=(m_width2%64)?(m_width2/64+1)*64:m_width2;
      memcpy(bittoshow,frg_bit,sizeof(unsigned char)*m_width*m_height);
      memcpy(sndbittoshow,I_frg_bit,sizeof(unsigned char)*m_width2*m_height2);
      for(i=0;i<m_width*m_height/8;i++)
      {  	
	     tempvar=0;
         for(k=0;k<8;k++)
         {
		    tempvar=tempvar+frg_bit[i*8+7-k]*(unsigned char)pow(2,k);
         }	
	    tempnum3[i]=tempvar;
      } 
      
      for(i=0;i<m_width2*m_height2/8;i++)
      {  	
	    tempvar2=0; 
        for(k=0;k<8;k++)
         {
            tempvar2=tempvar2+I_frg_bit[i*8+7-k]*(unsigned char)pow(2,k);
         }	
        tempnum4[i]=tempvar2;
      }
        
      Markimage=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(round*m_height));
      SndMarkimage=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(round2*m_height2));
      
      for(i=0;i<m_height;i++)
       for(j=0;j<m_width/8;j++)
         *(Markimage+i*8+j)=tempnum3[i*8+j];
    
      for(i=0;i<m_height2;i++)
       for(j=0;j<m_width2/8;j++)
         *(SndMarkimage+i*8+j)=tempnum4[i*8+j];

        image=(BITMAPINFO *)GlobalAlloc(GMEM_FIXED,44);
		if (image==NULL) 
		AfxMessageBox("Memory is not enought");

	    image->bmiHeader.biSize=40;
		image->bmiHeader.biBitCount=1;
		image->bmiHeader.biCompression=0;
		image->bmiHeader.biSizeImage=m_width*m_height/8;
		image->bmiHeader.biHeight=m_height;
		image->bmiHeader.biWidth=m_width;
		image->bmiHeader.biClrUsed=0;
		image->bmiHeader.biClrImportant=0;
		image->bmiHeader.biPlanes=1;
	    image->bmiHeader.biXPelsPerMeter=4724;
		image->bmiHeader.biYPelsPerMeter=4724;
		
		image->bmiColors[0].rgbBlue=0;
		image->bmiColors[0].rgbGreen=0;
		image->bmiColors[0].rgbRed=0;
		image->bmiColors[0].rgbReserved=0;
		image->bmiColors[1].rgbBlue=255;
		image->bmiColors[1].rgbGreen=255;
		image->bmiColors[1].rgbRed=255;
		image->bmiColors[1].rgbReserved=0;
        
		::StretchDIBits(pDC->m_hDC, //显示P帧脆弱水印                                            
		510,
		250,
		m_width,
		m_height,
		0,
		0,
		m_width,
		m_height,
        Markimage,
	    image,
		DIB_RGB_COLORS,
		SRCCOPY);

        ::StretchDIBits(pDC->m_hDC, //显示I帧脆弱水印                                               
		510,
		320,
		m_width2,
		m_height2,
		0,
		0,
		m_width2,
		m_height2,
        SndMarkimage,
	    image,
		DIB_RGB_COLORS,
		SRCCOPY);
      
        CString I_frg,P_frg;
        I_frg.Format("I_frame");
		P_frg.Format("P_frame");
        pDC->TextOut(585,300,I_frg);
		pDC->TextOut(585,370,P_frg);

        markfinish=true;
        multifinished=true;
        rofgfp_finished=true;
    }//end of RoFpFg_extract

   if(bridge_extract) //媒体桥 提取网址
   {   
	        
	   char extractedsite[50];
	   int bitnum=0;

	   for(int m=0;m<48*48;m++)
	   {
		   singalbit[2*m+1]=markbit[m]%2;
		   markbit[m]=markbit[m]>>1;
		   singalbit[2*m]=markbit[m]%2;
	   }  

	   for(i=0;;i++)
	   { 
		   unsigned char tempsymbol=0;
		   for(int k=0;k<8;k++)
		   {
			   tempsymbol+=singalbit[8*i+7-k]*(unsigned char)pow(2,k);
		   }
		   
		   extractedsite[i]=tempsymbol;
		   bitnum++;
		   if(extractedsite[i]=='\0')
			   break;
	   }
	   
	   CString eachsymbol,showsite;
	   
	   for(i=0;i<bitnum;i++)
	   {
		   eachsymbol.Format("%c",extractedsite[i]);
		   showsite+=eachsymbol;
	   }
	   
	   if(ifshow)
	   {
		   pDC->SetTextColor(RGB(200,100,150));
		   CString web,go;
		   go.Format("Going to ->");
		   web.Format("%s",showsite);
		   pDC->TextOut(470,270,showsite);
		   pDC->TextOut(500,250,go);
		   ShellExecute(NULL, "open", "Iexplore",showsite, NULL, SW_SHOWNORMAL);
	   }
	   
	   ifshow=false;
	   ifgo=false;

   }
	
 }// end of decode
}

/////////////////////////////////////////////////////////////////////////////
// CMyView printing

BOOL CMyView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMyView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMyView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMyView diagnostics

#ifdef _DEBUG
void CMyView::AssertValid() const
{
	CView::AssertValid();
}

void CMyView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMyDoc* CMyView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMyDoc)));
	return (CMyDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMyView message handlers

void CMyView::OnEncode() 
{

	CPropertyDlg properties("Encode Properties Setting",this,0);
	int ff;
	CString dd;
	clearmark();
	
	if(properties.DoModal()==IDOK)
	{
		m_szFilePathName=properties.encodeDlg.m_InitDir;
		m_szFileName=m_szFilePathName;
		// m_codebookName=properties.encodeDlg.m_codebookName;
		m_Type=properties.encodeDlg.typeindex;
		MaxFrame=properties.encodeDlg.m_MaxFrame;
        QP=properties.encodeDlg.m_QP;
		QPI=properties.encodeDlg.m_QPI;
		ifPsnr=properties.encodeDlg.m_ifPsnr;
		ifrecord=properties.encodeDlg.m_ifrecord;
		ifVQ=properties.encodeDlg.m_ifVQ;
		VisualVQ=properties.encodeDlg.m_VisualVQ;
		ifvisualbook=properties.encodeDlg.m_visualbook;
		ifgenerate=properties.encodeDlg.m_ifgenerate;
		Pbetween=properties.encodeDlg.m_Pbetween+1;
		ratecontrol=properties.m_page1.m_ifratecontrol;
		targetrate=properties.m_page1.m_bitrate;
		//watercodebook=properties.watmarDlg.m_watermarkbook;
		//ifbridge=properties.watmarDlg.m_choosebridge;
		//website=properties.watmarDlg.m_web;
	    
		switch(properties.watmarDlg.m_basic)
		{
		case 0:
			{
				if(properties.watmarDlg.m_algorithm=="VQ based video watermarking")
					vqwatermark=1;
				else if(properties.watmarDlg.m_algorithm=="MV based video watermarking")
					mvwatermark=1;
				else if(properties.watmarDlg.m_algorithm=="Multi-purpose video watermarking")
					multipurpose=1;
				else if(properties.watmarDlg.m_algorithm=="DCT parity based video watermarking")
					keymark=1;
			    else if(properties.watmarDlg.m_algorithm=="Robust & Fragile video watermark")
				{
					vqwatermark=1;
					robandfrg=1;
				}
				else if(properties.watmarDlg.m_algorithm=="Robust &Semi-fragile watermark")
				   rob_semf=1;
				else if(properties.watmarDlg.m_algorithm=="Robust,Fragile & Digital fingerprint")
                   RoFrFp=1;
				else
				{
					vqwatermark=0;
					mvwatermark=0;
					multipurpose=0;
					keymark=0;
					robandfrg=0;
					RoFrFp=0;
				}
			}
			break;
		case 1:
			{
			if(properties.watmarDlg.m_wmcodebook=="Generate codebook for VQ watermarking")
				watercodebook=true;
            if(properties.watmarDlg.m_wmcodebook=="Generate codebook for DCT-VQ watermarking")
                DCT_codebook=true;
			}
			break;
		case 2:
			ifbridge=true;
            website=properties.watmarDlg.m_web;
			break;
		default:
           ifgenerate=false;
           ifbridge=false;
		}
		
		switch(properties.m_page1.m_framerate)
		{
		case 1:
            ref_frame_rate=30.0;break;
		case 0:
            ref_frame_rate=25.0;break;
		case 2:
			ref_frame_rate=10.0;break;
		default:
			ref_frame_rate=25.0;
		}

		switch(properties.m_page1.m_rdopt) {
		case 0:
			rdoptimize=1;
			break;
		
		default:
            rdoptimize=0;
		}
         
		int kk=targetrate;
		ff=properties.m_page1.m_ifratecontrol;
		dd.Format("采用率控制，码率是%d，帧率是%f。",kk,ref_frame_rate);
        if(ff)
		MessageBox(dd);	
        
		if(ifbridge)
		 ConvertStrToChar(website);
	if(m_Type==2)
		CodeYUV();
    else
		CodeBmps();
	}
	else
		return;
}

void CMyView::CodeYUV()
{ 
	//率控制
  #ifndef OFFLINE_RATE_CONTROL
    float DelayBetweenFramesInSeconds;
    int CommBacklog;
  #else
    PictImage *stored_image = NULL;
    int start_rate_control;
  #endif

	CMyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	//初始化
	CProgressBar bar(_T("Encode Progress"), 50, MaxFrame);
	CFile file;
	if((file.Open(m_szFilePathName,CFile::modeRead|CFile::shareDenyNone))==NULL)
	{
		AfxMessageBox("Can not open the yuv file!");
		return;
	}

	PictImage *prev_image = NULL;
    PictImage *curr_image = NULL;
    PictImage *curr_recon = NULL;
    PictImage *prev_recon = NULL;
   	PictImage *filter_recon = NULL;
    
	int frame_no=0,first_frameskip=0;  
    int start=1;//从第1帧到第MaxFrame帧
    int orig_frameskip=1;//输入图像原始偏移
    int frameskip=1;//非发送帧
	int intel=0;//分段设计码书标记
	int *counter,*counter2,*intratimes;
    int I_Au_counter=0, &ref_I_Au=I_Au_counter;
	long int gopnum=0;
	int i;
    long int m=0/*训练矢量个数*/, bitsum=0/*总比特数*/;
    double lumpsnrsum=0, cbpsnrsum=0, crpsnrsum=0;
	int *codevector[MAXBOOKSIZE];
	void *hMem1,*hMem2;
	void *LfMem,*MfMem,*HfMem;
	int  IntraNumber[3]={0};//I帧DCT_VQ训练矢量个数
    int *Lf_codeword[2048*2],*Mf_codeword[1024*2],*Hf_codeword[1024*2];
    void *LcMem,*McMem,*HcMem;
    int Lbooksize=2048*2,MHbooksize=1024*2;
	
    dim_Low=4; dim_Mid=12; dim_Hig=48;

	long_vectors =0;//帧间编码的参数
	mv_outside_frame=0;
	counter=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int));
	*(counter)=0;//初始化次数置0
	counter2=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int));
	*(counter2)=0;//初始化次数置0
	intratimes=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int));
	*(intratimes)=0;//初始化次数置0

	Pict *pic = (Pict *)malloc(sizeof(Pict));
	Bits *bits = (Bits *)malloc(sizeof(Bits));
	//率控制
	Bits *total_bits = (Bits *)malloc(sizeof(Bits));
    Bits *intra_bits = (Bits *)malloc(sizeof(Bits));

    long int bit[27]={0}, sum[27]={0};
    int num[27]={0};

	pic->BQUANT = DEF_BQUANT;
    pic->seek_dist = DEF_SEEK_DIST;
    pic->use_gobsync = DEF_INSERT_SYNC;//=0
    pic->PB = 0;
    pic->TR = 0;
    pic->QP_mean = (float)0.0;
    pic->unrestricted_mv_mode = 0;
	pic->picture_coding_type =0; // PCT_INTRA;

    pic->source_format = SF_CIF;
	
	switch (pic->source_format) {
    case (SF_SQCIF):
      fprintf(stdout, "Encoding format: SQCIF (128x96)\n");
      pels = 128;
      lines = 96;
      break;
    case (SF_QCIF):
      fprintf(stdout, "Encoding format: QCIF (176x144)\n");
      pels = 176;
      lines = 144;
      break;
    case (SF_CIF):
      fprintf(stdout, "Encoding format: CIF (352x288)\n");
      pels = 352;
      lines = 288;
      break;
    case (SF_4CIF):
      fprintf(stdout, "Encoding format: 4CIF (704x576)\n");
      pels = 704;
      lines = 576;
      break;
    case (SF_16CIF):
      fprintf(stdout, "Encoding format: 16CIF (1408x1152)\n");
      pels = 1408;
      lines = 1152;
      break;
	case (SF_SIF):
		fprintf(stdout, "Encoding format: 16CIF (1408x1152)\n");
		pels = 352;
		lines = 240;
		break;
    default:
      fprintf(stderr,"Illegal coding format\n");
      exit(-1);
	}
    cpels = pels/2;
    m_orgHeight=lines;
	m_orgWidth=pels;
    m_pImageData=new unsigned char[pels*lines*3/2];
	curr_recon = InitImage(pels*lines);
	filter_recon=InitImage(pels*lines);

//率控制
  #ifndef OFFLINE_RATE_CONTROL
  // rate control variables 
  pic->bit_rate = targetrate;
  pic->src_frame_rate = (int)(ref_frame_rate / orig_frameskip);
  DelayBetweenFramesInSeconds = (float) 1.0/(float)pic->src_frame_rate;
  InitializeRateControl();
  #endif
  #ifdef OFFLINE_RATE_CONTROL
  start_rate_control = 0;//DEF_START_RATE_CONTROL;
  #else
  pic->target_frame_rate = (float)DEF_TARGET_FRAME_RATE;
  #endif
  CString tempname,recordfilename;
  CString outfilename=m_szFileName.Left(m_szFileName.GetLength()-4);

  int number;//decide how much memory to allocate
  if(ifvisualbook||ifgenerate)
	  number=4; 
  if(watercodebook)
	  number=4; //basic+3extend
 
  //建立码书输出文件
   if(ifgenerate||ifvisualbook||watercodebook)
	{	
	 CFileDialog dlg(FALSE,".VQbook",outfilename,OFN_OVERWRITEPROMPT,"vbk Files(*.vbk)|*.vbk|",NULL);
	 dlg.m_ofn.lStructSize=88;  
   
    if (dlg.DoModal()==IDOK)
	{
		bFlag=1;
		pDoc->UpdateAllViews(NULL);
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);
     hMem1=GlobalAlloc(GMEM_FIXED,MAXIMAGESIZE*64*sizeof(int));
     if(hMem1==NULL)
     {
		AfxMessageBox("无法分配内存!");
		return;
	 } 
	 for(int i=0;i<MAXIMAGESIZE;i++)
	 trainvector[i]=(int *)hMem1+i*64;
	}
	else
	{
	MessageBox("No VQ codebook file is saved.","系统提示",MB_OK);
	return;
	}
   }

   //建立DCT_VQ码书输出文件
    if(DCT_codebook)
	{	
	CFileDialog dlg(FALSE,".VQbook",outfilename,OFN_OVERWRITEPROMPT,"vbk Files(*.vbk)|*.vbk|",NULL);
	dlg.m_ofn.lStructSize=88;  
   
     if (dlg.DoModal()==IDOK)
	 {
		bFlag=1;
		pDoc->UpdateAllViews(NULL);
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);
	   
	 LfMem=GlobalAlloc(GMEM_FIXED,MAXIMAGESIZE*dim_Low*sizeof(int));
	 MfMem=GlobalAlloc(GMEM_FIXED,MAXIMAGESIZE*dim_Mid*sizeof(int));
	 HfMem=GlobalAlloc(GMEM_FIXED,MAXIMAGESIZE*dim_Hig*sizeof(int));

     if(LfMem==NULL)
     {
		AfxMessageBox("无法分配内存!");
		return;
	 } 
	 for(int i=0;i<MAXIMAGESIZE;i++)
	 Lowf_Trainvector[i]=(int *)LfMem+i*dim_Low;

	 if(MfMem==NULL)
     {
		AfxMessageBox("无法分配内存!");
		return;
	 }  
	 for(i=0;i<MAXIMAGESIZE;i++)
	 Midf_Trainvector[i]=(int *)MfMem+i*dim_Mid;

	 if(HfMem==NULL)
     {
		AfxMessageBox("无法分配内存!");
		return;
	 } 
	 for(i=0;i<MAXIMAGESIZE;i++)
	 Hghf_Trainvector[i]=(int *)HfMem+i*dim_Hig;
	 
	LcMem=GlobalAlloc(GMEM_FIXED,sizeof(int)*dim_Low*Lbooksize);
    McMem=GlobalAlloc(GMEM_FIXED,sizeof(int)*dim_Mid*MHbooksize);
    HcMem=GlobalAlloc(GMEM_FIXED,sizeof(int)*dim_Hig*MHbooksize);
    
	if(LcMem==NULL)
	{
	 MessageBox("Not enough memory!","System information",MB_ICONSTOP|MB_OK);
	 return;
	}
    if(McMem==NULL)
	{
	 MessageBox("Not enough memory!","System information",MB_ICONSTOP|MB_OK);
	 return;
	}
    if(HcMem==NULL)
	{
	 MessageBox("Not enough memory!","System information",MB_ICONSTOP|MB_OK);
	 return;
	}

    for(i=0;i<Lbooksize;i++)
	{
	  Lf_codeword[i]=(int*)LcMem+dim_Low*i;
	}
    
	for(i=0;i<MHbooksize;i++)
	{
	 Mf_codeword[i]=(int*)McMem+dim_Mid*i;
	 Hf_codeword[i]=(int*)HcMem+dim_Hig*i;
	}
   
	}
	else
	{
	  MessageBox("No VQ codebook file is saved.","系统提示",MB_OK);
	  return;
	}
	
   }

	//建立263输出文件
	if((!ifgenerate)&&(!ifvisualbook)) 
	{
	CString filetype;
    LPCSTR filefilter;
	
	if(vqwatermark||multipurpose||robandfrg||ifbridge||VisualVQ||rob_semf)
	{
	   filetype=".vvq";
       filefilter="Visual VQ+263 Files(*.vvq)|*.vvq|";
	}
	else if(ifVQ)
	{
	   filetype=".vq";
       filefilter="VQ+263 Files(*.vq)|*.vq|";
	}
    else if(RoFrFp)
    {
	   filetype=".DCTVQ";
       filefilter="DCTVQ+263 Files(*.DCTVQ)|*.DCTVQ|";
	}
    else
	{
	   filetype=".263";
       filefilter="263 Files(*.263)|*.263|";
	}

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT,filefilter,NULL);
	dlg.m_ofn.lStructSize=88;
    dlg.m_ofn.lpstrTitle="Save compressed video file";

	if (dlg.DoModal()==IDOK)
	{
		bFlag=1;
		pDoc->UpdateAllViews(NULL);
 
		CString tempname;
		tempname=dlg.GetFileName();
		tempname+=filetype;
	    if((streamfile.Open(tempname,CFile::modeWrite|CFile::modeCreate))==FALSE)
		AfxMessageBox("Can't create file!"); 
		streamfile.SeekToBegin();
	}
	else
    {
	    MessageBox("No file is saved.","系统提示",MB_OK);
	    return;
	}
    }//end of else ifgenerate 

	//模拟录制节目，输出YUV文件
	if(ifrecord) 
	{
		CFileDialog dlg(FALSE,".YUV",NULL,OFN_OVERWRITEPROMPT,"YUV Files(*.YUV)|*.YUV|",NULL);
		if (dlg.DoModal()==IDOK)
		{
     		CString tempname;
			tempname=dlg.GetPathName();
			recordfilename=tempname.Left(tempname.GetLength()-4);
			if((recordfile.Open(tempname,CFile::modeWrite|CFile::modeCreate))==FALSE)
				AfxMessageBox("Can't create file!"); 
			recordfile.SeekToBegin();
		}
		else
		{
		  MessageBox("No file is saved.","系统提示",MB_OK);
		  return;
		}
    }//end of if record
    
	initbits ();
		
    CTime StartTime=CTime::GetCurrentTime();
    CTimeSpan ElapsedTime;

    file.Read(m_pImageData,sizeof(BYTE)*pels*lines*3/2);
	pic->QUANT=QPI;
	curr_image = FillImage(m_pImageData);
	
    if((!DCT_codebook)&(!RoFrFp))
	curr_recon = CodeOneIntra(curr_image, QPI, bits, pic,intratimes,keymark,(robandfrg||rob_semf));
   	if(DCT_codebook)//DCT_codebook generation
    {
	   curr_recon =Intra_DCTbook_Gen(curr_image, bits, pic, Lowf_Trainvector,Midf_Trainvector,Hghf_Trainvector,IntraNumber);
	}
	if(RoFrFp)
    curr_recon =CodeIntra_DCTVQ(curr_image,bits, pic, Low_codeword,Mid_codeword,Hig_codeword,0,IntraNumber,ref_I_Au);

	bits->header += alignbits(); // pictures shall be byte aligned 

	//率控制
  AddBitsPicture(bits);
  memcpy(intra_bits,bits,sizeof(Bits));
  bitsum+=bits->total;
  ZeroBits(total_bits);
    //* number of seconds to encode *
  int chosen_frameskip=1;//jwp
    //* compute first frameskip *
  #ifdef OFFLINE_RATE_CONTROL
   float seconds = (MaxFrame - start + chosen_frameskip) * orig_frameskip/ ref_frame_rate;
   first_frameskip = chosen_frameskip;
   frameskip = chosen_frameskip;
  #else
  CommBacklog = intra_bits->total -(int)(DelayBetweenFramesInSeconds * pic->bit_rate);

  if (pic->bit_rate == 0) {
    frameskip = chosen_frameskip;
  }
  else {  //* rate control is used *
    frameskip = 1;
    while ( (int)(DelayBetweenFramesInSeconds*pic->bit_rate) <= CommBacklog) {
      CommBacklog -= (int) ( DelayBetweenFramesInSeconds * pic->bit_rate );
      frameskip += 1;
    }
  }
  first_frameskip = frameskip;
  #endif

    if(ifrecord)
	{
       recordfile.SeekToBegin();
       recordfile.Write(curr_recon->lum,pels*lines);
	   recordfile.Write(curr_recon->Cb,pels*lines/4);
	   recordfile.Write(curr_recon->Cr,pels*lines/4);
	}

    CString kk,m_Spsnr;
    if(ifPsnr)
	{
	if(psnrfile.Open(outfilename+".rtf",CFile::modeWrite|CFile::modeCreate)==FALSE)
    MessageBox("Cannot create the output psnr file!", "Error",MB_ICONERROR | MB_OK);
	SeekPsnr(curr_image,curr_recon,pels,lines,psnrs);
	frame_no=1;
	//kk.Format("第%d帧的lum峰值信噪比为%6.4fdB\n",frame_no,psnrs[0]);
    kk.Format("%4.2f ",psnrs[0]);
	m_Spsnr=kk;
	/*kk.Format("第%d帧的Cb峰值信噪比为%6.4fdB\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4fdB\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
    kk.Format("第%d帧的总比特数为%d\n", frame_no,bits->total);
    m_Spsnr+=kk;
	MessageBox(m_Spsnr);*/
	psnrfile.SeekToBegin();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
	}

   //第二帧
   pic->QUANT=QP;
   for(frame_no=start+first_frameskip;frame_no<=MaxFrame;frame_no+=frameskip)
   {
	file.Seek((frame_no-1)*pels*lines*3/2,SEEK_SET);
    file.Read(m_pImageData,sizeof(BYTE)*pels*lines*3/2);
	if(m_pImageData==NULL)
		  return;
  	  pic->picture_coding_type =1; // PCT_INTER;

	  if(!ratecontrol)
	  pic->QUANT=QP;
	  
      prev_image=curr_image;
	  prev_recon=curr_recon;
      curr_image = FillImage(m_pImageData);
	  pic->TR+=(((frameskip+(pic->PB?98:0))*orig_frameskip)%256);
	  if(frameskip+(pic->PB?98:0)>256)
	  MessageBox("Warning:frameskip>256");

     streamfile.SeekToEnd();
     if(((frame_no-1)%Pbetween)==0)
	 {
	   pic->picture_coding_type =0; //PCT_INTRA;
	   pic->QUANT=QPI;
       
	   if((!DCT_codebook)&&(!RoFrFp))
	   {
		curr_recon = CodeOneIntra(curr_image, QPI, bits, pic,intratimes,keymark,(robandfrg||rob_semf));
	    AddBitsPicture(bits);
	    bitsum+=bits->total;
	    memcpy(intra_bits,bits,sizeof(Bits));
	   }
       if(RoFrFp)
       {
		 curr_recon = CodeIntra_DCTVQ(curr_image,bits, pic, Low_codeword,Mid_codeword,Hig_codeword,(frame_no-1)/(Pbetween*2),IntraNumber,ref_I_Au);
	     AddBitsPicture(bits);
	     bitsum+=bits->total;
	     memcpy(intra_bits,bits,sizeof(Bits));
       }
	   if(DCT_codebook)//DCT_codebook generation
       {
		 curr_recon = Intra_DCTbook_Gen(curr_image, bits, pic, Lowf_Trainvector,Midf_Trainvector,Hghf_Trainvector,IntraNumber);
	     if((frame_no-Pbetween-1)%(2*Pbetween)==0)
		 {
		  DCTVQLLBGCodebookDesign(dim_Low,IntraNumber[0],2048,Lowf_Trainvector,Lf_codeword+intel*2048);
          DCTVQLLBGCodebookDesign_II(dim_Mid,IntraNumber[1],1024,Midf_Trainvector,Mf_codeword+intel*1024);
          //DCTVQLLBGCodebookDesign(dim_Hig,IntraNumber[2],1024,Hghf_Trainvector,Hf_codeword+intel*1024);
		  LBG(IntraNumber[2],Hghf_Trainvector,Hf_codeword+intel*1024,dim_Hig,1024);
          IntraNumber[0]=0;
		  IntraNumber[1]=0;
		  IntraNumber[2]=0;
		  intel++;
		 }
	   }
	 }
    else
	{ 
	 if(ifVQ)
	 {
	    //此段程序完成VQ对亮度残差块的编码
        CodeOneInterVQ(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,(frame_no/(Pbetween+1)));
	    AddBitsPicture(bits); 
		bitsum+=bits->total;
	 }
	 if(VisualVQ)
	 {
        CodeOneInterVisualVQ(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,(frame_no-1)/Pbetween,counter);
	    AddBitsPicture(bits);
		bitsum+=bits->total;
	 }
	 
     if(rdoptimize)
     {
		 RDOptCodeInter(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,sum,bit,num);
         AddBitsPicture(bits);
		 bitsum+=bits->total;
	 }
     
	 if(vqwatermark||multipurpose||robandfrg||ifbridge)//VQ video watermarking algorithm
     { 
		WatermarkVisualVQ(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,(frame_no-1)/Pbetween,counter,counter2,multipurpose);
	    AddBitsPicture(bits);
		bitsum+=bits->total;
     }
     
	 if(rob_semf)
	 {
		Semi_frg_Wm_VisualVQ(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,(frame_no-1)/Pbetween,counter);
	    AddBitsPicture(bits);
		bitsum+=bits->total;
	 }

	 if(ifvisualbook||watercodebook)
	 {  
        gopnum+=VisualBookGenerate(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,gopnum);

		if(frame_no==2)// 第一次生成开辟内存
		{
		hMem2=GlobalAlloc(GMEM_FIXED,64*(MaxFrame/Pbetween)*64*sizeof(int)*number);//每个GOP分配64个码字
	    if(hMem2==NULL)
        AfxMessageBox("无法分配内存!"); 
	    for(i=0;i<64*(MaxFrame/Pbetween)*number;i++)
		codevector[i]=(int*)hMem2+i*64;
		}

		if((((frame_no)%Pbetween)==0)&&(ifvisualbook))//每个GOP结束生成码字
		{
		 LBG(gopnum,trainvector,codevector+intel*256,64,256);
		 intel++;
		 gopnum=0;
		}
       
		if((((frame_no)%Pbetween)==0)&&(watercodebook))//生成用于水印的码书
		{
			LBG(gopnum,trainvector,codevector+intel*4*64,64,64);//Generate basic codebook
			//generate extend codebook and combine it with basic codebook
            Extendbook(codevector,intel*4*64,3,64);
			intel++;
			gopnum=0;
		}
	 }
	
	 if(ifgenerate)
     {
		 // 此段添加码书生成的程序
		 gopnum+=VisualBookGenerate(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,gopnum);
		 
		 if(frame_no==2)// 第一次生成开辟内存
		 {
			 hMem2=GlobalAlloc(GMEM_FIXED,256*(MaxFrame/Pbetween)*64*sizeof(int));//每个GOP分配64个码字
			 if(hMem2==NULL)
				 AfxMessageBox("无法分配内存!"); 
			 for(i=0;i<256*(MaxFrame/Pbetween);i++)
				 codevector[i]=(int*)hMem2+i*64;
		 }
		 
		 if(((frame_no)%Pbetween)==0)//每个GOP结束生存码字
		 {
			 LBG(gopnum,trainvector,codevector+intel*256,64,256);
			 intel++;
			 gopnum=0;
		 }
	 }
	 
	 if((!ifgenerate)&&(!ifVQ)&&(!VisualVQ)&&(!ifvisualbook)&&(!rdoptimize)&&(!vqwatermark)&&(!watercodebook)&&(!multipurpose)&&(!ifbridge)&&(!rob_semf)&&(!DCT_codebook))
	 {
	   CodeOneInter(prev_image,curr_image,prev_recon,curr_recon,pic->QUANT,frameskip,bits,pic,counter,counter2,mvwatermark);
	   AddBitsPicture(bits);
	   bitsum+=bits->total;
	 } 

	}
   bits->header += alignbits (); //* pictures shall be byte aligned *

   if(ifrecord)//节目录制
   {
       recordfile.SeekToEnd();
       recordfile.Write(curr_recon->lum,pels*lines);
	   recordfile.Write(curr_recon->Cb,pels*lines/4);
	   recordfile.Write(curr_recon->Cr,pels*lines/4);
   }
   
   if(ifPsnr)
   {
	SeekPsnr(curr_image,curr_recon,pels,lines,psnrs);
	//kk.Format("第%d帧的lum峰值信噪比为%6.4f\n",frame_no,psnrs[0]);
    kk.Format("%4.2f  ",psnrs[0]);
	m_Spsnr=kk;
	/*kk.Format("第%d帧的Cb峰值信噪比为%6.4f\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4f\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;*/
	
	lumpsnrsum+=psnrs[0];
	cbpsnrsum+=psnrs[1];
	crpsnrsum+=psnrs[2];

	psnrfile.SeekToEnd();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
   }
//率控制

   AddBits(total_bits, bits);
#ifndef OFFLINE_RATE_CONTROL
    if (pic->bit_rate != 0 && pic->PB)
      CommBacklog -= (int)
      ( DelayBetweenFramesInSeconds*pic->bit_rate ) * pdist;

    if (pic->bit_rate != 0) 
	{
      UpdateRateControl(bits->total);

      CommBacklog += bits->total;
      frameskip = 1;
      CommBacklog -= (int)
        (frameskip * DelayBetweenFramesInSeconds *pic->bit_rate);

      while ( (int)(DelayBetweenFramesInSeconds*pic->bit_rate) <= CommBacklog)
      {
        CommBacklog -= (int) ( DelayBetweenFramesInSeconds * pic->bit_rate );
        frameskip += 1;
      }
    }
#else
    //* Aim for the targetrate with a once per frame rate control scheme *
    if (targetrate != 0)
      if (frame_no - start > (MaxFrame - start) * start_rate_control/100.0)

        pic->QUANT = FrameUpdateQP(total_bits->total + intra_bits->total,
           bits->total / (pic->PB?2:1),
           (MaxFrame-frame_no) / chosen_frameskip ,
           pic->QUANT, targetrate, seconds);
   frameskip = chosen_frameskip;
#endif
      CString kkk,kk2;
	  kkk.Format("第%d帧的总比特数为%d\n", frame_no,bits->total);
	  kk2.Format("——————————————————————\n");
	 /* if(ifPsnr)
	  {
		  psnrfile.SeekToEnd();
	      psnrfile.Write(kkk,kkk.GetLength());
		  psnrfile.Write(kk2,kk2.GetLength());
	  }*/

	//显示进度信息
	CString str;
	str.Format("%d%% complete", frame_no*100/MaxFrame);
	bar.SetText(str);
	bar.StepIt();
    PeekAndPump(); //调用函数实现消息的转发
   }//end for frame_no

 if(ifgenerate)
 {	// 得到所有训练矢量,用LBG算法设计码书,并写进码书文件
   	CFileDialog dlg(FALSE,".VQbook",outfilename,OFN_OVERWRITEPROMPT,"vbk Files(*.vbk)|*.vbk|",NULL);
	dlg.m_ofn.lStructSize=88;
	if (dlg.DoModal()==IDOK)
	{
		bFlag=1;
		pDoc->UpdateAllViews(NULL);
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);
	}
	else
	{
	 MessageBox("No VQ codebook file is saved.","系统提示",MB_OK);
	 return;
	}

	FILE *fp;
	fp=fopen(outfilename,"wb");
	if(fp==NULL)
	AfxMessageBox("无法生成码书文件!"); 
	// 写码书头文件
    BOOKFILEHEADER  bfh;
	bfh.CodeBookSize=256*(MaxFrame/Pbetween);
	strcpy((char *)bfh.FileFlag,"256GRAY"); 
	bfh.PixelBits=8;
	bfh.ImageBlockHeight=8;
	bfh.ImageBlockWidth=8;
	fseek(fp,0,SEEK_SET);
    if(fwrite(&bfh,sizeof(BOOKFILEHEADER),1,fp)==NULL)
    {
	AfxMessageBox("无法写入码书头文件!"); 
	return;
	}
	fseek(fp,20,SEEK_SET);
	if(fwrite(&codevector[0][0],sizeof(int)*64,256*(100/Pbetween),fp)==NULL)
	{
	AfxMessageBox("无法写入码字!"); 
	return;
	}
	else
	AfxMessageBox("码书已成功生成!"); 
 }
 
 if(ifvisualbook||watercodebook)
 {	 
	CFileDialog dlg(FALSE,".VQbook",outfilename,OFN_OVERWRITEPROMPT,"vbk Files(*.vbk)|*.vbk|",NULL);
	dlg.m_ofn.lStructSize=88;
	if (dlg.DoModal()==IDOK)
	{
		bFlag=1;
		pDoc->UpdateAllViews(NULL);
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);
	}
	else
	{
		MessageBox("No VQ codebook file is saved.","系统提示",MB_OK);
	    return;
	}

	FILE *fp;
	fp=fopen(outfilename,"wb");
	if(fp==NULL)
	AfxMessageBox("无法生成码书文件!"); 
	// 写码书头文件
    BOOKFILEHEADER  bfh;
	bfh.CodeBookSize=64*(MaxFrame/Pbetween)*number;//此处需要根据是否为水印码书加以改动
	strcpy((char *)bfh.FileFlag,"256GRAY"); 
	bfh.PixelBits=8;
	bfh.ImageBlockHeight=8;
	bfh.ImageBlockWidth=8;
	fseek(fp,0,SEEK_SET);
    if(fwrite(&bfh,sizeof(BOOKFILEHEADER),1,fp)==NULL)
    {
	  AfxMessageBox("无法写入码书头文件!"); 
	  return;
	}
	fseek(fp,20,SEEK_SET);
	if(fwrite(&codevector[0][0],sizeof(int)*64,64*(MaxFrame/Pbetween)*number,fp)==NULL)
	{
	  AfxMessageBox("无法写入码字!"); 
	  return;
	}
	else
	AfxMessageBox("码书已成功生成!"); 
 }
 
 if(DCT_codebook)
 {
   	FILE *fp;
	fp=fopen(outfilename,"wb");
	if(fp==NULL)
	AfxMessageBox("无法生成码书文件!"); 
	// 写码书头文件
    DCTBOOKFILEHEADER header;
	Lbooksize=2048*intel; MHbooksize=1024*intel;
    header.CodebookSize[0]=Lbooksize;
	header.CodebookSize[1]=MHbooksize;
	header.CodebookSize[2]=MHbooksize;
	header.dimension[0]=dim_Low;
	header.dimension[1]=dim_Mid;
	header.dimension[2]=dim_Hig;

	strcpy((char *)header.FileFlag,"DCTBOOK"); 
	
	fseek(fp,0,SEEK_SET);
    if(fwrite(&header,sizeof(DCTBOOKFILEHEADER),1,fp)==NULL)
    {
	AfxMessageBox("无法写入码书头文件!"); 
	return;
	}

	fseek(fp,20,SEEK_SET);

	if(fwrite(&Lf_codeword[0][0],sizeof(int)*dim_Low,Lbooksize,fp)==NULL)
	{
	AfxMessageBox("无法写入码字!"); 
	return;
	}
   	if(fwrite(&Mf_codeword[0][0],sizeof(int)*dim_Mid,MHbooksize,fp)==NULL)
	{
	AfxMessageBox("无法写入码字!"); 
	return;
	}
	if(fwrite(&Hf_codeword[0][0],sizeof(int)*dim_Hig,MHbooksize,fp)==NULL)
	{
	AfxMessageBox("无法写入码字!"); 
	return;
	}
	
	AfxMessageBox("码书已成功生成!");
	
	GlobalFree(LcMem);
	GlobalFree(McMem);
	GlobalFree(HcMem);

 }

 if(keymark||robandfrg||rob_semf) //写密钥
 {
  KEYFILEHEADER keyheader;
  CFileDialog openkey(FALSE,".key",outfilename,OFN_HIDEREADONLY,"Key File(*.key)|*.key|",NULL);
  openkey.m_ofn.lStructSize=88;
  if(openkey.DoModal()==IDOK)
  {
    tempname=openkey.GetFileName();
	FILE *fp;
	if(fp=fopen(tempname,"wb"))
	{
	strcpy((char *)keyheader.FileFlag,"KeyFile"); 
	keyheader.keynumber=*intratimes;
	fseek(fp,0,SEEK_SET);
	if(fwrite(&keyheader,sizeof(KEYFILEHEADER),1,fp)==NULL)//密钥头文件
    MessageBox("Sorry! Unable to write file header.","System information",MB_ICONHAND|MB_OK);
	fseek(fp,20,SEEK_SET);
	if(fwrite(keybit,sizeof(unsigned char),*intratimes,fp)==NULL)
    MessageBox("Sorry! Unable to write key bit.","System information",MB_ICONHAND|MB_OK);
	}
	else
    MessageBox("Sorry! Unable to save key file.","System information",MB_ICONHAND|MB_OK);
  }
 }
//  pDoc->SetModifiedFlag(TRUE);
  streamfile.Close();
  file.Close();
  if(ifPsnr)
  psnrfile.Close();
  if(ifrecord)
  recordfile.Close();
  delete prev_image;
  delete prev_recon;
  delete curr_image;
  curr_recon=NULL;

  //  delete curr_recon;
  free(bits);
  free(pic);
  GlobalFree(hMem1);
  GlobalFree(hMem2);
  GlobalFree(counter);
  GlobalFree(counter2);
  GlobalFree(intratimes);
 
    long hours,minutes,second;//计算所用的时间
    ElapsedTime = CTime::GetCurrentTime() - StartTime;
	bFlag=0;    
	hours = ElapsedTime.GetTotalHours();
    minutes = ElapsedTime.GetTotalMinutes();
    second = ElapsedTime.GetTotalSeconds();
    second = second + 60*minutes + 3600*hours;
	lumpsnrsum=lumpsnrsum/(frame_no-1);//统计平均信躁比 
    cbpsnrsum=cbpsnrsum/(frame_no-1);
	crpsnrsum=crpsnrsum/(frame_no-1);
	csTimeElapse.Format("编码%d帧视频序列,耗时:%d秒,\n%d帧的总比特数为%ld,\nY分量平均PSNR=%f,Cb分量平均PSNR=%f,Cr分量平均PSNR=%f,counter=%d",
		frame_no-1,second,frame_no-1,bitsum,lumpsnrsum,cbpsnrsum,crpsnrsum,*counter);
    MessageBox(csTimeElapse);
	//pDoc->UpdateAllViews(NULL);
//	delete m_pImageData;
}


void CMyView::CodeBmps()
{
   //初始化
	CMyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CProgressBar bar(_T("Encode Progress"), 50, MaxFrame);

    m_pImageData=new unsigned char[352*288+352*288/2];
	m_pImageData = OpenImageFile(m_szFilePathName, &m_orgWidth, &m_orgHeight, m_Type);

    PictImage *prev_image = NULL;
    PictImage *curr_image = NULL;
    PictImage *curr_recon = NULL;
    PictImage *prev_recon = NULL;

	int frame_no,first_frameskip=0;  
    int start=2;//从第1帧到第MaxFrame帧
    int orig_frameskip=1;//输入图像原始偏移
    int frameskip=1;//非发送帧
	int *counter,*counter2;//嵌入水印的次数

	long_vectors =0;//帧间编码的参数
	mv_outside_frame=0;
	counter=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int));
	*(counter)=0;//初始化次数置0
	counter2=(int *)GlobalAlloc(GMEM_FIXED,sizeof(int));
	*(counter2)=0;//初始化次数置0

    Pict *pic = (Pict *)malloc(sizeof(Pict));
	Bits *bits = (Bits *)malloc(sizeof(Bits));
	pic->BQUANT = DEF_BQUANT;
    pic->seek_dist = DEF_SEEK_DIST;
    pic->use_gobsync = DEF_INSERT_SYNC;//=0
    pic->PB = 0;
    pic->TR = 0;
    pic->QP_mean = (float)0.0;
    pic->unrestricted_mv_mode = 0;
	pic->picture_coding_type =0; // PCT_INTRA;
	if((m_orgHeight==288)&&(m_orgWidth==352))
       pic->source_format = SF_CIF;
	switch (pic->source_format) {
    case (SF_SQCIF):
      fprintf(stdout, "Encoding format: SQCIF (128x96)\n");
      pels = 128;
      lines = 96;
      break;
    case (SF_QCIF):
      fprintf(stdout, "Encoding format: QCIF (176x144)\n");
      pels = 176;
      lines = 144;
      break;
    case (SF_CIF):
      fprintf(stdout, "Encoding format: CIF (352x288)\n");
      pels = 352;
      lines = 288;
      break;
    case (SF_4CIF):
      fprintf(stdout, "Encoding format: 4CIF (704x576)\n");
      pels = 704;
      lines = 576;
      break;
    case (SF_16CIF):
      fprintf(stdout, "Encoding format: 16CIF (1408x1152)\n");
      pels = 1408;
      lines = 1152;
      break;
    default:
      fprintf(stderr,"Illegal coding format\n");
      exit(-1);
	}
    cpels = pels/2;
	curr_recon = InitImage(pels*lines);

	//建立输出文件
	CString outfilename=m_szFileName.Left(m_szFileName.GetLength()-4);
	CFileDialog dlg(FALSE,".263",outfilename,OFN_OVERWRITEPROMPT,"263 Files(*.263)|*.263|",NULL);
	dlg.m_ofn.lStructSize=88;
	if (dlg.DoModal()==IDOK)
	{  
		bFlag=1;
		pDoc->UpdateAllViews(NULL);

        CString tempname;
		tempname=dlg.GetPathName();
		outfilename=tempname.Left(tempname.GetLength()-4);

	    if((streamfile.Open(tempname,CFile::modeWrite|CFile::modeCreate))==FALSE)
			AfxMessageBox("Can't create file!"); 
		streamfile.SeekToBegin();
     	initbits ();
		
		CTime StartTime=CTime::GetCurrentTime();
        CTimeSpan ElapsedTime;

	pic->QUANT=QPI;
	curr_image = FillImage(m_pImageData);
	curr_recon = CodeOneIntra(curr_image, QPI, bits, pic,counter,0,0);
	bits->header += alignbits (); /* pictures shall be byte aligned */

	CString kk,m_Spsnr;
   if(ifPsnr)
   {
	if(psnrfile.Open(outfilename+".rtf",CFile::modeWrite|CFile::modeCreate)==FALSE)
    MessageBox("Cannot create the output psnr file!", "Error",MB_ICONERROR | MB_OK);
	SeekPsnr(curr_image,curr_recon,pels,lines,psnrs);
	frame_no=1;
	kk.Format("第%d帧的lum峰值信噪比为%6.4f\n",frame_no,psnrs[0]);
	m_Spsnr=kk;
	kk.Format("第%d帧的Cb峰值信噪比为%6.4f\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4f\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
	MessageBox(m_Spsnr);
	psnrfile.SeekToBegin();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
   }

	 //第二帧 
  for(frame_no=start+first_frameskip;frame_no<=MaxFrame;frame_no+=frameskip)
	{
  	  m_szFilePathName = GetNextFileName(m_szFilePathName,1);
   	  m_pImageData = OpenImageFile(m_szFilePathName, &m_orgWidth, &m_orgHeight, m_Type);
	  if(m_pImageData==NULL)
		  return;
	   
  	  pic->picture_coding_type =1; // PCT_INTER;
	  pic->QUANT=QP;
      prev_image=curr_image;
	  prev_recon=curr_recon;
      curr_image = FillImage(m_pImageData);
	  pic->TR+=(((frameskip+(pic->PB?98:0))*orig_frameskip)%256);
	  if(frameskip+(pic->PB?98:0)>256)
		MessageBox("Warning:frameskip>256");

     streamfile.SeekToEnd();
     if(((frame_no-1)%Pbetween)==0)
	 {
	   pic->picture_coding_type =0; // PCT_INTRA;
	   pic->QUANT=QPI;
       curr_recon = CodeOneIntra(curr_image, QPI, bits, pic,counter,0,0);
	 }
    else
    CodeOneInter(prev_image,curr_image,prev_recon,curr_recon,QP,frameskip,bits,pic,counter,counter2,0);
    bits->header += alignbits (); /* pictures shall be byte aligned */
	
   if(ifPsnr)
   {	
	SeekPsnr(curr_image,curr_recon,pels,lines,psnrs);
	kk.Format("第%d帧的lum峰值信噪比为%6.4f\n",frame_no,psnrs[0]);
	m_Spsnr=kk;
	kk.Format("第%d帧的Cb峰值信噪比为%6.4f\n",frame_no,psnrs[1]);
	m_Spsnr+=kk;
	kk.Format("第%d帧的Cr峰值信噪比为%6.4f\n",frame_no,psnrs[2]);
	m_Spsnr+=kk;
	psnrfile.SeekToEnd();
	psnrfile.Write(m_Spsnr,m_Spsnr.GetLength());
   }

	//显示进度信息
	CString str;
	str.Format("%d%% complete", frame_no*100/MaxFrame);
	bar.SetText(str);
	bar.StepIt();
    PeekAndPump(); //调用函数实现消息的转发

   }//end for frame_no

//  pDoc->SetModifiedFlag(TRUE);
  if(ifPsnr)
  psnrfile.Close();
  streamfile.Close();
  delete prev_image;
  delete prev_recon;
  delete curr_image;
  curr_recon=NULL;
//  delete curr_recon;
  free(bits);
  free(pic);
  GlobalFree(counter);
  GlobalFree(counter2);
    long hours,minutes,second;//计算所用的时间
    ElapsedTime = CTime::GetCurrentTime() - StartTime;
	bFlag=0;
    hours = ElapsedTime.GetTotalHours();
    minutes = ElapsedTime.GetTotalMinutes();
    second = ElapsedTime.GetTotalSeconds();
    second = second + 60*minutes + 3600*hours;

    csTimeElapse.Format("编码%d帧图像序列,耗时:%d秒！",frame_no-1,second);
    MessageBox(csTimeElapse);
	pDoc->UpdateAllViews(NULL);

	}
	else
    MessageBox("No file is saved.","系统提示",MB_OK);
	delete m_pImageData;
}

BOOL CMyView::PeekAndPump()
{

	static MSG msg;
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) 
	{
		if (!AfxGetApp()->PumpMessage()) 
		{
			::PostQuitMessage(0);
			return FALSE;
		}	
	}
	return TRUE;

}

void CMyView::OnDecode() 
{	
	CMyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CDC *pDC=GetDC();
    
	bFlag=0;
	pDoc->UpdateAllViews(NULL);

	CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"263 Files(*.263)|*.263|");
	decfile.m_ofn.lpstrTitle="打开263文件";
	decfile.m_ofn.lStructSize=88;
	clearmark();
	if(decfile.DoModal()==IDOK)
	{
	DecfileName=decfile.GetPathName();//获取263文件路径
	firstframe=1;
	bFlag=2;
	h263=true;
    ifopen=true;
	pDoc->UpdateAllViews(NULL);
	}
	else
	return;
}


void SeekPsnr(PictImage *curr, PictImage *recon, int width,int height, double psnr[3])
{
	int i;
	double MSE1=0;
	double MSE2=0;

    for(i=0;i<width*height;i++)
	    MSE1+=((double)recon->lum[i]-(double)curr->lum[i])
			    *((double)recon->lum[i]-(double)curr->lum[i]);
	MSE1/=(width*height);
	if(MSE1==0)
		psnr[0]=0;
	else
		psnr[0]=10*log10(255*255/MSE1);

	MSE1=0;
	for(i=0;i<((width*height)>>2);i++)
	{
		MSE1+=((double)recon->Cb[i]-(double)curr->Cb[i])
			    *((double)recon->Cb[i]-(double)curr->Cb[i]);
		MSE2+=((double)recon->Cr[i]-(double)curr->Cr[i])
			    *((double)recon->Cr[i]-(double)curr->Cr[i]);
	}  
	MSE1/=(width*height)>>2;
	MSE2/=(width*height)>>2;
	if(MSE1==0)
		psnr[1]=0;
	else
		psnr[1]=10*log10(255*255/MSE1);
	if(MSE2==0)
		psnr[2]=0;
	else
		psnr[2]=10*log10(255*255/MSE2);

}



void CMyView::OnFileOpen() 
{
	CDC* pDC;
    pDC=GetDC();

	CFile look_result;
	CString resultshow="";
	CString kk;
   	CFileDialog lookresult(TRUE,NULL,NULL,OFN_HIDEREADONLY,"文本 Files(*.txt;*.doc)|*.txt;*.doc|");
	lookresult.m_ofn.lpstrTitle="打开编码结果文件";
	lookresult.m_ofn.lpstrInitialDir="F:\\standard_pictures\\MISSUSA_raw";
	if(lookresult.DoModal()==IDOK)
	{
	    resultshow=lookresult.GetPathName();
	    if (look_result.Open(resultshow,CFile::modeRead)==NULL)
		{
	    AfxMessageBox("Can not open the result file!");
	    return;
		}
	    look_result.SeekToBegin(); 
	}
	
}




void CMyView::OnDecodevq() 
{
	// TODO: Add your command handler code here
CMyDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

bFlag=0;
pDoc->UpdateAllViews(NULL);

CEncodeDlg encode;
encode.Openbook2();
CDC *pDC=GetDC();
CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"VQ+263 Files(*.vq)|*.vq|");
decfile.m_ofn.lpstrTitle="打开263文件";
decfile.m_ofn.lStructSize=88;
clearmark();	
if(decfile.DoModal()==IDOK)
{DecfileName=decfile.GetPathName();//获取263文件路径
	firstframe=1;
bFlag=2;
vqdecode=true;
ifopen=true;
pDoc->UpdateAllViews(NULL);}
else
 return;	
}


void CMyView::OnUpdateDecode(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
 
	if(h263)
		pCmdUI->SetCheck(1);
 	else
 	    pCmdUI->SetCheck(0);
}

void CMyView::OnUpdateDecodevq(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
		if(vqdecode)
		pCmdUI->SetCheck(1);
 	else
 	    pCmdUI->SetCheck(0);
}

BOOL CMyView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CView::OnDrop(pDataObject, dropEffect, point);
}

void CMyView::OnVisualdec() /*基于视觉的VQ解码*/
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

	bFlag=0;
	pDoc->UpdateAllViews(NULL);

    CEncodeDlg encode;
    encode.Openbook2();
    CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Visual VQ+263 Files(*.vvq)|*.vvq|");
    decfile.m_ofn.lpstrTitle="打开263文件";
	decfile.m_ofn.lStructSize=88;
	clearmark();
    if(decfile.DoModal()==IDOK)
    {DecfileName=decfile.GetPathName();//获取263文件路径
		firstframe=1;
    bFlag=2;
    visualcode=true;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);}
	else
	 return;
}

void CMyView::OnUpdateVisualdec(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
		if(visualcode)
		pCmdUI->SetCheck(1);
    	else
 	    pCmdUI->SetCheck(0);
}


void CMyView::ExtractWatermark() //提取水印信号
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

	bFlag=0;
	pDoc->UpdateAllViews(NULL);

    CEncodeDlg encode;
    encode.Openbook2();
    CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Visual VQ+263 Files(*.vvq)|*.vvq|");
    decfile.m_ofn.lpstrTitle="打开263文件";
		decfile.m_ofn.lStructSize=88;
			clearmark();
    if(decfile.DoModal()==IDOK)
	{
	DecfileName=decfile.GetPathName();//获取263文件路径
	firstframe=1;
    bFlag=2;
	basicextract=true;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);	
	}
	else
	 return;
}

void CMyView::OnMvDM() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

	bFlag=0;
	pDoc->UpdateAllViews(NULL);

    CEncodeDlg encode;
    CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"263 Files(*.263)|*.263|");
    decfile.m_ofn.lpstrTitle="打开263文件";
		decfile.m_ofn.lStructSize=88;
			clearmark();
    if(decfile.DoModal()==IDOK)
	{DecfileName=decfile.GetPathName();//获取263文件路径
    bFlag=2;
	dmmvextract=true;
	h263=true;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);}
	else
		return;
}

void CMyView::clearmark()
{
	vqwatermark=false;
	mvwatermark=false;
    h263=false;
	vqdecode=false;
    visualcode=false;
	RoFrFp=false;
    basicextract=false;
	dmmvextract=false;
	multiextract=false;
	dctparity=false;
	robfrgextract=false;
	bridge_extract=false;
    rob_semifrg_extract=false;
	RoFgFp_extract=false;

	ifPsnr=false;
	ifrecord=false;
	ifVQ=false;
	VisualVQ=false;
	ifvisualbook=false;
	ifgenerate=false;
	ifbridge=false;
	ifopen=false;
	ifgo=false;
	ifshow=false;
}

void CMyView::OnMultiExtract() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

	bFlag=0;
	pDoc->UpdateAllViews(NULL);

    CEncodeDlg encode;
    encode.Openbook2();
    CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Visual VQ+263 Files(*.vvq|*.vvq|");
    decfile.m_ofn.lpstrTitle="打开263文件";
	decfile.m_ofn.lStructSize=88;
		clearmark();
    if(decfile.DoModal()==IDOK)
	{
	DecfileName=decfile.GetPathName();//获取263文件路径
	firstframe=1;
    bFlag=2;
	multiextract=true;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);	
	}
	else
	return;
}



void CMyView::OnUpdateMultiExtract(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(multiextract)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}


void CMyView::OnUpdateMvdm(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(dmmvextract)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
	
}

void CMyView::OnUpdateMenuitem32783(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(basicextract)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMyView::OnCalNC() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	bFlag=0;
    pDoc->UpdateAllViews(NULL);	
	NCdialogue nc;
	nc.DoModal();
}

void CMyView::OnDctparity() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    
	bFlag=0;
	pDoc->UpdateAllViews(NULL);

	CString keyfilename;
	KEYFILEHEADER keyheader;
	FILE *fp;
	CFileDialog openkey(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Key File(*.key)|*.key|");
	openkey.m_ofn.lStructSize=88;
	if(openkey.DoModal()==IDOK)
	{
	  keyfilename=openkey.GetFileName();
      if(fp=fopen(keyfilename,"rb"))
	  { 
		fseek(fp,0,SEEK_SET);
	    if(fread(&keyheader,sizeof(KEYFILEHEADER),1,fp)==NULL)//读文件头错误
		{MessageBox("Sorry! Unable to open key file","System information",MB_ICONHAND|MB_OK);
		return;}
        else
		{
		  if((strcmp(keyheader.FileFlag,"KeyFile")))//格式错误
          {MessageBox("You have open a key file with incorrect format","System information",MB_ICONHAND|MB_OK);
		  return;}
          else
		  { 
			fseek(fp,20,SEEK_SET);
            if(fread(keybit,sizeof(unsigned char),keyheader.keynumber,fp)==NULL)
			{MessageBox("Sorry! Unable to read key bits","System information",MB_ICONHAND|MB_OK);
			return;}
            else
			MessageBox("You have successfully open a key file!","System information",MB_ICONINFORMATION|MB_OK);
		  }
		   
		}
	  }
	 else//打开文件失败
	 {
	  MessageBox("Sorry! Unable to open key file","System information",MB_ICONHAND|MB_OK);
	  return;
	 }
	}
	
	CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"263 Files(*.263)|*.263|");
    decfile.m_ofn.lpstrTitle="打开263文件";
    decfile.m_ofn.lStructSize=88;
	clearmark();
    if(decfile.DoModal()==IDOK)
	{
	DecfileName=decfile.GetPathName();//获取263文件路径
	firstframe=1;
    bFlag=2;
	dctparity=1;
	h263=1;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);
	}
	else
	  return;
}

void CMyView::OnUpdateDctparity(CCmdUI* pCmdUI) 
{
	// TODO: Add your  command update UI handler code here
	if(dctparity)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMyView::Record(PictImage *recon, int width, int height, CFile file)
{
 int i;
 for(i=0;i<width*height;i++) //Y
  file.Write(&recon->lum[i],sizeof(unsigned char));
 for(i=0;i<width*height/4;i++)
  file.Write(&recon->Cb[i],sizeof(unsigned char));
 for(i=0;i<width*height/4;i++)
  file.Write(&recon->Cr[i],sizeof(unsigned char));
}


void CMyView::Savwmark() 
{
	// TODO: Add your command handler code here
	bFlag=0;	
	CMyDoc *pDoc=GetDocument();
	ASSERT(pDoc);
	pDoc->UpdateAllViews(NULL);
    if(!SaveMark(m_width,m_height,Markimage,image))
     MessageBox("Sorry, the extracted watermark can not be saved!","Information system", MB_ICONINFORMATION|MB_OK);
	markfinish=false;
}

void CMyView::OnUpdateSavwmark(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(markfinish)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CMyView::Savemultimark() 
{
	// TODO: Add your command handler code here
	bFlag=0;
	CMyDoc *pDoc=GetDocument();
	ASSERT(pDoc);
	pDoc->UpdateAllViews(NULL);
    
    if(!SaveMark(m_width,m_height,Markimage,image))
    MessageBox("Sorry, the extracted watermark can not be saved!","Information system", MB_ICONINFORMATION|MB_OK);

    if(!SaveMark(m_width2,m_height2,SndMarkimage,image))
    MessageBox("Sorry, the extracted watermark can not be saved!","Information system", MB_ICONINFORMATION|MB_OK);
	
    markfinish=false;
}

void CMyView::OnUpdateSavemultimark(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(multifinished)
	 pCmdUI->Enable(true);
	else
	 pCmdUI->Enable(false);
}

void CMyView::OnRfwm() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	
	bFlag=0;
	pDoc->UpdateAllViews(NULL);

	CString keyfilename;
	KEYFILEHEADER keyheader;
	FILE *fp;
	CFileDialog openkey(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Key File(*.key)|*.key|");
	openkey.m_ofn.lpstrTitle="打开密钥文件";
	openkey.m_ofn.lStructSize=88; 
	if(openkey.DoModal()==IDOK)
	{
		keyfilename=openkey.GetFileName();
		if(fp=fopen(keyfilename,"rb"))
		{ 
			fseek(fp,0,SEEK_SET);
			if(fread(&keyheader,sizeof(KEYFILEHEADER),1,fp)==NULL)//读文件头错误
			{MessageBox("Sorry! Unable to open key file","System information",MB_ICONHAND|MB_OK);
			return;}
			else
			{
				if((strcmp(keyheader.FileFlag,"KeyFile")))//格式错误
				{MessageBox("You have open a key file with incorrect format","System information",MB_ICONHAND|MB_OK);
				return;}
				else
				{ 
					fseek(fp,20,SEEK_SET);
					if(fread(keybit,sizeof(unsigned char),keyheader.keynumber,fp)==NULL)
					{MessageBox("Sorry! Unable to read key bits","System information",MB_ICONHAND|MB_OK);
					return;}
					else
						MessageBox("You have successfully open a key file!","System information",MB_ICONINFORMATION|MB_OK);
				}
				
			}
		}
		else//打开文件失败
		{
			MessageBox("Sorry! Unable to open key file","System information",MB_ICONHAND|MB_OK);
			return;
		}
	}
	else
	return;

	CEncodeDlg encode;// 打开码书
    encode.Openbook2();	
	CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Visual VQ+263 Files(*.vvq)|*.vvq|");
    decfile.m_ofn.lpstrTitle="打开含水印的263文件";
	decfile.m_ofn.lStructSize=88;
		clearmark();
    if(decfile.DoModal()==IDOK)
		DecfileName=decfile.GetPathName();//获取263文件路径
	else
		return;
	firstframe=1;
    bFlag=2;
    robfrgextract=1;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);	
}


void CMyView::OnUpdateRfwm(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(robfrgextract)
	 pCmdUI->SetCheck(1);
	else
	 pCmdUI->SetCheck(0);
}

void CMyView::On2nc() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	bFlag=0;
    pDoc->UpdateAllViews(NULL);	
	MultiNC mnc;
	mnc.DoModal();
}

void CMyView::OnUpdate2nc(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(multifinished)
	 pCmdUI->Enable(true);
	else
	 pCmdUI->Enable(false);
}

void CMyView::OnUpdateMenuitem32786(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(markfinish)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CMyView::OnBiterror() 
{
	// TODO: Add your command handler code here
	NoiseAttack noise;
	noise.DoModal();
}

void CMyView::DPSEUDO(unsigned char *pmwmdata,unsigned char *rewmdata,int N1,int N2,int number,int seed)
{
	int bit[18];
	int i,j,scale,temp;
	int tempkey;
	int sequence[4096];
	if (number<4||number>(65536*2)) return;
	
    //取得m序列的阶数
	scale=0;//scale级反馈移位寄存器--用于产生长为2^scale-1的m序列     
    temp=number;
    while(temp!=1)//求阶数
	{    
		temp=temp/2;
		scale=scale+1; 
	}
	
	tempkey=seed;//置随机序列的种子
	
	for (i=0;i<4095;i++) //周期为64*64,产生4095个伪随机值（种子给定）
	{
		for(j=0;j<scale;j++) //取各位0/1值-->bit[i]
		{
			temp=tempkey>>j;
			bit[j]=temp&1;
		}
		switch(scale)//作模2加法
		{
		case 2:
			temp=bit[0]+bit[1];
			break;
		case 3:
			temp=bit[0]+bit[2];
			break;
		case 4:
			temp=bit[0]+bit[3];
			break;
		case 5:
			temp=bit[0]+bit[3];
			break;
		case 6:
			temp=bit[0]+bit[5];
			break;
		case 7:
			temp=bit[0]+bit[4];
			break;
		case 8:
			temp=bit[0]+bit[4]+bit[5]+bit[6];
			break;
		case 9:
			temp=bit[0]+bit[5];
			break;
		case 10:
			temp=bit[0]+bit[7];
			break;
		case 11:
			temp=bit[0]+bit[9];
			break;
		case 12:
			temp=bit[0]+bit[6]+bit[8]+bit[11];
			break;
		case 13:
			temp=bit[0]+bit[9]+bit[10]+bit[12];
			break;
		case 14:
			temp=bit[0]+bit[4]+bit[8]+bit[13];
			break;
		case 15:
			temp=bit[0]+bit[14];
			break;
		case 16:
			temp=bit[0]+bit[4]+bit[13]+bit[15];
			break;
		case 17:
			temp=bit[0]+bit[14];
			break;
		default:
			break;
		}
		bit[scale]=temp&1;
		tempkey=(int)(bit[scale]*(pow(2,(scale-1)))+(tempkey>>1));
		sequence[i]=tempkey;
		
	}
	sequence[4095]=0;
	int k=0;
	for(i=0;i<N1*N2;i++)
	{
		j=sequence[i];
		if(j<48*48)
		{
		  rewmdata[k]=pmwmdata[j];
		  k++;
		}
	}
}


void CMyView::OnNormalplay() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	speed=30;
	ifgo=true;
	ifshow=true;
	pDoc->UpdateAllViews(NULL);	
}

void CMyView::OnSlowplay() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	speed=100;
	ifgo=true;
	ifshow=true;
	pDoc->UpdateAllViews(NULL);	
}

void CMyView::OnFastplay() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	speed=5;
	ifgo=true;
	ifshow=true;
	pDoc->UpdateAllViews(NULL);	
}

void CMyView::OnRecord() 
{
	// TODO: Add your command handler code here
    CMyDoc *pDoc=GetDocument();
    ASSERT_VALID(pDoc);

	CFileDialog record(FALSE,NULL,NULL,NULL,"YUV video file (*.yuv)|*.yuv|");
	record.m_ofn.lpstrTitle="Record decoded video into YUV file";
	record.m_ofn.lStructSize=88;

	if(record.DoModal()==IDOK)
	{
	   CString yuvname;
	   yuvname=record.GetFileName();
	   yuvname+=".yuv";
	   
	   if(yuvfile.Open(yuvname,CFile::modeWrite|CFile::modeCreate)==NULL)
	   {
		   MessageBox("Sorry! Unable to read key bits","System information",MB_ICONHAND|MB_OK);
	       return;
	   }

	   decode_record=true;
	}
    
	else
	  return;

	ifgo=true;
	ifshow=true;
	pDoc->UpdateAllViews(NULL);	
}

void CMyView::OnFileNew() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    bFlag=0;
	pDoc->UpdateAllViews(NULL);	
}





void CMyView::OnSize(UINT nType, int cx, int cy) 
{
	 CView::OnSize(nType, cx, cy);
	 // TODO: Add your message handler code here
	 CMyDoc* pDoc = GetDocument();
     ASSERT_VALID(pDoc);
	 ifgo=false;
	 clearmark();
	 if(nType==SIZE_RESTORED)
	 {
	  bFlag=0;
      pDoc->UpdateAllViews(NULL);
	 }
}

void CMyView::ConvertStrToChar(CString &embedsite)
{
    char site[50];
	
	CString web;
	web="http://"+embedsite;
	
	int i,k,tempnum;
	
	int test=web.GetLength();

	char *temp;
	temp=(char*)GlobalAlloc(GMEM_FIXED, sizeof(char)); 
	
	for(i=0;i<web.GetLength();i++)
	{
		*temp=web.GetAt(i);
		strcpy(site+i,temp);
	}
	site[web.GetLength()]='\0';
	
	unsigned char numstyle[50];
	
	for(i=0;i<web.GetLength()+1;i++)
		numstyle[i]=site[i];
		
	for(i=0;i<web.GetLength()+1;i++)
	{   
		tempnum=numstyle[i];
		for(k=0;k<8;k++)
		{  
			singalbit[i*8+7-k]=tempnum%2;
			tempnum=tempnum>>1;
		}
	}
  
	for(i=0;i<(web.GetLength()+1)*4;i++) // 2 位水印
	{
		markbit[i]=singalbit[i*2]*2+singalbit[i*2+1];
	}
	
	GlobalFree(temp);
}

void CMyView::OnBridge_extract() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	
	bFlag=0;
	pDoc->UpdateAllViews(NULL);
	
    CEncodeDlg encode;
    encode.Openbook2();
    CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Visual VQ+263 Files(*.vvq)|*.vvq|");
    decfile.m_ofn.lpstrTitle="打开263文件";
	decfile.m_ofn.lStructSize=88;
		clearmark();
    if(decfile.DoModal()==IDOK)
	{
	DecfileName=decfile.GetPathName();//获取263文件路径
	firstframe=1;
    bFlag=2;
	bridge_extract=true;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);
	}
	else
		return;
}

void CMyView::OnUpdateNormalplay(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if(ifopen)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CMyView::OnUpdateFastplay(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if(ifopen)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CMyView::OnUpdateSlowplay(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if(ifopen)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CMyView::OnUpdateRecord(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
    if(ifopen)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CMyView::OnRobsemifrgwm() 
{
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	
	bFlag=0;
	pDoc->UpdateAllViews(NULL);

	CString keyfilename;
	KEYFILEHEADER keyheader;
	FILE *fp;
	CFileDialog openkey(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Key File(*.key)|*.key|");
	openkey.m_ofn.lpstrTitle="打开密钥文件";
	openkey.m_ofn.lStructSize=88; 
	if(openkey.DoModal()==IDOK)
	{
		keyfilename=openkey.GetFileName();
		if(fp=fopen(keyfilename,"rb"))
		{ 
			fseek(fp,0,SEEK_SET);
			if(fread(&keyheader,sizeof(KEYFILEHEADER),1,fp)==NULL)//读文件头错误
			{MessageBox("Sorry! Unable to open key file","System information",MB_ICONHAND|MB_OK);
			return;}
			else
			{
				if((strcmp(keyheader.FileFlag,"KeyFile")))//格式错误
				{MessageBox("You have open a key file with incorrect format","System information",MB_ICONHAND|MB_OK);
				return;}
				else
				{ 
					fseek(fp,20,SEEK_SET);
					if(fread(keybit,sizeof(unsigned char),keyheader.keynumber,fp)==NULL)
					{
					MessageBox("Sorry! Unable to read key bits","System information",MB_ICONHAND|MB_OK);
					return;
					}
					else
					MessageBox("You have successfully open a key file!","System information",MB_ICONINFORMATION|MB_OK);
				}
				
			}
		}
		else//打开文件失败
		{
			MessageBox("Sorry! Unable to open key file","System information",MB_ICONHAND|MB_OK);
			return;
		}
	}
	else
	return;

	CEncodeDlg encode;// 打开码书
    encode.Openbook2();	
	CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Visual VQ+263 Files(*.vvq)|*.vvq|");
    decfile.m_ofn.lpstrTitle="打开含水印的263文件";
	decfile.m_ofn.lStructSize=88;
		clearmark();
    if(decfile.DoModal()==IDOK)
		DecfileName=decfile.GetPathName();//获取263文件路径
	else
		return;
	firstframe=1;
    bFlag=2;
    rob_semifrg_extract=1;
	ifopen=true;
    pDoc->UpdateAllViews(NULL);	
}

void CMyView::OnTrimark() 
{
	// TODO: Add your command handler code here
    CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	
	CEncodeDlg encode;// 打开码书
    if(encode.DCTBOOK_Open(false)==0)
	return;
	
    CDC *pDC=GetDC();
    CFileDialog decfile(TRUE,NULL,NULL,OFN_HIDEREADONLY,"DCT_VQ Files(*.DCTVQ)|*.DCTVQ|");
    decfile.m_ofn.lpstrTitle="打开含水印的263文件";
	decfile.m_ofn.lStructSize=88;
		clearmark();
    if(decfile.DoModal()==IDOK)
		DecfileName=decfile.GetPathName();//获取263文件路径
	else
		return;
	firstframe=1;
    bFlag=2;
	RoFgFp_extract=true;
	ifopen=true;
	I_num_toshow=-1;
    pDoc->UpdateAllViews(NULL);
}

void CMyView::OnRfview() 
{
	// TODO: Add your command handler code here
	CMyDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
	bFlag=0;
    pDoc->UpdateAllViews(NULL);	
	ViewNC vNC;
	vNC.m_total=120;
	if(I_num_toshow!=0)
	vNC.m_wms=120/I_num_toshow;
	UpdateData(TRUE);
	vNC.DoModal();
}



bool CMyView::SaveMark(int width, int height, unsigned char *image_data,BITMAPINFO* image)
{
    bool state=false;
    CFileDialog savemark(FALSE,NULL,NULL,NULL,"Bitmap image(*.bmp)|*.bmp|");
	savemark.m_ofn.lStructSize=88;
    FILE *fp;
	int round=(width%64)?(width/64+1)*64:width;
	if(savemark.DoModal()==IDOK)
	{
      CString imagename;
	  imagename=savemark.GetFileName();
	  imagename+=".bmp";
	  if(fp=fopen(imagename,"wb"))
	  {  
         fseek(fp,0,SEEK_SET);
		 BITMAPFILEHEADER markheader;
		 markheader.bfType=0x4d42;
		 markheader.bfSize=width*round;
		 markheader.bfReserved1=markheader.bfReserved2=0;
		 markheader.bfOffBits=62;
		 if(fwrite(&markheader,sizeof(BITMAPFILEHEADER),1,fp)==NULL)
         {
            state=false;
            return state;
         }
         fseek(fp,14,SEEK_SET);
		 if(fwrite(image,48,1,fp)==NULL)
         {
            state=false;
            return state;
         }
		 fseek(fp,markheader.bfOffBits,SEEK_SET);
		 if(fwrite(image_data,sizeof(unsigned char),markheader.bfSize,fp)==NULL)
         {
            state=false;
            return state;
         }
         state=true;
		 fclose(fp);
	  }
	}
  return state;
}



void CMyView::OnAttackMethod() 
{
	// TODO: Add your command handler code here
	VideoAttack v_at;
    v_at.DoModal();
}

void CMyView::OnUpdateTrimark(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(RoFgFp_extract)
    pCmdUI->SetCheck(true);
    else
    pCmdUI->SetCheck(false);
}

void CMyView::OnUpdateRfview(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(rofgfp_finished)
     pCmdUI->Enable(true);
    else
     pCmdUI->Enable(false);
}
