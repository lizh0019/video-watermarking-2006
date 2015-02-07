#include"stdafx.h"
#include"Global.h"
#include <math.h>
/**********************************************************************
 *
 *	Name:        MotionEstimation
 *	Description:	Estimate all motionvectors for one MB
 *	
 *	Input:        pointers to current an previous image,
 *        pointers to current slice and current MB
 *	Returns:	
 *	Side effects:	motion vector imformation in MB changed
 *   
 ***********************************************************************/


void MotionEstimation(unsigned char *curr, unsigned char *prev, int x_curr,
              int y_curr, int xoff, int yoff, int seek_dist, 
              MotionVector *MV[6][MBR+1][MBC+2], int *SAD_0)

{

  int Min_FRAME[5];
  MotionVector MV_FRAME[5];
  unsigned char *act_block,*aa,*ii;
  unsigned char *search_area, *adv_search_area = NULL, *zero_area = NULL;
  int sxy,i,k,j,l;
  int ihigh,ilow,jhigh,jlow,h_length,v_length;
  int adv_ihigh,adv_ilow,adv_jhigh,adv_jlow,adv_h_length,adv_v_length;
  int xmax,ymax,block,sad,lx;
  int adv_x_curr, adv_y_curr,xvec,yvec;

  xmax = pels;
  ymax = lines;
  sxy = seek_dist;
  if (!long_vectors) {//��0ʸ��Ϊ���ĵ�������Χ
    sxy = mmin(15, sxy);  
  }
  else {
    /* Maximum extended search range centered around _predictor_ */
    sxy = mmin(15 - (2*DEF_8X8_WIN+1), sxy);
    xoff = mmin(16,mmax(-16,xoff));
    yoff = mmin(16,mmax(-16,yoff));
  }
  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  ilow = x_curr + xoff - sxy;
  ihigh = x_curr + xoff + sxy;

  jlow = y_curr + yoff - sxy;
  jhigh = y_curr + yoff + sxy;

  if (!mv_outside_frame) {
    if (ilow<0) ilow = 0;
    if (ihigh>xmax-16) ihigh = xmax-16;
    if (jlow<0) jlow = 0;
    if (jhigh>ymax-16) jhigh = ymax-16;
  }

  h_length = ihigh - ilow + 16;
  v_length = jhigh - jlow + 16;
  act_block = LoadArea(curr, x_curr, y_curr, 16, 16, pels);  //װ�뵱ǰ������
  search_area = LoadArea(prev, ilow, jlow, h_length, v_length, lx);  //װ��������Χ�飺��ʼilow,jlow;��Сh_length, v_length

  for (k = 0; k < 5; k++) {
    Min_FRAME[k] = INT_MAX;
    MV_FRAME[k].x = 0;
    MV_FRAME[k].y = 0;
    MV_FRAME[k].x_half = 0;
    MV_FRAME[k].y_half = 0;
  }

  /* 0�˶�ʸ������ */
  if (x_curr-ilow         < 0        || y_curr-jlow         < 0        ||
      x_curr-ilow+MB_SIZE > h_length || y_curr-jlow+MB_SIZE > v_length) {
    /* 0�˶�ʸ��λ�ò���search_area�� */
    zero_area = LoadArea(prev, x_curr, y_curr, 16, 16, lx);  //װ��0�˶�ʸ����
    *SAD_0 = SAD_Macroblock(zero_area, act_block, 16, Min_FRAME[0]) -
       PREF_NULL_VEC;
    free(zero_area);
  }
  else {
    /* 0�˶�ʸ��λ����search_area�� */
    ii = search_area + (x_curr-ilow) + (y_curr-jlow)*h_length;
    *SAD_0 = SAD_Macroblock(ii, act_block, h_length, Min_FRAME[0]) - //�������������е�ǰ��λ�õĺ���뵱ǰ��֮��Ĳ�ֵ
       PREF_NULL_VEC;
  }

  if (xoff == 0 && yoff == 0) {
    Min_FRAME[0] = *SAD_0;
    MV_FRAME[0].x = 0;
    MV_FRAME[0].y = 0;
  }
  else {
    ii = search_area + (x_curr+xoff-ilow) + (y_curr+yoff-jlow)*h_length;
    Min_FRAME[0] = SAD_Macroblock(ii, act_block, h_length, Min_FRAME[0]);
    MV_FRAME[0].x = xoff;
    MV_FRAME[0].y = yoff;
  }
 
  /* �������� */
  for (l = 1; l <= sxy; l++) {
    i = x_curr + xoff - l;
    j = y_curr + yoff - l;
    for (k = 0; k < 8*l; k++) {
      if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {

        /* 16x16���������MV */
        ii = search_area + (i-ilow) + (j-jlow)*h_length; 
        sad = SAD_Macroblock(ii, act_block, h_length, Min_FRAME[0]);
        if (sad < Min_FRAME[0]) {
          MV_FRAME[0].x = i - x_curr;
          MV_FRAME[0].y = j - y_curr;
          Min_FRAME[0] = sad;
        }

      }
      if      (k<2*l) i++;
      else if (k<4*l) j++;
      else if (k<6*l) i--;
      else            j--;
    }      
  }


  if (advanced) {

    /* 8x8��������16x16ʸ��Ϊ���ģ���Ȼ����ȫ����������*/

    xvec = MV_FRAME[0].x;
    yvec = MV_FRAME[0].y;
    
    if (!long_vectors) {
      if (xvec > 15 - DEF_8X8_WIN) { xvec =  15 - DEF_8X8_WIN ;}
      if (yvec > 15 - DEF_8X8_WIN) { yvec =  15 - DEF_8X8_WIN ;}

      if (xvec < -15 + DEF_8X8_WIN) { xvec =  -15 + DEF_8X8_WIN ;}
      if (yvec < -15 + DEF_8X8_WIN) { yvec =  -15 + DEF_8X8_WIN ;}
    }

    adv_x_curr = x_curr  + xvec;
    adv_y_curr = y_curr  + yvec;

    sxy = DEF_8X8_WIN;

    adv_ilow = adv_x_curr - sxy;
    adv_ihigh = adv_x_curr + sxy;

    adv_jlow = adv_y_curr - sxy;
    adv_jhigh = adv_y_curr + sxy;

    adv_h_length = adv_ihigh - adv_ilow + 16;
    adv_v_length = adv_jhigh - adv_jlow + 16;

    adv_search_area = LoadArea(prev, adv_ilow, adv_jlow, 
               adv_h_length, adv_v_length, lx);

    for (block = 0; block < 4; block++) {
      ii = adv_search_area + (adv_x_curr-adv_ilow) + ((block&1)<<3) + 
        (adv_y_curr-adv_jlow + ((block&2)<<2) )*adv_h_length;
      aa = act_block + ((block&1)<<3) + ((block&2)<<2)*16;
      Min_FRAME[block+1] = SAD_Block(ii,aa,adv_h_length,Min_FRAME[block+1]);
      MV_FRAME[block+1].x = MV_FRAME[0].x;
      MV_FRAME[block+1].y = MV_FRAME[0].y;
    }

    /* �������� */
    for (l = 1; l <= sxy; l++) {
      i = adv_x_curr - l;
      j = adv_y_curr - l;
      for (k = 0; k < 8*l; k++) {
        if (i>=adv_ilow && i<=adv_ihigh && j>=adv_jlow && j<=adv_jhigh) {
          
          /* 8x8���������MVs */
          for (block = 0; block < 4; block++) {
            ii = adv_search_area + (i-adv_ilow) + ((block&1)<<3) + 
              (j-adv_jlow + ((block&2)<<2) )*adv_h_length;
            aa = act_block + ((block&1)<<3) + ((block&2)<<2)*16;
            sad = SAD_Block(ii, aa, adv_h_length, Min_FRAME[block+1]);//ʧ�溯��
            if (sad < Min_FRAME[block+1]) {
              MV_FRAME[block+1].x = i - x_curr;
              MV_FRAME[block+1].y = j - y_curr;
              Min_FRAME[block+1] = sad;
            }
          }
          
        }
        if      (k<2*l) i++;
        else if (k<4*l) j++;
        else if (k<6*l) i--;
        else            j--;
      }      
    }

  }

  i = x_curr/MB_SIZE+1;
  j = y_curr/MB_SIZE+1;

  if (!advanced) { // ����˶�ʸ��
    MV[0][j][i]->x = MV_FRAME[0].x;
    MV[0][j][i]->y = MV_FRAME[0].y;
    MV[0][j][i]->min_error = Min_FRAME[0];
  }
  else {
    for (k = 0; k < 5; k++) {
      MV[k][j][i]->x = MV_FRAME[k].x;
      MV[k][j][i]->y = MV_FRAME[k].y;
      MV[k][j][i]->min_error = Min_FRAME[k];
    }
  }

  free(act_block);
  free(search_area);
  if (advanced)
    free(adv_search_area);
  return;
}

