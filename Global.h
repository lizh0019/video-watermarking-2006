#ifndef _INC_GlobalAPI
#define _INC_GlobalAPI 

#include "OutputVlc.h"

#define OFFLINE_RATE_CONTROL//jwp 
//SAD
#define PREF_NULL_VEC 100
#define PREF_16_VEC 200
#define PREF_PBDELTA_NULL_VEC 50

#define MAXIMAGESIZE 250000
#define MAXBOOKSIZE 2000

#define DEF_START_FRAME   0
#define PCT_INTER                       1
#define PCT_INTRA                       0
#define ON                              1
#define OFF                             0

#define PBMODE_NORMAL                   0
#define PBMODE_MVDB                     1
#define PBMODE_CBPB_MVDB                2

#define MBC   88//按16CIF设置
#define MBR   72

#define MB_SIZE 16
#define PSC        1
#define PSC_LENGTH        17
#define mmax(a, b)        ((a) > (b) ? (a) : (b))
#define mmin(a, b)        ((a) < (b) ? (a) : (b))
#define mnint(a)	((a) < 0 ? (int)(a - 0.5) : (int)(a + 0.5))


#define SF_SQCIF                        1  /* 001 */
#define SF_QCIF                         2  /* 010 */
#define SF_CIF                          3  /* 011 */
#define SF_4CIF                         4  /* 100 */
#define SF_16CIF                        5  /* 101 */

/* Quantization parameters */
#define DEF_INTER_QUANT   10/* default inter quantization parameter (also option "-q <n>") */
#define DEF_INTRA_QUANT   10/* default intra quantization parameter (also option "-I <n>") */
/* BQUANT parameter for PB-frame coding
 *   (n * QP / 4 )
 *
 *  BQUANT  n
 *   0      5
 *   1      6
 *   2      7
 *   3      8
 * ( also option "-Q <BQUANT>" ) */
#define DEF_BQUANT   2

#define MODE_INTER                      0
#define MODE_INTER_Q                    1
#define MODE_INTER4V                    2
#define MODE_INTRA                      3
#define MODE_INTRA_Q                    4
#define MAXVECTOR                       3000
/*************************************************************************/
/* Search windows */
/* default integer pel search seek distance ( also option "-s <n> " ) */
#define DEF_SEEK_DIST   15
/* default integer search window for 8x8 search centered
   around 16x16 vector. When it is zero only half pel estimation
   around the integer 16x16 vector will be performed */
/* for best performance, keep this small, preferably zero,
   but do your own simulations if you want to try something else */
#define DEF_8X8_WIN     0
/* default search window for PB delta vectors */
/* keep this small also */
#define DEF_PBDELTA_WIN   2
/*************************************************************************/
/*************************************************************************/

/* Miscellaneous */

/* write repeated reconstructed frames to disk (useful for variable
 * framerate, since sequence will be saved at 25 Hz)
 * Can be changed at run-time with option "-m" */
#define DEF_WRITE_REPEATED   NO

/* write bitstream trace to files trace.intra / trace
 * (also option "-t") */
#define DEF_WRITE_TRACE   NO

#ifdef OFFLINE_RATE_CONTROL
/* start rate control after DEF_START_RATE_CONTROL % of sequence
 * has been encoded. Can be changed at run-time with option "-R <n>" */
#define DEF_START_RATE_CONTROL   0
#else
/* default target frame rate when rate control is used */
#define DEF_TARGET_FRAME_RATE 10.0
#endif

/* headerlength on concatenated 4:1:1 YUV input file
 * Can be changed at run-time with option -e <headerlength> */
#define DEF_HEADERLENGTH   0

/* insert sync after each DEF_INSERT_SYNC for increased error robustness
 * 0 means do not insert extra syncs */
#define DEF_INSERT_SYNC   0

/*************************************************************************/

extern int advanced;
extern int syntax_arith_coding;

extern int headerlength;  
extern int pels;
extern int cpels;
extern int lines;
extern int trace;
extern int pb_frames;
extern int mv_outside_frame;
extern int long_vectors;
extern float target_framerate;

