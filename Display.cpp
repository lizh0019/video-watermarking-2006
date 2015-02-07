#include "stdafx.h"
#include "视频编解码器.h"
#include "Display.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define START	0
#define STOP    5

int firstframe;
static unsigned char *dithered_image;
static unsigned char ytab[256+16];
static unsigned char utab[128+16];
static unsigned char vtab[128+16];
static unsigned char pixel[256];
extern unsigned char Robit[20][64],Fpbit[20][64];
unsigned char finalbit[4096];

extern bool vqdecode;
extern BOOL basicextract;
extern bool bridge_extract;
extern BOOL robfrgextract;
extern BOOL rob_semifrg_extract;
extern BOOL dctparity;
extern bool ifshow;
extern bool ifgo;
extern BOOL RoFgFp_extract;

double Fp_NC[20],Ro_NC[20];
extern unsigned char singalbit[4096];
extern unsigned char secondsingalbit[4096];
extern unsigned char key[44096];
extern unsigned char key2[44096];
extern unsigned char frg_bit[16384];
extern int I_num_toshow;

CDisplay::CDisplay()
{
      image  =NULL;
      hPalPrev=NULL;
      pbmi    =NULL;
}

CDisplay::~CDisplay()
{

}
 unsigned char *refframe[3],*bframe[3],*newframe[3];
 unsigned char *edgeframe[3], *edgeframeorig[3], *exnewframe[3];
 extern unsigned char *oldrefframe[3];
 int verbose,outtype,expand,refidct,trace;
 CFile h263file;
 CString outputname;
 unsigned char *clp;
 int MBC, MBR;
 int horizontal_size,vertical_size,mb_width,mb_height;
 int coded_picture_width, coded_picture_height;
 int chrom_width,chrom_height,blk_cnt;
 int pict_type,newgob,tr_framenum;	
 int matrix_coefficients,source_format;
 extern ldecode *ld,base;
 extern int quant;
 void ditherframe(unsigned char *src[]);