/**********************************************************************
 *
 *	Name:        LoadArea
 *	Description:    fills array with a square of image-data
 *	
 *	Input:	       pointer to image and position, x and y size
 *	Returns:       pointer to area
 *	Side effects:  memory allocated to array
 *
 *	Date: 940203	Author: PGB
 *                      Mod: KOL
 *
 ***********************************************************************/


unsigned char *LoadArea(unsigned char *im, int x, int y, 
        int x_size, int y_size, int lx)
{
  unsigned char *res = (unsigned char *)malloc(sizeof(char)*x_size*y_size);
  unsigned char *in;
  unsigned char *out;
  int i = x_size;
  int j = y_size;

  in = im + (y*lx) + x;
  out = res;

  while (j--) {
    while (i--)
      *out++ = *in++;
    i = x_size;
    in += lx - x_size;
  };
  return res;
}

/**********************************************************************
 *
 *	Name:        SAD_Macroblock
 *	Description:    fast way to find the SAD of one vector
 *	
 *	Input:	        pointers to search_area and current block,
 *                      Min_F1/F2/FR
 *	Returns:        sad_f1/f2
 *	Side effects:
 *
 *	Date: 940203        Author: PGB
 *                      Mod:    KOL
 *
 ***********************************************************************/


int SAD_Macroblock(unsigned char *ii, unsigned char *act_block,
           int h_length, int Min_FRAME)
{
  int i;
  int sad = 0;
  unsigned char *kk;

  kk = act_block;
  i = 16;
  while (i--) {
    sad += (abs(*ii     - *kk     ) +abs(*(ii+1 ) - *(kk+1) )
            +abs(*(ii+2) - *(kk+2) ) +abs(*(ii+3 ) - *(kk+3) )
            +abs(*(ii+4) - *(kk+4) ) +abs(*(ii+5 ) - *(kk+5) )
            +abs(*(ii+6) - *(kk+6) ) +abs(*(ii+7 ) - *(kk+7) )
            +abs(*(ii+8) - *(kk+8) ) +abs(*(ii+9 ) - *(kk+9) )
            +abs(*(ii+10)- *(kk+10)) +abs(*(ii+11) - *(kk+11))
            +abs(*(ii+12)- *(kk+12)) +abs(*(ii+13) - *(kk+13))
            +abs(*(ii+14)- *(kk+14)) +abs(*(ii+15) - *(kk+15)) );

    ii += h_length;
    kk += 16;
   if (sad > Min_FRAME)
      return INT_MAX;
  } 
  return sad;
}

