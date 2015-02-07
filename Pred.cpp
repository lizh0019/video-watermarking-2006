#include"stdafx.h"
#include"Global.h"

/**********************************************************************
 *
 *	Name:        Predict_P
 *	Description:    Predicts P macroblock in advanced or normal
 *                      mode
 *	
 *	Input:        pointers to current and previous frames
 *        and previous interpolated image,
 *                      position and motion vector array
 *	Returns:	pointer to MB_Structure of data to be coded
 *	Side effects:	allocates memory to MB_Structure
 *
 *
 ***********************************************************************/

MB_Structure *Predict_P(PictImage *curr_image, PictImage *prev_image,
        unsigned char *prev_ipol, int x, int y, 
        MotionVector *MV[6][MBR+1][MBC+2], int PB)
{
  int m,n;
  int curr[16][16];
  int pred[16][16];
  MotionVector *fr0,*fr1,*fr2,*fr3,*fr4;
  int sum, dx, dy;
  int xmb, ymb;

  MB_Structure *pred_error = (MB_Structure *)malloc(sizeof(MB_Structure));
    
  xmb = x/MB_SIZE+1;
  ymb = y/MB_SIZE+1;

  fr0 = MV[0][ymb][xmb];
  fr1 = MV[1][ymb][xmb];
  fr2 = MV[2][ymb][xmb];
  fr3 = MV[3][ymb][xmb];
  fr4 = MV[4][ymb][xmb];

  /* װ�뵱ǰ��鵽curr */
  FindMB(x, y, curr_image->lum, curr);

  /* ���ڰ�����MVԤ�⵱ǰ�� */
  if (advanced) {
    FindPredOBMC(x, y, MV, prev_ipol, &pred[0][0], 0, PB);
    FindPredOBMC(x, y, MV, prev_ipol, &pred[0][8], 1, PB);
    FindPredOBMC(x, y, MV, prev_ipol, &pred[8][0], 2, PB);
    FindPredOBMC(x, y, MV, prev_ipol, &pred[8][8], 3, PB);
  }
  else 
    FindPred(x, y, fr0, prev_ipol, &pred[0][0], 16, 0); // �õ�Ԥ��ĺ��

  /* ����Ԥ�� */
  if (fr0->Mode == MODE_INTER || fr0->Mode == MODE_INTER_Q) {
    for (n = 0; n < MB_SIZE; n++)
      for (m = 0; m < MB_SIZE; m++) 
        pred_error->lum[n][m] = (int)(curr[n][m] - pred[n][m]);

    dx = 2*fr0->x + fr0->x_half;
    dy = 2*fr0->y + fr0->y_half;
    dx = ( dx % 4 == 0 ? dx >> 1 : (dx>>1)|1 );
    dy = ( dy % 4 == 0 ? dy >> 1 : (dy>>1)|1 );

    DoPredChrom_P(x, y, dx, dy, curr_image, prev_image, pred_error);// Ԥ��ɫ��飨�ǲ�ֵͼ��
  }

  else if (fr0->Mode == MODE_INTER4V) {
    for (n = 0; n < MB_SIZE; n++)
      for (m = 0; m < MB_SIZE; m++) 
        pred_error->lum[n][m] = (int)(curr[n][m] - pred[n][m]);

    sum = 2*fr1->x + fr1->x_half + 2*fr2->x + fr2->x_half +
      2*fr3->x + fr3->x_half + 2*fr4->x + fr4->x_half ; 
    dx = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

    sum = 2*fr1->y + fr1->y_half + 2*fr2->y + fr2->y_half +
      2*fr3->y + fr3->y_half + 2*fr4->y + fr4->y_half;
    dy = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

    DoPredChrom_P(x, y, dx, dy, curr_image, prev_image, pred_error);
  }

  else
    fprintf(stderr,"Illegal Mode in Predict_P (pred.c)\n");

  return pred_error;
}


/***********************************************************************
 *
 *	Name:        Predict_B
 *	Description:    Predicts the B macroblock in PB-frame prediction
 *	
 *	Input:	        pointers to current frame, previous recon. frame,
 *                      pos. in image, MV-data, reconstructed macroblock
 *                      from image ahead
 *	Returns:        pointer to differential MB data after prediction
 *	Side effects:   allocates memory to MB_structure
 *
 *
 ***********************************************************************/