void CDisplay::play_movie(CDC *pDC,CString szFileName,long int sp,bool rec,CFile &recfile)
{
	verbose = 0;
    outtype = T_X11;  //结果输出类型
    nDitherType=8;    //图像变换类型 每象素8比特
    ld = &base;        
	trace = 0;   
	expand = 0; 
    hDC=pDC->GetSafeHdc();
    unsigned short gusState =START; 
    int loopflag(0);
    int framenum,embednum,embednum2;
    if ((base.infile=h263file.Open(szFileName,CFile::modeRead))==0)
	{   
		//打开263文件
		AfxMessageBox("Can not open input file!");
	    return;
	}
    
	firstframe = 1;
    pDC->SetBkColor(0xffffff);
    pDC->SetTextColor(RGB(255,100,0));
    pDC->TextOut(450,50,"Video Decode Information");

    CString out;
    int inum(0),pnum(0);
     do
	{
	  if(firstframe!=1)
          h263file.Seek(0l,CFile::begin);//change
      m_getbits.initbits();
      framenum = tr_framenum = 0;
   
      while (m_gethdr.getheader())
	  {
        if (firstframe)//第一帧
		{
       	  initdecoder();//初始化解码器
		  if (outtype==T_X11)
		  {
            init_dither(nDitherType);//初始化显示图像的类型
            init_display();          //初始化显示的调色板，bmp文件头
		  }
		pDC->TextOut(450,80,szPictureFormat);
        out.Format("Initial QP:Q=%d   ",quant);
		pDC->TextOut(450,100,out);
        firstframe=0;
		embednum=0;
		embednum2=0;
		}
	    //开始解码图像
        //if(framenum!=20&&framenum!=5&&framenum!=30)
	    m_getpic.getpicture(&framenum,&embednum,&embednum2);//解码一帧图象    
		
		pDC->SetTextColor(RGB(255,100,0));
		out.Format("Current QP: Q=%d    ",quant);
        pDC->TextOut(450,120,out);
	    
		if(pict_type==0)
		{
		  out="Current frame type: Intra";
		  inum++;
		}
	    else 
		{
		  out="Current frame type: Inter";
		  pnum++;
		}
	     
		number.Format("Current Frame No. %d", framenum+1);
        pDC->TextOut(450,160,number);

        framenum++;
		tr_framenum++;
	    
		clock_t clock1,clock2;

		if((framenum==1)&&(!ifgo))
		{ 
		  dither(refframe,rec,recfile);
		  ifshow=false;
		  break;
		}
	
	   //控制播放速度
		clock1=clock();
	    for(;;)
		{
		  clock2=clock();
		  if((clock2-clock1>=sp)&&ifgo)
          {
			 dither(refframe,rec,recfile);//显示解码图像
		     break;
		  } 
		}
	    if (gusState==STOP) //控制显示
          break;
	  }
	}
	while(loopflag);//结束
    
    h263file.Close();
	if(rec)
    recfile.Close();
    
    if(dctparity)
	ChioseOneBit(key,singalbit,embednum,2304);
	if(robfrgextract||rob_semifrg_extract)
   	ChioseOneBit(key2,secondsingalbit,embednum2,4096);
    if(RoFgFp_extract&&ifgo)
    {
	  MajorityVote(I_num_toshow,m_getpic.ro_bit,Robit,m_getpic.fp_bit,Fpbit,64*6);
      Cau_NC(I_num_toshow,Robit,Fpbit,Ro_NC,Fp_NC);
	}
    if (outtype==T_X11)//释放掉分配的空间
      exit_display();
	toDeleteNewSpace();

}
/*****************************************************************************/
//初始化解码器
void CDisplay::initdecoder()
{
  int i, cc, size;

  if (!(clp=new unsigned char[1024]))
  {
	  AfxMessageBox("new failed!");
      return;
  }
  clp += 384;
  for (i=-384; i<640; i++) //区间[0,255]
    clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  matrix_coefficients = 5;

  if (source_format == SF_CIF) 
  { // 输入码流为 CIF
    MBC = 22;
    MBR = 18;
	szPictureFormat="Video Format:352*288";
  }
  else if (source_format == SF_QCIF) 
  { // 输入码流为 QCIF
    MBC = 11;
    MBR = 9;
  	szPictureFormat="Video Format:176*144";
  }
  else if (source_format == SF_SQCIF)
  { // 输入码流为 SQCIF
	MBC = 8;
    MBR = 6;
    szPictureFormat="Video Format:120*96";
  }
  else if (source_format == SF_SIF)
  {  // 输入码流为 SQCIF
	  MBC = 22;
	  MBR = 15;
	  szPictureFormat="Video Format:352*240";
  }
  else
    exit(1);

  horizontal_size = MBC*16;
  vertical_size = MBR*16;
  mb_width = horizontal_size/16;
  mb_height = vertical_size/16;
  coded_picture_width = 16*mb_width;
  coded_picture_height = 16*mb_height;
  chrom_width =  coded_picture_width>>1;
  chrom_height = coded_picture_height>>1;
  blk_cnt = 6;

     //初始化帧存储器
  for (cc=0; cc<3; cc++)
  {
    if (cc==0)
      size = coded_picture_width*coded_picture_height;
    else
      size = chrom_width*chrom_height;
    if (!(refframe[cc] = new unsigned char[size]))
	{
		AfxMessageBox("new failed!");
		return;
	}
    if (!(oldrefframe[cc] =new unsigned char[size]))
	{
		AfxMessageBox("new failed!");
		return;
	}
    if (!(bframe[cc] =new unsigned char[size]))
	{
		AfxMessageBox("new failed!");
		return;
	}
  }
  for (cc=0; cc<3; cc++) 
  {
    if (cc==0) 
	{
      size = (coded_picture_width+64)*(coded_picture_height+64);
      if (!(edgeframeorig[cc] =new unsigned char[size]))
	  {
		  AfxMessageBox("new failed!");
		  return;
	  }
      edgeframe[cc] = edgeframeorig[cc] + (coded_picture_width+64) * 32 + 32;
    }
    else
	{
      size = (chrom_width+32)*(chrom_height+32);
      if (!(edgeframeorig[cc] =new unsigned char[size]))
	  {
		  AfxMessageBox("new failed!");
		  return;
	  }
      edgeframe[cc] = edgeframeorig[cc] + (chrom_width+32) * 16 + 16;
    }
  }
  if (expand) 
  {
    for (cc=0; cc<3; cc++) 
	{
      if (cc==0)
	     size = coded_picture_width*coded_picture_height*4;
      else
         size = chrom_width*chrom_height*4;
      
      if (!(exnewframe[cc] = new unsigned char[size]))
	  {
		  AfxMessageBox("new failed!");
		  return;
	  }
	}
  } 
  m_idct.init_idct();  // IDCT
}
/*****************************************************************************/
void CDisplay::init_dither(int bpp)
{
  int i, v;
  if ( bpp!=8 )
  { 
     AfxMessageBox("unsuported dither type!");
     return;
  }
  bpp/=8;
  int  size;
  size=bpp*coded_picture_width*coded_picture_height;
  if(!(dithered_image = new unsigned char[size]))
  {
	  AfxMessageBox("new failed!");
	  return;
  }

  for (i=-8; i<256+8; i++)
  {
    v = i>>4;
    if (v<1)
      v = 1;
    else if (v>14)
      v = 14;
    ytab[i+8] = v<<4;
  }

  for (i=0; i<128+16; i++)
  {
    v = (i-40)>>4;
    if (v<0)
      v = 0;
    else if (v>3)
      v = 3;
    utab[i] = v<<2;
    vtab[i] = v;
  }

  for (i=0; i<256; i++)
     pixel[i]=i;

}
/*****************************************************************************/
void CDisplay::init_display()//初始化显示，创建调色板
{
  pbmi= (PBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER) + 240 * sizeof(RGBQUAD));
  pbmi->bmiHeader.biSize = (LONG)sizeof(BITMAPINFOHEADER);
  pbmi->bmiHeader.biPlanes = 1;
  pbmi->bmiHeader.biCompression = 0l;
  pbmi->bmiHeader.biSizeImage = 0l;
  pbmi->bmiHeader.biXPelsPerMeter = 0l;
  pbmi->bmiHeader.biYPelsPerMeter = 0l;
  pbmi->bmiHeader.biClrUsed = 240;
  pbmi->bmiHeader.biClrImportant = 240;
  pbmi->bmiHeader.biBitCount = nDitherType;
  pbmi->bmiHeader.biWidth = coded_picture_width ;
  pbmi->bmiHeader.biHeight= coded_picture_height;
  if ( pbmi->bmiHeader.biBitCount==8 )
  {
     // 8 BPP，建立调色板 
     LOGPALETTE *plgpl;
     short      *pPalIndex;
     int         crv, cbu, cgu, cgv;
     int         y, u, v;
     int         i;

     plgpl = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) + 240 * sizeof(PALETTEENTRY));
     plgpl->palNumEntries = 240;
     plgpl->palVersion = 0x300;
     pPalIndex=(short *)pbmi->bmiColors;

     // 矩阵参数 
     crv = convmat[matrix_coefficients][0];
     cbu = convmat[matrix_coefficients][1];
     cgu = convmat[matrix_coefficients][2];
     cgv = convmat[matrix_coefficients][3];

     for (i=16; i<240; i++)
     {
       // 颜色空间转换
       y = 16*((i>>4)&15) + 8;
       u = 32*((i>>2)&3)  - 48;
       v = 32*(i&3)       - 48;

       y = 76309 * (y - 16); // (255/219)*65536 

       plgpl->palPalEntry[i].peRed   = clp[(y + crv*v + 32768)>>16];
       plgpl->palPalEntry[i].peGreen = clp[(y - cgu*u -cgv*v + 32768)>>16];
       plgpl->palPalEntry[i].peBlue  = clp[(y + cbu*u + 32786)>>16];
       pPalIndex[i]=i;
     }
     hpal = CreatePalette(plgpl);
     free(plgpl);
     hPalPrev=SelectPalette(hDC,hpal,FALSE);
     RealizePalette(hDC);
  }
}