//extern FILE *tf;

/*the whole image structure*/
typedef struct pict_image {
  unsigned char *lum;        /* Luminance plane        */
  unsigned char *Cr;        /* Cr plane        */
  unsigned char *Cb;        /* Cb plane        */
} PictImage;

typedef struct
{
  char FileFlag[8];
  // int booktype;
  unsigned short int PixelBits;
  unsigned short int CodeBookSize;
  unsigned short int ImageBlockWidth;
  unsigned short int ImageBlockHeight;
  // char flag[8];
}BOOKFILEHEADER,FAR *LPBOOKFILEHEADER;

typedef struct
{
  char FileFlag[8];
  unsigned short int CodebookSize[3];
  unsigned short int dimension[3];
}DCTBOOKFILEHEADER;

typedef struct 
{
 char FileFlag[8];
 unsigned short int keynumber;
}KEYFILEHEADER;

/* MicroBlock structure */
typedef struct mb_structure {
  int lum[16][16];
  int Cr[8][8];
  int Cb[8][8];
} MB_Structure;


typedef struct pict {
	int TR;  /* Time reference */
	int source_format;  
	int picture_coding_type;
	int unrestricted_mv_mode;
    int PB; 
    int QUANT;
    int BQUANT;           /* which quantizer to use for B-MBs in PB-frame */
    int TRB;              /* Time reference for B-picture */
          
    int bit_rate;
    int src_frame_rate;
    float target_frame_rate;

  int DQUANT;
  int MB;
  int seek_dist;        /* Motion vector search window */
  int use_gobsync;      /* flag for gob_sync */
  int MODB;             /* B-frame mode */
  
  float QP_mean;        /* mean quantizer */
} Pict;

/* Structure for counted bits */
typedef struct bits_counted {
  int Y;
  int C;
  int vec;
  int CBPY;
  int CBPCM;
  int MODB;
  int CBPB;
  int COD;
  int header;
  int DQUANT;
  int total;
  int no_inter;
  int no_inter4v;
  int no_intra;
} Bits;

/* Motionvector structure */

typedef struct motionvector {
  int x;        /* mv 的水平分量 */
  int y;        /* mv 的垂直分量 */
  int x_half;       /* 水平半象素 */
  int y_half;       /* 垂直半象素 */
  int min_error;    /* 该矢量的最小误差	 */
  int Mode;       /* 宏块编码类型 */
} MotionVector;
#define NO_VEC                          999

struct  NCDATA
{
  int Corrct;
  int Wrong;
  double NC;
};

//CodeInter.cpp
void CodeOneInter(PictImage *prev,PictImage *curr,PictImage *prev_recon,PictImage *curr_recon,
                  int QP, int frameskip, Bits *bits, Pict *pic,int *embedtimes, int *P_frg_times,bool mvmark);
void ZeroVec(MotionVector *MV);
void MarkVec(MotionVector *MV);
void CopyVec(MotionVector *MV2, MotionVector *MV1);
int EqualVec(MotionVector *MV2, MotionVector *MV1);
unsigned char *InterpolateImage(unsigned char *image, int width, int height);

void MotionEstimatePicture(unsigned char *curr, unsigned char *prev,unsigned char *prev_ipol, int seek_dist,MotionVector *MV[6][MBR+1][MBC+2], int gobsync);
void MEPictureWatermark(unsigned char *curr, unsigned char *prev,unsigned char *prev_ipol, int seek_dist,MotionVector *MV[6][MBR+1][MBC+2], int gobsync,int *embedtimes,int indicator);
void RDMotEstPicture(unsigned char *curr, unsigned char *prev, Pict *pic,  
					 unsigned char *prev_ipol,PictImage *prev_recon, int seek_dist, 
					 MotionVector *MV[6][MBR+1][MBC+2], int gobsync,int QP);

void ZeroMBlock(MB_Structure *data);

void CodeOneInterVQ(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
  			      int QP, int frameskip, Bits *bits, Pict *pic,int framenum);

void  CodeOneInterVisualVQ(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
  			      int QP, int frameskip, Bits *bits, Pict *pic,int framenum,int *times);

