// Getvlc.cpp: implementation of the CGetvlc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "��Ƶ�������.h"
#include "Getvlc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern int quiet,fault,trace; 
CGetvlc::CGetvlc()
{

}

CGetvlc::~CGetvlc()
{

}

int CGetvlc::getCBPY()
{
  int code;
  code = m_getbits.showbits(6);
  if (code < 2) 
  { //��Ч��CBPY
    fault = 1;
    return -1;
  }
  if (code>=48)
  {
    m_getbits.flushbits(2);
    return 0;
  }
  m_getbits.flushbits(CBPYtab[code].len);
  return CBPYtab[code].code;
}

int CGetvlc::getTMNMV()
{
  int code;
  if (m_getbits.getbits1())//Ϊ"1"ʱ  �˶�ʸ��Ϊ0
     return 0;
 
  if ((code = m_getbits.showbits(12))>=512)
  {
    code = (code>>8) - 2;//MV�������
    m_getbits.flushbits(TMNMVtab0[code].len);//�ṹ�壬MV�ĳ���
                           //װ��N������
    return TMNMVtab0[code].code;
  }

  if (code>=128)
  {
    code = (code>>2) -32;
    m_getbits.flushbits(TMNMVtab1[code].len);
    return TMNMVtab1[code].code;
  }

  if ((code-=5)<0)
  {
    if (!quiet)
      AfxMessageBox("Invalid motion_vector code!");
    fault=1;
    return 0;
   }

  m_getbits.flushbits(TMNMVtab2[code].len);
  return TMNMVtab2[code].code;

}

int CGetvlc::getMCBPC()
{
 int code;
  code = m_getbits.showbits(9);//�õ�������
  if (code == 1)
  {
    // macroblock stuffing 
    m_getbits.flushbits(9);//����������ˢ��
    return -1;
  }

  if (code == 0) 
  {
    fault = 1;
    return 0;
  }
    
  if (code>=256)
  {
    m_getbits.flushbits(1);
    return 0;
  }
    
  m_getbits.flushbits(MCBPCtab[code].len);
  return MCBPCtab[code].code;

}

int CGetvlc::getMODB()
{
 int code;
  int MODB;
  code = m_getbits.showbits(2);

  if (code < 2)
  {
    MODB = 0;
    m_getbits.flushbits(1);
  }
  else if (code == 2)
  {
    MODB = 1;
    m_getbits.flushbits(2);
  }
  else 
  { /* code == 3 */
     AfxMessageBox("11: MODB = 2!");
    MODB = 2;
    m_getbits.flushbits(2);
  }
  return MODB;

}

int CGetvlc::getMCBPCintra()
{
  int code;
  code = m_getbits.showbits(9);

  if (code == 1) 
  {
    // macroblock stuffing 
    m_getbits.flushbits(9);
    return -1;
  }

  if (code < 8)
  {
    fault = 1;
    return 0;
  }
  code >>= 3;//��stuffing�������6����
  if (code>=32)
  {
    m_getbits.flushbits(1);
    return 3;
  }
  m_getbits.flushbits(MCBPCtabintra[code].len);
  return MCBPCtabintra[code].code;

}