/*****************************************************************************/
void CDisplay::dither(unsigned char *src[],bool recd,CFile &recdfile)
{
   ditherframe(src);
 
   if(recd)
   {
	  recdfile.Write(src[0],coded_picture_height*coded_picture_width);
	  recdfile.Write(src[1],coded_picture_height*coded_picture_width/4);
	  recdfile.Write(src[2],coded_picture_height*coded_picture_width/4);
	  
   }

  SetDIBitsToDevice(hDC,60,60,coded_picture_width,
  	                   coded_picture_height,
		               0,0,0,coded_picture_height,
    		           dithered_image,pbmi,DIB_PAL_COLORS);
  
}
/*****************************************************************************/

void ditherframe(unsigned char *src[])
{
  int i,j;
  int y,u,v;
  unsigned char *py,*pu,*pv,*dst;

  py = src[0];
  pu = src[1];
  pv = src[2];

#ifdef _WIN32
  dst = dithered_image+(coded_picture_height-1)*coded_picture_width;
#else
  dst = dithered_image;
#endif

  for (j=0; j<coded_picture_height; j+=4)
  {
    // line j + 0 
    for (i=0; i<coded_picture_width; i+=4)
    {
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y]|utab[u]|vtab[v]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+8]|utab[u+8]|vtab[v+8]];
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+2]|utab[u+2]|vtab[v+2]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+10]|utab[u+10]|vtab[v+10]];
    }
   
    pu -= chrom_width;
    pv -= chrom_width;
  
#ifdef _WIN32
    dst -= 2*coded_picture_width;
#endif

    /* line j + 1 */
    for (i=0; i<coded_picture_width; i+=4)
    {
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+12]|utab[u+12]|vtab[v+12]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+4]|utab[u+4]|vtab[v+4]];
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+14]|utab[u+14]|vtab[v+14]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+6]|utab[u+6]|vtab[v+6]];
    }

#ifdef _WIN32
    dst -= 2*coded_picture_width;
#endif

    // line j + 2 
    for (i=0; i<coded_picture_width; i+=4)
    {
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+3]|utab[u+3]|vtab[v+3]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+11]|utab[u+11]|vtab[v+11]];
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+1]|utab[u+1]|vtab[v+1]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+9]|utab[u+9]|vtab[v+9]];
    }

    pu -= chrom_width;
    pv -= chrom_width;
  
