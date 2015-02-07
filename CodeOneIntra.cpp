
#include"stdafx.h"
#include"Global.h"
#include<math.h>
#include"Getpic.h"



extern int *codeword[MAXBOOKSIZE];
extern int *trainvector[MAXIMAGESIZE];

extern unsigned char markbit[5000];
extern unsigned char singalbit[4096];
extern unsigned char orgsecondsingalbit[4096];
extern unsigned char secondsingalbit[4096];
extern unsigned char originalsingalbit[4096];
unsigned char keybit[44096];

extern int dim_Low,dim_Mid,dim_Hig;

//帧内编码
PictImage *CodeOneIntra(PictImage *curr, int QP, Bits *bits, Pict *pic, int *ebtime,bool keymark,bool indicator)
{
  PictImage *recon;
  MB_Structure *data = (MB_Structure *)malloc(sizeof(MB_Structure));
  int * qcoeff;
  int Mode =3;//  MODE_INTRA;
  int CBP,COD;
  int i,j;
  int  intracounter=0;
  recon = InitImage(pels*lines);
  ZeroBits(bits);
  
  pic->QUANT = QP;
  bits->header += CountBitsPicture(pic);

  COD = 0; 
  for ( j = 0; j < lines/MB_SIZE; j++) 
  {
    /* 如果允许GOB同步，则输出GOB同步头  */
    if (pic->use_gobsync && j != 0)
      bits->header += CountBitsSlice(j,QP);

    for ( i = 0; i < pels/MB_SIZE; i++) 
	{
      pic->MB = i + j * (pels/MB_SIZE);
      bits->no_intra++;
      FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, data);//写亮度图像16*16 curr:图像数据 data:宏块树组
      FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, data);//写色度图像2*8*8 
      qcoeff = MB_Encode(data, QP, Mode); //变换并量化后数据到qcoeff=16*16+2*8*8个单元
      
	  if((keymark||indicator)&&(intracounter<2304))
	  {
		WM_intra(keybit,qcoeff,ebtime,indicator); 
	    intracounter+=6;
	  }
    
	  CBP = FindCBP(qcoeff,Mode,64);

      if (!syntax_arith_coding) 
	  {
       CountBitsMB(Mode,COD,CBP,0,pic,bits);//输出宏块层信息
       CountBitsCoeff(qcoeff, Mode, CBP,bits,64,1);
      } 

      MB_Decode(qcoeff, data, QP, Mode);  //反变换
      Clip(data);                       //使 0<=data<=255 
      ReconImage(i,j,data,recon);     //将重建宏块data输出到整个图像recon中
      free(qcoeff);
    }
  }
  pic->QP_mean = (float)QP;

  free(data);
  return recon;
}

PictImage *CodeIntra_DCTVQ(PictImage *curr, Bits *bits, Pict *pic,int **Lcword,int**Mcword,int **Hcword, int gopnumber,int *ebtime,int &Au_times)
{
  PictImage *recon;
  MB_Structure *data = (MB_Structure *)malloc(sizeof(MB_Structure));
  int Mode =3;//  MODE_INTRA;
  int CBP,COD;
  int i,j,k=0,comp=0;
  int embedtime=0,indexnum=0;
  int frg_perframe=0, &frg_frame=frg_perframe;
  recon = InitImage(pels*lines);
  ZeroBits(bits);
  int index[6][3]={0},index_bit[3]={11,10,10};
  int *L_Index,*MH_Index;
  int QP=8;
  unsigned char robust_wm[64],fngprt[64]; 
  
  for(i=0;i<64;i++)
  { 
      robust_wm[i]=0;
      fngprt[i]=0;
	if(i%2==0)
	  robust_wm[i]=1;
    if(i%2==0)
	  fngprt[i]=1;
  }

  pic->QUANT =QP;
  bits->header += CountBitsPicture(pic);
  L_Index=new int [(lines/MB_SIZE)*(pels/MB_SIZE)*6];
  MH_Index=new int [(lines/MB_SIZE)*(pels/MB_SIZE)*6*2];
  COD = 0; 
 
  for ( j = 0; j < lines/MB_SIZE; j++) 
  {
    /* 如果允许GOB同步，则输出GOB同步头 */
    if (pic->use_gobsync && j != 0)
      bits->header += CountBitsSlice(j,QP);

    for ( i = 0; i < pels/MB_SIZE; i++) 
	{
      pic->MB = i + j * (pels/MB_SIZE);
      bits->no_intra++;
      FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, data);   //写亮度图像16*16 curr:图像数据 data:宏块数组
      FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, data); //写色度图像2*8*8 
      
      if((embedtime<64*6)&&(k%4==0))
	  {
		MB_DCTVQ_Embed(data,Lcword+gopnumber*2048,Mcword+gopnumber*1024,Hcword+gopnumber*1024,
			           fngprt+(embedtime%64),robust_wm+(embedtime%64),index,
                       2048,1024,1024,frg_frame,Au_times);
        embedtime+=4;
	  }
      else
	   MB_DCTVQ_Encode(data,Lcword+gopnumber*2048,Mcword+gopnumber*1024,Hcword+gopnumber*1024,index,
                      2048,1024,1024,frg_frame,Au_times);

      for(comp=0;comp<6;comp++)
	  {
		 L_Index[indexnum+comp]=index[comp][0];
		 MH_Index[indexnum*2+comp*2]=index[comp][1];
         MH_Index[indexnum*2+comp*2+1]=index[comp][2];
	  }

	  indexnum+=6;
      k++;;

	  CBP = 63;
      if (!syntax_arith_coding) 
	  {
        CountBitsMB(Mode,COD,CBP,0,pic,bits);//输出宏块层信息
        Output_DCTVQIndex(index,index_bit);
      } 

      MB_DCTVQ_Decode(data,Lcword+gopnumber*2048,Mcword+gopnumber*1024,Hcword+gopnumber*1024,index);

      Clip(data);                     //使 0<=data<=255 
      ReconImage(i,j,data,recon);     //将重建宏块data输出到整个图像recon中
    }
  }
  pic->QP_mean = (float)QP;
  
  /*bits->Y=VQ_index_Huff(indexnum,2048,L_Index);
  bits->C=VQ_index_Huff(indexnum*2,1024,MH_Index);*/
  
  bits->total=bits->Y+bits->C;
  
  free(data);
  delete [] L_Index;
  delete [] MH_Index;
  return recon;
}

