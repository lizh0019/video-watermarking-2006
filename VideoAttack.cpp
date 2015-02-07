// VideoAttack.cpp : implementation file
//

#include "stdafx.h"
#include "ÊÓÆµ±à½âÂëÆ÷.h"
#include "VideoAttack.h"
#include "AttackMethod.h"
#include "Global.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VideoAttack dialog


VideoAttack::VideoAttack(CWnd* pParent /*=NULL*/)
	: CDialog(VideoAttack::IDD, pParent)
{
	//{{AFX_DATA_INIT(VideoAttack)
	m_name = _T("");
	m_InputType = _T("");
	atk_type = -1;
	m_text = _T("");
	//}}AFX_DATA_INIT
}


void VideoAttack::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VideoAttack)
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	DDX_Control(pDX, IDC_PROGRESS1, m_process);
	DDX_Text(pDX, IDC_EDIT1, m_name);
	DDX_CBString(pDX, IDC_COMBO1, m_InputType);
	DDX_Radio(pDX, IDC_FILTER, atk_type);
	DDX_Text(pDX, IDC_TEXT, m_text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VideoAttack, CDialog)
	//{{AFX_MSG_MAP(VideoAttack)
	ON_BN_CLICKED(IDC_OPENYUV, OnOpenyuv)
	ON_BN_CLICKED(IDC_ATTACK, OnAttack)
	ON_BN_CLICKED(IDC_ROTATE, OnRotate)
	ON_BN_CLICKED(IDC_CROP, OnCrop)
	ON_BN_CLICKED(IDC_NOISE, OnNoise)
	ON_BN_CLICKED(IDC_BLUR, OnBlur)
	ON_BN_CLICKED(IDC_FILTER, OnFilter)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VideoAttack message handlers

BOOL VideoAttack::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_process.SetRange(0,100);
    m_process.SetPos(0);
    m_slider.SetRange(0,100);
    m_slider.SetTicFreq(10);
    m_InputType="CIF(352¡Á288)";
    ifopen=false;
    m_pos=0;
    Clear();
    GetDlgItem(IDC_ATTACK)->EnableWindow(false);
    GetDlgItem(IDC_SLIDER1)->ShowWindow(false);
    UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void VideoAttack::OnOpenyuv() 
{
	// TODO: Add your control notification handler code here
	CFileDialog open(true,NULL,NULL,OFN_HIDEREADONLY,"YUV files (*.yuv)|*.yuv");
    if(open.DoModal()==IDOK)
    {
      CFile raw;
      m_name=open.GetFileName();
      video_path=open.GetPathName();
      UpdateData(false);
	  ifopen=true;
      GetDlgItem(IDC_ATTACK)->EnableWindow(true);
      return;
    }
    else
    return;
}