#ifdef _WIN32
    dst -= 2*coded_picture_width;
#endif

    // line j + 3 
    for (i=0; i<coded_picture_width; i+=4)
    {
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+15]|utab[u+15]|vtab[v+15]];
      y = *py++;
      *dst++ = pixel[ytab[y+7]|utab[u+7]|vtab[v+7]];
      y = *py++;
      u = *pu++ >> 1;
      v = *pv++ >> 1;
      *dst++ = pixel[ytab[y+13]|utab[u+13]|vtab[v+13]];
      y = *py++;
      
      *dst++ = pixel[ytab[y+5]|utab[u+5]|vtab[v+5]];
    }

#ifdef _WIN32
    dst -= 2*coded_picture_width;
#endif
  }
}
/*****************************************************************************/

//释放所分配的内存及调色板
void CDisplay::exit_display()
{
   if (pbmi)
   {
      free(pbmi);
      pbmi=NULL;
   }
   if (hPalPrev)
   {
       SelectPalette(hDC,hPalPrev,FALSE);
       DeleteObject(hpal);
       hPalPrev=NULL;
   }

}
/*****************************************************************************/

void CDisplay::toDeleteNewSpace()
{
    //delete []clp;
	delete []dithered_image;
	for(int i=0;i<3;i++)
	{
	   delete []refframe[i];
	   delete []bframe[i];
	   delete []edgeframeorig[i];
	   if(expand) 
	     delete []exnewframe[i];
       delete []oldrefframe[i];
	}
  
}

void CDisplay::ChioseOneBit(unsigned char *multibit, unsigned char *onebit, int multinum, int shownum)
{
  //multinum:total extract bits; shownum: 2304
  int i,j,num1=0,num0=0;
  for(i=0;i<shownum;i++)
  {
    for(j=0;j<multinum/shownum;j++)
	{
	  if(*(multibit+j*shownum+i)==1)
		num1++;
	  if(*(multibit+j*shownum+i)==0)
		num0++;
	}
	if(num1>=num0)
	 *(onebit+i)=1;
	else
	 *(onebit+i)=0;
	num1=num0=0;
  }
 
}

void CDisplay::MajorityVote(int SegNum, unsigned char Ro[20][384],unsigned char Ro_to_show[20][64],
							unsigned char Fp[20][384],unsigned char Fp_to_show[20][64],int capacity)
{
  int k=0,l=0,n=0;
  for(k=0;k<=SegNum;k++)
  {
	for(l=0;l<capacity;l++)
	{ 
	  Fp_to_show[k][l%64]+=Fp[k][l];
	  Ro_to_show[k][l%64]+=Ro[k][l]; 
	}
    for(n=0;n<64;n++)
	{
	  if(Fp_to_show[k][n]>=capacity/128)
         Fp_to_show[k][n]=1;
	  else
		 Fp_to_show[k][n]=0;
	  
	  if(Ro_to_show[k][n]>=capacity/128)
         Ro_to_show[k][n]=1;
	  else
		 Ro_to_show[k][n]=0;
	}

  }
}

void CDisplay::Cau_NC(int SegNum, unsigned char Ro[20][64], unsigned char Fp[20][64],double RobustNC[20],double FngPtNC[20])
{
  int k=0,n=0,i=0;
  int *Ro_correct,*Ro_wrong,*Fp_correct,*Fp_wrong;
  unsigned char OriRo[64],OriFp[64];
  
  Ro_correct=new int [64];
  Ro_wrong  =new int [64];
  Fp_correct=new int [64];
  Fp_wrong  =new int [64];
  
  for(i=0;i<64;i++)
  { 
      OriRo[i]=0;
      OriFp[i]=0;
	if(i%2==0)
	  OriRo[i]=1;
    if(i%2==0)
	  OriFp[i]=1;
  }
  
  for(i=0;i<64;i++)
  {
    Ro_correct[i]=0;
    Ro_wrong[i]=0;
    Fp_correct[i]=0;
    Fp_wrong[i]=0;
  }

  for(k=0;k<=SegNum;k++)
    for(n=0;n<64;n++)
	{
	  if(Ro[k][n]==OriRo[n])
		 Ro_correct[k]++;
	  else
		 Ro_wrong[k]++;
	  if(Fp[k][n]==OriFp[n])
		 Fp_correct[k]++;
	  else
		 Fp_wrong[k]++;
	}
	
  for(k=0;k<=SegNum;k++)
  {
    RobustNC[k]=(double)Ro_correct[k]/64;
    FngPtNC[k] =(double)Fp_correct[k]/64;
  }

  delete [] Ro_correct;
  delete [] Ro_wrong;
  delete [] Fp_correct;
  delete [] Fp_wrong;
}
