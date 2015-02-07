// Display.h: interface for the CDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISPLAY_H__5DCE47A3_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_)
#define AFX_DISPLAY_H__5DCE47A3_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_

#include "Idct.h"	// Added by ClassView
#include "Getbits.h"	// Added by ClassView
#include "Gethdr.h"	// Added by ClassView
#include "Getpic.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDisplay  
{
public:
	void Cau_NC(int SegNum, unsigned char Ro[20][64], unsigned char Fp[20][64],double RobustNC[20],double FngPtNC[20]);
	void MajorityVote(int SegNum, unsigned char Ro[20][384],unsigned char Ro_to_show[20][64],unsigned char Fp[20][384],unsigned char Fp_to_show[20][64],int capacity);
	void ChioseOneBit(unsigned char *multibit, unsigned char *onebit, int multinum, int shownum);
	CString conclusion,number;
	CString szPictureFormat;
	CIdct m_idct;
	int nDitherType;
	PBITMAPINFO pbmi;
	HPALETTE hpal,hPalPrev;
	HDC hDC;
    char  *image;
	CGetpic m_getpic;
	CGethdr m_gethdr;
	CGetbits m_getbits;
	void play_movie(CDC *pDC,CString szFileName,long int sp,bool rec,CFile &recfile);
	void exit_display(void);
	void dither(unsigned char *src[],bool recd,CFile &recdfile);           
	void init_dither(int bpp);
	void init_display();
	CDisplay();
	virtual ~CDisplay();

private:
	void toDeleteNewSpace();
	void initdecoder(void);
};

#endif // !defined(AFX_DISPLAY_H__5DCE47A3_490D_11D5_9DCC_5254AB2B9F00__INCLUDED_)
