// Getpic.h: interface for the CGetpic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETPIC_H__D3C9E465_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_GETPIC_H__D3C9E465_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Getvlc.h"	// Added by ClassView
#include "Gethdr.h"	// Added by ClassView
#include "Getbits.h"// Added by ClassView
#include "Getblk.h"	// Added by ClassView

#include "Idct.h"	// Added by ClassView
#include "Recon.h"	// Added by ClassView
#include "Store.h"	// Added by ClassView
#include "Sarcode.h"// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define MAXBOOKSIZE 2000

class CGetpic  
{
public:
	double authen_count;
	void Authen_Ifrmae(int index[6][3],unsigned char *Ifrg_ex,int *number);
	void MB_DCTVQ_Extract(int index[6][3],unsigned char *robust,unsigned char *fngprt, int *ex_time);
	unsigned char ro_bit[20][384],fp_bit[20][384];
	unsigned char * CheckPosition(int index,int bitnumber);
	void KeyMark_Extract(unsigned char *filebit, unsigned char *waterbit, int coeff[], int sum, int *times);
	void ExtractVMinMV(int mvx,int mvy, int *sumbit, unsigned char mark[1500]);
	int GetAngle(int x,int y);
	void addVQblock(int comp, int bx, int by, int addflag,int ind,int *codeword[MAXBOOKSIZE]);
	void deVQ(int *codeword[MAXBOOKSIZE],int index,short *block);
	CSarcode m_sacode;
	CStore m_store;
	CRecon m_recon;
	CIdct m_idct;
	CGetblk m_getblk;
	CGetvlc m_getvlc;
	CGetbits m_getbits;
	CGethdr m_gethdr;
	void interpolate_image(unsigned char *in, unsigned char *out, int width, int height);
	CGetpic();
	virtual ~CGetpic();
	void getpicture(int *framenum,int *waternum,int *waternum2);

private:
	void make_edge_image(unsigned char *src, unsigned char *dst, int width, int height, int edge);
	void find_bidir_chroma_limits(int vec, int *start, int*stop);
	void find_bidir_limits(int vec, int *start, int*stop, int nhv);
	void reconblock_b(int comp,int bx,int by,int mode,int bdx, int bdy);
	void addblock (int comp, int bx, int by,int addflag);
	int find_pmv(int x, int y, int block, int comp);
	int motion_decode(int vec,int pmv);
	void clearblock(int comp);
	void getMBs(int framenum,int * sumbit,int *sumbit2);
};

#endif // !defined(AFX_GETPIC_H__D3C9E465_48B2_11D5_9DCC_5254AB2B9F00__INCLUDED_)
