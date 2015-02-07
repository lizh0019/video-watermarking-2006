#include "stdafx.h"
#include"Global.h"
#include <math.h>
void Generate_P_Authenbits(MotionVector *MV,unsigned char *WM,unsigned char *P_Authbit)
{
  unsigned char MV_feature[2];
  MV_feature[0]=(unsigned char)fabs(MV->x_half%2);
  MV_feature[1]=(unsigned char)fabs(MV->y_half%2);
 
  for(int k=0;k<2;k++)
	P_Authbit[k]=MV_feature[k]^(*(WM+k));
}

int *MB_Fragile_Embed(MB_Structure *mb_orig, int QP, int I,unsigned char *Authen_bit)
{
  int        i, j, k, l, row, col,c=0;
  int        fblock[64];
  int        coeff[384];
  int        *coeff_ind;
  int        *qcoeff;
  int        *qcoeff_ind;
  int        pos=20;
  
  if ((qcoeff=(int *)malloc(sizeof(int)*384)) == 0) {
    fprintf(stderr,"mb_encode(): Couldn't allocate qcoeff.\n");
    exit(-1);
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
      Quant(coeff_ind,qcoeff_ind,QP,I); // 量化DCT系数
	  
	  if(c==1||c==2)//嵌入脆弱水印
	  {
	   if(((unsigned char)fabs(qcoeff_ind[pos]%2))^((unsigned char)fabs(qcoeff_ind[pos+1]%2))^((unsigned char)fabs(qcoeff_ind[pos+2]%2))!=Authen_bit[c-1])
         qcoeff_ind[pos+2]+=1;
	  }
      c++;
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