void  WatermarkVisualVQ(PictImage *prev,PictImage *curr,
						PictImage *pr,PictImage *curr_recon,
						int QP, int frameskip, Bits *bits, Pict *pic,int framenum,int *embedtimes,int *embedtimes2,bool multi);

void Semi_frg_Wm_VisualVQ(PictImage *prev,PictImage *curr,
					  	PictImage *pr,PictImage *curr_recon,
						int QP, int frameskip, Bits *bits, Pict *pic,int framenum,int *embedtimes);

void  RDOptCodeInter(PictImage *prev,PictImage *curr,
					  PictImage *pr,PictImage *curr_recon,
					  int QP, int frameskip, Bits *bits, Pict *pic,long int*,long int*, int*);


int BookGenerate(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic,long int mn);

int VisualBookGenerate(PictImage *prev,PictImage *curr,
				  PictImage *pr,PictImage *curr_recon,
				  int QP, int frameskip, Bits *bits, Pict *pic,long int mn);

void LBG(long int m, int*TrainVectors[MAXIMAGESIZE], int * CodeVectors[MAXBOOKSIZE],int dimension,int booksize);

void Extendbook(int * CodeVectors[MAXBOOKSIZE],int num,int k,int booksize);

void ExtCodeword(int *CodeVectors, int m, int dimension);



//mot_est.cpp
void MotionEstimation(unsigned char *curr, unsigned char *prev, int x_curr,
              int y_curr, int xoff, int yoff, int seek_dist, 
              MotionVector *MV[6][MBR+1][MBC+2], int *SAD_0);

void RDMotionEstimate(unsigned char *curr, unsigned char *prev, int x_curr,
					  int y_curr, int xoff, int yoff, int seek_dist, int QP,Pict *pic,
					  MotionVector *MV[6][MBR+1][MBC+2],unsigned char *prev_ipol,
					  PictImage *prev_recon,int *SAD_0,int newgob);

unsigned char *LoadArea(unsigned char *im, int x, int y, 
        int x_size, int y_size, int lx);
int SAD_Macroblock(unsigned char *ii, unsigned char *act_block,
           int h_length, int Min_FRAME);
int SAD_Block(unsigned char *ii, unsigned char *act_block,
              int h_length, int min_sofar);
int SAD_MB_Bidir(unsigned char *ii, unsigned char *aa, unsigned char *bb, 
         int width, int min_sofar);
int SAD_MB_integer(int *ii, int *act_block, int h_length, int min_sofar);
void FindMB(int x, int y, unsigned char *image, int MB[16][16]);


//pred.cpp
MB_Structure *Predict_P(PictImage *curr_image, PictImage *prev_image,
        unsigned char *prev_ipol, int x, int y, 
        MotionVector *MV[6][MBR+1][MBC+2], int PB);
MB_Structure *Predict_B(PictImage *curr_image, PictImage *prev_image,
        unsigned char *prev_ipol,int x, int y,
        MotionVector *MV[5][MBR+1][MBC+2],
        MB_Structure *recon_P, int TRD,int TRB);
MB_Structure* MB_Recon_B(PictImage* prev_image, \
					MB_Structure *diff, \
         		unsigned char *prev_ipol, \
               int x, int y,    \
         		MotionVector *MV[5][MBR+1][MBC+2], \
         		MB_Structure *recon_P, \
               int TRD, int TRB);
void FindForwLumPredPB(unsigned char *prev_ipol, int x_curr, int y_curr, 
               MotionVector *fr, int *pred, int TRD, int TRB, 
               int bdx, int bdy, int bs, int comp);
void FindBiDirLumPredPB(int *recon_P, MotionVector *fr, int *pred, int TRD, 
        int TRB, int bdx, int bdy, int nh, int nv);
void FindBiDirChrPredPB(MB_Structure *recon_P, int dx, int dy, 
        MB_Structure *pred);