MB_Structure *Predict_B(PictImage *curr_image, PictImage *prev_image,
        unsigned char *prev_ipol,int x, int y,
        MotionVector *MV[5][MBR+1][MBC+2],
        MB_Structure *recon_P, int TRD,int TRB)
{
  int i,j,k;
  int dx, dy, sad, sad_min=INT_MAX, curr[16][16], bdx=0, bdy=0;
  MB_Structure *p_err = (MB_Structure *)malloc(sizeof(MB_Structure));
  MB_Structure *pred = (MB_Structure *)malloc(sizeof(MB_Structure));
  MotionVector *f[5];
  int xvec, yvec, mvx, mvy;

  for (k = 0; k <= 4; k++)
    f[k] = MV[k][y/MB_SIZE+1][x/MB_SIZE+1];

  /* Find MB in current image */
  FindMB(x, y, curr_image->lum, curr);

  if (f[0]->Mode == MODE_INTER4V) {  /* Mode INTER4V */
    /* Find forward prediction */

    /* Luma */
    for (j = -DEF_PBDELTA_WIN; j <= DEF_PBDELTA_WIN; j++) {
      for (i = -DEF_PBDELTA_WIN; i <= DEF_PBDELTA_WIN; i++) {

        FindForwLumPredPB(prev_ipol, x, y, f[1], &pred->lum[0][0], 
          TRD, TRB, i, j, 8, 0);
        FindForwLumPredPB(prev_ipol, x, y, f[2], &pred->lum[0][8], 
          TRD, TRB, i, j, 8, 1);
        FindForwLumPredPB(prev_ipol, x, y, f[3], &pred->lum[8][0], 
          TRD, TRB, i, j, 8, 2);
        FindForwLumPredPB(prev_ipol, x, y, f[4], &pred->lum[8][8], 
          TRD, TRB, i, j, 8, 3);

        sad = SAD_MB_integer(&curr[0][0],&pred->lum[0][0], 16,INT_MAX);
        if (i == 0 && j == 0)
          sad -= PREF_PBDELTA_NULL_VEC;
        if (sad < sad_min) {
          sad_min = sad;
          bdx = i;
          bdy = j;
        }
      }
    }

    FindForwLumPredPB(prev_ipol,x,y,f[1],&pred->lum[0][0],TRD,TRB,bdx,bdy,8,0);
    FindForwLumPredPB(prev_ipol,x,y,f[2],&pred->lum[0][8],TRD,TRB,bdx,bdy,8,1);
    FindForwLumPredPB(prev_ipol,x,y,f[3],&pred->lum[8][0],TRD,TRB,bdx,bdy,8,2);
    FindForwLumPredPB(prev_ipol,x,y,f[4],&pred->lum[8][8],TRD,TRB,bdx,bdy,8,3);

    /* chroma vectors are sum of B luma vectors divided and rounded */
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      xvec += TRB*(2*f[k]->x + f[k]->x_half)/TRD + bdx;
      yvec += TRB*(2*f[k]->y + f[k]->y_half)/TRD + bdy;
    }

    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(x, y, dx, dy, prev_image, pred);

    /* Find bidirectional prediction */
    FindBiDirLumPredPB(&recon_P->lum[0][0], f[1], &pred->lum[0][0], 
               TRD, TRB, bdx, bdy, 0, 0);
    FindBiDirLumPredPB(&recon_P->lum[0][8], f[2], &pred->lum[0][8], 
               TRD, TRB, bdx, bdy, 1, 0);
    FindBiDirLumPredPB(&recon_P->lum[8][0], f[3], &pred->lum[8][0], 
               TRD, TRB, bdx, bdy, 0, 1);
    FindBiDirLumPredPB(&recon_P->lum[8][8], f[4], &pred->lum[8][8], 
               TRD, TRB, bdx, bdy, 1, 1);

    /* chroma vectors are sum of B luma vectors divided and rounded */
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      mvx = 2*f[k]->x + f[k]->x_half;
      mvy = 2*f[k]->y + f[k]->y_half;
      xvec += bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx;
      yvec += bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    }

    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, dx, dy, pred); 
  }

  else {  /* Mode INTER or INTER_Q */
    /* Find forward prediction */

    for (j = -DEF_PBDELTA_WIN; j <= DEF_PBDELTA_WIN; j++) {
      for (i = -DEF_PBDELTA_WIN; i <= DEF_PBDELTA_WIN; i++) {

        dx = i; dy = j;
        /* To keep things simple I turn off PB delta vectors at the edges */
        if (!mv_outside_frame) {
          if (x == 0) dx = 0;
          if (x == pels - MB_SIZE) dx = 0;
          if (y == 0) dy = 0;
          if (y == lines - MB_SIZE) dy = 0;
        }

        if (f[0]->Mode == MODE_INTRA || f[0]->Mode == MODE_INTRA_Q) {
          dx = dy = 0;
        }

        if (f[0]->x == 0 && f[0]->y == 0 && 
            f[0]->x_half == 0 && f[0]->y_half == 0) {
          dx = dy = 0;
        }

        FindForwLumPredPB(prev_ipol, x, y, f[0], &pred->lum[0][0], 
          TRD, TRB, dx, dy, 16, 0);

        sad = SAD_MB_integer(&curr[0][0],&pred->lum[0][0], 16, INT_MAX);
        if (i == 0 && j == 0)
          sad -= PREF_PBDELTA_NULL_VEC;
        if (sad < sad_min) {
          sad_min = sad;
          bdx = dx;
          bdy = dy;
        }
      }
    }
    FindForwLumPredPB(prev_ipol,x,y,f[0],&pred->lum[0][0],TRD,TRB,
              bdx,bdy,16,0);

    xvec = 4 * (TRB*(2*f[0]->x + f[0]->x_half) / TRD + bdx);
    yvec = 4 * (TRB*(2*f[0]->y + f[0]->y_half) / TRD + bdy);
    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(x, y, dx, dy, prev_image, pred);

    /* Find bidirectional prediction */
    FindBiDirLumPredPB(&recon_P->lum[0][0], f[0], &pred->lum[0][0], 
               TRD, TRB, bdx, bdy, 0, 0);
    FindBiDirLumPredPB(&recon_P->lum[0][8], f[0], &pred->lum[0][8], 
               TRD, TRB, bdx, bdy, 1, 0);
    FindBiDirLumPredPB(&recon_P->lum[8][0], f[0], &pred->lum[8][0], 
               TRD, TRB, bdx, bdy, 0, 1);
    FindBiDirLumPredPB(&recon_P->lum[8][8], f[0], &pred->lum[8][8], 
               TRD, TRB, bdx, bdy, 1, 1);

    /* chroma vectors */
    mvx = 2*f[0]->x + f[0]->x_half;
    xvec = bdx == 0 ? (TRB-TRD) * mvx / TRD : TRB * mvx / TRD + bdx - mvx;
    xvec *= 4;

    mvy = 2*f[0]->y + f[0]->y_half;
    yvec = bdy == 0 ? (TRB-TRD) * mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    yvec *= 4;
      
    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, dx, dy, pred); 
  }

  /* store PB-deltas */
  MV[5][y/MB_SIZE+1][x/MB_SIZE+1]->x = bdx; /* is in half pel format */
  MV[5][y/MB_SIZE+1][x/MB_SIZE+1]->y = bdy;
  MV[5][y/MB_SIZE+1][x/MB_SIZE+1]->x_half = 0;
  MV[5][y/MB_SIZE+1][x/MB_SIZE+1]->y_half = 0;


  /* Do the actual prediction */
  for (j = 0; j < MB_SIZE; j++) 
    for (i = 0; i < MB_SIZE; i++) 
      p_err->lum[j][i] = 
        *(curr_image->lum+x+i + (y+j)*pels) - pred->lum[j][i];

  y >>= 1;
  x >>= 1;
  for (j = 0; j < (MB_SIZE>>1); j++)			
    for (i = 0; i < (MB_SIZE>>1); i++) {		
      p_err->Cr[j][i] = *(curr_image->Cr+x+i + (y+j)*cpels) - pred->Cr[j][i];
      p_err->Cb[j][i] = *(curr_image->Cb+x+i + (y+j)*cpels) - pred->Cb[j][i];
    }

  free(pred);
  return p_err;
}

