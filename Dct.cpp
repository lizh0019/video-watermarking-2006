#include"stdafx.h"
#include"Global.h"



#include <math.h>
#define PI 3.14159265358979323846


int	zigzag[8][8] = {
  {0, 1, 5, 6,14,15,27,28},
  {2, 4, 7,13,16,26,29,42},
  {3, 8,12,17,25,30,41,43},
  {9,11,18,24,31,40,44,53},
  {10,19,23,32,39,45,52,54},
  {20,22,33,38,46,51,55,60},
  {21,34,37,47,50,56,59,61},
  {35,36,48,49,57,58,62,63},
};

/**********************************************************************
 *
 *	Name:        Dct
 *	Description:	Does dct on an 8x8 block, does zigzag-scanning of
 *        coefficients
 *
 *	Input:        64 pixels in a 1D array
 *	Returns:	64 coefficients in a 1D array
 *	Side effects:	
 *
 *	Date: 930128	Author: Robert.Danielsen@nta.no
 *
 **********************************************************************/

int Dct( int *block, int *coeff)
{
  int          j1, i, j, k;
  float	       b[8];
  float        b1[8];
  float        d[8][8];
  float f0=(float).7071068;
  float f1=(float).4903926;
  float f2=(float).4619398;
  float f3=(float).4157348;
  float f4=(float).3535534;
  float f5=(float).2777851;
  float f6=(float).1913417;
  float f7=(float).0975452;

  for (i = 0, k = 0; i < 8; i++, k += 8) {
    for (j = 0; j < 8; j++) {
      b[j] = (float)block[k+j];
    }
    /* Horizontal transform */
    for (j = 0; j < 4; j++) {
      j1 = 7 - j;
      b1[j] = b[j] + b[j1];
      b1[j1] = b[j] - b[j1];
    }
    b[0] = b1[0] + b1[3];
    b[1] = b1[1] + b1[2];
    b[2] = b1[1] - b1[2];
    b[3] = b1[0] - b1[3];
    b[4] = b1[4];
    b[5] = (b1[6] - b1[5]) * f0;
    b[6] = (b1[6] + b1[5]) * f0;
    b[7] = b1[7];
    d[i][0] = (b[0] + b[1]) * f4;
    d[i][4] = (b[0] - b[1]) * f4;
    d[i][2] = b[2] * f6 + b[3] * f2;
    d[i][6] = b[3] * f6 - b[2] * f2;
    b1[4] = b[4] + b[5];
    b1[7] = b[7] + b[6];
    b1[5] = b[4] - b[5];
    b1[6] = b[7] - b[6];
    d[i][1] = b1[4] * f7 + b1[7] * f1;
    d[i][5] = b1[5] * f3 + b1[6] * f5;
    d[i][7] = b1[7] * f7 - b1[4] * f1;
    d[i][3] = b1[6] * f3 - b1[5] * f5;
  }
  /* Vertical transform */
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 4; j++) {
      j1 = 7 - j;
      b1[j] = d[j][i] + d[j1][i];
      b1[j1] = d[j][i] - d[j1][i];
    }
    b[0] = b1[0] + b1[3];
    b[1] = b1[1] + b1[2];
    b[2] = b1[1] - b1[2];
    b[3] = b1[0] - b1[3];
    b[4] = b1[4];
    b[5] = (b1[6] - b1[5]) * f0;
    b[6] = (b1[6] + b1[5]) * f0;
    b[7] = b1[7];
    d[0][i] = (b[0] + b[1]) * f4;
    d[4][i] = (b[0] - b[1]) * f4;
    d[2][i] = b[2] * f6 + b[3] * f2;
    d[6][i] = b[3] * f6 - b[2] * f2;
    b1[4] = b[4] + b[5];
    b1[7] = b[7] + b[6];
    b1[5] = b[4] - b[5];
    b1[6] = b[7] - b[6];
    d[1][i] = b1[4] * f7 + b1[7] * f1;
    d[5][i] = b1[5] * f3 + b1[6] * f5;
    d[7][i] = b1[7] * f7 - b1[4] * f1;
    d[3][i] = b1[6] * f3 - b1[5] * f5;
  }
  /* Zigzag - scanning */
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      *(coeff + zigzag[i][j]) = (int)(d[i][j]);
    }
  }
  return 0;
}


/**********************************************************************
 *
 *	Name:        idct
 *	Description:	Descans zigzag-scanned coefficients and does
 *        inverse dct on 64 coefficients
 *                      single precision floats
 *
 *	Input:        64 coefficients, block for 64 pixels
 *	Returns:        0
 *	Side effects:	
 *
 *	Date: 930128	Author: Robert.Danielsen@nta.no
 *
 **********************************************************************/

int idct(int *coeff,int *block)
{
  int                j1, i, j;
  double b[8], b1[8], d[8][8];
  double f0=.7071068;
  double f1=.4903926;
  double f2=.4619398;
  double f3=.4157348;
  double f4=.3535534;
  double f5=.2777851;
  double f6=.1913417;
  double f7=.0975452;
  double e, f, g, h;

  /* Horizontal */

  /* Descan coefficients first */

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      b[j] = *( coeff + zigzag[i][j]);
    }
    e = b[1] * f7 - b[7] * f1;
    h = b[7] * f7 + b[1] * f1;
    f = b[5] * f3 - b[3] * f5;
    g = b[3] * f3 + b[5] * f5;

    b1[0] = (b[0] + b[4]) * f4;
    b1[1] = (b[0] - b[4]) * f4;
    b1[2] = b[2] * f6 - b[6] * f2;
    b1[3] = b[6] * f6 + b[2] * f2;
    b[4] = e + f;
    b1[5] = e - f;
    b1[6] = h - g;
    b[7] = h + g;
    
    b[5] = (b1[6] - b1[5]) * f0;
    b[6] = (b1[6] + b1[5]) * f0;
    b[0] = b1[0] + b1[3];
    b[1] = b1[1] + b1[2];
    b[2] = b1[1] - b1[2];
    b[3] = b1[0] - b1[3];

    for (j = 0; j < 4; j++) {
      j1 = 7 - j;
      d[i][j] = b[j] + b[j1];
      d[i][j1] = b[j] - b[j1];
    }
  }

  /* Vertical */
  
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      b[j] = d[j][i];
    }
    e = b[1] * f7 - b[7] * f1;
    h = b[7] * f7 + b[1] * f1;
    f = b[5] * f3 - b[3] * f5;
    g = b[3] * f3 + b[5] * f5;

    b1[0] = (b[0] + b[4]) * f4;
    b1[1] = (b[0] - b[4]) * f4;
    b1[2] = b[2] * f6 - b[6] * f2;
    b1[3] = b[6] * f6 + b[2] * f2;
    b[4] = e + f;
    b1[5] = e - f;
    b1[6] = h - g;
    b[7] = h + g;

    b[5] = (b1[6] - b1[5]) * f0;
    b[6] = (b1[6] + b1[5]) * f0;
    b[0] = b1[0] + b1[3];
    b[1] = b1[1] + b1[2];
    b[2] = b1[1] - b1[2];
    b[3] = b1[0] - b1[3];

    for (j = 0; j < 4; j++) {
      j1 = 7 - j;
      d[j][i] = b[j] + b[j1];
      d[j1][i] = b[j] - b[j1];
    }
  }

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      *(block + i * 8 + j) = mnint(d[i][j]);
    }
  }
  return 0;
}