void MB_DCTbook_Gen(MB_Structure *mb_orig,int *LTrain[],int*MTrain[],int *HTrain[], int *lnum, int *mnum, int *hnum)
{
  int        i, j, k, l, row, col;
  int        fblock[64];
  int        coeff[384];
  int        *coeff_ind;

  coeff_ind = coeff;
 
  for (k=0;k<16;k+=8) 
  { // 此循环完成8×8分块
    for (l=0;l<16;l+=8) 
	{
      for (i=k,row=0;row<64;i++,row+=8)
	  {
        for (j=l,col=0;col<8;j++,col++)
		{
          fblock[row+col] = mb_orig->lum[i][j];
        }
      }
   
   Dct(fblock,coeff_ind); // DCT变换结果存于coeff_ind
   //划分三个频带,形成矢量
   memcpy(LTrain[*lnum],coeff_ind,sizeof(int)*dim_Low);
   (*lnum)++;
   memcpy(MTrain[*mnum],coeff_ind+dim_Low,sizeof(int)*dim_Mid);
   (*mnum)++;
   memcpy(HTrain[*hnum],coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
   (*hnum)++;

   coeff_ind += 64;
     
    }
  }

  for (i=0;i<8;i++) 
  {
    for (j=0;j<8;j++)
	{
      *(fblock+i*8+j) = mb_orig->Cb[i][j];
    }
  }
  
  Dct(fblock,coeff_ind);
 
  memcpy(LTrain[*lnum],coeff_ind,sizeof(int)*dim_Low);
  (*lnum)++;
  memcpy(MTrain[*mnum],coeff_ind+dim_Low,sizeof(int)*dim_Mid);
  (*mnum)++;
  memcpy(HTrain[*hnum],coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
  (*hnum)++;

  coeff_ind += 64;
  
  for (i=0;i<8;i++) 
  {
    for (j=0;j<8;j++) 
	{
      *(fblock+i*8+j) = mb_orig->Cr[i][j];
    }
  }

  Dct(fblock,coeff_ind);
  memcpy(LTrain[*lnum],coeff_ind,sizeof(int)*dim_Low);
  (*lnum)++;
  memcpy(MTrain[*mnum],coeff_ind+dim_Low,sizeof(int)*dim_Mid);
  (*mnum)++;
  memcpy(HTrain[*hnum],coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
  (*hnum)++;
  
}

PictImage *Intra_DCTbook_Gen(PictImage *curr, Bits *bits, Pict *pic, int *LTV[MAXIMAGESIZE],int *MTV[MAXIMAGESIZE],int *HTV[MAXIMAGESIZE],int TrainNumber[3])
{
  PictImage *recon;
  MB_Structure *data = (MB_Structure *)malloc(sizeof(MB_Structure));
  int Mode =3;//  MODE_INTRA;
  int COD;
  int i,j;
  int  intracounter=0;
  recon = InitImage(pels*lines);
  ZeroBits(bits);
  
  int QP=8;
  pic->QUANT = QP;
  bits->header += CountBitsPicture(pic);

  COD = 0; 
  for ( j = 0; j < lines/MB_SIZE; j++) 
  {
    // 如果允许GOB同步，则输出GOB同步头
    if (pic->use_gobsync && j != 0)
      bits->header += CountBitsSlice(j,QP);
    for ( i = 0; i < pels/MB_SIZE; i++) 
	{
      pic->MB = i + j * (pels/MB_SIZE);
      bits->no_intra++;
      FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, data);//写亮度图像16*16 curr:图像数据 data:宏块树组
      FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, data);//写色度图像2*8*8 
      MB_DCTbook_Gen(data,LTV,MTV,HTV,TrainNumber,TrainNumber+1,TrainNumber+2); 
    }
  }
  pic->QP_mean = (float)QP;

  free(data);
  return recon;
}

void Clip(MB_Structure *data)
{
  int m,n;

  for (n = 0; n < 16; n++) 
    for (m = 0; m < 16; m++) 
      data->lum[n][m] = mmin(255,mmax(0,data->lum[n][m]));
  for (n = 0; n < 8; n++) 
    for (m = 0; m < 8; m++) {
      data->Cr[n][m] = mmin(255,mmax(0,data->Cr[n][m]));
      data->Cb[n][m] = mmin(255,mmax(0,data->Cb[n][m]));
    }
  
}

/**********************************************************************/

void ReconImage (int i, int j, MB_Structure *data, PictImage *recon)
{
  int n;
  register int m;

  int x_curr, y_curr;

  x_curr = i * MB_SIZE;
  y_curr = j * MB_SIZE;

  /* Fill in luminance data */
  for (n = 0; n < MB_SIZE; n++)
    for (m= 0; m < MB_SIZE; m++) {
      *(recon->lum + x_curr+m + (y_curr+n)*pels) = (unsigned char)data->lum[n][m]; 
    }

  /* Fill in chrominance data */
  for (n = 0; n < (MB_SIZE>>1); n++)	
    for (m = 0; m < (MB_SIZE>>1); m++) {
      *(recon->Cr + (x_curr>>1)+m + ((y_curr>>1)+n)*cpels) = (unsigned char)data->Cr[n][m];
      *(recon->Cb + (x_curr>>1)+m + ((y_curr>>1)+n)*cpels) = (unsigned char)data->Cb[n][m];
    }
  return;
}


/***********************************************************************/

void Quant(int *coeff, int *qcoeff, int QP, int Mode)
{
  int i;
  for (i = 0; i < 64; i++) 
    qcoeff[i] = coeff[i]/(2*QP);
    qcoeff[0]=qcoeff[0]*2*QP/8;
  return;
}

/***********************************************************************/

void Dequant(int *qcoeff, int *rcoeff, int QP, int Mode)
{
  int i;
    for (i = 0; i < 64; i++)
      rcoeff[i] = qcoeff[i]*2*QP;
      rcoeff[0]=rcoeff[0]*8/(2*QP);
  return;
}

/***********************************************************************/

void FillLumBlock( int x, int y, PictImage *image, MB_Structure *data)
{
  int n;
  register int m;

  for (n = 0; n < MB_SIZE; n++)
    for (m = 0; m < MB_SIZE; m++)
      data->lum[n][m] = 
        (int)(*(image->lum + x+m + (y+n)*pels));
  return;
}

/***********************************************************************/

void FillChromBlock(int x_curr, int y_curr, PictImage *image,
            MB_Structure *data)
{
  int n;
  register int m;
  int x, y;

  x = x_curr>>1;
  y = y_curr>>1;

  for (n = 0; n < (MB_SIZE>>1); n++)
    for (m = 0; m < (MB_SIZE>>1); m++) {
      data->Cr[n][m] = 
        (int)(*(image->Cr +x+m + (y+n)*cpels));
      data->Cb[n][m] = 
        (int)(*(image->Cb +x+m + (y+n)*cpels));
    }
  return;
}

/***********************************************************************/

int *MB_Encode(MB_Structure *mb_orig, int QP, int I)
{
  int        i, j, k, l, row, col;
  int        fblock[64];
  int        coeff[384];
  int        *coeff_ind;
  int        *qcoeff;
  int        *qcoeff_ind;

  if ((qcoeff=(int *)malloc(sizeof(int)*384)) == 0) {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  coeff_ind = coeff;
  qcoeff_ind = qcoeff;
  for (k=0;k<16;k+=8) { // 此循环完成8×8分块
    for (l=0;l<16;l+=8) {
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          fblock[row+col] = mb_orig->lum[i][j];
        }
      }
      Dct(fblock,coeff_ind); // DCT变换结果存于coeff_ind
      Quant(coeff_ind,qcoeff_ind,QP,I); // 量化DCT系数
      coeff_ind += 64;
      qcoeff_ind += 64;
    }
  }
  
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cb[i][j];
    }
  }
  Dct(fblock,coeff_ind);
  Quant(coeff_ind,qcoeff_ind,QP,I); 
  coeff_ind += 64;
  qcoeff_ind += 64;
  
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cr[i][j];
    }
  }
  Dct(fblock,coeff_ind);
  Quant(coeff_ind,qcoeff_ind,QP,I); 
  return qcoeff;
}