/***********************************************************************
 *
 *	Name:        MB_Recon_B
 *	Description:    Reconstructs the B macroblock in PB-frame
 *                      prediction
 *	
 *	Input:	        pointers previous recon. frame, pred. diff.,
 *                      pos. in image, MV-data, reconstructed macroblock
 *                      from image ahead
 *	Returns:        pointer to reconstructed MB data 
 *	Side effects:   allocates memory to MB_structure
 *
 *
 ***********************************************************************/

MB_Structure* MB_Recon_B(PictImage* prev_image, \
					MB_Structure *diff, \
         		unsigned char *prev_ipol, \
               int x, int y,    \
         		MotionVector *MV[5][MBR+1][MBC+2], \
         		MB_Structure *recon_P, \
               int TRD, int TRB)
{
  int i,j,k;
  int dx, dy, bdx, bdy, mvx, mvy, xvec, yvec;
  MB_Structure *recon_B = (MB_Structure *)malloc(sizeof(MB_Structure));
  MB_Structure *pred = (MB_Structure *)malloc(sizeof(MB_Structure));
  MotionVector *f[5];

  for (k = 0; k <= 4; k++)
    f[k] = MV[k][y/MB_SIZE+1][x/MB_SIZE+1];

  bdx = MV[5][y/MB_SIZE+1][x/MB_SIZE+1]->x;
  bdy = MV[5][y/MB_SIZE+1][x/MB_SIZE+1]->y;

  if (f[0]->Mode == MODE_INTER4V) {  /* Mode INTER4V */
    /* Find forward prediction */

    /* Luma */
    FindForwLumPredPB(prev_ipol,x,y,f[1],&pred->lum[0][0],TRD,TRB,bdx,bdy,8,0);
    FindForwLumPredPB(prev_ipol,x,y,f[2],&pred->lum[0][8],TRD,TRB,bdx,bdy,8,1);
    FindForwLumPredPB(prev_ipol,x,y,f[3],&pred->lum[8][0],TRD,TRB,bdx,bdy,8,2);
    FindForwLumPredPB(prev_ipol,x,y,f[4],&pred->lum[8][8],TRD,TRB,bdx,bdy,8,3);

    /* chroma vectors are sum of B luma vectors divided and rounded */
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      xvec += TRB*(2*f[k]->x + f[k]->x_half)/TRD + bdx;
      yvec += TRB*(2*f[k]->y + f[k]->y_half)/TRD + bdy;
    }

    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(x, y, dx, dy, prev_image, pred);

    /* Find bidirectional prediction */
    FindBiDirLumPredPB(&recon_P->lum[0][0], f[1], &pred->lum[0][0], 
               TRD, TRB, bdx, bdy, 0, 0);
    FindBiDirLumPredPB(&recon_P->lum[0][8], f[2], &pred->lum[0][8], 
               TRD, TRB, bdx, bdy, 1, 0);
    FindBiDirLumPredPB(&recon_P->lum[8][0], f[3], &pred->lum[8][0], 
               TRD, TRB, bdx, bdy, 0, 1);
    FindBiDirLumPredPB(&recon_P->lum[8][8], f[4], &pred->lum[8][8], 
               TRD, TRB, bdx, bdy, 1, 1);

    /* chroma vectors are sum of B luma vectors divided and rounded */
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      mvx = 2*f[k]->x + f[k]->x_half;
      mvy = 2*f[k]->y + f[k]->y_half;
      xvec += bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx;
      yvec += bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    }

    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, dx, dy, pred); 

  }
  else {  /* Mode INTER or INTER_Q */
    /* Find forward prediction */
    
    FindForwLumPredPB(prev_ipol,x,y,f[0],&pred->lum[0][0],TRD,TRB,
              bdx,bdy,16,0);

    xvec = 4 * (TRB*(2*f[0]->x + f[0]->x_half) / TRD + bdx);
    yvec = 4 * (TRB*(2*f[0]->y + f[0]->y_half) / TRD + bdy);
    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(x, y, dx, dy, prev_image, pred);

    /* Find bidirectional prediction */
    FindBiDirLumPredPB(&recon_P->lum[0][0], f[0], &pred->lum[0][0], 
               TRD, TRB, bdx, bdy, 0, 0);
    FindBiDirLumPredPB(&recon_P->lum[0][8], f[0], &pred->lum[0][8], 
               TRD, TRB, bdx, bdy, 1, 0);
    FindBiDirLumPredPB(&recon_P->lum[8][0], f[0], &pred->lum[8][0], 
               TRD, TRB, bdx, bdy, 0, 1);
    FindBiDirLumPredPB(&recon_P->lum[8][8], f[0], &pred->lum[8][8], 
               TRD, TRB, bdx, bdy, 1, 1);

    /* chroma vectors */
    mvx = 2*f[0]->x + f[0]->x_half;
    xvec = bdx == 0 ? (TRB-TRD) * mvx / TRD : TRB * mvx / TRD + bdx - mvx;
    xvec *= 4;

    mvy = 2*f[0]->y + f[0]->y_half;
    yvec = bdy == 0 ? (TRB-TRD) * mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    yvec *= 4;
      
    /* round values according to TABLE 16/H.263 */
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, dx, dy, pred); 

  }

  /* Reconstruction */
  for (j = 0; j < MB_SIZE; j++) 
    for (i = 0; i < MB_SIZE; i++) 
      recon_B->lum[j][i] = pred->lum[j][i] + diff->lum[j][i];
        
  for (j = 0; j < (MB_SIZE>>1); j++)	
    for (i = 0; i < (MB_SIZE>>1); i++) {	
      recon_B->Cr[j][i] = pred->Cr[j][i] + diff->Cr[j][i];
      recon_B->Cb[j][i] = pred->Cb[j][i] + diff->Cb[j][i];
    }
  
  free(pred);
  return recon_B;
}

/**********************************************************************
 *
 *	Name:	       FindForwLumPredPB
 *	Description:   Finds the forward luma  prediction in PB-frame 
 *                     pred.
 *	
 *	Input:	       pointer to prev. recon. frame, current positon,
 *                     MV structure and pred. structure to fill
 *
 *
 ***********************************************************************/

