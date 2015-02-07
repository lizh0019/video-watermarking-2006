//OutputVlc.h gives the VlcTable and the declaration of output functions.
#ifndef _INC_OutputVlcAPI
#define _INC_OutputVlcAPI

/* type definitions for variable length code table entries */


#define sign(a)        ((a) < 0 ? 1 : 0)

/* Motion vectors */

static VLCtable mvtab[33] =
{
  {1,1}, {1,2}, {1,3}, {1,4}, {3,6}, {5,7}, {4,7}, {3,7},
  {11,9}, {10,9}, {9,9}, {17,10}, {16,10}, {15,10}, {14,10}, {13,10},
  {12,10}, {11,10}, {10,10}, {9,10}, {8,10}, {7,10}, {6,10}, {5,10},
  {4,10}, {7,11}, {6,11}, {5,11}, {4,11}, {3,11}, {2,11}, {3,12},
  {2,12}
};


/* CBPCM (MCBPC) Indexing by cbpc in first two bits, mode in last two.
 CBPC as in table 4/H.263, MB type (mode): 3 = 01, 4 = 10.
 Example: cbpc = 01 and mode = 4 gives index = 0110 = 6. */

static VLCtable cbpcm_intra_tab[15] =
{
  {0x01,9}, {0x01,1}, {0x01,4}, {0x00,0},
  {0x00,0}, {0x01,3}, {0x01,6}, {0x00,0},
  {0x00,0}, {0x02,3}, {0x02,6}, {0x00,0},
  {0x00,0}, {0x03,3}, {0x03,6}
};


/* CBPCM inter.
   Addressing: 5 bit ccmmm (cc = CBPC, mmm = mode (1-4 binary)) */

static VLCtable cbpcm_inter_tab[29] =
{
  {1,1}, {3,3}, {2,3}, {3,5}, {4,6}, {1,9}, {0,0}, {0,0},
  {3,4}, {7,7}, {5,7}, {4,8}, {4,9}, {0,0}, {0,0}, {0,0},
  {2,4}, {6,7}, {4,7}, {3,8}, {3,9}, {0,0}, {0,0}, {0,0},
  {5,6}, {5,9}, {5,8}, {3,7}, {2,9}
};


/* CBPY. Straightforward indexing */

static VLCtable cbpy_tab[16] =
{
  {3,4}, {5,5}, {4,5}, {9,4}, {3,5}, {7,4}, {2,6}, {11,4},
  {2,5}, {3,6}, {5,4}, {10,4}, {4,4}, {8,4}, {6,4}, {3,2}
};


/* DCT coefficients. Four tables, two for last = 0, two for last = 1.
   the sign bit must be added afterwards. */

/* first part of coeffs for last = 0. Indexed by [run][level-1] */

static VLCtable coeff_tab0[2][12] =
{
  /* run = 0 */
  {
    {0x02, 2}, {0x0f, 4}, {0x15, 6}, {0x17, 7},
    {0x1f, 8}, {0x25, 9}, {0x24, 9}, {0x21,10},
    {0x20,10}, {0x07,11}, {0x06,11}, {0x20,11}
  },
  /* run = 1 */
  {
    {0x06, 3}, {0x14, 6}, {0x1e, 8}, {0x0f,10},
    {0x21,11}, {0x50,12}, {0x00, 0}, {0x00, 0},
    {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  }
};

/* rest of coeffs for last = 0. indexing by [run-2][level-1] */

static VLCtable coeff_tab1[25][4] =
{
  /* run = 2 */
  {
    {0x0e, 4}, {0x1d, 8}, {0x0e,10}, {0x51,12}
  },
  /* run = 3 */
  {
    {0x0d, 5}, {0x23, 9}, {0x0d,10}, {0x00, 0}
  },
  /* run = 4-26 */
  {
    {0x0c, 5}, {0x22, 9}, {0x52,12}, {0x00, 0}
  },
  {
    {0x0b, 5}, {0x0c,10}, {0x53,12}, {0x00, 0}
  },
  {
    {0x13, 6}, {0x0b,10}, {0x54,12}, {0x00, 0}
  },
  {
    {0x12, 6}, {0x0a,10}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x11, 6}, {0x09,10}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x10, 6}, {0x08,10}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x16, 7}, {0x55,12}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x15, 7}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x14, 7}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1c, 8}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1b, 8}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x21, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x20, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1f, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1e, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1d, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1c, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1b, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x1a, 9}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x22,11}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x23,11}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x56,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  },
  {
    {0x57,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}
  }
};

/* first coeffs of last = 1. indexing by [run][level-1] */

static VLCtable coeff_tab2[2][3] =
{
  /* run = 0 */
  {
    {0x07, 4}, {0x19, 9}, {0x05,11}
  },
  /* run = 1 */
  {
    {0x0f, 6}, {0x04,11}, {0x00, 0}
  }
};

/* rest of coeffs for last = 1. indexing by [run-2] */

static VLCtable coeff_tab3[40] =
{
  {0x0e, 6}, {0x0d, 6}, {0x0c, 6},
  {0x13, 7}, {0x12, 7}, {0x11, 7}, {0x10, 7},
  {0x1a, 8}, {0x19, 8}, {0x18, 8}, {0x17, 8},
  {0x16, 8}, {0x15, 8}, {0x14, 8}, {0x13, 8},
  {0x18, 9}, {0x17, 9}, {0x16, 9}, {0x15, 9},    
  {0x14, 9}, {0x13, 9}, {0x12, 9}, {0x11, 9},    
  {0x07,10}, {0x06,10}, {0x05,10}, {0x04,10},    
  {0x24,11}, {0x25,11}, {0x26,11}, {0x27,11},    
  {0x58,12}, {0x59,12}, {0x5a,12}, {0x5b,12},    
  {0x5c,12}, {0x5d,12}, {0x5e,12}, {0x5f,12},
  {0x00, 0}               
};
extern int put_mv (int,int);
extern int put_cbpcm_intra (int, int);
extern int put_cbpcm_inter (int, int);
extern int put_cbpy (int, int);
extern int put_coeff (int last, int run, int level,int ifout);

 void initbits();
extern void putbits (int n, int value);
extern int alignbits ();
extern int bitcount();
#endif //!_INC_OutputVlcAPI