int *MB_EncodeVQ(MB_Structure *mb_orig, int *codeword[MAXBOOKSIZE],int QP, int I,int index[6],int booksize)
{
  int        i, j, k, l, row, col,num,c=0;
  int        fblock[64];
  int        coeff[384];
  int        *coeff_ind;
  int        *qcoeff;
  int        *qcoeff_ind;
  int        s=0;
  double     min=0,d=0;
 
  if ((qcoeff=(int *)malloc(sizeof(int)*384)) == 0) {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }
  coeff_ind = coeff;
  qcoeff_ind = qcoeff;
  for (k=0;k<16;k+=8) { // 此循环完成8×8分块
    for (l=0;l<16;l+=8) {
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          fblock[row+col] = mb_orig->lum[i][j];
        }
      }
    min=VectorDistance(64,fblock,*codeword);
		index[c]=0;
  for(num=1;num<booksize;num++)
  {
    d=0;
	for(s=0;s<64;s++)
	{
	   d=d+(*(fblock+s)-*(*(codeword+num)+s))*(*(fblock+s)-*(*(codeword+num)+s));
	  // if(d>min)
	  // break;
	}
	if(d<=min)
	{
		index[c]=num;
		min=d;
		d=0;	
	}
  }
      c++;
	  coeff_ind += 64;
      qcoeff_ind += 64;
	}
  }

/***************************************************************************
// test code
  CString test,tempchar;
  for(i=0;i<64;i++)
  {
  if((i%7==0)&&(i>0))
  tempchar.Format("%3d\n",fblock[i]);
  else
  tempchar.Format("%3d",fblock[i]);
  test+=tempchar;
  }
  for(c=0;c<4;c++)
  {
    tempchar.Format("%4d,",index[c]);
	test+=tempchar;
  }
  AfxMessageBox(test);
  
/***************************************************************************/
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cb[i][j];
    }
  }

        min=VectorDistance(64,fblock,*codeword);
		index[c]=0;
		for(num=1;num<booksize;num++)
		{
			d=0;
			for(s=0;s<64;s++)
			{
				d=d+(*(fblock+s)-*(*(codeword+num)+s))*(*(fblock+s)-*(*(codeword+num)+s));
			//	if(d>min)
				//	break;
			}
	    	if(d<=min)
			{
				index[c]=num;
				min=d;
				d=0;	
			}
		}

	  c++;
  coeff_ind += 64;
  qcoeff_ind += 64;

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cr[i][j];
    }
  }
   min=VectorDistance(64,fblock,*codeword);
		index[c]=0;
 
		for(num=1;num<booksize;num++)
		{
			d=0;
			for(s=0;s<64;s++)
			{
				d=d+(*(fblock+s)-*(*(codeword+num)+s))*(*(fblock+s)-*(*(codeword+num)+s));
			//	if(d>min)
			//	break;
			}
		 if(d<=min)
			{
				index[c]=num;
				min=d;
				d=0;	
			}
		}
 
return 0;
}



//Embed Fragile watermark into VQ indices
void FragileWm_EmbedVQ(MB_Structure *mb_orig, int *codeword[MAXBOOKSIZE],int booksize,int index[6],unsigned char *wmb,int pos)
{
  int       i, j, k, l, row, col,num,c=0;
  int       fblock[64];
  int       s=0;
  int       min=0,d=0;	
  CGetpic pic;
 
  for (k=0;k<16;k+=8) { // 此循环完成8×8分块
    for (l=0;l<16;l+=8) {
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          fblock[row+col] = mb_orig->lum[i][j];
        }
      }
  

   index[c]=(int)(pow(2,pos)*wmb[c]);
   min=0;
   for(s=0;s<64;s++)
   {
    min+=(*(fblock+s)-*(*(codeword+index[c])+s))*(*(fblock+s)-*(*(codeword+index[c])+s));
   }

  for(num=0;num<booksize;num++)
  {
   if(*(pic.CheckPosition(num,8)+pos)==wmb[c])
	{
	d=0;
	for(s=0;s<64;s++)
	{
	   d=d+(*(fblock+s)-*(*(codeword+num)+s))*(*(fblock+s)-*(*(codeword+num)+s));
	}
	if(d<=min)
	{
		index[c]=num;
		min=d;
		d=0;	
	}
    
   }
  } 
   c++;
	}
  }

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cb[i][j];
    }
  }

        index[c]=(int)(pow(2,pos)*wmb[c]);
		min=0;
        for(s=0;s<64;s++)
		{
		 min+=(*(fblock+s)-*(*(codeword+index[c])+s))*(*(fblock+s)-*(*(codeword+index[c])+s));
		}
		for(num=0;num<booksize;num++)
		{
			if(*(pic.CheckPosition(num,8)+pos)==wmb[c])
			{
			   d=0;
			   for(s=0;s<64;s++)
			   {
				d=d+(*(fblock+s)-*(*(codeword+num)+s))*(*(fblock+s)-*(*(codeword+num)+s));
		    	//	if(d>min)
				//	break;
			   }
	           if(d<=min)
			   {
				index[c]=num;
				min=d;
				d=0;	
			   }
			}
		}
      c++;

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cr[i][j];
    }
  }
        
        index[c]=(int)(pow(2,pos)*wmb[c]);
		min=0.0;
        for(s=0;s<64;s++)
		{
		 min+=(*(fblock+s)-*(*(codeword+index[c])+s))*(*(fblock+s)-*(*(codeword+index[c])+s));
		}
		for(num=0;num<booksize;num++)
		{
			if(*(pic.CheckPosition(num,8)+pos)==wmb[c])
			{
			d=0;
			for(s=0;s<64;s++)
			{
				d=d+(*(fblock+s)-*(*(codeword+num)+s))*(*(fblock+s)-*(*(codeword+num)+s));
			//	if(d>min)
			//	break;
			}
		    if(d<=min)
			{
				index[c]=num;
				min=d;
				d=0;	
			}

			}
		}
       
}


//Water VQ 
void Watermark_EncodeVQ(int index[6],unsigned char wmb[6],int booksize)
{
 
/////////////////以下程序实现索引的水印嵌入///////////////////////////
  int i;
  for(i=0;i<6;i++)
  {
	if(wmb[i]!=0)//嵌入水印
	index[i]=booksize+3*index[i]+(*(wmb+i)-1);
  }
  
}