void FindBiDirLimits(int vec, int *start, int *stop, int nhv);
void FindBiDirChromaLimits(int vec, int *start, int *stop);
void BiDirPredBlock(int xstart, int xstop, int ystart, int ystop,
            int xvec, int yvec, int *recon, int *pred, int bl);
void DoPredChrom_P(int x_curr, int y_curr, int dx, int dy,
           PictImage *curr, PictImage *prev, 
           MB_Structure *pred_error);
void FindHalfPel(int x, int y, MotionVector *fr, unsigned char *prev, 
         int *curr, int bs, int comp);
void FindPred(int x, int y, MotionVector *fr, unsigned char *prev, 
              int *pred, int bs, int comp);
void FindPredOBMC(int x, int y, MotionVector *MV[6][MBR+1][MBC+2], 
          unsigned char *prev, int *pred, int comp, int PB);
MB_Structure *MB_Recon_P(PictImage *prev_image, unsigned char *prev_ipol,
         MB_Structure *diff, int x_curr, int y_curr, 
         MotionVector *MV[6][MBR+1][MBC+2], int PB);
void ReconLumBlock_P(int x, int y, MotionVector *fr, 
             unsigned char *prev, int *data, int bs, int comp);
void ReconChromBlock_P(int x_curr, int y_curr, int dx, int dy,
               PictImage *prev, MB_Structure *data);
void FindChromBlock_P(int x_curr, int y_curr, int dx, int dy,
              PictImage *prev, MB_Structure *data);
int ChooseMode(unsigned char *curr, int x_pos, int y_pos, int min_SAD);
int ModifyMode(int Mode, int dquant);


//CodeOneIntra.cpp
PictImage *CodeOneIntra(PictImage *curr, int QP, Bits *bits, Pict *pic,int *ebtime,bool ifwatermark,bool indicator);
void Clip(MB_Structure *data);
void ReconImage (int i, int j, MB_Structure *data, PictImage *recon);
PictImage *Intra_DCTbook_Gen(PictImage *curr, Bits *bits, Pict *pic, int *LTV[MAXIMAGESIZE],
							 int *MTV[MAXIMAGESIZE],int *HTV[MAXIMAGESIZE],int TrainNumber[3]);
PictImage *CodeIntra_DCTVQ(PictImage *curr, Bits *bits, Pict *pic,int **Lcword,int**Mcword,int **Hcword, int gopnumber,int *ebtime,int &Au_times);
void Quant(int *coeff, int *qcoeff, int QP, int Mode);
void Dequant(int *qcoeff, int *rcoeff, int QP, int Mode);
void FillLumBlock( int x, int y, PictImage *image, MB_Structure *data);
void FillChromBlock(int x_curr, int y_curr, PictImage *image,MB_Structure *data);

int *MB_Encode(MB_Structure *mb_orig, int QP, int I);
void Watermark_EncodeVQ(int index[6],unsigned char wmb[6],int booksize);
void MV_WatermarkEmbed(MotionVector *mv,  unsigned char *wmb,int Mode);
void WM_intra(unsigned char *keymark, int *Quancoeff,int *times,bool multi);

int GetDirection(int x,int y);
void Getsymbol(int angle,MotionVector *mv);

int *MB_EncodeVQ(MB_Structure *mb_orig, int *codeword[MAXBOOKSIZE],int QP, int I,int index[6],int booksize);
double VectorDistance(int dimension, int *Train, int *Code);
int MB_Decode(int *qcoeff, MB_Structure *mb_recon, int QP, int I);
void FragileWm_EmbedVQ(MB_Structure *mb_orig, int *codeword[MAXBOOKSIZE],int booksize,int index[6],unsigned char *wmb,int pos);

int MB_Distortion(int *qcoeff, MB_Structure *mb_recon, int QP);

int MB_DecodeVQ( MB_Structure *mb_recon, int *codeword[MAXBOOKSIZE],int QP, int I,int index[6],int indicator[6]);
int	FindVQCBP( MB_Structure *mb_orig, int *codeword[MAXBOOKSIZE],int index[6],int indicator[6]);
void memorytrainvector(MB_Structure *mb_orig, int*trainvector[MAXIMAGESIZE],int m);