int SAD_Block(unsigned char *ii, unsigned char *act_block,
              int h_length, int min_sofar)
{
  int i;
  int sad = 0;
  unsigned char *kk;

  kk = act_block;
  i = 8;
  while (i--) {
    sad += (abs(*ii     - *kk     ) +abs(*(ii+1 ) - *(kk+1) )
            +abs(*(ii+2) - *(kk+2) ) +abs(*(ii+3 ) - *(kk+3) )
            +abs(*(ii+4) - *(kk+4) ) +abs(*(ii+5 ) - *(kk+5) )
            +abs(*(ii+6) - *(kk+6) ) +abs(*(ii+7 ) - *(kk+7) ));

    ii += h_length;
    kk += 16;
    if (sad > min_sofar)
      return INT_MAX;
  } 
  return sad;
}

int SAD_MB_Bidir(unsigned char *ii, unsigned char *aa, unsigned char *bb, 
         int width, int min_sofar)
{
  int i, sad = 0;
  unsigned char *ll, *kk;
  kk = aa;
  ll = bb;
  i = 16;
  while (i--) {
    sad += (abs(*ii     - ((*kk    + *ll    )>>1)) +
            abs(*(ii+1) - ((*(kk+1)+ *(ll+1))>>1)) +
            abs(*(ii+2) - ((*(kk+2)+ *(ll+2))>>1)) +
            abs(*(ii+3) - ((*(kk+3)+ *(ll+3))>>1)) +
            abs(*(ii+4) - ((*(kk+4)+ *(ll+4))>>1)) +
            abs(*(ii+5) - ((*(kk+5)+ *(ll+5))>>1)) +
            abs(*(ii+6) - ((*(kk+6)+ *(ll+6))>>1)) +
            abs(*(ii+7) - ((*(kk+7)+ *(ll+7))>>1)) +
            abs(*(ii+8) - ((*(kk+8)+ *(ll+8))>>1)) +
            abs(*(ii+9) - ((*(kk+9)+ *(ll+9))>>1)) +
            abs(*(ii+10) - ((*(kk+10)+ *(ll+10))>>1)) +
            abs(*(ii+11) - ((*(kk+11)+ *(ll+11))>>1)) +
            abs(*(ii+12) - ((*(kk+12)+ *(ll+12))>>1)) +
            abs(*(ii+13) - ((*(kk+13)+ *(ll+13))>>1)) +
            abs(*(ii+14) - ((*(kk+14)+ *(ll+14))>>1)) +
            abs(*(ii+15) - ((*(kk+15)+ *(ll+15))>>1)));

    ii += width;
    kk += width;
    ll += width;
    if (sad > min_sofar)
      return INT_MAX;
  } 
  return sad;
}