// Embed watermark in motion vector
void MV_WatermarkEmbed(MotionVector *mv, unsigned char *wmb,int Mode)
{
  int angle;
  angle=GetDirection(mv->x,mv->y);
  //找到半象素的角度值
  if(*wmb>0)
  {
   if((*wmb)%2==0)
   {
	angle=angle+(*wmb)/2;
	if(angle>7)
	angle=angle-8;
   }
   if((*wmb)%2!=0)
   {
	angle=angle-((*wmb)+1)/2;
	if(angle<0)
	angle=angle+8;
   }
  }

  //调整运动矢量的奇偶性
  Getsymbol(angle,mv);
  
  //调整半象素精度
  if(angle%2==1)
  {
	if(mv->x>0)
	mv->x_half=-1;
    if(mv->x<0)
	mv->x_half=1;
	if(mv->y>0)
	mv->y_half=-1;
	if(mv->y<0)
	mv->y_half=1;
  }
  if(angle==0||angle==4)
  {
	mv->y_half=0;
	if(mv->x>0)
    mv->x_half=-1;
    if(mv->x<0)
    mv->x_half=1;
  }
  if(angle==2||angle==6)
  { 
	mv->x_half=0;
	if(mv->y>0)
	mv->y_half=-1;
	if(mv->y<0)
	mv->y_half=1;
  }
  
}



//得到MV的特征角度
int GetDirection(int x,int y)
{
  int a;
  if((x>0)&&(y==0))//0
      a=0;
  if((x>0)&&(y>0))//45
      a=1;
  if((x==0)&&(y>0))//90
      a=2;
  if((x<0)&&(y>0))//135
      a=3;
  if((x<0)&&(y==0))//180
	  a=4;
  if((x<0)&&(y<0))//225
	  a=5;
  if((x==0)&&(y<0))//270
	  a=6;
  if((x>0)&&(y<0))//315
	  a=7;
return a;
}



void Getsymbol(int t,MotionVector *mv)
{   

	
   if(t==1) //45 偶偶
	{   
		if((mv->x%2!=0)&&(mv->x<0))
		mv->x=mv->x-1;
        if((mv->x%2!=0)&&(mv->x>0))
		mv->x=mv->x+1;
		if((mv->y%2!=0)&&(mv->y<0))
		mv->y=mv->y-1;
        if((mv->y%2!=0)&&(mv->y>0))
		mv->y=mv->y+1;
	}
	if(t==3)//135 奇偶
	{
		if((mv->x%2==0)&&(mv->x<0))
			mv->x=mv->x-1;
        if((mv->x%2==0)&&(mv->x>0))
			mv->x=mv->x+1;
		if((mv->y%2!=0)&&(mv->y<0))
			mv->y=mv->y-1;
        if((mv->y%2!=0)&&(mv->y>0))
			mv->y=mv->y+1;
	}
	if(t==5)//225 奇奇
	{
		if((mv->x%2==0)&&(mv->x<0))
			mv->x=mv->x-1;
        if((mv->x%2==0)&&(mv->x>0))
			mv->x=mv->x+1;
		if((mv->y%2==0)&&(mv->y<0))
			mv->y=mv->y-1;
        if((mv->y%2==0)&&(mv->y>0))
			mv->y=mv->y+1;
	}
	if(t==7)//315 偶奇
	{
		if((mv->x%2!=0)&&(mv->x<0))
			mv->x=mv->x-1;
        if((mv->x%2!=0)&&(mv->x>0)) 
			mv->x=mv->x+1;
		if((mv->y%2==0)&&(mv->y<0))
			mv->y=mv->y-1;
        if((mv->y%2==0)&&(mv->y>0))
			mv->y=mv->y+1;
	}
	if(t==2)//90 偶
	{
		if((mv->y%2!=0)&&(mv->y<0))
			mv->y=mv->y-1;
        if((mv->y%2!=0)&&(mv->y>0))
			mv->y=mv->y+1;
	}
	if(t==6)//270 奇
	{
		if((mv->y%2==0)&&(mv->y<0))
			mv->y=mv->y-1;
        if((mv->y%2==0)&&(mv->y>0))
			mv->y=mv->y+1;
	}
   	if(t==0)//0 偶
	{
		if((mv->x%2!=0)&&(mv->x<0))
			mv->x=mv->x-1;
        if((mv->x%2!=0)&&(mv->x>0))
			mv->x=mv->x+1;
	}
    if(t==4)//180 奇
	{
		if((mv->x%2==0)&&(mv->x<0))
			mv->x=mv->x-1;
        if((mv->x%2==0)&&(mv->x>0))
			mv->x=mv->x+1;
	}
}

//基于系数(量化后)统计特性的水印
void WM_intra(unsigned char *keymark,int *Quancoeff,int *times,bool multi)
{
	int i=0,t=0,n=0,sum=0,aver;
    int feature[6];
   
	for(t=0;t<6;t++)
       sum+=*(Quancoeff+64*t);
    	aver=sum/6;
	
	for(t=0;t<6;t++)
	{	
		feature[t]=(*(Quancoeff+64*t)>=aver)?1:0;
		  if(multi)
            *(keymark+*(times)+t)=secondsingalbit[(*(times)+t)%2304]^feature[t];
		  else
			*(keymark+*(times)+t)=singalbit[(*(times)+t)%2304]^feature[t];
	}
	*(times)+=6;

	sum=0;
}


// 用VQ进行帧间编码
void CodeOneInterVQ(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic, int framenum)
{

	ZeroBits(bits); // 清零
	// CEncodeDlg encode;
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int i,j,k;
	 int newgob,Mode=0;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
     int  index[6],VQCBP[6];
   	 bool  vqencode=false;
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */

	 for(i=0;i<6;i++)
		VQCBP[i]=1;

	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}
	/* 整数和半象素运动估值 */
    MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
	                    pic->seek_dist, MV, pic->use_gobsync);


  #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      // 初始化码率控制 
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; // 复制旧 QP 
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; // 复制旧 QP 
    #endif

	dquant = 0; 

	for ( j = 0; j < lines/MB_SIZE; j++) {

    
  #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      // 更新QP      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     //更新dquant 
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	     
		 //当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) 
       
		 //Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         // MV[0][j+1][i+1]->Mode = Mode;
         
		 Mode=MV[0][j+1][i+1]->Mode;
		 pic->MB = i + j * (pels/MB_SIZE);

         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }

     	 /* INTER宏块 */
          if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
	       MB_EncodeVQ(diff,codeword+framenum*256,QP_xmitted,Mode,index,256); //对宏块数据（P块为残差数据）进行VQ量化
           CBP = FindVQCBP(diff,codeword+framenum*64,index,VQCBP);
		   vqencode=true;
		  }
		  else/*INTRA*/
		  {	
			qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
            CBP = FindCBP(qcoeff_P, Mode, 64); 
			vqencode=false;
		  }
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)&&(vqencode==false)) 
           ZeroMBlock(diff); //宏块数据设为0
         else if (vqencode==true) 
         MB_DecodeVQ(diff,codeword+framenum*256, QP_xmitted, Mode,index,VQCBP);//反变换INTER
		 else if (vqencode==false) 
	     MB_Decode(qcoeff_P,diff, QP_xmitted, Mode);//反变换INTRA
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
        
          if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0
		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          //当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 //并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码
         if (!syntax_arith_coding)
             CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
          else {   //正常编码宏块 
         if (!syntax_arith_coding) { //VLC 
              CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            else {
             //MODE_INTRA 或 MODE_INTRA_Q 
             bits->no_intra++;
             if (pic->PB)
               CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
		     if (vqencode)
			  CountBitsIndex(index,8,bits,VQCBP);//输出系数
             else  if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64,1);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		}//end Normal MB
   
         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
        //free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  //free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
  return;
}




