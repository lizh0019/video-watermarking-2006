// AttackMethod.h: interface for the AttackMethod class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATTACKMETHOD_H__526C9187_6647_4881_900D_85021607C840__INCLUDED_)
#define AFX_ATTACKMETHOD_H__526C9187_6647_4881_900D_85021607C840__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include"Global.h"
class AttackMethod  
{
public:
	void Rotate(PictImage *org, PictImage *rot, int width, int height, double angle);
	void Noise(PictImage *org, PictImage *noise, int pels,int lines, double percent);
	void Filter(PictImage *org,PictImage *enhance,int width,int height);
	void Crop(PictImage *org,PictImage *enhance,int width,int height);
	void AttackMethod::Blur(PictImage *org, PictImage *blur, int width, int height, int nTotLen, int nTotTime);
	void AttackMethod::ContrastEn(PictImage *org,PictImage *enhance,int width,int height);
	AttackMethod();
	virtual ~AttackMethod();

};

#endif // !defined(AFX_ATTACKMETHOD_H__526C9187_6647_4881_900D_85021607C840__INCLUDED_)