void FindForwLumPredPB(unsigned char *prev_ipol, int x_curr, int y_curr, 
               MotionVector *fr, int *pred, int TRD, int TRB, 
               int bdx, int bdy, int bs, int comp)
{
  int i,j;
  int xvec,yvec,lx;

  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  /* Luma */
  xvec = (TRB)*(2*fr->x + fr->x_half)/TRD + bdx;
  yvec = (TRB)*(2*fr->y + fr->y_half)/TRD + bdy;

  x_curr += ((comp&1)<<3);
  y_curr += ((comp&2)<<2);

  for (j = 0; j < bs; j++) {
    for (i = 0; i < bs; i++) {
      *(pred+i+j*16) = *(prev_ipol + (i+x_curr)*2 + xvec +
         ((j+y_curr)*2 + yvec)*lx*2);
    }
  }

  return;
}


/**********************************************************************
 *
 *	Name:	       FindBiDirLumPredPB
 *	Description:   Finds the bi-dir. luma prediction in PB-frame 
 *                     prediction
 *	
 *	Input:	       pointer to future recon. data, current positon,
 *                     MV structure and pred. structure to fill
 *
 *
 ***********************************************************************/

void FindBiDirLumPredPB(int *recon_P, MotionVector *fr, int *pred, int TRD, 
        int TRB, int bdx, int bdy, int nh, int nv)
{
  int xstart,xstop,ystart,ystop;
  int xvec,yvec, mvx, mvy;

  mvx = 2*fr->x + fr->x_half;
  mvy = 2*fr->y + fr->y_half;

  xvec = (bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx);
  yvec = (bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy);

  /* Luma */

  FindBiDirLimits(xvec,&xstart,&xstop,nh);
  FindBiDirLimits(yvec,&ystart,&ystop,nv);

  BiDirPredBlock(xstart,xstop,ystart,ystop,xvec,yvec, recon_P,pred,16);

  return;
}

/**********************************************************************
 *
 *	Name:	       FindBiDirChrPredPB
 *	Description:   Finds the bi-dir. chroma prediction in PB-frame 
 *                     prediction
 *	
 *	Input:	       pointer to future recon. data, current positon,
 *                     MV structure and pred. structure to fill
 *
 *
 ***********************************************************************/

void FindBiDirChrPredPB(MB_Structure *recon_P, int dx, int dy, 
        MB_Structure *pred)
{
  int xstart,xstop,ystart,ystop;

  FindBiDirChromaLimits(dx,&xstart,&xstop);
  FindBiDirChromaLimits(dy,&ystart,&ystop);

  BiDirPredBlock(xstart,xstop,ystart,ystop,dx,dy,
         &recon_P->Cb[0][0], &pred->Cb[0][0],8);
  BiDirPredBlock(xstart,xstop,ystart,ystop,dx,dy,
         &recon_P->Cr[0][0], &pred->Cr[0][0],8);

  return;
}

void FindBiDirLimits(int vec, int *start, int *stop, int nhv)
{

  /* limits taken from C loop in section G5 in H.263 */
  *start = mmax(0,(-vec+1)/2 - nhv*8);
  *stop = mmin(7,15-(vec+1)/2 - nhv*8);

  return;

}
  
void FindBiDirChromaLimits(int vec, int *start, int *stop)
{

  /* limits taken from C loop in section G5 in H.263 */
  *start = mmax(0,(-vec+1)/2);
  *stop = mmin(7,7-(vec+1)/2);

  return;
}


void BiDirPredBlock(int xstart, int xstop, int ystart, int ystop,
            int xvec, int yvec, int *recon, int *pred, int bl)
{
  int i,j,pel;
  int xint, yint;
  int xh, yh;

  xint = xvec>>1;
  xh = xvec - 2*xint;
  yint = yvec>>1;
  yh = yvec - 2*yint;

  if (!xh && !yh) {
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
        pel = *(recon +(j+yint)*bl + i+xint);
        *(pred + j*bl + i) = (mmin(255,mmax(0,pel)) + *(pred + j*bl + i))>>1;
      }
    }
  }
  else if (!xh && yh) {
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
        pel = (*(recon +(j+yint)*bl + i+xint)       + 
               *(recon +(j+yint+yh)*bl + i+xint) + 1)>>1;
        *(pred + j*bl + i) = (pel + *(pred + j*bl + i))>>1;
      }
    }
  }
  else if (xh && !yh) {
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
        pel = (*(recon +(j+yint)*bl + i+xint)       + 
               *(recon +(j+yint)*bl + i+xint+xh) + 1)>>1;
        *(pred + j*bl + i) = (pel + *(pred + j*bl + i))>>1;
      }
    }
  }
  else { /* xh && yh */
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
        pel = (*(recon +(j+yint)*bl + i+xint)       + 
               *(recon +(j+yint+yh)*bl + i+xint) + 
               *(recon +(j+yint)*bl + i+xint+xh) + 
               *(recon +(j+yint+yh)*bl + i+xint+xh)+2)>>2;
        *(pred + j*bl + i) = (pel + *(pred + j*bl + i))>>1;
      }
    }
  }
  return;
}

/**********************************************************************
 *
 *	Name:        DoPredChrom_P
 *	Description:	Does the chrominance prediction for P-frames
 *	
 *	Input:        motion vectors for each field,
 *        current position in image,
 *        pointers to current and previous image,
 *        pointer to pred_error array,
 *        (int) field: 1 if field coding
 *        
 *	Side effects:	fills chrom-array in pred_error structure
 *
 *
 ***********************************************************************/

