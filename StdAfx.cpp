// stdafx.cpp : source file that includes just the standard includes
//	��Ƶ�������.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

//picture specific data (picture header)
  int vbv_delay; //video buffering verifier delay (1/90000 seconds)
//ͼ������ 
//reconstructed frames  ָ������
  unsigned char *newrefframe[3], *oldrefframe[3];
//original frames
  unsigned char *neworgframe[3], *oldorgframe[3];
// prediction of current frame
  unsigned char *predframe[3];
// 8*8 block data
  short (*blocks)[64];//ָ�������ָ��

//�����Ϣ
  struct mbinfo *mbinfor;
//���뻺����Ϣ
  ldecode *ld,base;

//ͼ��ߴ�
  int block_count;  //����еÿ���
  double frame_rate;   //frames per second 
  double bit_rate;     //bits per second
  int vbv_buffer_size; //size of VBV buffer (* 16 kbit)


