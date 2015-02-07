#include"stdafx.h"
#include"Global.h"
#include<math.h>
#include"Getpic.h"

extern unsigned char secondsingalbit[4096];
extern unsigned char originalsingalbit[4096];
extern unsigned char p_fragile[4096];
extern int dim_Low,dim_Mid,dim_Hig;

void MB_DCTVQ_Encode(MB_Structure *mb_orig, int **codewordL,int **codewordM,int **codewordH,int index[6][3],
                     int booksize1,int booksize2,int booksize3,int &frg_frame,int &Au_num)
{
  int        i, j, k, l, row, col,c=0;
  int        fblock[64];
  int        coeff[384];
  int        *coeff_ind;
  int        *qcoeff;
  int        *qcoeff_ind;
  int        s=0,pos=8;
  double     mind=0,d=0;
  int        *Lcoeff_vector,*Mcoeff_vector,*Hcoeff_vector;
  unsigned char authen_bit[6];
  CGetpic pic;
  
  if ((qcoeff=(int *)malloc(sizeof(int)*384)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  if ((Lcoeff_vector=(int *)malloc(sizeof(int)*dim_Low)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  if ((Mcoeff_vector=(int *)malloc(sizeof(int)*dim_Mid)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  if ((Hcoeff_vector=(int *)malloc(sizeof(int)*dim_Hig)) == 0) 
  {
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
	  //划分三个频带,形成矢量
	  memcpy(Lcoeff_vector,coeff_ind,sizeof(int)*dim_Low);
	  memcpy(Mcoeff_vector,coeff_ind+dim_Low,sizeof(int)*dim_Mid);
	  memcpy(Hcoeff_vector,coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);

	  index[c][0]=VQ_Encode(Lcoeff_vector,codewordL,0,booksize1,dim_Low);
      index[c][1]=VQ_Encode(Mcoeff_vector,codewordM,0,booksize2,dim_Mid);

      if((frg_frame<1024)&&(Au_num<4096)&&(c<2))
      {
        unsigned char a[3];
        a[0]=*(pic.CheckPosition(index[c][0],11)+pos);
        a[1]=*(pic.CheckPosition(index[c][1],10)+pos);
        a[2]=*(p_fragile+Au_num%4096);
        authen_bit[c]=a[0]^a[1]^a[2];//认证位
        index[c][2]=VQ_Constrain(Hcoeff_vector,codewordH,0,booksize3,dim_Hig,pos,authen_bit[c]);
        frg_frame++;
        Au_num++;
      }
      else
      index[c][2]=VQ_Encode(Hcoeff_vector,codewordH,0,booksize3,dim_Hig);

      coeff_ind += 64;
      c++;
    }
  }

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cb[i][j];
    }
  }
      Dct(fblock,coeff_ind);
	  memcpy(Lcoeff_vector,coeff_ind,sizeof(int)*dim_Low);
	  memcpy(Mcoeff_vector,coeff_ind+dim_Low,sizeof(int)*dim_Mid);
	  memcpy(Hcoeff_vector,coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
      
	  index[c][0]=VQ_Encode(Lcoeff_vector,codewordL,0,booksize1,dim_Low);
      index[c][1]=VQ_Encode(Mcoeff_vector,codewordM,0,booksize2,dim_Mid);
      if((frg_frame<1024)&&(Au_num<4096))
      {
        unsigned char a[3];
        a[0]=*(pic.CheckPosition(index[c][0],11)+pos);
        a[1]=*(pic.CheckPosition(index[c][1],10)+pos);
        a[2]=*(p_fragile+Au_num%4096);
        authen_bit[c]=a[0]^a[1]^a[2];//认证位
        index[c][2]=VQ_Constrain(Hcoeff_vector,codewordH,0,booksize3,dim_Hig,pos,authen_bit[c]);
        frg_frame++;
        Au_num++;
      }
      else
      index[c][2]=VQ_Encode(Hcoeff_vector,codewordH,0,booksize3,dim_Hig);
  
      coeff_ind += 64;
      c++;

  for (i=0;i<8;i++) {
    for (j=0;j<8;j++) {
      *(fblock+i*8+j) = mb_orig->Cr[i][j];
    }
  }
      Dct(fblock,coeff_ind);
	  memcpy(Lcoeff_vector,coeff_ind,sizeof(int)*dim_Low);
	  memcpy(Mcoeff_vector,coeff_ind+dim_Low,sizeof(int)*dim_Mid);
	  memcpy(Hcoeff_vector,coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
   	  
      index[c][0]=VQ_Encode(Lcoeff_vector,codewordL,0,booksize1,dim_Low);
      index[c][1]=VQ_Encode(Mcoeff_vector,codewordM,0,booksize2,dim_Mid);
      
      if((frg_frame<1024)&&(Au_num<4096))
      {
        unsigned char a[3];
        a[0]=*(pic.CheckPosition(index[c][0],11)+pos);
        a[1]=*(pic.CheckPosition(index[c][1],10)+pos);
        a[2]=*(p_fragile+Au_num%4096);
        authen_bit[c]=a[0]^a[1]^a[2];//认证位
        index[c][2]=VQ_Constrain(Hcoeff_vector,codewordH,0,booksize3,dim_Hig,pos,authen_bit[c]);
        frg_frame++;
        Au_num++;
      }
      else
      index[c][2]=VQ_Encode(Hcoeff_vector,codewordH,0,booksize3,dim_Hig);

	  free(qcoeff);
	  free(Lcoeff_vector);
	  free(Mcoeff_vector);
	  free(Hcoeff_vector);
}


void MB_DCTVQ_Embed(MB_Structure *mb_orig, int **codewordL,int **codewordM,int **codewordH,
					 unsigned char Lwm[6],unsigned char Mwm[6],
					 int index[6][3],int booksize1,int booksize2,int booksize3,int &frg_frame,int &Au_num)
{
  int           i, j, k, l, row, col,c=0;
  int           fblock[64];
  int           coeff[384];
  int           *coeff_ind;
  int           *qcoeff;
  int           *qcoeff_ind;
  int           s=0,pos=8;
  int           *Lcoeff_vector,*Mcoeff_vector,*Hcoeff_vector;
  int           L_lowlim[6],L_higlim[6],M_lowlim[6],M_higlim[6];
  double        mind=0,d=0;
  unsigned char authen_bit[6];
  CGetpic       pic;

  if ((qcoeff=(int *)malloc(sizeof(int)*384)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    return;
  }

 if ((Lcoeff_vector=(int *)malloc(sizeof(int)*dim_Low)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  if ((Mcoeff_vector=(int *)malloc(sizeof(int)*dim_Mid)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }

  if ((Hcoeff_vector=(int *)malloc(sizeof(int)*dim_Hig)) == 0) 
  {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
  }


  for(int comp=0;comp<6;comp++)
  {
	if(Lwm[comp]==0)
	{
	  L_lowlim[comp]=0;
	  L_higlim[comp]=booksize1/2;
	}
	else
	{
	  L_lowlim[comp]=booksize1/2;
	  L_higlim[comp]=booksize1;
	}
    if(Mwm[comp]==0)
	{
	  M_lowlim[comp]=0;
	  M_higlim[comp]=booksize2/2;
	}
	else
    {
	  M_lowlim[comp]=booksize2/2;
	  M_higlim[comp]=booksize2;
	}
  }

  coeff_ind = coeff;
  qcoeff_ind = qcoeff;
  
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
	  memcpy(Lcoeff_vector,coeff_ind,sizeof(int)*dim_Low);
	  memcpy(Mcoeff_vector,coeff_ind+dim_Low,sizeof(int)*dim_Mid);
	  memcpy(Hcoeff_vector,coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
	  
      index[c][0]=VQ_Encode(Lcoeff_vector,codewordL,L_lowlim[c],L_higlim[c],dim_Low);
      index[c][1]=VQ_Encode(Mcoeff_vector,codewordM,M_lowlim[c],M_higlim[c],dim_Mid);
      
      if((frg_frame<1024)&&(Au_num<4096)&&(c<2))
      {
        unsigned char a[3];
        a[0]=*(pic.CheckPosition(index[c][0],11)+pos);
        a[1]=*(pic.CheckPosition(index[c][1],10)+pos);
        a[2]=*(p_fragile+Au_num%4096);
        authen_bit[c]=a[0]^a[1]^a[2];//认证位
        index[c][2]=VQ_Constrain(Hcoeff_vector,codewordH,0,booksize3,dim_Hig,pos,authen_bit[c]);
        frg_frame++;
        Au_num++;
      }
      else
      index[c][2]=VQ_Encode(Hcoeff_vector,codewordH,0,booksize3,dim_Hig);
      
      coeff_ind += 64;
      c++;
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
	  memcpy(Lcoeff_vector,coeff_ind,sizeof(int)*dim_Low);
	  memcpy(Mcoeff_vector,coeff_ind+dim_Low,sizeof(int)*dim_Mid);
	  memcpy(Hcoeff_vector,coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
      
      index[c][0]=VQ_Encode(Lcoeff_vector,codewordL,0,booksize1,dim_Low);
      index[c][1]=VQ_Encode(Mcoeff_vector,codewordM,0,booksize2,dim_Mid);
     
      if((frg_frame<1024)&&(Au_num<4096))
      {
        unsigned char a[3];
        a[0]=*(pic.CheckPosition(index[c][0],11)+pos);
        a[1]=*(pic.CheckPosition(index[c][1],10)+pos);
        a[2]=*(p_fragile+Au_num%4096);
        authen_bit[c]=a[0]^a[1]^a[2];//认证位
        index[c][2]=VQ_Constrain(Hcoeff_vector,codewordH,0,booksize3,dim_Hig,pos,authen_bit[c]);
        frg_frame++;
        Au_num++;
      }
      else
      index[c][2]=VQ_Encode(Hcoeff_vector,codewordH,0,booksize3,dim_Hig);

      coeff_ind += 64;
      c++;

     for (i=0;i<8;i++) 
     {
      for (j=0;j<8;j++) 
      {
        *(fblock+i*8+j) = mb_orig->Cr[i][j];  
      }
     }
     
      Dct(fblock,coeff_ind);
	  memcpy(Lcoeff_vector,coeff_ind,sizeof(int)*dim_Low);
	  memcpy(Mcoeff_vector,coeff_ind+dim_Low,sizeof(int)*dim_Mid);
	  memcpy(Hcoeff_vector,coeff_ind+(dim_Low+dim_Mid),sizeof(int)*dim_Hig);
      
      index[c][0]=VQ_Encode(Lcoeff_vector,codewordL,0,booksize1,dim_Low);
      index[c][1]=VQ_Encode(Mcoeff_vector,codewordM,0,booksize2,dim_Mid);
      if((frg_frame<1024)&&(Au_num<4096))
      {
        unsigned char a[3];
        a[0]=*(pic.CheckPosition(index[c][0],11)+pos);
        a[1]=*(pic.CheckPosition(index[c][1],10)+pos);
        a[2]=*(p_fragile+Au_num%4096);
        authen_bit[c]=a[0]^a[1]^a[2];//认证位
        index[c][2]=VQ_Constrain(Hcoeff_vector,codewordH,0,booksize3,dim_Hig,pos,authen_bit[c]);
        frg_frame++;
        Au_num++;
      }
      else
      index[c][2]=VQ_Encode(Hcoeff_vector,codewordH,0,booksize3,dim_Hig);
    
	free(qcoeff);  
}

void MB_DCTVQ_Decode(MB_Structure *mb_recon, int **codewordL,int **codewordM,int **codewordH,int index[6][3])
{
  int	i, j, k, l, row, col,c=0;
  int	*iblock;
  int	*qcoeff_ind;
  int	*rcoeff, *rcoeff_ind;

  if ((iblock = (int *)malloc(sizeof(int)*64)) == NULL) 
  {
    fprintf(stderr,"MB_Coder: Could not allocate space for iblock\n");
    exit(-1);
  }
  if ((rcoeff = (int *)malloc(sizeof(int)*384)) == NULL) 
  {
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

  rcoeff_ind = rcoeff;//解码后的DCT系数

  for (k=0;k<16;k+=8) 
  { 
    for (l=0;l<16;l+=8)
	{
      memcpy(rcoeff_ind,*(codewordL+index[c][0]),sizeof(int)*dim_Low);
	  memcpy(rcoeff_ind+dim_Low,*(codewordM+index[c][1]),sizeof(int)*dim_Mid);
	  memcpy(rcoeff_ind+(dim_Low+dim_Mid),*(codewordH+index[c][2]),sizeof(int)*dim_Hig);
      idct(rcoeff_ind,iblock); 

      qcoeff_ind += 64;
      rcoeff_ind += 64;
	  c++;
      for (i=k,row=0;row<64;i++,row+=8) {
        for (j=l,col=0;col<8;j++,col++) {
          mb_recon->lum[i][j] = *(iblock+row+col); //0<=i,j<=15;
        }
      }
    }
  }
  
   memcpy(rcoeff_ind,*(codewordL+index[c][0]),sizeof(int)*dim_Low);
   memcpy(rcoeff_ind+dim_Low,*(codewordM+index[c][1]),sizeof(int)*dim_Mid);
   memcpy(rcoeff_ind+(dim_Low+dim_Mid),*(codewordH+index[c][2]),sizeof(int)*dim_Hig);
   idct(rcoeff_ind,iblock); 

  rcoeff_ind += 64;
  for (i=0;i<8;i++) 
  {
    for (j=0;j<8;j++) 
	{
      mb_recon->Cb[i][j] = *(iblock+i*8+j);
    }
  }
  c++;

  memcpy(rcoeff_ind,*(codewordL+index[c][0]),sizeof(int)*dim_Low);
  memcpy(rcoeff_ind+dim_Low,*(codewordM+index[c][1]),sizeof(int)*dim_Mid); 
  memcpy(rcoeff_ind+(dim_Low+dim_Mid),*(codewordH+index[c][2]),sizeof(int)*dim_Hig);
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
}