void DoPredChrom_P(int x_curr, int y_curr, int dx, int dy,
           PictImage *curr, PictImage *prev, 
           MB_Structure *pred_error)
{
  int m,n;

  int x, y, ofx, ofy, pel, lx;
  int xint, yint;
  int xh, yh;

  lx = (mv_outside_frame ? pels/2 + (long_vectors?32:16) : pels/2);

  x = x_curr>>1;
  y = y_curr>>1;

  xint = dx>>1;
  xh = dx & 1;
  yint = dy>>1;
  yh = dy & 1;

  if (!xh && !yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=*(prev->Cr+ofx    + (ofy)*lx);
        pred_error->Cr[n][m] = (int)(*(curr->Cr + x+m + (y+n)*cpels) - pel);

        pel=*(prev->Cb+ofx    + (ofy)*lx);
        pred_error->Cb[n][m] = (int)(*(curr->Cb + x+m + (y+n)*cpels) - pel);
      }
    }
  }
  else if (!xh && yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx    + (ofy+yh)*lx) + 1)>>1;

        pred_error->Cr[n][m] = 
          (int)(*(curr->Cr + x+m + (y+n)*cpels) - pel);

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx    + (ofy+yh)*lx) + 1)>>1;

        pred_error->Cb[n][m] = 
          (int)(*(curr->Cb + x+m + (y+n)*cpels) - pel);
      
      }
    }
  }
  else if (xh && !yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx+xh + (ofy   )*lx) + 1)>>1;

        pred_error->Cr[n][m] = 
          (int)(*(curr->Cr + x+m + (y+n)*cpels) - pel);

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx+xh + (ofy   )*lx) + 1)>>1;

        pred_error->Cb[n][m] = 
          (int)(*(curr->Cb + x+m + (y+n)*cpels) - pel);
      
      }
    }
  }
  else { /* xh && yh */
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {
        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx+xh + (ofy   )*lx)+
             *(prev->Cr+ofx    + (ofy+yh)*lx)+
             *(prev->Cr+ofx+xh + (ofy+yh)*lx)+
             2)>>2;

        pred_error->Cr[n][m] = 
          (int)(*(curr->Cr + x+m + (y+n)*cpels) - pel);

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx+xh + (ofy   )*lx)+
             *(prev->Cb+ofx    + (ofy+yh)*lx)+
             *(prev->Cb+ofx+xh + (ofy+yh)*lx)+
             2)>>2;

        pred_error->Cb[n][m] = 
          (int)(*(curr->Cb + x+m + (y+n)*cpels) - pel);
      
      }
    }
  }
  return;
}

/**********************************************************************
 *
 *	Name:        FindHalfPel
 *	Description:	Find the optimal half pel prediction
 *	
 *	Input:        position, vector, array with current data
 *        pointer to previous interpolated luminance,
 *
 *	Returns:
 *
 *            950208    Mod: Karl.Lillevold@nta.no
 *
 ***********************************************************************/


void FindHalfPel(int x, int y, MotionVector *fr, unsigned char *prev, // ��������������ֵ���ͼ��
         int *curr, int bs, int comp)
{
  int i, m, n;
  int half_pel;
  int start_x, start_y, stop_x, stop_y, new_x, new_y, lx;
  int min_pos;
  int AE, AE_min;
  CPoint search[9];

  start_x = -1;
  stop_x = 1;
  start_y = -1;
  stop_y = 1;

  new_x = x + fr->x;
  new_y = y + fr->y;

  new_x += ((comp&1)<<3);
  new_y += ((comp&2)<<2);

  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  if (!mv_outside_frame) {
    if ((new_x) <= 0) 
      start_x = 0;
    if ((new_y) <= 0) 
      start_y = 0;
    if ((new_x) >= (pels-bs)) 
      stop_x = 0;
    if ((new_y) >= (lines-bs)) 
      stop_y = 0;
  }

  search[0].x = 0;             search[0].y = 0;
  search[1].x = start_x;       search[1].y = start_y; /*   1 2 3   */
  search[2].x = 0;             search[2].y = start_y; /*   4 0 5   */
  search[3].x = stop_x;        search[3].y = start_y; /*   6 7 8   */
  search[4].x = start_x;       search[4].y = 0;
  search[5].x = stop_x;        search[5].y = 0;
  search[6].x = start_x;       search[6].y = stop_y;
  search[7].x = 0;             search[7].y = stop_y;
  search[8].x = stop_x;        search[8].y = stop_y;

  AE_min = INT_MAX;
  min_pos = 0;
  for (i = 0; i < 9; i++) {
    AE = 0;
    for (n = 0; n < bs; n++) {
      for (m = 0; m < bs; m++) {
        /* ���������� */
        half_pel = *(prev + 2*new_x + 2*m + search[i].x +
             (2*new_y + 2*n + search[i].y)*lx*2);
        AE += abs(half_pel - *(curr + m + n*16));
      }
    }

    if (AE < AE_min) {
      AE_min = AE;
      min_pos = i;
    }
  }

  /* �洢����ֵ */
  fr->min_error = AE_min;
  fr->x_half = search[min_pos].x;
  fr->y_half = search[min_pos].y;

  return;
}

/**********************************************************************
 *
 *	Name:        FindPred
 *	Description:	Find the prediction block
 *	
 *	Input:        position, vector, array for prediction
 *        pointer to previous interpolated luminance,
 *
 *	Side effects:	fills array with prediction
 *
 *            950208    Mod: Karl.Lillevold@nta.no
 *
 ***********************************************************************/


void FindPred(int x, int y, MotionVector *fr, unsigned char *prev, 
              int *pred, int bs, int comp)
{
  int m, n;
  int new_x, new_y;
  int lx;

  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  new_x = x + fr->x;
  new_y = y + fr->y;

  new_x += ((comp&1)<<3);
  new_y += ((comp&2)<<2);


  /* ���Ԥ������ */
  for (n = 0; n < bs; n++) {
    for (m = 0; m < bs; m++) {
      *(pred + m + n*16) = *(prev + (new_x + m)*2 + fr->x_half +
             ((new_y + n)*2 + fr->y_half)*lx*2);
    }	
  }
  return;
}

/**********************************************************************
 *
 *	Name:        FindPredOBMC
 *	Description:	Find the OBMC prediction block
 *	
 *	Input:        position, vector, array for prediction
 *        pointer to previous interpolated luminance,
 *
 *	Returns:
 *	Side effects:	fills array with prediction
 *
 *
 ***********************************************************************/