int BookGenerate(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic,long int mn/*当前已有的训练矢量数目*/)
{
	ZeroBits(bits); // 清零
	// CEncodeDlg encode;
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int i,j,k;
	 int newgob,Mode,vecnum=0/*本帧的训练矢量数目*/;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
	 	 /* 缓冲器控制变量 */
	 bool  vqencode=false;

     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */
	 
	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值 */
    MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
	                    pic->seek_dist, MV, pic->use_gobsync);


    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        //bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        //bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);

         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		   if(mn+vecnum+6<MAXIMAGESIZE)
		   {
		     memorytrainvector(diff, trainvector, mn+vecnum);
		     vecnum=vecnum+6;
		   }
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }
         /* P或INTRA宏块 */
          qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
          CBP = FindCBP(qcoeff_P, Mode, 64); 
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
           ZeroMBlock(diff); //宏块数据设为0
         else
           MB_Decode(qcoeff_P, diff, QP_xmitted, Mode);//反变换
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
        
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             ;//CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             //CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             //CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             //CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               ;//CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
            }
            if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              ;//CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         // free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
  return vecnum;

}


int VisualBookGenerate(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic,long int mn/*当前已有的训练矢量数目*/)
{
   	 ZeroBits(bits); // 清零
	 // CEncodeDlg encode;
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int i,j,k;
	 int TH=650;//code book
	 int newgob,Mode,vecnum=0/*本帧的训练矢量数目*/;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
	 	 /* 缓冲器控制变量 */
	 bool  vqencode=false;
     int   fastmove=0;
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */
	 
	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值 */
    MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
	                    pic->seek_dist, MV, pic->use_gobsync);


    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        //bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        //bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);
         
		 fastmove=((pow((2*MV[0][j+1][i+1]->x + MV[0][j+1][i+1]->x_half),2)+pow((2*MV[0][j+1][i+1]->y + MV[0][j+1][i+1]->y_half),2))>=TH)?1:0;

         if ((Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V)&&fastmove) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		   if(mn+vecnum+6<MAXIMAGESIZE)
		   {
		     memorytrainvector(diff, trainvector, mn+vecnum);
		     vecnum=vecnum+6;
		   }
		 }
         else if(Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) 
		 {
		   diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         
         else 
		 {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }
        
         /* P或INTRA宏块 */
          qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
          CBP = FindCBP(qcoeff_P, Mode, 64); 
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
           ZeroMBlock(diff); //宏块数据设为0
         else
           MB_Decode(qcoeff_P, diff, QP_xmitted, Mode);//反变换
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
        
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             ;//CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             //CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             //CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             //CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               ;//CountBitsVectors(MV, bits, i, j, Mode, newgob, pic);
            }
            if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              ;//CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         // free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
  return vecnum;

}


double VectorDistance(int dimension, int *Train, int *Code)
{

int m;
double Vd=0;
 for(m=0;m<dimension;m++)
 {Vd=Vd+(*(Train+m)-*(Code+m))*(*(Train+m)-*(Code+m));}
 return Vd;

}

/************************************************************************/
     
int MB_Decode(int *qcoeff, MB_Structure *mb_recon, int QP, int I)
{
  int	i, j, k, l, row, col;
  int	*iblock;
  int	*qcoeff_ind;
  int	*rcoeff, *rcoeff_ind;

  if ((iblock = (int *)malloc(sizeof(int)*64)) == NULL) {
    fprintf(stderr,"MB_Coder: Could not allocate space for iblock\n");
    exit(-1);
  }
  if ((rcoeff = (int *)malloc(sizeof(int)*384)) == NULL) {
    fprintf(stderr,"MB_Coder: Could not allocate space for rcoeff\n");
    exit(-1);
  }  

  int *coeff_show;
  if ((coeff_show = (int *)malloc(sizeof(int)*384)) == NULL) 
  {
    fprintf(stderr,"MB_Coder: Could not allocate space for iblock\n");
    exit(-1);
  }

  for (i = 0; i < 16; i++)
    for (j = 0; j < 16; j++)
      mb_recon->lum[j][i] = 0;
  for (i = 0; i < 8; i++) 
    for (j = 0; j < 8; j++) 
	{
      mb_recon->Cb[j][i] = 0;
      mb_recon->Cr[j][i] = 0;
    }

  qcoeff_ind = qcoeff;
  rcoeff_ind = rcoeff;

  for (k=0;k<16;k+=8) 
  { 
    for (l=0;l<16;l+=8) 
	{
      Dequant(qcoeff_ind,rcoeff_ind,QP,I);
      idct(rcoeff_ind,iblock); 
      qcoeff_ind += 64;
      rcoeff_ind += 64;
      for (i=k,row=0;row<64;i++,row+=8)
	  {
        for (j=l,col=0;col<8;j++,col++) 
		{
          mb_recon->lum[i][j] = *(iblock+row+col); //0<=i,j<=15;
        }
      }
    }
  }
  
  Dequant(qcoeff_ind,rcoeff_ind,QP,I);
  idct(rcoeff_ind,iblock); 
  qcoeff_ind += 64;
  rcoeff_ind += 64;
  
  for (i=0;i<8;i++) 
  {
    for (j=0;j<8;j++)
	{
      mb_recon->Cb[i][j] = *(iblock+i*8+j);
    }
  }
  Dequant(qcoeff_ind,rcoeff_ind,QP,I);
  idct(rcoeff_ind,iblock); 

  for (i=0;i<8;i++) 
  {
    for (j=0;j<8;j++) 
	{
      mb_recon->Cr[i][j] = *(iblock+i*8+j);
    }
  }
  free(iblock);
  free(rcoeff);
  return 0;
}


/*此函数计算由不同运动矢量带来的失真
*mb_recon:真实差值
*qcoeff:变换系数
*/

int MB_Distortion(int *qcoeff, MB_Structure *mb_recon, int QP)
{
	int	i, j, k, l, row, col;
	int	*iblock;
	int	*qcoeff_ind;
	int	*rcoeff, *rcoeff_ind;
    int  distortion;

	if ((iblock = (int *)malloc(sizeof(int)*64)) == NULL) {
		fprintf(stderr,"MB_Coder: Could not allocate space for iblock\n");
		exit(-1);
	}
	if ((rcoeff = (int *)malloc(sizeof(int)*384)) == NULL) {
		fprintf(stderr,"MB_Coder: Could not allocate space for rcoeff\n");
		exit(-1);
	}  
	
			qcoeff_ind = qcoeff;
			rcoeff_ind = rcoeff;
			
			for (k=0;k<16;k+=8) { 
				for (l=0;l<16;l+=8) {
					Dequant(qcoeff_ind,rcoeff_ind,QP,0);
					idct(rcoeff_ind,iblock); 
					qcoeff_ind += 64;
					rcoeff_ind += 64;
					for (i=k,row=0;row<64;i++,row+=8) {
						for (j=l,col=0;col<8;j++,col++) {
							distortion+=(mb_recon->lum[i][j]-*(iblock+row+col))*(mb_recon->lum[i][j]-*(iblock+row+col));
						}
					}
				}
			}

			free(iblock);
			free(rcoeff);
			return distortion;
}