int SAD_MB_integer(int *ii, int *act_block, int h_length, int min_sofar)
{
  int i, sad = 0, *kk;

  kk = act_block;
  i = 16;
  while (i--) {
    sad += (abs(*ii     - *kk     ) +abs(*(ii+1 ) - *(kk+1) )
            +abs(*(ii+2) - *(kk+2) ) +abs(*(ii+3 ) - *(kk+3) )
            +abs(*(ii+4) - *(kk+4) ) +abs(*(ii+5 ) - *(kk+5) )
            +abs(*(ii+6) - *(kk+6) ) +abs(*(ii+7 ) - *(kk+7) )
            +abs(*(ii+8) - *(kk+8) ) +abs(*(ii+9 ) - *(kk+9) )
            +abs(*(ii+10)- *(kk+10)) +abs(*(ii+11) - *(kk+11))
            +abs(*(ii+12)- *(kk+12)) +abs(*(ii+13) - *(kk+13))
            +abs(*(ii+14)- *(kk+14)) +abs(*(ii+15) - *(kk+15)) );

    ii += h_length;
    kk += 16;
    if (sad > min_sofar)
      return INT_MAX;
  } 
  return sad;
}

/**********************************************************************
 *
 *	Name:        FindMB
 *	Description:	Picks out one MB from picture
 *	
 *	Input:        position of MB to pick out,
 *        pointer to frame data, empty 16x16 array	
 *	Returns:	
 *	Side effects:	fills array with MB data
 *
 *	Date: 930119	Author: Karl Olav Lillevold
 *
 ***********************************************************************/

void FindMB(int x, int y, unsigned char *image, int MB[16][16])

{
  int n;
  register int m;

  for (n = 0; n < MB_SIZE; n++)
    for (m = 0; m < MB_SIZE; m++)
      MB[n][m] = *(image + x+m + (y+n)*pels);
}


/*�˺���ȷ������-ʧ�����ŵ��˶�ʸ��*/