void FindPredOBMC(int x, int y, MotionVector *MV[6][MBR+1][MBC+2], 
          unsigned char *prev, int *pred, int comp, int PB)
{
  int m, n;
  int pc,pt,pb,pr,pl;
  int nxc,nxt,nxb,nxr,nxl;
  int nyc,nyt,nyb,nyr,nyl;
  int xit,xib,xir,xil;
  int yit,yib,yir,yil;
  int vect,vecb,vecr,vecl;
  int c8,t8,l8,r8;
  int ti8,li8,ri8;
  int xmb, ymb, lx;
  MotionVector *fc,*ft,*fb,*fr,*fl;

  int Mc[8][8] = {
    {4,5,5,5,5,5,5,4},
    {5,5,5,5,5,5,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,5,5,5,5,5,5},
    {4,5,5,5,5,5,5,4},
  };
  int Mt[8][8] = {
    {2,2,2,2,2,2,2,2},
    {1,1,2,2,2,2,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  };
  int Mb[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,2,2,2,2,1,1},
    {2,2,2,2,2,2,2,2},
  };
  int Mr[8][8] = {
    {0,0,0,0,1,1,1,2},
    {0,0,0,0,1,1,2,2},
    {0,0,0,0,1,1,2,2},
    {0,0,0,0,1,1,2,2},
    {0,0,0,0,1,1,2,2},
    {0,0,0,0,1,1,2,2},
    {0,0,0,0,1,1,2,2},
    {0,0,0,0,1,1,1,2},
  };
  int Ml[8][8] = {
    {2,1,1,1,0,0,0,0},
    {2,2,1,1,0,0,0,0},
    {2,2,1,1,0,0,0,0},
    {2,2,1,1,0,0,0,0},
    {2,2,1,1,0,0,0,0},
    {2,2,1,1,0,0,0,0},
    {2,2,1,1,0,0,0,0},
    {2,1,1,1,0,0,0,0},
  };

  xmb = x/MB_SIZE+1;
  ymb = y/MB_SIZE+1;

  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  c8  = (MV[0][ymb][xmb]->Mode == MODE_INTER4V ? 1 : 0);

  t8  = (MV[0][ymb-1][xmb]->Mode == MODE_INTER4V ? 1 : 0);
  ti8 = (MV[0][ymb-1][xmb]->Mode == MODE_INTRA ? 1 : 0);
  ti8 = (MV[0][ymb-1][xmb]->Mode == MODE_INTRA_Q ? 1 : ti8);

  l8  = (MV[0][ymb][xmb-1]->Mode == MODE_INTER4V ? 1 : 0);
  li8 = (MV[0][ymb][xmb-1]->Mode == MODE_INTRA ? 1 : 0);
  li8 = (MV[0][ymb][xmb-1]->Mode == MODE_INTRA_Q ? 1 : li8);
  
  r8  = (MV[0][ymb][xmb+1]->Mode == MODE_INTER4V ? 1 : 0);
  ri8 = (MV[0][ymb][xmb+1]->Mode == MODE_INTRA ? 1 : 0);
  ri8 = (MV[0][ymb][xmb+1]->Mode == MODE_INTRA_Q ? 1 : ri8);

  if (PB) {
    ti8 = li8 = ri8 = 0;
  }

  switch (comp+1) {

  case 1:
    vect = (ti8 ? (c8 ? 1 : 0) : (t8 ? 3 : 0)); 
    yit  = (ti8 ? ymb : ymb - 1); 
    xit = xmb;

    vecb = (c8 ? 3 : 0) ; yib = ymb; xib = xmb;

    vecl = (li8 ? (c8 ? 1 : 0) : (l8 ? 2 : 0)); 
    yil = ymb; 
    xil = (li8 ? xmb : xmb-1);

    vecr = (c8 ? 2 : 0) ; yir = ymb; xir = xmb;

    /* edge handling */
    if (ymb == 1) {
      yit = ymb;
      vect = (c8 ? 1 : 0);
    }
    if (xmb == 1) {
      xil = xmb;
      vecl = (c8 ? 1 : 0);
    }
    break;
  
  case 2:
    vect = (ti8 ? (c8 ? 2 : 0) : (t8 ? 4 : 0)); 
    yit = (ti8 ? ymb : ymb-1); 
    xit = xmb;

    vecb = (c8 ? 4 : 0) ; yib = ymb; xib = xmb;
    vecl = (c8 ? 1 : 0) ; yil = ymb; xil = xmb;

    vecr = (ri8 ? (c8 ? 2 : 0) : (r8 ? 1 : 0)); 
    yir = ymb; 
    xir = (ri8 ? xmb : xmb+1);

    /* edge handling */
    if (ymb == 1) {
      yit = ymb;
      vect = (c8 ? 2 : 0);
    }
    if (xmb == pels/16) {
      xir = xmb;
      vecr = (c8 ? 2 : 0);
    }
    break;

  case 3:
    vect = (c8 ? 1 : 0) ; yit = ymb  ; xit = xmb;
    vecb = (c8 ? 3 : 0) ; yib = ymb  ; xib = xmb;
      
    vecl = (li8 ? (c8 ? 3 : 0) : (l8 ? 4 : 0)); 
    yil = ymb;  
    xil = (li8 ? xmb : xmb-1);
    
    vecr = (c8 ? 4 : 0) ; yir = ymb  ; xir = xmb;

    /* edge handling */
    if (xmb == 1) {
      xil = xmb;
      vecl = (c8 ? 3 : 0);
    }
    break;

  case 4:
    vect = (c8 ? 2 : 0) ; yit = ymb  ; xit = xmb;
    vecb = (c8 ? 4 : 0) ; yib = ymb  ; xib = xmb;
    vecl = (c8 ? 3 : 0) ; yil = ymb  ; xil = xmb;

    vecr = (ri8 ? (c8 ? 4 : 0) : (r8 ? 3 : 0)); 
    yir = ymb; 
    xir = (ri8 ? xmb : xmb+1);

    /* edge handling */
    if (xmb == pels/16) {
      xir = xmb;
      vecr = (c8 ? 4 : 0);
    }
    break;

  default:
    fprintf(stderr,"Illegal block number in FindPredOBMC (pred.c)\n");
    exit(-1);
    break;
  }

  fc = MV[c8 ? comp + 1: 0][ymb][xmb];

  ft = MV[vect][yit][xit];
  fb = MV[vecb][yib][xib];
  fr = MV[vecr][yir][xir];
  fl = MV[vecl][yil][xil];

  nxc = 2*x + ((comp&1)<<4); nyc = 2*y + ((comp&2)<<3);
  nxt = nxb = nxr = nxl = nxc;
  nyt = nyb = nyr = nyl = nyc;

  nxc += 2*fc->x + fc->x_half; nyc += 2*fc->y + fc->y_half;
  nxt += 2*ft->x + ft->x_half; nyt += 2*ft->y + ft->y_half;
  nxb += 2*fb->x + fb->x_half; nyb += 2*fb->y + fb->y_half;
  nxr += 2*fr->x + fr->x_half; nyr += 2*fr->y + fr->y_half;
  nxl += 2*fl->x + fl->x_half; nyl += 2*fl->y + fl->y_half;

  /* Fill pred. data */
  for (n = 0; n < 8; n++) {
    for (m = 0; m < 8; m++) {

      /* Find interpolated pixel-value */
      pc = *(prev + nxc + 2*m + (nyc + 2*n)*lx*2) * Mc[n][m];
      pt = *(prev + nxt + 2*m + (nyt + 2*n)*lx*2) * Mt[n][m];
      pb = *(prev + nxb + 2*m + (nyb + 2*n)*lx*2) * Mb[n][m];
      pr = *(prev + nxr + 2*m + (nyr + 2*n)*lx*2) * Mr[n][m];
      pl = *(prev + nxl + 2*m + (nyl + 2*n)*lx*2) * Ml[n][m];

      /*$pc = *(prev + nxc + 2*m + (nyc + 2*n)*lx*2) * 8;
      pt = *(prev + nxt + 2*m + (nyt + 2*n)*lx*2) * 0;;
      pb = *(prev + nxb + 2*m + (nyb + 2*n)*lx*2) * 0;
      pr = *(prev + nxr + 2*m + (nyr + 2*n)*lx*2) * 0;
      pl = *(prev + nxl + 2*m + (nyl + 2*n)*lx*2) * 0;$*/

      *(pred + m + n*16) = (pc+pt+pb+pr+pl+4)>>3;
    }	
  }
  return;
}