int MB_DecodeVQ( MB_Structure *mb_recon, int *codeword[MAXBOOKSIZE],int QP, int I,int index[6],int indicator[6])
{
 
  int	i, j, k, l, row, col,c=0;
  for (i = 0; i < 16; i++)
    for (j = 0; j < 16; j++)
      mb_recon->lum[i][j] = 0;
  for (i = 0; i < 8; i++) 
    for (j = 0; j < 8; j++) {
      mb_recon->Cb[i][j] = 0;
      mb_recon->Cr[i][j] = 0;
    }

/**********************************Y分量解码********************************/
  for (k=0;k<16;k+=8) { 
    for (l=0;l<16;l+=8) {
	   for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          mb_recon->lum[i][j] =codeword[index[c]][8*(i-(c/2)*8)+j%8];
        }
      }
	 c++; }
  }
   
/**********************************Cb分量解码********************************/ 

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      mb_recon->Cb[i][j] = codeword[index[c]][8*i+j];
    }
  }
  c++;
 
/**********************************Cr分量解码********************************/
	for (i=0;i<8;i++) {
	  for (j=0;j<8;j++) {
	    mb_recon->Cb[i][j] = codeword[index[c]][8*i+j];
		  }
	  }
  
  return 0;
}

void memorytrainvector(MB_Structure *mb_orig, int *trainvector[MAXIMAGESIZE],int m)
{
  int i, j, k, l, row, col,c=0;

 for (k=0;k<16;k+=8) { // 此循环完成8×8分块
    for (l=0;l<16;l+=8) {
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          *(*(trainvector+m+c)+row+col) = mb_orig->lum[i][j];
        }
      }
	  c++; 
    }
  }
  
  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
     *(*(trainvector+m+c)+8*i+j)= mb_orig->Cb[i][j];
    }
  }
 c++;
   for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(*(trainvector+m+c)+8*i+j) = mb_orig->Cr[i][j];
    }
  }
   	
}


void  LBG(long int m, int*TrainVectors[MAXIMAGESIZE], int * CodeVectors[MAXBOOKSIZE],int dimension,int booksize)
{
   long int i,l;
   short int j,k;
   double d0,d1,d=0,min,*dd,par;
   int *number,*index;
   
   number=new int [m];
   index=new int [m];
   dd=new double [dimension];

   //Initial codevector Using random number generator
   par=(double)(m-1)/(double)0x7fff;
   srand((unsigned)time(NULL));
   
   for(i=0;i<booksize;i++)
   {
     l=(unsigned long int)((double)rand()*par);
 	 memcpy(CodeVectors[i],TrainVectors[l],dimension*sizeof(int));
   }
   d0=9.99e+60;   
   
   for(;;)
   {  
     d1=0.0;
 	 for(j=0;j<booksize;j++)
	   number[j]=0;
	 for(i=0;i<m;i++)
	 {
	    index[i]=0;
		d=0.0;
	   for(k=0;k<dimension;k++)
	   {  
         d=d+((double)TrainVectors[i][k]-(double)CodeVectors[0][k])*((double)TrainVectors[i][k]-(double)CodeVectors[0][k]);
	   }
	    min=d;
		for(j=1;j<booksize;j++)
		{
		  d=0.0;
          for(k=0;k<dimension;k++)
		  {  
             d=d+((double)TrainVectors[i][k]-(double)CodeVectors[j][k])*((double)TrainVectors[i][k]-(double)CodeVectors[j][k]);
		    if(d>min)//若当前距离d>min则跳出
			{d=0;
	        break;}
		  if((d<min)&&(k==dimension-1))
		  {
		    min=d;
		    index[i]=j;
			d=0;
		  }
		 }//end of k
		}
	    number[index[i]]=number[index[i]]+1;
		d1=d1+min;
	 }
	 if((d0-d1)/d1<=0.00001)break;
	 d0=d1;
	 for(j=0;j<booksize;j++)
	 {
	    if(number[j]!=0)
		{
		  for(k=0;k<dimension;k++)
		  {
		    dd[k]=0.0;
		  }
		  for(l=0;l<m;l++)
		  {
		    if(index[l]==j)
			{
			  for(k=0;k<dimension;k++)
			  {
    	        dd[k]=dd[k]+(double)TrainVectors[l][k];
			  }
			}
		  }
		  for(k=0;k<dimension;k++)
		  {
		    CodeVectors[j][k]=dd[k]/number[j];
		  }
		}
		else
		{
		  l=(unsigned long int)((double)rand()*par);
          memcpy(CodeVectors[j],TrainVectors[l],dimension*sizeof(int));
		}
	 }
   }
						
}

void Extendbook(int * CodeVectors[MAXBOOKSIZE],int num,int k,int booksize) //Extend basic codebook
{
// num:当前GOP码书之前的码字总数
// k:扩展码书的个数=3
//booksize:每个基本码书的大小
int i,m;
for(i=0;i<booksize;i++)
{
//Get extend codeword for each basic codeword
  for(m=0;m<k;m++)
  {
	memcpy(CodeVectors[num+booksize+m+3*i],CodeVectors[num+i],64*sizeof(int));//Copy basic codeword 
    ExtCodeword(CodeVectors[num+booksize+m],m,64);//Extend codeword
  }
}
}

void ExtCodeword(int *CodeVectors, int m, int dimension)//Extend codeword
{   
	double par;
	int k;
	int change[3];
	for(k=0;k<=m;k++)//m=0,1,2
	{
	par=(double)(dimension-1)/(double)0x7fff;//产生3随即数
	srand((unsigned)time(NULL));
	change[k]=(int)((double)rand()*par);
	*(CodeVectors+change[k])=*(CodeVectors+change[k])+1;
	}
}


void CodeOneInterVisualVQ(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
  			      int QP, int frameskip, Bits *bits, Pict *pic, int framenum,int *times)
{
	 ZeroBits(bits); // 清零
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int  index[6],VQCBP[6]={1,1,1,1,1,1};
     bool  vqencode=false;
	 int i,j,k,TH=600;//VQ
	 int newgob,Mode;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
     bool fastmove=0;// 是否快速运动块
	 /* 缓冲器控制变量 */
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */
    
	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值 */
		MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
		pic->seek_dist, MV, pic->use_gobsync);
	

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);
         
         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }

		 fastmove=((pow((2*MV[0][j+1][i+1]->x + MV[0][j+1][i+1]->x_half),2)+pow((2*MV[0][j+1][i+1]->y + MV[0][j+1][i+1]->y_half),2))>=TH)?1:0;
         /* P或INTRA宏块 */
         if((Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V)&&fastmove)
         {
		   MB_EncodeVQ(diff,codeword+framenum*256,QP_xmitted,Mode,index,256); //对宏块数据（P块为残差数据）进行VQ量化
		   CBP =63;// FindVQCBP(diff,codeword+framenum*64,index,VQCBP);
		   vqencode=true;
		   fastmove=0;	
		 }
		 else
		 {
		   qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
           CBP = FindCBP(qcoeff_P, Mode, 64);
		   vqencode=false;
		 }
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
         ZeroMBlock(diff); //宏块数据设为0
         else if (vqencode==true) 
         MB_DecodeVQ(diff,codeword+framenum*256, QP_xmitted, Mode,index,VQCBP);//反变换INTER
		 else if (vqencode==false) 
	     MB_Decode(qcoeff_P,diff, QP_xmitted, Mode);//反变换INTRA
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
        
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            if (vqencode==true)
              CountBitsIndex(index,8, bits,VQCBP);
			else if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64,1);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         if(!vqencode)
		 free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
  return;
}