void VideoAttack::OnAttack() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
    int width,height;
    
    if(m_InputType=="CIF(352*288)")
    {
        width=352;
        height=288;
    }
    else if(m_InputType=="QCIF(176*144)")
    {
        width=176;
        height=144;
    }
    else if(m_InputType=="SIF(352*240)")
    {
        width=352;
        height=240;
    }
    else
    {
        width=352;
        height=288;
    }
    
    PictImage *curr_recon = NULL;
   	PictImage *atk_recon = NULL;
    unsigned char  *m_pImageData;
    CFile file,outfile;
    int frame_no,MaxFrame;
    AttackMethod atk;
    if(file.Open(video_path,CFile::modeRead)==NULL)//Can not open watermar iage
	{
		MessageBox("Can not open the selected file!","Open video file",MB_ICONSTOP|MB_OK);
        return;
	}
    else
    {   
        m_pImageData = new unsigned char [sizeof(BYTE)*width*height*3/2];
        filesize=file.GetLength();
        MaxFrame=(int)(filesize/(sizeof(BYTE)*width*height*3/2));
        CFileDialog save(false,NULL,NULL,OFN_HIDEREADONLY,"yuv files(*.yuv)|*.yuv",NULL);
        if(save.DoModal()==IDOK)
        {
          
          m_process.SetRange(0,MaxFrame-1);
          curr_recon = InitImage(width*height);
          atk_recon  = InitImage(width*height);
          
          CString outname=save.GetFileName();
          outname+=".yuv";

          outfile.Open(outname,CFile::modeWrite|CFile::modeCreate);
          outfile.Seek(0,SEEK_SET);
          
          for(frame_no=0;frame_no<MaxFrame;frame_no++)
          {
	        file.Seek((frame_no)*width*height*3/2,SEEK_SET);
            outfile.Seek((frame_no)*width*height*3/2,SEEK_SET);
            m_process.SetPos(frame_no);
            if(file.Read(m_pImageData,sizeof(BYTE)*width*height*3/2)==NULL)
            {
              MessageBox("Sorry, errors occur when reading the video file!","Video Attack",MB_ICONSTOP|MB_OK);
              return;
            }
            else
            {
             memcpy(curr_recon->lum, m_pImageData, width*height*sizeof(BYTE));
             memcpy(curr_recon->Cb, m_pImageData + width*height*sizeof(BYTE), width*height*sizeof(BYTE)/4);
             memcpy(curr_recon->Cr, m_pImageData + width*height + width*height*sizeof(BYTE)/4, width*height*sizeof(BYTE)/4);
            
             switch(atk_type) 
             {
                case 0:
                atk.Filter(curr_recon,atk_recon,width,height);
            	break;
                case 1:
                atk.Blur(curr_recon,atk_recon,width,height,3,3);
            	break;
                case 2:
                atk.Rotate(curr_recon,atk_recon,width,height,angle);
            	break;
                case 3:
                atk.Crop(curr_recon,atk_recon,width,height);
            	break;
                case 4:
                atk.Noise(curr_recon,atk_recon,width,height,percent);
            	break;
            }
            
            outfile.Write(atk_recon->lum,width*height*sizeof(BYTE));
            outfile.Write(atk_recon->Cb,width*height*sizeof(BYTE)/4);
            outfile.Write(atk_recon->Cr,width*height*sizeof(BYTE)/4);

            Invalidate(true);
          }
        }
      }
    }
}

void VideoAttack::OnRotate() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_SLIDER1)->ShowWindow(true);
    GetDlgItem(IDC_TEXT)->ShowWindow(false);
    Clear();
    if_rotate=true;
    atk_type=2;
    m_process.SetPos(0);
    UpdateData(false);
}

void VideoAttack::OnCrop() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_SLIDER1)->ShowWindow(false);
    GetDlgItem(IDC_TEXT)->ShowWindow(false);
    Clear();
    atk_type=3;
    m_process.SetPos(0);
    UpdateData(false);
}

void VideoAttack::OnNoise() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_SLIDER1)->ShowWindow(true);
    GetDlgItem(IDC_TEXT)->ShowWindow(false);
    Clear();
    if_noise=true;
    atk_type=4;
    m_process.SetPos(0);
    UpdateData(false);
}

void VideoAttack::OnBlur() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_SLIDER1)->ShowWindow(false);
    GetDlgItem(IDC_TEXT)->ShowWindow(false);
    Clear();
    atk_type=1;
    m_process.SetPos(0);
    UpdateData(false);
}

void VideoAttack::OnFilter() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_SLIDER1)->ShowWindow(false);
    GetDlgItem(IDC_TEXT)->ShowWindow(false);
    Clear();
    atk_type=0;
    m_process.SetPos(0);
    UpdateData(false);
}

void VideoAttack::Clear()
{
  if_rotate=false;
  if_noise=false;
}

void VideoAttack::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_pos=m_slider.GetPos();
    m_slider.SetRange(0,100);
    UpdateData(true);
    if(if_rotate)
    {
      angle=((double)m_pos/100)*5;
      m_text.Format("Angle=%1.2f",angle);
      GetDlgItem(IDC_TEXT)->ShowWindow(true);
    }
    if(if_noise)
    {
      percent=m_pos;
      m_text.Format("Percent=%3d%%",m_pos);
      GetDlgItem(IDC_TEXT)->ShowWindow(true);
    }
	UpdateData(false);
	*pResult = 0;
}

void VideoAttack::OnCancel() 
{
	// TODO: Add your control notification handler code here
	OnOK();
}