/**********************************************************************
 *
 *	Name:        ReconMacroblock_P
 *	Description:	Reconstructs MB after quantization for P_images
 *	
 *	Input:        pointers to current and previous image,
 *        current slice and mb, and which mode
 *        of prediction has been used
 *	Returns:
 *	Side effects:
 *
 *
 ***********************************************************************/

MB_Structure *MB_Recon_P(PictImage *prev_image, unsigned char *prev_ipol,
         MB_Structure *diff, int x_curr, int y_curr, 
         MotionVector *MV[6][MBR+1][MBC+2], int PB)
{
  MB_Structure *recon_data = (MB_Structure *)malloc(sizeof(MB_Structure));
  MotionVector *fr0,*fr1,*fr2,*fr3,*fr4;
  int pred[16][16];
  int dx, dy, sum;
  int i,j;

  fr0 = MV[0][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1];

  if (advanced) {
    if (fr0->Mode == MODE_INTER || fr0->Mode == MODE_INTER_Q) {
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[0][0], 0, PB);
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[0][8], 1, PB);
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[8][0], 2, PB);
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[8][8], 3, PB);
      for (j = 0; j < MB_SIZE; j++) 
        for (i = 0; i < MB_SIZE; i++) 
          diff->lum[j][i] += pred[j][i];

      dx = 2*fr0->x + fr0->x_half;
      dy = 2*fr0->y + fr0->y_half;
      dx = ( dx % 4 == 0 ? dx >> 1 : (dx>>1)|1 );
      dy = ( dy % 4 == 0 ? dy >> 1 : (dy>>1)|1 );
      ReconChromBlock_P(x_curr, y_curr, dx, dy, prev_image, diff);
    }
    else if (fr0->Mode == MODE_INTER4V) { /* Inter 8x8 */
      
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[0][0], 0, PB);
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[0][8], 1, PB);
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[8][0], 2, PB);
      FindPredOBMC(x_curr, y_curr, MV, prev_ipol, &pred[8][8], 3, PB);
      for (j = 0; j < MB_SIZE; j++) 
        for (i = 0; i < MB_SIZE; i++) 
          diff->lum[j][i] += pred[j][i];

      fr1 = MV[1][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1];
      fr2 = MV[2][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1];
      fr3 = MV[3][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1];
      fr4 = MV[4][y_curr/MB_SIZE+1][x_curr/MB_SIZE+1];

      sum = 2*fr1->x + fr1->x_half + 2*fr2->x + fr2->x_half +
        2*fr3->x + fr3->x_half + 2*fr4->x + fr4->x_half ; 
      dx = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

      sum = 2*fr1->y + fr1->y_half + 2*fr2->y + fr2->y_half +
        2*fr3->y + fr3->y_half + 2*fr4->y + fr4->y_half;
      dy = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

      ReconChromBlock_P(x_curr, y_curr, dx, dy, prev_image, diff);
    }
  }
  else {
    if (fr0->Mode == MODE_INTER || fr0->Mode == MODE_INTER_Q) {
      /* Inter 16x16 */
      ReconLumBlock_P(x_curr,y_curr,fr0,prev_ipol,&diff->lum[0][0],16,0);

      dx = 2*fr0->x + fr0->x_half;
      dy = 2*fr0->y + fr0->y_half;
      dx = ( dx % 4 == 0 ? dx >> 1 : (dx>>1)|1 );
      dy = ( dy % 4 == 0 ? dy >> 1 : (dy>>1)|1 );
      ReconChromBlock_P(x_curr, y_curr, dx, dy, prev_image, diff);
    }
  }

  memcpy(recon_data, diff, sizeof(MB_Structure));

  return recon_data;
}

/**********************************************************************
 *
 *	Name:        ReconLumBlock_P
 *	Description:	Reconstructs one block of luminance data
 *	
 *	Input:        position, vector-data, previous image, data-block
 *	Returns:
 *	Side effects:	reconstructs data-block
 *
 *
 ***********************************************************************/

void ReconLumBlock_P(int x, int y, MotionVector *fr, 
             unsigned char *prev, int *data, int bs, int comp)
{
  int m, n;
  int x1, y1, lx;

  lx = (mv_outside_frame ? pels + (long_vectors?64:32) : pels);

  x1 = 2*(x + fr->x) + fr->x_half;
  y1 = 2*(y + fr->y) + fr->y_half;
  
  x1 += ((comp&1)<<4);
  y1 += ((comp&2)<<3);

  for (n = 0; n < bs; n++) {
    for (m = 0; m < bs; m++) {
      *(data+m+n*16) += (int)(*(prev+x1+2*m + (y1+2*n)*2*lx));
    }
  }

  return;
}