//Countbit.cpp
int CountBitsPicture(Pict *pic);
void CountBitsMB(int Mode, int COD, int CBP, int CBPB, Pict *pic, Bits *bits);
int CountBitsSlice(int slice, int quant);
int CountBitsCoeff(int *qcoeff, int Mode, int CBP, Bits *bits, int ncoeffs, int ifout);
int CodeCoeff(int Mode, int *qcoeff, int block, int ncoeffs,int ifout);
int FindCBP(int *qcoeff, int Mode, int ncoeffs);
void ZeroBits(Bits *bits);
void AddBits(Bits *total, Bits *bits);
void AddBitsPicture(Bits *bits);


void FindPMV(MotionVector *MV[6][MBR+1][MBC+2], int x, int y,int *pmv0, int *pmv1, int block, int newgob, int half_pel);
int CountBitsVectors(MotionVector *MV[6][MBR+1][MBC+2], Bits *bits, int x, int y, int Mode, int newgob, Pict *pic,int ifout);
void CountBitsIndex(int index[6],int bit,Bits *bits,int indicator[6]);
void Output_DCTVQIndex(int index[6][3],int bit[3]);

//Dct.cpp
int Dct( int *block, int *coeff);
int idct(int *coeff,int *block);
void init_idctref();

//Lable LBG.cpp
short int DCTVQLLBGCodebookDesign(unsigned short int dimension, unsigned long int blocks, unsigned short int booksize,
								  int *TrainVector[MAXIMAGESIZE], int *CodeVector[MAXBOOKSIZE]);

short int DCTVQLLBGCodebookDesign_II(unsigned short int dimension, unsigned long int blocks, unsigned short int booksize,
								  int *TrainVector[MAXIMAGESIZE], int *CodeVector[MAXBOOKSIZE]);

//VQindexhuffman.cpp
int VQ_index_Huff(int index_num,int word_num,int *VQindex);

//DCTVQ.cpp
void MB_DCTVQ_Encode(MB_Structure *mb_orig, int **codewordL,int **codewordM,int **codewordH,int index[6][3],
                     int booksize1,int booksize2,int booksize3,int &frg_frame,int &Au_num);
void MB_DCTVQ_Decode(MB_Structure *mb_orig, int **codewordL,int **codewordM,int **codewordH,int index[6][3]);
void MB_DCTVQ_Embed(MB_Structure *mb_orig, int **codewordL,int **codewordM,int **codewordH,
					 unsigned char Lwm[6],unsigned char Mwm[6],
                     int index[6][3],int booksize1,int booksize2,int booksize3,int &frg_frame,int &Au_num);

//P_Authen.cpp
void Generate_P_Authenbits(MotionVector *MV,unsigned char *WM,unsigned char *P_Authbit);
int *MB_Fragile_Embed(MB_Structure *mb_orig, int QP, int I,unsigned char *Authen_bit);

//IO.cpp
PictImage *FillImage(unsigned char *in);
PictImage *InitImage(int size);
extern void FreeImage(PictImage *image);

//RateControl.cpp
#ifdef OFFLINE_RATE_CONTROL
extern int FrameUpdateQP(int buf, int bits, int frames_left, int QP, int B,float seconds);
#else
extern int UpdateQuantizer(int mb, float QP_mean, int pict_type, float bit_rate,
                    int mb_width, int mb_height, int bitcount) ;

extern int InitializeQuantizer(int pict_type, float bit_rate,
                                           float target_frame_rate, float QP_mean);
extern void InitializeRateControl();
extern void UpdateRateControl(int bits);
#endif

//GetNC.cpp
NCDATA GetNC(int width, int height, unsigned char *Org, unsigned char *Wm);
BOOL  OpenWm(int m_width,int m_height,unsigned char *orgbit, CString &name);

//VQ_Encode.cpp
int VQ_Encode(int *vect,int **codeword,int Low,int High, int dimension);
int VQ_Constrain(int *vect,int **codeword,int Low,int High, int dimension,int pos, unsigned char wm_bit);

#endif //!_INC_GlobalAPI
