// WatermarkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "视频编解码器.h"
#include "WatermarkDlg.h"
#include<math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

unsigned char *ImageBytes;
unsigned char *SndImageBytes;
unsigned char *TrdImageBytes;
unsigned char singalbit[4096];
unsigned char secondsingalbit[4096];
unsigned char originalsingalbit[4096];
unsigned char orgsecondsingalbit[4096];
unsigned char markbit[5000];
unsigned char secondmarkbit[5000];
unsigned char tribit[1500];
unsigned char secondtribit[1500];
unsigned char p_fragile[4096];

/////////////////////////////////////////////////////////////////////////////
// CWatermarkDlg property page

IMPLEMENT_DYNCREATE(CWatermarkDlg, CPropertyPage)

CWatermarkDlg::CWatermarkDlg() : CPropertyPage(CWatermarkDlg::IDD)
{
	//{{AFX_DATA_INIT(CWatermarkDlg)
	m_keyname = _T("");
	m_2ndkeyname = _T("");
	m_algorithm = _T("");
	m_web = _T("");
	m_basic = -1;
	m_wmcodebook = _T("");
	//}}AFX_DATA_INIT
	ifopen=false;
	if2ndopen=false;
	fingeropen=false;
}

CWatermarkDlg::~CWatermarkDlg()
{
	GlobalFree(ImageBytes);
	GlobalFree(SndImageBytes);
	GlobalFree(TrdImageBytes);
}

void CWatermarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWatermarkDlg)
	DDX_Text(pDX, IDC_EDIT1, m_keyname);
	DDX_Text(pDX, IDC_EDIT2, m_2ndkeyname);
	DDX_CBString(pDX, IDC_COMBO2, m_algorithm);
	DDX_Text(pDX, IDC_EDIT4, m_web);
	DDV_MaxChars(pDX, m_web, 50);
	DDX_Radio(pDX, IDC_BASIC, m_basic);
	DDX_CBString(pDX, IDC_WMCBGEN, m_wmcodebook);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWatermarkDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CWatermarkDlg)
	ON_BN_CLICKED(ID_OPENKEY, OnOpenkey)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_2ndwatermark, On2ndwatermark)
	ON_BN_CLICKED(IDC_BASIC, OnBasic)
	ON_BN_CLICKED(IDC_GEN, OnGen)
	ON_BN_CLICKED(IDC_MB, OnMb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWatermarkDlg message handlers

void CWatermarkDlg::OnOpenkey() 
{  
	// 打开水印图像
	CFile imp;
	int filesize;
    unsigned char t;
	LPBITMAPFILEHEADER bitfile;
    LPBITMAPINFOHEADER bitinfo;
	char temp[50];
	CString m_keypath;
	ifopen=false;
	CFileDialog openkey(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Bitmap(*.bmp)|*.bmp|");
    if(openkey.DoModal()==IDOK)
	{
	    m_keyname=openkey.GetFileName();
		m_keypath=openkey.GetPathName();
	    UpdateData(false);
		if(imp.Open(m_keypath,CFile::modeRead)==NULL)//Can not open watermar iage
		{
			sprintf(temp,"Can not open the selcted bitmap!");
			MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
			return;
		}
		else//Read pixels
		{   
			filesize=imp.GetLength();
			ImageBytes=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(filesize));
			imp.Read(ImageBytes,filesize);	
			bitfile=(LPBITMAPFILEHEADER)ImageBytes;
			bitinfo=(LPBITMAPINFOHEADER)(ImageBytes+sizeof(BITMAPFILEHEADER));
			BITMAPINFO* image=(BITMAPINFO *)ImageBytes+sizeof(BITMAPFILEHEADER);
            width=bitinfo->biWidth;
			height=bitinfo->biHeight;
			Bytesoff=bitfile->bfOffBits;
			
			if(filesize!=446)
			{
				sprintf(temp,"You have open an image with improper size!");
			    MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				return;
			}
			
			if(bitfile->bfType!=0x4d42)
			{ 
				sprintf(temp,"You have open a file that it not supported by this software!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return;
			} 
			
			if(bitinfo->biBitCount!=1)
			{ 
				sprintf(temp,"Sorry it is not 2 bit bitmap!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return;
			} 
			
			//If the image is compressed or not
			if(bitinfo->biCompression!=0)
			{ 
				sprintf(temp,"Sorry,it is compressed image!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return;
			}
			
		ifopen=true;
		
		//get the value of each watermarkbit
		for(int i=0;i<height;i++)
		{
		  for(int j=0;j<6;j++)
		  {
		    t=*(ImageBytes+Bytesoff+i*6+j+2*i);
	    	for(int k=0;k<8;k++)
			{   
		     originalsingalbit[i*width+j*8+7-k]=t%2;
	         t=t>>1;
			}
		  }
       	}	

		//bellow codes are for test
	   	/*for(i=0;i<width*height;i++)
		{
			
		    if((i%2)==0)
			singalbit[i]=0;
			else
			singalbit[i]=1;
		    /*unsigned char m=0;
			for(int k=0;k<8;k++)
			{
			 m=m+singalbit[i*8+7-k]*pow(2,k);	
			}

			*(ImageBytes+Bytesoff+i+2*(i/6))=m;
		
		}*/
        for(i=0;i<width*height/8;i++)
		{
		    unsigned char m=0;
			for(int k=0;k<8;k++)
			{
			 m=m+originalsingalbit[i*8+7-k]*(unsigned char)pow(2,k);	
			}

			*(ImageBytes+Bytesoff+i+2*(i/6))=m;
		
		}
		PSEUDO(originalsingalbit,singalbit,64,64,4096,45);
		////////// Change singalbit in to 2 bit number/////////////
		for(i=0;i<width*height/2;i++) // 2 位水印
		{
		markbit[i]=singalbit[i*2]*2+singalbit[i*2+1];
		}
    
		for(i=0;i<width*height/3;i++) // 3位水印
		{
          tribit[i]=singalbit[i*3]*4+singalbit[i*3+1]*2+singalbit[i*3+2];
		}
         
		}
	 
		}
	 OnPaint();
    
}

LRESULT CWatermarkDlg::OnWizardNext() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CPropertyPage::OnWizardNext();
}



void CWatermarkDlg::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    CClientDC * pDC=new CClientDC(this);
    if(ifopen)
    {  	
        ::StretchDIBits(pDC->m_hDC,                                                
            381,
            50,
            width,
            height,
            0,
            0,
            width,
            height,
            ImageBytes+Bytesoff,
            (BITMAPINFO *)(ImageBytes+sizeof(BITMAPFILEHEADER)),
            DIB_RGB_COLORS,
            SRCCOPY);
    }
    
    if(if2ndopen)
    {  
        ::StretchDIBits(pDC->m_hDC,                                                
            381,
            105,
            width,
            height,
            0,
            0,
            width,
            height,
            SndImageBytes+Bytesoff,
            (BITMAPINFO *)(SndImageBytes+sizeof(BITMAPFILEHEADER)),
            DIB_RGB_COLORS,
            SRCCOPY);
    }
}




BOOL CWatermarkDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	OnPaint();
	basic_click=0;
	gen_click=0;
	bridge_click=0;
	m_basic=0;
	GetDlgItem(IDC_EDIT4)->EnableWindow(false);
	GetDlgItem(IDC_WMCBGEN)->EnableWindow(false);
	UpdateData(FALSE);
	// TODO: Add extra initialization here
	//GetDlgItem(IDC_2ndwatermark)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




void CWatermarkDlg::On2ndwatermark() 
{
	// TODO: Add your control notification handler code here
	// 打开水印图像
	CFile imp;
	int filesize;
    unsigned char t;
	LPBITMAPFILEHEADER bitfile;
    LPBITMAPINFOHEADER bitinfo;
	unsigned char orgsecondsingalbit[4096];
	char temp[50];
	CString m_keypath;
	if2ndopen=false;
	CFileDialog openkey(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Bitmap(*.bmp)|*.bmp|");
    if(openkey.DoModal()==IDOK)
	{
	    m_2ndkeyname=openkey.GetFileName();
		m_keypath=openkey.GetPathName();
	    UpdateData(false);
		if(imp.Open(m_keypath,CFile::modeRead)==NULL)//Can not open watermar iage
		{
            sprintf(temp,"Can not open the selcted bitmap!");
			MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
			return;
		}
		else//Read pixels
		{   
		    filesize=imp.GetLength();
			SndImageBytes=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(filesize));
	     	imp.Read(SndImageBytes,filesize);	
		    bitfile=(LPBITMAPFILEHEADER)SndImageBytes;
		    bitinfo=(LPBITMAPINFOHEADER)(SndImageBytes+sizeof(BITMAPFILEHEADER));
			int m,l;
			m=sizeof(BITMAPFILEHEADER);
			l=sizeof(BITMAPINFOHEADER);
			BITMAPINFO* image=(BITMAPINFO *)SndImageBytes+sizeof(BITMAPFILEHEADER);
            width=bitinfo->biWidth;
			height=bitinfo->biHeight;
			Bytesoff=bitfile->bfOffBits;
			int p=sizeof(BITMAPINFO);
			if(filesize!=574)
			{
				sprintf(temp,"You have open an image with improper size!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				return;
			}
			
			if(bitfile->bfType!=0x4d42)
			{ 
				sprintf(temp,"You have open a file that it not supported by this software!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return;
			} 
			
			if(bitinfo->biBitCount!=1)
			{ 
				sprintf(temp,"Sorry it is not 2 bit bitmap!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return;
			} 
			
			//If the image is compressed or not
			if(bitinfo->biCompression!=0)
			{ 
				sprintf(temp,"Sorry,it is compressed image!");
				MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return;
			}
			
		if2ndopen=true;
        fingeropen=true;
	   //get the value of each watermarkbit
       for(int i=0;i<height;i++)
		{
		  for(int j=0;j<width/8;j++)
		  {
		    t=*(SndImageBytes+Bytesoff+i*width/8+j);
	    	for(int k=0;k<8;k++)
			{   
		     orgsecondsingalbit[i*width+j*8+7-k]=t%2;
             p_fragile[i*width+j*8+7-k]=t%2;
	         t=t>>1;
			}
		  }
	      
       	}

       
        memcpy(secondsingalbit,orgsecondsingalbit,sizeof(unsigned char)*width*height);

		////////// Change singalbit in to 2 bit number/////////////
		for(i=0;i<width*height/2;i++) // 2 位水印
		 secondmarkbit[i]=secondsingalbit[i*2]*2+secondsingalbit[i*2+1];
    
		for(i=0;i<width*height/3;i++) // 3位水印
          secondtribit[i]=secondsingalbit[i*3]*4+secondsingalbit[i*3+1]*2+secondsingalbit[i*3+2];
       
		}
		
	}
	OnPaint();
    
}



void CWatermarkDlg::PSEUDO(unsigned char *origwmdata,unsigned char *pmwmdata,int N1,int N2,int number,int seed)
{
	int bit[18];
	int i,j,scale,temp;
	int tempkey;
	int sequence[4096];
	int newsequence[4096];
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
		   newsequence[k]=j;
			k++;
        }
	}
	for(i=0;i<48*48;i++)
	{
		j=newsequence[i];
		pmwmdata[j]=origwmdata[i];
	}

}






void CWatermarkDlg::OnBasic() 
{
	// TODO: Add your control notification handler code here
    UpdateData(TRUE);
	if(m_basic==0)
	{
		GetDlgItem(ID_OPENKEY)->EnableWindow(true);
		GetDlgItem(IDC_2ndwatermark)->EnableWindow(true);
		GetDlgItem(IDC_COMBO2)->EnableWindow(true);
		GetDlgItem(IDC_EDIT4)->EnableWindow(false);
		GetDlgItem(IDC_WMCBGEN)->EnableWindow(false);
		UpdateData(FALSE);
	}
}


void CWatermarkDlg::OnGen() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_basic==1)
	{
		GetDlgItem(ID_OPENKEY)->EnableWindow(false);
		GetDlgItem(IDC_2ndwatermark)->EnableWindow(false);
		GetDlgItem(IDC_COMBO2)->EnableWindow(false);
		GetDlgItem(IDC_EDIT4)->EnableWindow(false);
		GetDlgItem(IDC_WMCBGEN)->EnableWindow(true);
		UpdateData(FALSE);
	}
}

void CWatermarkDlg::OnMb() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_basic==2)
	{
		GetDlgItem(ID_OPENKEY)->EnableWindow(false);
		GetDlgItem(IDC_2ndwatermark)->EnableWindow(false);
		GetDlgItem(IDC_COMBO2)->EnableWindow(false);
		GetDlgItem(IDC_WMCBGEN)->EnableWindow(false);
		GetDlgItem(IDC_EDIT4)->EnableWindow(true);	
		UpdateData(FALSE);
	}

}