//VQ watermark in video sequence
void  WatermarkVisualVQ(PictImage *prev,PictImage *curr,
						PictImage *pr,PictImage *curr_recon,
						int QP, int frameskip, Bits *bits, Pict *pic,int framenum,int *embedtimes,int *embedtimes2,bool multi)
{
	ZeroBits(bits); // 清零
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int  index[6],VQCBP[6];
     bool  vqencode=false;
	 int i,j,k,TH=15;//codebook partition
	 int newgob,Mode;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
     bool fastmove=0;// 是否快速运动块
	 /* 缓冲器控制变量 */
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */

	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值 */
   	if(multi)// 运动矢量嵌水印(多功能)
		MEPictureWatermark(curr->lum, prev_recon->lum, prev_ipol,
		pic->seek_dist, MV, pic->use_gobsync,embedtimes2,1);
    else
		MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
		pic->seek_dist, MV, pic->use_gobsync);

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);
         
         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }

		 fastmove=((fabs(2*MV[0][j+1][i+1]->x + MV[0][j+1][i+1]->x_half)+fabs(2*MV[0][j+1][i+1]->y + MV[0][j+1][i+1]->y_half)>=TH))?1:0;
       	 /* P或INTRA宏块 */
         if((Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V)&&fastmove)
         {
		   MB_EncodeVQ(diff,codeword+framenum*256,QP_xmitted,Mode,index,64); //对宏块数据（P块为残差数据）进行VQ量化
           Watermark_EncodeVQ(index,markbit+(*embedtimes)*6,64);//将水印嵌入码字索引
		   CBP=63;//FindVQCBP(diff,codeword+framenum*64,index,VQCBP);
		   vqencode=true;
		   fastmove=0;
           (*embedtimes)++;//嵌入水印的次数
		  }
		 else
		 {qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
          CBP = FindCBP(qcoeff_P, Mode, 64);
		  vqencode=false;
		 }
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
           ZeroMBlock(diff); //宏块数据设为0
         else if (vqencode==true) 
         MB_DecodeVQ(diff,codeword+framenum*256, QP_xmitted, Mode,index,VQCBP);//反变换INTER
		 else if (vqencode==false) 
	     MB_Decode(qcoeff_P,diff, QP_xmitted, Mode);//反变换INTRA
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
		 
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            if (vqencode==true)
			{  
			 CountBitsIndex(index,8,bits,VQCBP); 
			}
			else if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64,1);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         if(!vqencode)
		 free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
		  
  return;
}


//VQ Semi-fragile watermark in video sequence
void Semi_frg_Wm_VisualVQ(PictImage *prev,PictImage *curr,
						PictImage *pr,PictImage *curr_recon,
						int QP, int frameskip, Bits *bits, Pict *pic,int framenum,int *embedtimes)
{
	 ZeroBits(bits); // 清零
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int  index[6],VQCBP[6];
     bool  vqencode=false;
	 int i,j,k,TH=650;//fragile
	 int newgob,Mode;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
     bool fastmove=0;// 是否快速运动块
	 /* 缓冲器控制变量 */
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */
     
	 int tempnumber=0;

	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值 */
   
		MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
		pic->seek_dist, MV, pic->use_gobsync);

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);
         
         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }

		 fastmove=((pow((2*MV[0][j+1][i+1]->x + MV[0][j+1][i+1]->x_half),2)+pow((2*MV[0][j+1][i+1]->y + MV[0][j+1][i+1]->y_half),2))>=TH)?1:0;
         
		 /* P或INTRA宏块 */
         if((Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V)&&fastmove)
         {
		    if(tempnumber<120)
			{
			  FragileWm_EmbedVQ(diff,codeword+framenum*256,256,index,originalsingalbit+(*embedtimes)%4096,3);
			  tempnumber+=6;
			  (*embedtimes)+=6;//嵌入水印的次数
			}
			else
			 MB_EncodeVQ(diff,codeword+framenum*256,QP_xmitted,Mode,index,256); //对宏块数据（P块为残差数据）进行VQ量化
			
			CBP =63;// FindVQCBP(diff,codeword+framenum*64,index,VQCBP);
		    vqencode=true;
		    fastmove=0;
		  }
		 else
		 {
		  qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
          CBP = FindCBP(qcoeff_P, Mode, 64);
		  vqencode=false;
		 }
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
           ZeroMBlock(diff); //宏块数据设为0
         else if (vqencode==true) 
         MB_DecodeVQ(diff,codeword+framenum*256, QP_xmitted, Mode,index,VQCBP);//反变换INTER
		 else if (vqencode==false) 
	     MB_Decode(qcoeff_P,diff, QP_xmitted, Mode);//反变换INTRA
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
		 
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            if (vqencode==true)
			{  
			 CountBitsIndex(index,8,bits,VQCBP); 
			}
			else if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64,1);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         if(!vqencode)
		 free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;
  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
		  
  return;
}



/*速率-失真最优帧间编码*/
void  RDOptCodeInter(PictImage *prev,PictImage *curr,
					 PictImage *pr,PictImage *curr_recon,
		    		  int QP, int frameskip, Bits *bits, Pict *pic,long int*sum,long int*bit, int*num)
{
     ZeroBits(bits); // 清零
	 unsigned char *prev_ipol,*pi,*orig_lum;
     PictImage *prev_recon=NULL;
	 MotionVector *MV[6][MBR+1][MBC+2]; 
	 MotionVector ZERO = {0,0,0,0,0};
	 int i,j,k;
	 int newgob,Mode;
  	 int *qcoeff_P;
	 int CBP, CBPB=0;
     MB_Structure *recon_data_P; 
     MB_Structure *diff; 
	 /* 缓冲器控制变量 */
     float QP_cumulative = (float)0.0;
     int abs_mb_num = 0, QuantChangePostponed = 0;
     int QP_new, QP_prev, dquant, QP_xmitted=QP;
     QP_new = QP_xmitted = QP_prev = QP; /* 复制旧QP */

	 /* 图象插值 */
	 if(!mv_outside_frame)
	 {
	 pi = InterpolateImage(pr->lum,pels,lines);
     prev_ipol = pi;
     prev_recon = pr; 
     orig_lum = prev->lum;
	 }
	 
     /* 为每个编码块标记MV */
    for (i = 1; i < (pels>>4)+1; i++) {
      for (k = 0; k < 6; k++) {
        MV[k][0][i] = (MotionVector *)malloc(sizeof(MotionVector));
        MarkVec(MV[k][0][i]);
	  }
      MV[0][0][i]->Mode = MODE_INTRA;
	}
    /* 超出图象边界的MV置为0 */
    for (i = 0; i < (lines>>4)+1; i++) { // 每列的第一个和最后一个宏块
      for (k = 0; k < 6; k++) {
        MV[k][i][0] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][0]);
        MV[k][i][(pels>>4)+1] = (MotionVector *)malloc(sizeof(MotionVector));
        ZeroVec(MV[k][i][(pels>>4)+1]);
	  }
      MV[0][i][0]->Mode = MODE_INTRA;
      MV[0][i][(pels>>4)+1]->Mode = MODE_INTRA;
	}

	/* 整数和半象素运动估值,并得到速率－最优的运动矢量*/
	
	/*MotionEstimatePicture(curr->lum, prev_recon->lum, prev_ipol,
		pic->seek_dist, MV, pic->use_gobsync);*/
	
	RDMotEstPicture(curr->lum, prev_recon->lum, pic,  
		            prev_ipol,prev_recon, pic->seek_dist, 
		            MV, pic->use_gobsync, QP);

#ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {
      /* 初始化码率控制 */
      QP_new = InitializeQuantizer(PCT_INTER, (float)pic->bit_rate, 
               (pic->PB ? pic->target_frame_rate/2 : pic->target_frame_rate),
               pic->QP_mean);
      QP_xmitted = QP_prev = QP_new; 
	  }
      else {
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
	  }
    #else
      QP_new = QP_xmitted = QP_prev = QP; /* 复制旧 QP */
    #endif

	dquant = 0; 
	for ( j = 0; j < lines/MB_SIZE; j++) {

    #ifndef OFFLINE_RATE_CONTROL
      if (pic->bit_rate != 0) {

      AddBitsPicture(bits);
      /* 更新QP */      
      QP_new =  UpdateQuantizer(abs_mb_num, pic->QP_mean, PCT_INTER, 
           (float)pic->bit_rate, pels/MB_SIZE, lines/MB_SIZE, 
           bits->total);
	  }
    #endif

      newgob = 0;
      if (j == 0) {
        pic->QUANT = QP;
        bits->header += CountBitsPicture(pic);//计算图象层码字
	  }
      else if (pic->use_gobsync && j%pic->use_gobsync == 0) {
        bits->header += CountBitsSlice(j,QP); //输出GOB同步头
        newgob = 1;
	  }
	  for ( i = 0; i < pels/MB_SIZE; i++) {
	     /* 更新dquant */
         dquant = QP_new - QP_prev;
         if (dquant != 0 && i != 0 && MV[0][j+1][i+1]->Mode == MODE_INTER4V) {
           dquant = 0;
           QP_xmitted = QP_prev;
           QuantChangePostponed = 1;
		 }
         else {
           QP_xmitted = QP_new;
           QuantChangePostponed = 0;
		 }
		 if (dquant > 2)  { dquant =  2; QP_xmitted = QP_prev + dquant;}
         if (dquant < -2) { dquant = -2; QP_xmitted = QP_prev + dquant;}

         pic->DQUANT = dquant;
	
         /* 当dquant != 0，修改宏块类型 (例如 MODE_INTER -> MODE_INTER_Q) */
         Mode = ModifyMode(MV[0][j+1][i+1]->Mode,pic->DQUANT);
         MV[0][j+1][i+1]->Mode = Mode;

		 pic->MB = i + j * (pels/MB_SIZE);

         if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) {
         /* 预测P-宏块 */
           diff = Predict_P(curr,prev_recon,prev_ipol,
           i*MB_SIZE,j*MB_SIZE,MV,pic->PB);
		 }
         else {
           diff = (MB_Structure *)malloc(sizeof(MB_Structure));
           FillLumBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写亮度图像  curr:图像数据 diff:宏块数组
           FillChromBlock(i*MB_SIZE, j*MB_SIZE, curr, diff);//写色度图像
		 }

         /* P或INTRA宏块 */
         qcoeff_P = MB_Encode(diff, QP_xmitted, Mode); //对宏块数据（P块为残差数据）进行DCT变换量化
         CBP = FindCBP(qcoeff_P, Mode, 64); 
		 
         if (CBP == 0 && (Mode == MODE_INTER || Mode == MODE_INTER_Q)) 
           ZeroMBlock(diff); //宏块数据设为0
         else
           MB_Decode(qcoeff_P, diff, QP_xmitted, Mode);//反变换
         recon_data_P = MB_Recon_P(prev_recon, prev_ipol,diff,i*MB_SIZE,j*MB_SIZE,MV,pic->PB);//重建P图象
         Clip(recon_data_P); //使 0<=recon_data_P<=255 
         free(diff);
        
         if(pic->PB==0)
         ZeroVec(MV[5][j+1][i+1]); //PB帧矢量差置为0

		 if ((CBP==0) && (CBPB==0) && 
			 (EqualVec(MV[0][j+1][i+1],&ZERO)) && 
             (EqualVec(MV[5][j+1][i+1],&ZERO)) &&
             (Mode == MODE_INTER || Mode == MODE_INTER_Q)) {
          /* 当 CBP 和 CBPB 为0, 16x16 运动矢量为0,PB矢量差为0，
			 并且编码模式为MODE_INTER或MODE_INTER_Q，跳过该宏块编码*/
         if (!syntax_arith_coding)
             CountBitsMB(Mode,1,CBP,CBPB,pic,bits);//输出宏块层信息
		 }
         else {      /* 正常编码宏块 */
          if (!syntax_arith_coding) { /* VLC */
             CountBitsMB(Mode,0,CBP,CBPB,pic,bits);
            if (Mode == MODE_INTER  || Mode == MODE_INTER_Q) {
             bits->no_inter++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);//输出运动矢量数据
            }
            else if (Mode == MODE_INTER4V) {
             bits->no_inter4v++;
             CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            else {
             /* MODE_INTRA 或 MODE_INTRA_Q */
             bits->no_intra++;
             if (pic->PB)
               CountBitsVectors(MV, bits, i, j, Mode, newgob, pic,1);
            }
            if (CBP || Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
              CountBitsCoeff(qcoeff_P, Mode, CBP, bits, 64,1);//输出系数
		  } // end VLC 
         
         QP_prev = QP_xmitted;
		 }//end Normal MB

         abs_mb_num++;
         QP_cumulative += QP_xmitted;     
     
         ReconImage(i,j,recon_data_P,curr_recon);//重建图象
         free(recon_data_P);
         free(qcoeff_P);
	  }//end for i
	}//end for j

  pic->QP_mean = QP_cumulative/(float)abs_mb_num;

  /* 释放内存 */
  free(pi);
  for (j = 0; j < (lines>>4)+1; j++)
    for (i = 0; i < (pels>>4)+2; i++) 
      for (k = 0; k < 6; k++)
        free(MV[k][j][i]);
  return;
 }