void RDMotionEstimate(unsigned char *curr, unsigned char *prev, int x_curr,
				 int y_curr, int xoff, int yoff, int seek_dist, int QP,Pict *pic,
				 MotionVector *MV[6][MBR+1][MBC+2],unsigned char *prev_ipol,
				 PictImage *prev_recon,int *SAD_0,int newgob)
{
  
  int Min_FRAME[5];
  MotionVector MV_FRAME[5];
  // Bits *bits;
  int curr_mb[16][16];
  unsigned char *act_block,*aa,*ii;
  unsigned char *search_area, *adv_search_area = NULL, *zero_area = NULL;
  int sxy,i,k,j,l,Mode;
  // int sad16,CBP;
  int CBPB=0;
  //int m=0,m1=0,m2=0,m3=0,m5=0,temptest=0,temptest2=0,minidistortion=INT_MAX;// 
  int ihigh,ilow,jhigh,jlow,h_length,v_length;
  // int adv_ihigh,adv_ilow,adv_jhigh,adv_jlow,adv_h_length,adv_v_length;
  int xmax,ymax,block,sad,lx;
  // int adv_x_curr, adv_y_curr,xvec,yvec;
  // MB_Structure *differ; 
  int *qcoeff_P;
  MotionVector ZERO = {0,0,0,0,0};
  
  long int distortion, mvbit=0, dctbit=0,judge;
  double minjudge;
  xmax = pels;
  ymax = lines;
  sxy = seek_dist;
  if (!long_vectors) {//��0ʸ��Ϊ���ĵ�������Χ
    sxy = mmin(15, sxy);  
  }
  else {
    /* Maximum extended search range centered around _predictor_ */
    sxy = mmin(15 - (2*DEF_8X8_WIN+1), sxy);
    xoff = mmin(16,mmax(-16,xoff));
    yoff = mmin(16,mmax(-16,yoff));
  }
  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  ilow = x_curr + xoff - sxy;
  ihigh = x_curr + xoff + sxy;

  jlow = y_curr + yoff - sxy;
  jhigh = y_curr + yoff + sxy;

  if (!mv_outside_frame) {
    if (ilow<0) ilow = 0;
    if (ihigh>xmax-16) ihigh = xmax-16;
    if (jlow<0) jlow = 0;
    if (jhigh>ymax-16) jhigh = ymax-16;
  }

  xmax = pels;
  ymax = lines;
  sxy = seek_dist;
  if (!long_vectors) {//��0ʸ��Ϊ���ĵ�������Χ
	  sxy = mmin(15, sxy);  
  }
  else {
	  /* Maximum extended search range centered around _predictor_ */
	  sxy = mmin(15 - (2*DEF_8X8_WIN+1), sxy);
	  xoff = mmin(16,mmax(-16,xoff));
	  yoff = mmin(16,mmax(-16,yoff));
  }
  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);
  
  ilow = x_curr + xoff - sxy;
  ihigh = x_curr + xoff + sxy;
  
  jlow = y_curr + yoff - sxy;
  jhigh = y_curr + yoff + sxy;
  
  if (!mv_outside_frame) {
	  if (ilow<0) ilow = 0;
	  if (ihigh>xmax-16) ihigh = xmax-16;
	  if (jlow<0) jlow = 0;
	  if (jhigh>ymax-16) jhigh = ymax-16;
  }
  
  h_length = ihigh - ilow + 16;
  v_length = jhigh - jlow + 16;
  act_block = LoadArea(curr, x_curr, y_curr, 16, 16, pels);  //װ�뵱ǰ������
  search_area = LoadArea(prev, ilow, jlow, h_length, v_length, lx);  //װ��������Χ�飺��ʼilow,jlow;��Сh_length, v_length
  
  for (k = 0; k < 5; k++) {
	  Min_FRAME[k] = INT_MAX;
	  minjudge= INT_MAX;
	  MV_FRAME[k].x = 0;
	  MV_FRAME[k].y = 0;
	  MV_FRAME[k].x_half = 0;
	  MV_FRAME[k].y_half = 0;
  }
  
  /* 0�˶�ʸ������ */
  if (x_curr-ilow         < 0        || y_curr-jlow         < 0        ||
      x_curr-ilow+MB_SIZE > h_length || y_curr-jlow+MB_SIZE > v_length) {
	  /* 0�˶�ʸ��λ�ò���search_area�� */
	  zero_area = LoadArea(prev, x_curr, y_curr, 16, 16, lx);  //װ��0�˶�ʸ����
	  *SAD_0 = SAD_Macroblock(zero_area, act_block, 16, Min_FRAME[0]) -
		  PREF_NULL_VEC;
	  free(zero_area);
  }
  else {
	  /* 0�˶�ʸ��λ����search_area�� */
	  ii = search_area + (x_curr-ilow) + (y_curr-jlow)*h_length;
	  *SAD_0 = SAD_Macroblock(ii, act_block, h_length, Min_FRAME[0]) - //�������������е�ǰ��λ�õĺ���뵱ǰ��֮��Ĳ�ֵ
		  PREF_NULL_VEC;
  }
  
  if (xoff == 0 && yoff == 0) {
	  Min_FRAME[0] = *SAD_0;
	  MV_FRAME[0].x = 0;
	  MV_FRAME[0].y = 0;
  }
  else {
	  ii = search_area + (x_curr+xoff-ilow) + (y_curr+yoff-jlow)*h_length;
	  Min_FRAME[0] = SAD_Macroblock(ii, act_block, h_length, Min_FRAME[0]);
	  MV_FRAME[0].x = xoff;
	  MV_FRAME[0].y = yoff;
  }
  minjudge=Min_FRAME[0];//((Min_FRAME[0])/QP-80)*4+Min_FRAME[0];
 /* �������� */
  for (l = 1; l <= sxy; l++) {
    i = x_curr + xoff - l;
    j = y_curr + yoff - l;
    for (k = 0; k < 8*l; k++) {
      if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {
          
		  //16x16���������MV 
		  ii = search_area + (i-ilow) + (j-jlow)*h_length; 
		  sad = SAD_Macroblock(ii, act_block, h_length, minjudge);
		  /*MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->x=i - x_curr;
		  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->y=j - y_curr;
		  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->x_half=0;
		  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->y_half=0;
		  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->min_error=sad;*/

		  
		 //*********************************R-D optimization*********************************************
          Bits  *tempbit;
          //mvbit=CountBitsVectors(MV, tempbit, x_curr/MB_SIZE, y_curr/MB_SIZE, Mode, newgob, pic,0);//����ʸ�����ݱ�����
          dctbit=(MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->min_error)/QP-80;// Ԥ��DCT��ϵ��*/
		  judge=sad;//+4*(dctbit+mvbit);
		  if (judge<minjudge) {
			  MV_FRAME[0].x = i - x_curr;
			  MV_FRAME[0].y = j - y_curr;
			  minjudge=judge;
			  //m=mvbit;// 
			  //m5=dctbit+mvbit;// 
		  }
       	  /* if(sad<minidistortion) {//
			  m1 = i - x_curr;//
			  m2 = j - y_curr;// 
			  m3= MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->min_error;// 
              minidistortion = sad;
			  temptest=mvbit;
			  temptest2=MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->min_error/QP-80+mvbit;
		  }*/
         mvbit=dctbit=0;
	 }
      if      (k<2*l) i++;
      else if (k<4*l) j++;
      else if (k<6*l) i--;
      else            j--;
    }      

  }
 
  if (!advanced) { // ����˶�ʸ��
	  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->x = MV_FRAME[0].x;
	  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->y = MV_FRAME[0].y;
	  MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1]->min_error = Min_FRAME[0];
  }

  /*CString inform;// 
  if(Min_FRAME[0]>minidistortion)// 
  {
   inform.Format("�����˶�ʸ��x=%d,y=%d,\n����ʧ��=%d,����ʧ�������=%d,����ʧ��MV������=%d,\n��Сʧ��=%d,��Сʧ�������=%d,��Сʧ��MV������=%d",MV_FRAME[0].x,MV_FRAME[0].y,Min_FRAME[0],m5,m,m3,temptest2,temptest);
  // AfxMessageBox(inform);
  }*/
  
  free(act_block);
  free(search_area);
  if (advanced)
  free(adv_search_area);
  return;
}