/**********************************************************************
 *
 *	Name:        ReconChromBlock_P
 *	Description:        Reconstructs chrominance of one block in P frame
 *	
 *	Input:        position, vector-data, previous image, data-block
 *	Returns:        
 *	Side effects:	reconstructs data-block
 *
 *
 ***********************************************************************/

void ReconChromBlock_P(int x_curr, int y_curr, int dx, int dy,
               PictImage *prev, MB_Structure *data)

{
  int m,n;

  int x, y, ofx, ofy, pel,lx;
  int xint, yint;
  int xh, yh;

  lx = (mv_outside_frame ? pels/2 + (long_vectors?32:16) : pels/2);

  x = x_curr>>1;
  y = y_curr>>1;

  xint = dx>>1;
  xh = dx & 1;
  yint = dy>>1;
  yh = dy & 1;
    
  if (!xh && !yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=*(prev->Cr+ofx    + (ofy   )*lx);
        data->Cr[n][m] += pel;

        pel=*(prev->Cb+ofx    + (ofy   )*lx);
        data->Cb[n][m] += pel;
      }
    }
  }
  else if (!xh && yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx    + (ofy+yh)*lx) + 1)>>1;

        data->Cr[n][m] += pel;

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx    + (ofy+yh)*lx) + 1)>>1;

        data->Cb[n][m] += pel;
      
      }
    }
  }
  else if (xh && !yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx+xh + (ofy   )*lx) + 1)>>1;

        data->Cr[n][m] += pel;

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx+xh + (ofy   )*lx) + 1)>>1;

        data->Cb[n][m] += pel;
      
      }
    }
  }
  else { /* xh && yh */
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {
        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx+xh + (ofy   )*lx)+
             *(prev->Cr+ofx    + (ofy+yh)*lx)+
             *(prev->Cr+ofx+xh + (ofy+yh)*lx)+
             2)>>2;

        data->Cr[n][m] += pel;

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx+xh + (ofy   )*lx)+
             *(prev->Cb+ofx    + (ofy+yh)*lx)+
             *(prev->Cb+ofx+xh + (ofy+yh)*lx)+
             2)>>2;

        data->Cb[n][m] += pel;
      
      }
    }
  }
  return;
}

/**********************************************************************
 *
 *	Name:        FindChromBlock_P
 *	Description:        Finds chrominance of one block in P frame
 *	
 *	Input:        position, vector-data, previous image, data-block
 *
 *
 ***********************************************************************/

void FindChromBlock_P(int x_curr, int y_curr, int dx, int dy,
              PictImage *prev, MB_Structure *data)

{
  int m,n;

  int x, y, ofx, ofy, pel,lx;
  int xint, yint;
  int xh, yh;

  lx = (mv_outside_frame ? pels/2 + (long_vectors?32:16) : pels/2);

  x = x_curr>>1;
  y = y_curr>>1;

  xint = dx>>1;
  xh = dx & 1;
  yint = dy>>1;
  yh = dy & 1;
    
  if (!xh && !yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=*(prev->Cr+ofx    + (ofy   )*lx);
        data->Cr[n][m] = pel;

        pel=*(prev->Cb+ofx    + (ofy   )*lx);
        data->Cb[n][m] = pel;
      }
    }
  }
  else if (!xh && yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx    + (ofy+yh)*lx) + 1)>>1;

        data->Cr[n][m] = pel;

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx    + (ofy+yh)*lx) + 1)>>1;

        data->Cb[n][m] = pel;
      
      }
    }
  }
  else if (xh && !yh) {
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {

        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx+xh + (ofy   )*lx) + 1)>>1;

        data->Cr[n][m] = pel;

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx+xh + (ofy   )*lx) + 1)>>1;

        data->Cb[n][m] = pel;
      
      }
    }
  }
  else { /* xh && yh */
    for (n = 0; n < 8; n++) {
      for (m = 0; m < 8; m++) {
        ofx = x + xint + m;
        ofy = y + yint + n;
        pel=(*(prev->Cr+ofx    + (ofy   )*lx)+
             *(prev->Cr+ofx+xh + (ofy   )*lx)+
             *(prev->Cr+ofx    + (ofy+yh)*lx)+
             *(prev->Cr+ofx+xh + (ofy+yh)*lx)+
             2)>>2;

        data->Cr[n][m] = pel;

        pel=(*(prev->Cb+ofx    + (ofy   )*lx)+
             *(prev->Cb+ofx+xh + (ofy   )*lx)+
             *(prev->Cb+ofx    + (ofy+yh)*lx)+
             *(prev->Cb+ofx+xh + (ofy+yh)*lx)+
             2)>>2;

        data->Cb[n][m] = pel;
      
      }
    }
  }
  return;
}



/**********************************************************************
 *
 *	Name:        ChooseMode
 *	Description:    chooses coding mode
 *	
 *	Input:	        pointer to original fram, min_error from 
 *                      integer pel search, DQUANT
 *	Returns:        1 for Inter, 0 for Intra
 *
 *
 ***********************************************************************/

int ChooseMode(unsigned char *curr, int x_pos, int y_pos, int min_SAD)
{
  int i,j;
  int MB_mean = 0, A = 0;
  int y_off;

  for (j = 0; j < MB_SIZE; j++) {
    y_off = (y_pos + j) * pels;
    for (i = 0; i < MB_SIZE; i++) {
      MB_mean += *(curr + x_pos + i + y_off);
    }
  }
  MB_mean /= (MB_SIZE*MB_SIZE);
  for (j = 0; j < MB_SIZE; j++) {
    y_off = (y_pos + j) * pels;
    for (i = 0; i < MB_SIZE; i++) {
      A += abs( *(curr + x_pos + i + y_off) - MB_mean );
    }
  }

  if (A < (min_SAD - 500)) 
    return MODE_INTRA;
  else
    return MODE_INTER;
}

int ModifyMode(int Mode, int dquant)
{

  if (Mode == MODE_INTRA) {
    if(dquant!=0)
      return MODE_INTRA_Q;
    else
      return MODE_INTRA;
  }
  else{ 
    if(dquant!=0)
      return MODE_INTER_Q;
    else
      return Mode;
  }
}
