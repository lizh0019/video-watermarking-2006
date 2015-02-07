// Getpic.cpp: implementation of the CGetpic class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "视频编解码器.h"
#include "Getpic.h"

#include  <math.h>

#include "indices.h" 
#include "sactbls.h"
#include"P_AuthenGen.h"

extern int *codeword[MAXBOOKSIZE];
extern int **Low_codeword,**Mid_codeword,**Hig_codeword;
extern unsigned char markbit[5000];
extern unsigned char singalbit[4096];
extern unsigned char keybit[44096];
extern unsigned char secondsingalbit[4096];

unsigned char key[44096];
unsigned char key2[44096];
unsigned char frg_bit[16384];
unsigned char I_frg_bit[16384];

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define sign(a)         ((a) < 0 ? -1 : 1)
#define mmax(a, b)  	((a) > (b) ? (a) : (b))
#define mmin(a, b)  	((a) < (b) ? (a) : (b))

extern  vqdecode;
int index[6];
extern  h263;
extern  visualcode;
extern  basicextract;
extern  dmmvextract;
extern  multiextract;
extern  dctparity;
extern  robfrgextract;
extern  rob_semifrg_extract;
extern  bridge_extract;
extern  RoFgFp_extract;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
 int MV[2][5][MBR_MAX+1][MBC_MAX+2];
 int modemap[MBR_MAX+1][MBC_MAX+2];
 unsigned char trimark[1500];
 unsigned char secondtrimark[1500];
 unsigned char P_Auth_Extract[16384];
extern int mv_outside_frame;
extern int	zigzag[8][8];
extern unsigned char *edgeframe[3], *edgeframeorig[3], *exnewframe[3],*bframe[3];
extern unsigned char *newframe[3],*oldrefframe[3],*refframe[3];
extern int horizontal_size,vertical_size,mb_width,mb_height;
extern int coded_picture_width, coded_picture_height,pb_frame;
extern int chrom_width,chrom_height,expand,outtype,newgob;
extern int MBC,MBR,syntax_arith_coding,quiet,quant,trace,fault,tr_framenum;
extern int trb,trd,blk_cnt,pict_type,adv_pred_mode,long_vectors;
extern unsigned char *clp;
extern ldecode *ld;
int I_num_toshow;

CGetpic::CGetpic()
{

}

CGetpic::~CGetpic()
{

}

void CGetpic::getpicture(int *framenum,int *waternum,int *waternum2)
{
  int i;
  unsigned char *tmp;
 
  for (i=0; i<3; i++) 
  {
    tmp = oldrefframe[i];
    oldrefframe[i] = refframe[i];
    refframe[i] = tmp;
    newframe[i] = refframe[i];
  }
  //级预测模式  
  if (mv_outside_frame && *framenum > 0) 
  {
    make_edge_image(oldrefframe[0],edgeframe[0],coded_picture_width,
		               coded_picture_height,32);
    make_edge_image(oldrefframe[1],edgeframe[1],chrom_width, chrom_height,16);
    make_edge_image(oldrefframe[2],edgeframe[2],chrom_width, chrom_height,16);
  }
  
  //得到图像的块数据
  getMBs(*framenum,waternum,waternum2);

  if (pb_frame) 
  {
    if (expand && outtype == T_X11)//扩张模式 得到图像时间上的分辨率
	{ //计算插值图像 Y U V
      interpolate_image(bframe[0], exnewframe[0], coded_picture_width, coded_picture_height);
      interpolate_image(bframe[1], exnewframe[1], chrom_width, chrom_height);
      interpolate_image(bframe[2], exnewframe[2], chrom_width, chrom_height);
      //存储变换后的图像
      m_store.storeframe(exnewframe, *framenum);
    }
    else
      m_store.storeframe(bframe,*framenum);
   
   *framenum += pb_frame;

#ifdef USE_TIME
    if (framerate > 0)
      doframerate();
#endif
  }//end of(pb_frame)

  if (expand && outtype == T_X11) 
  {  //计算插值图像 增加时间上的分辨率
    interpolate_image(newframe[0], exnewframe[0],
		                coded_picture_width, coded_picture_height);
    interpolate_image(newframe[1], exnewframe[1],
		                chrom_width, chrom_height);
    interpolate_image(newframe[2], exnewframe[2],
		                chrom_width, chrom_height);
     //存储变换后的图像文件
    m_store.storeframe(exnewframe, *framenum);
  }
  else
    m_store.storeframe(newframe,*framenum);

}

/*****************************************************************************/
void CGetpic::getMBs(int framenum,int * sumbit,int *sumbit2)
{
  int comp;
  int MBA, MBAmax;
  int bx, by;
  CString outshow;

  int COD=0,MCBPC, CBPY, CBP=0, CBPB=0, MODB=0, Mode=0, DQUANT,fblock=0;
  int COD_index, CBPY_index, MODB_index, DQUANT_index, MCBPC_index;
  int INTRADC_index, YCBPB_index, UVCBPB_index, mvdbx_index, mvdby_index;
  int mvx, mvy, mvy_index, mvx_index, pmv0, pmv1, xpos, ypos, gob, i,k;
  int mvdbx=0, mvdby=0, pmvdbx, pmvdby, gfid, YCBPB, UVCBPB, gobheader_read;
  int startmv,stopmv,offset,bsize,last_done=0,pCBP=0,pCBPB=0,pCOD=0;
  int DQ_tab[4] = {-1,-2,1,2},qc[6];
  int TH=650,sum=0,robcounter=0;
  short *bp,DCT_coeff[12][64]={0};
  bool fast=false,pfast=false,feature=false,P_Authen=false;
  int DCTVQ_index[6][3],index_bit[3]={11,10,10};
  int tempnumber=0;
  int DCTVQ_block[12][64];
  int frextract_no=0;
  unsigned char MV_feature[2],DCT_feature;
  int pos=20;
  int num_key=0;
  int I_Au_frame=0;
  
  if(pict_type==PCT_INTRA)
  {
      I_num_toshow++;
  }
  // 每帧图象的宏块数
  MBAmax = mb_width*mb_height;
  MBA = 0; // 宏块标志 
  newgob = 0;
  //初始化图像边上的宏块的运动矢量
  // 1  标记图象上边沿MV 
  for (i = 1; i < MBC+1; i++) 
  {
    for (k = 0; k < 5; k++)
	{
      MV[0][k][0][i] = NO_VEC;
      MV[1][k][0][i] = NO_VEC;
    }
    modemap[0][i] = MODE_INTRA;//编码模式设为帧内
  }
  // 2 标记图象边沿的MV为0 
  for (i = 0; i < MBR+1; i++) 
  {
    for (k = 0; k < 5; k++) 
	{
      MV[0][k][i][0] = 0;
      MV[1][k][i][0] = 0;
      MV[0][k][i][MBC+1] = 0;
      MV[1][k][i][MBC+1] = 0;
    }
    modemap[i][0] = MODE_INTRA;
    modemap[i][MBC+1] = MODE_INTRA;
  }


  fault = 0;
  gobheader_read = 0;
  //开始解码：
  //1.先确定 COD
  //2.根据COD的值决定是否解码块头信息Mcbpc,Cbpy,Mode,MV
  //3.根据MV,Mode,CBPY,进行DCT变换恢复上一块的block数据(MBA-1)
  //4.根据Mode,CBPY 从Huffman系数中得到(Run level,last)恢复当前块的DCT系数
  for (;;) 
  {
	if (trace)
	{
		outshow.Format("frame %d, MB %d\n",tr_framenum,MBA);
        AfxMessageBox(outshow);
		outshow.Empty();
	}
  resync:
    if (fault) 
	{
      outshow.Format("Warning: A Fault Condition Has Occurred - Resyncing!");
      AfxMessageBox(outshow);
	  m_gethdr.startcode();  // sync on new startcode 
      fault = 0;
    }
    //确定是否是头信息
    if (!(m_getbits.showbits(22)>>6)) //图像起始码
	{ // startcode 
      m_gethdr.startcode();  
      // 字节对齐,例如使用 PSTUF, GSTUF 或 ESTUF
      if (m_getbits.showbits(22) == (32|SEC)) 
	  { // EOS检测是否为序列结束标志 
	    if (!(syntax_arith_coding && MBA < MBAmax))
	        return;
      }
      else if ((m_getbits.showbits(22) == PSC<<5) ) //PSC=1
	  { // 新图像的开始
      	if (!(syntax_arith_coding && MBA < MBAmax)) 
	       return;
      }
      else 
	  {
	    if (!(syntax_arith_coding && MBA%MBC))
		{
	     if (syntax_arith_coding)
		 {   // SAC   
			 gob = (m_getbits.showbits(22) & 31); 
	         if (gob * mb_width != MBA) 
	           goto finish_gob;
		 }
    	 gob = m_gethdr.getheader() - 1;
	     if (gob > MBR) 
		 {
	       if (!quiet)
	         AfxMessageBox("GN out of range!");
	       return ;
		 } 
	     gfid = m_getbits.getbits(2);  //GFID 
 
	     quant = m_getbits.getbits(5);
	     if(trace)
		 {
	        outshow.Format("GQUANT: %d\n", quant);
			AfxMessageBox(outshow);
		 }
	     xpos = 0;
	     ypos = gob;
	     MBA = ypos * mb_width;
	  
	     newgob = 1;
	     gobheader_read = 1;
	     if (syntax_arith_coding) 
	        m_sacode.decoder_reset();// init SAD buffer
		} 
      } 
    }//end of(!(m_getbits.showbits(22)>>6)) 

  finish_gob:  
    //开始解MB
    if (!gobheader_read)
	{
      xpos = MBA%mb_width;  //当前宏块的位置
      ypos = MBA/mb_width;
     if (xpos == 0 && ypos > 0)
	      newgob = 0;
    }
    else 
      gobheader_read = 0;

    if (MBA>=MBAmax) 
      return; /* 所有宏块解码完毕 */
 /*************************************************************/
	// 1.确定COD
 /*************************************************************/
  read_cod:
    if (syntax_arith_coding) //Sac code 
	{ 
      if (pict_type == PCT_INTER)
	  {
	     COD_index = m_sacode.decode_a_symbol(cumf_COD);
	     COD = codtab[COD_index];
      }
      else
	     COD = 0;// I帧不使用COD, COD=0
	}
    else  //比特编码
	{
      if(pict_type == PCT_INTER) 
	      COD =m_getbits.showbits(1);
      else
       	  COD = 0; // I_picture-> not skipped 
	  
    } 
   /******************************************************/ 
	//2 根据COD确定是否要解码 其他块头信息(MCBPC CBPY MV)
   /*****************************************************/
    if(!COD) // COD == 0 --> not skipped
	{   
      if (syntax_arith_coding)
	  {
	    if (pict_type == PCT_INTER) 
		{
	        MCBPC_index = m_sacode.decode_a_symbol(cumf_MCBPC);
	        MCBPC = mcbpctab[MCBPC_index];
		}	 
	    else
		{
	        MCBPC_index = m_sacode.decode_a_symbol(cumf_MCBPC_intra);
	        MCBPC = mcbpc_intratab[MCBPC_index];
		} 
	    if (trace) 
		{
			outshow.Format("MCBPC Index: %d MCBPC: %d ",MCBPC_index, MCBPC);
			AfxMessageBox(outshow);
		}
       
	  }
      else //DCT_Huff
	  {   
	     if (pict_type == PCT_INTER)
	        m_getbits.flushbits(1); //flush COD bit 
      	 if (pict_type == PCT_INTRA) 
	        MCBPC = m_getvlc.getMCBPCintra();
      	 else
	        MCBPC = m_getvlc.getMCBPC();
      }
      if (fault)
		  goto resync;
      
      if (MCBPC == -1) // stuffing 
	     goto read_cod;   //read next COD without advancing MB count 
      else 
	  {      // normal MB data 
	    Mode = MCBPC & 7;
	   //To Get MODB and CBPB 
	    if (pb_frame) 
		{ 
	        CBPB = 0;
 	        if (syntax_arith_coding) 
			{ 
	            MODB_index = m_sacode.decode_a_symbol(cumf_MODB);
	            MODB = modb_tab[MODB_index];
			} 
	        else 
	            MODB = m_getvlc.getMODB();
	        if (MODB == PBMODE_CBPB_MVDB) 
			{
	          if (syntax_arith_coding) 
			  { 
	             for(i=0; i<4; i++)
				 { 
	      	        YCBPB_index = m_sacode.decode_a_symbol(cumf_YCBPB);
		            YCBPB = ycbpb_tab[YCBPB_index];
		            CBPB |= (YCBPB << (6-1-i));
				 } 
 
	             for(i=4; i<6; i++)
				 { 
		             UVCBPB_index = m_sacode.decode_a_symbol(cumf_UVCBPB);
		             UVCBPB = uvcbpb_tab[UVCBPB_index];
		             CBPB |= (UVCBPB << (6-1-i));
				 } 
			  } 
	          else
	             CBPB = m_getbits.getbits(6);
			  if (trace)
			  {
	              outshow.Format("CBPB = %d\n",CBPB);
		          AfxMessageBox(outshow);
			  } 
			}  
		}//end of(pb_frame)  
        //解码 CBPY
	    if (syntax_arith_coding)
		{ 
	      if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
		  { // Intra 
	         CBPY_index = m_sacode.decode_a_symbol(cumf_CBPY_intra);
	         CBPY = cbpy_intratab[CBPY_index];
		  } 
	      else
		  {
	         CBPY_index = m_sacode.decode_a_symbol(cumf_CBPY);
	         CBPY = cbpytab[CBPY_index];
		  } 
	      if (trace)
		  {
			  outshow.Format("CBPY Index: %d CBPY %d ",CBPY_index, CBPY);
              AfxMessageBox(outshow);
		  }
		} 
	    else 
	       CBPY = m_getvlc.getCBPY();
	    if (trace)
		{
	      outshow.Format("CBPY = %d",CBPY);
		  AfxMessageBox(outshow);
		}
 	    // 解码 Mode and CBP 
		if(Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
	         if (!syntax_arith_coding)	// Intra 
	           CBPY = CBPY^15;	 
    	CBP = (CBPY << 2) | (MCBPC >> 4);
      }
      
      if(Mode == MODE_INTER4V && !adv_pred_mode) 
      if(!quiet)
    	 AfxMessageBox("8x8 vectors not allowed in normal prediction mode\n");
          
      //确定是否要解码DQUANT
	  if(Mode == MODE_INTER_Q || Mode == MODE_INTRA_Q)
	  { // Read DQUANT if necessary
	     if (syntax_arith_coding) 
		 {
	         DQUANT_index = m_sacode.decode_a_symbol(cumf_DQUANT);
	         DQUANT = dquanttab[DQUANT_index] - 2; 
	         quant +=DQUANT;
	         if (trace)
			 {
				 outshow.Format("DQUANT Index: %d DQUANT %d \n",DQUANT_index, DQUANT);
		         AfxMessageBox(outshow);
			 }		 
		 } 
	     else 
		 { 
	          DQUANT = m_getbits.getbits(2);
	          quant += DQ_tab[DQUANT];
		 } 
	     if (trace) 
		 {
	         outshow.Format("DQUANT:%d  tab=%d ",DQUANT,DQ_tab[DQUANT]);
	         AfxMessageBox(outshow);
		 } 
         //调整量化因子到预定的范围内
	    if (quant > 31 || quant < 1)
		{ 
	       if (!quiet)
	          AfxMessageBox("Quantizer out of range: clipping!");
	        quant = mmax(1,mmin(31,quant));
	      // could set fault-flag and resync here 
		} 
	  }//end of (dquant decode) 
     //解码 motion vectors********************************** 
      if (Mode == MODE_INTER || Mode == MODE_INTER_Q 
	      ||Mode == MODE_INTER4V || pb_frame) 
	  {
	      if (Mode == MODE_INTER4V)
		  { 
		      startmv = 1; 
		      stopmv = 4;
		  } 
	      else
		  { 
		     startmv = 0;
		     stopmv = 0;
		  } 
          for (k = startmv; k <= stopmv; k++) //k表示每个宏块的MV个数
		  { 
	         if (syntax_arith_coding) 
			 { 
	             mvx_index = m_sacode.decode_a_symbol(cumf_MVD);
	             mvx = mvdtab[mvx_index];
	             mvy_index = m_sacode.decode_a_symbol(cumf_MVD);
	             mvy = mvdtab[mvy_index];
	             if (trace)
				 { 
					 outshow.Format("mvx_index: %d mvy_index: %d \n", mvy_index, mvx_index);
				     AfxMessageBox(outshow);
				 }
			 }  
	         else //Get Mv in HuffMan  
			 { 
	              mvx = m_getvlc.getTMNMV();
	              mvy = m_getvlc.getTMNMV();
			 } 

	         pmv0 = find_pmv(xpos,ypos,k,0);// 预测运动矢量.x
	         pmv1 = find_pmv(xpos,ypos,k,1);
	         mvx  = motion_decode(mvx, pmv0);// pmv0差分运动矢量.y
	         mvy  = motion_decode(mvy, pmv1);
	         if (trace) 
			 { 
	           outshow.Format("mvx: %d   mvy: %d", mvx,mvy);
	           AfxMessageBox(outshow);
			 } 
	         //检测运动矢量是否指向图像内部 
	         if (!mv_outside_frame)
			 { 
	           bsize = k ? 8 : 16;
	           offset = k ? (((k-1)&1)<<3) : 0;
	          //仅检测整象素(mvx/2)的MV
	          if((xpos<<4) + (mvx/2) + offset < 0 ||
		          (xpos<<4) + (mvx/2) + offset > (mb_width<<4) - bsize) 
			  { 
	            if (!quiet)
		            AfxMessageBox("mvx out of range: searching for sync!");
	             fault = 1;
			  } 
	          offset = k ? (((k-1)&2)<<2) : 0;
	          if((ypos<<4) + (mvy/2) + offset < 0 ||
		          (ypos<<4) + (mvy/2) + offset > (mb_height<<4) - bsize) 
			  {
	             if(!quiet)
		           AfxMessageBox("mvy out of range: searching for sync!");
	             fault = 1;
			  } 
			 }//end of(!mv_outside_frame) 
	         MV[0][k][ypos+1][xpos+1] = mvx;//半象素精度
	         MV[1][k][ypos+1][xpos+1] = mvy;
        
			 //基于运动矢量水印提取
			 if((dmmvextract||multiextract||RoFgFp_extract)&&(fabs(mvx)<=25)&&(fabs(mvx)>=5)&&(fabs(mvy)<=25)&&(fabs(mvy)>=5)) 
			 {
               if((*sumbit)<4096)
               { 
                 if(dmmvextract)
			       ExtractVMinMV(mvx, mvy, sumbit, trimark);
			     if(multiextract)
                   ExtractVMinMV(mvx, mvy, sumbit2, secondtrimark);
               }
			 }
			 
              if((RoFgFp_extract)&&(fabs(mvx)<=25)&&(fabs(mvx)>=2)&&(fabs(mvy)<=25)&&(fabs(mvy)>=2)) 
              {
                 MV_feature[0]=fabs(mvx%2);
                 MV_feature[1]=fabs(mvy%2);
				 P_Authen=true;
              }

		  }//end of ( normal MV Decode) 
	
		  // PB frame delta vectors 
 	      if (pb_frame) 
		  { 
	          if (MODB == PBMODE_MVDB || MODB == PBMODE_CBPB_MVDB) 
			  {  
	             if (syntax_arith_coding) 
				 { 
	                 mvdbx_index = m_sacode.decode_a_symbol(cumf_MVD);
	                 mvdbx = mvdtab[mvdbx_index];
	      	         mvdby_index = m_sacode.decode_a_symbol(cumf_MVD);
	                 mvdby = mvdtab[mvdby_index];
				 } 
	             else//Huff_Decode
				 { 
	                 mvdbx = m_getvlc.getTMNMV();
	                 mvdby = m_getvlc.getTMNMV();
				 } 
	             mvdbx = motion_decode(mvdbx, 0);
	             mvdby = motion_decode(mvdby, 0);
	              if (trace) 
				  { 
	                  outshow.Format("MVDB x: %d MVDB y: %d ", mvdbx, mvdby);
	                  AfxMessageBox(outshow);
				  } 
			  } 
	          else 
			  { 
	               mvdbx = 0; 
	               mvdby = 0;
			  } 
		  }//end of(pb_frame) 
      }//end of(MV Decode) 
     if(fault) 
		  goto resync;
    }
    else //COD=1 没有块头信息 进行下一宏块的编码
	{  // COD == 1 -->跳空 MB 
      if(MBA>=MBAmax)
	      return; // all macroblocks decoded 
      if (!syntax_arith_coding)
	      if (pict_type == PCT_INTER)
	          m_getbits.flushbits(1);
      Mode = MODE_INTER;
      //Reset CBP 
      CBP = CBPB = 0;
      //Reset motion vectors 
      MV[0][0][ypos+1][xpos+1] = 0;
      MV[1][0][ypos+1][xpos+1] = 0;
      mvdbx = 0;
      mvdby = 0;
    }

    // Store Mode
    modemap[ypos+1][xpos+1] = Mode;
    if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
      if (!pb_frame)
	       MV[0][0][ypos+1][xpos+1]=MV[1][0][ypos+1][xpos+1] = 0;
   /*************************************************************/
      //3.解码前一宏块的块数据信息（重建预测块，IDCT  Pre+Idct) 
   /************************************************************/
reconstruct_mb:
    // 前一宏块中的象素左上角的坐标 // one delayed because of OBMC 
    if (xpos > 0) 
	{
      bx = 16*(xpos-1);
      by = 16*ypos;
    }
    else 
	{
      bx = coded_picture_width-16;
      by = 16*(ypos-1);
    }
    if (MBA > 0)
	{
      Mode = modemap[by/16+1][bx/16+1];// bx,by表示宏块的位置×16
      // 前向运动补偿 for B-frame
      if (pb_frame)
	   m_recon.reconstruct(bx,by,0,pmvdbx,pmvdby);//P=0 B模式
      // 运动补偿 for P-frame oldframe经运动补偿后－>newframe
      if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode == MODE_INTER4V)
	    m_recon.reconstruct(bx,by,1,0,0);//P=1 预测模式
      //copy or add block data into P-picture
      
	  for (comp=0; comp<blk_cnt; comp++)
	  {
	     if((h263)/*&&(pict_type == PCT_INTRA)*/)//H.263解码,I+P
		 { // inverse DCT 
	       if(Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
		   { 
            m_idct.idct(ld->block[comp]);//ld->block中存放IDCT数据
	        addblock(comp,bx,by,0);//将ld->block中的数据放入图像中
		   } 
	       else  if(pCBP & (1<<(blk_cnt-1-comp))) //若263中去掉此段,和VQ质量一样 
		   { 
	        //No need to to do this for blocks with no coeffs 
            m_idct.idct(ld->block[comp]);
	        addblock(comp,bx,by,1);//将ld->block中的数据放入图像（newframe）中
		   }
	
		 }
	     
		 else if((RoFgFp_extract)) //DCT_VQ解码
		 {
		   if(Mode == MODE_INTRA || Mode == MODE_INTRA_Q) //I/intra
		   { 
            m_idct.idct(ld->block[comp]);//ld->block中存放IDCT数据
	        addblock(comp,bx,by,0);//将ld->block中的数据放入图像中
		   } 
     	   else  if(pCBP & (1<<(blk_cnt-1-comp))) //P/inter
		   { 
	        //No need to to do this for blocks with no coeffs 
            m_idct.idct(ld->block[comp]);
	        addblock(comp,bx,by,1);//将ld->block中的数据放入图像（newframe）中
		   }
		 }
		 
		 else if(vqdecode)// VQ 解码,I+P
		 {
		 
		   if(Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
		   { 
		    m_idct.idct(ld->block[comp]);//ld->block中存放IDCT数据
	        addblock(comp,bx,by,0);//将ld->block中的数据放入图像中
			// 此段程序在VQ+263中跳过!
		   } 

		   else if( (pCBP & (1<<(blk_cnt-1-comp))) ) 
		   addVQblock(comp,bx,by,1,index[comp],codeword);
		 }

		 else if(visualcode||basicextract||multiextract||robfrgextract||bridge_extract||rob_semifrg_extract)//visual VQ
		 {
             if(pfast&&(Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V))//INTER
			 {
			   addVQblock(comp,bx,by,1,index[comp],codeword+framenum/20*256);
			 }  
		     else
			 { 
                if(Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
				{ 
                m_idct.idct(ld->block[comp]);//ld->block中存放IDCT数据
	            addblock(comp,bx,by,0);//将ld->block中的数据放入图像中
				} 
	           else if(pCBP & (1<<(blk_cnt-1-comp))) //若263中去掉此段,和VQ质量一样 
			   { 
	           //No need to to do this for blocks with no coeffs 
               m_idct.idct(ld->block[comp]);
	           addblock(comp,bx,by,1);//将ld->block中的数据放入图像（newframe）中
			   }  
			 }
			 
		 }

	   }//解码宏块(normal) end of comp
   
   if (pb_frame) 
	  {  // add block data into B-picture
	      for (comp = 6; comp<blk_cnt+6; comp++)
		  { 
	        if((h263)||(pict_type == PCT_INTRA))
			 {if(!pCOD || adv_pred_mode)
	             reconblock_b(comp-6,bx,by,Mode,pmvdbx,pmvdby);
	         if( (pCBPB & (1<<(blk_cnt-1-comp%6))) )
			 { 
                 m_idct.idct(ld->block[comp]);
	             addblock(comp,bx,by,1);
			 } 
			}
			else if(vqdecode)
			 {
				if(!pCOD || adv_pred_mode)
	             reconblock_b(comp-6,bx,by,Mode,pmvdbx,pmvdby);
		        if(Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
				{ 
                   addVQblock(comp,bx,by,0,index[comp],codeword);
				} 
	            else if(Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V)// if( (pCBP & (1<<(blk_cnt-1-comp))) ) // if( (pCBPB & (1<<(blk_cnt-1-comp%6))) )
			    addVQblock(comp,bx,by,1,index[comp],codeword);
			}
		  } 
      }//end of(pb_frame) 
    } // end if (MBA > 0)
	/***************************************************************/
    //4.根据(COD,Mode,CBP,)从缓存中得到比特数并放入ld->block中
	/***************************************************************/
    if (!COD) // 解码6个宏块
	{
      Mode = modemap[ypos+1][xpos+1];
      // decode blocks 
      for (comp=0; comp<blk_cnt; comp++)
	  {
       clearblock(comp);//将ld->block中的数据首先清零
	
	   if((pict_type == PCT_INTRA)&&(!RoFgFp_extract)||h263)//DCT_VQ P帧解码;H.263 I+P;
	   { 
		 if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
		 { // Intra 
	         bp = ld->block[comp];//传递地址 结果在ld->block中
	         if(syntax_arith_coding)
			 {
	            INTRADC_index = m_sacode.decode_a_symbol(cumf_INTRADC);
	            bp[0] = intradctab[INTRADC_index];
	            if (trace)
				{
	                outshow.Format("INTRADC Index: %d INTRADC: %d \n", INTRADC_index, bp[0]);
				    AfxMessageBox(outshow);
				}
			 } 
	         else 
			  bp[0] = m_getbits.getbits(8);//直流分量
			 
	         if (trace) 
			 {
	             outshow.Format("DC[%d]is: %d",comp,(int)bp[0]);
	             AfxMessageBox(outshow);
			 } 

	         if (bp[0] == 128)
	            if(!quiet)
	               AfxMessageBox("Illegal DC-coeff: 1000000!");
	         if (bp[0] == 255)//标准中所定
	             bp[0] = 128;
			 
           if((dctparity)&&(pict_type == PCT_INTRA)) //密钥水印
		   {
				 sum+=bp[0];
				 qc[comp]=bp[0];
		   }
		   
		   if((robfrgextract||rob_semifrg_extract)&&(pict_type == PCT_INTRA)) //密钥水印
		   {
			   sum+=bp[0];
			   qc[comp]=bp[0];
		   }

	         bp[0] *= 8; // Iquant
			 //解码交流分量
	         if( (CBP & (1<<(blk_cnt-1-comp))) ) 
			 {   //从比特流中得到数据放入ld->block[]中
	             if(!syntax_arith_coding)
	                m_getblk.getblock(comp,0);
	             else 
	                m_getblk.get_sac_block(comp,0);
			 } 
			 
		 }//end of (Intra)
	     else 
		 { // Inter 没有直流系数 直接解码交流系数
	        if ( (CBP & (1<<(blk_cnt-1-comp))) ) 
			{    //从比特流中得到数据放入ld->block[]中
	           if(!syntax_arith_coding)
	              m_getblk.getblock(comp,1);
	           else
	              m_getblk.get_sac_block(comp,1);
			} 
		 }//end block decode 
	   }
	   
	   /****************  解码I帧的DCT系数的VQ索引  *********************/
	   else if(RoFgFp_extract) //DCT_VQ Decode
	   {  
		  if(pict_type==PCT_INTRA)
		  {     
			for(int n=0;n<3;n++)
			{
			 DCTVQ_index[comp][n]=m_getbits.getbits(index_bit[n]);
			}

			for(int l=0;l<4;l++) //DCT_VQ重构
			 DCTVQ_block[comp][l]=*(*(Low_codeword+framenum/40*2048+DCTVQ_index[comp][0])+l);
            for(l=4;l<16;l++)
			 DCTVQ_block[comp][l]=*(*(Mid_codeword+framenum/40*1024+DCTVQ_index[comp][1])+l-4);
			for(l=16;l<64;l++)
		     DCTVQ_block[comp][l]=*(*(Hig_codeword+framenum/40*1024+DCTVQ_index[comp][2])+l-16);
			
			for(l=0;l<8;l++) //zig-zag
			  for(int k=0;k<8;k++)
			 ld->block[comp][8*l+k]=DCTVQ_block[comp][zigzag[l][k]];
            
			if((comp==3)&&(frextract_no<64*6)&&(num_key%4==0))
			MB_DCTVQ_Extract(DCTVQ_index,ro_bit[I_num_toshow],fp_bit[I_num_toshow],&frextract_no);
		    
			if((comp==5)&&(*sumbit2<4096)&&(I_Au_frame<1024))
            {
              Authen_Ifrmae(DCTVQ_index,I_frg_bit,sumbit2);
              I_Au_frame+=4;
            }
		  }
		  else if (pict_type==PCT_INTER)
		  {
           if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
			{ // Intra 
	         bp = ld->block[comp];//传递地址 结果在ld->block中
	         bp[0] = m_getbits.getbits(8);//直流分量
	         if (trace) 
			 {
	             outshow.Format("DC[%d]is: %d",comp,(int)bp[0]);
	             AfxMessageBox(outshow);
			 } 
	         if (bp[0] == 128)
	            if(!quiet)
	               AfxMessageBox("Illegal DC-coeff: 1000000!");
	         if (bp[0] == 255)//标准中所定
	             bp[0] = 128;
	         bp[0] *= 8; // Iquant
			 //解码交流分量
	         if( (CBP & (1<<(blk_cnt-1-comp))) ) 
			 {   //从比特流中得到数据放入ld->block[]中
	             if(!syntax_arith_coding)
	                m_getblk.getblock(comp,0);
	             else 
	                m_getblk.get_sac_block(comp,0);
			 } 
		 }//end of (Intra) 
	     else 
		 { // Inter 没有直流系数 直接解码交流系数
	        if ((CBP & (1<<(blk_cnt-1-comp)))) 
			{    //从比特流中得到数据放入ld->block[]中
	           if(!syntax_arith_coding)
			   { 
				 if(P_Authen)
				 {
				    m_getblk.P_authen_getblk(comp,1,DCT_coeff);	  
				    if(comp==1||comp==2)
                    {
					   DCT_feature=((unsigned char)fabs(DCT_coeff[comp][pos]%2))^((unsigned char)fabs(DCT_coeff[comp][pos+1]%2))^((unsigned char)fabs(DCT_coeff[comp][pos+2]%2));
					   frg_bit[*sumbit]=DCT_feature^MV_feature[comp-1];  
					   (*sumbit)++;
                    }
				 }
				 else
				 m_getblk.getblock(comp,1);
			   }
               else
	              m_getblk.get_sac_block(comp,1);
			} 
            else if(comp<blk_cnt)
            {
              if(P_Authen&&(comp==1||comp==2))
              {
                 DCT_feature=0;
                 frg_bit[*sumbit]=DCT_feature^MV_feature[comp-1]; 
                 (*sumbit)++;
              }
            }
		 } 

		  }
	   }
       /*******************************************************************/
       else if(vqdecode)// VQ模式
		 { 
			if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V) //INTER
            index[comp]= m_getbits.getbits(10);
			else // INTRA
			{bp = ld->block[comp];//传递地址 结果在ld->block中
	         if(syntax_arith_coding)
			 {
	            INTRADC_index = m_sacode.decode_a_symbol(cumf_INTRADC);
	            bp[0] = intradctab[INTRADC_index];
	            if (trace)
				{
	                outshow.Format("INTRADC Index: %d INTRADC: %d \n", INTRADC_index, bp[0]);
				    AfxMessageBox(outshow);
				}
			 } 
	         else 
		        bp[0] = m_getbits.getbits(8);//直流分量
	         if (trace) 
			 {
	             outshow.Format("DC[%d]is: %d",comp,(int)bp[0]);
	             AfxMessageBox(outshow);
			 } 

	         if (bp[0] == 128)
	            if(!quiet)
	               AfxMessageBox("Illegal DC-coeff: 1000000!");
	         if (bp[0] == 255)//标准中所定
	             bp[0] = 128;
	         bp[0] *= 8; // Iquant
			 //解码交流分量
	         if( (CBP & (1<<(blk_cnt-1-comp))) ) 
			 {   //从比特流中得到数据放入ld->block[]中
	             if(!syntax_arith_coding)
	                m_getblk.getblock(comp,0);
	             else 
	                m_getblk.get_sac_block(comp,0);
			 } 
			}
 
		 }//end of VQ decode 
        else if (visualcode||basicextract||multiextract||robfrgextract||bridge_extract||rob_semifrg_extract)
		{
          //在此添加视频VQ的解码代码
	     fast=((MV[0][0][ypos+1][xpos+1]*MV[0][0][ypos+1][xpos+1]+MV[1][0][ypos+1][xpos+1]*MV[1][0][ypos+1][xpos+1])>=TH)?1:0;
   	     
		 if(fast&&(Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode==MODE_INTER4V))//INTER
		 {    
			if(visualcode)
		    index[comp]= m_getbits.getbits(8);
	        
			if(basicextract||multiextract||robfrgextract||bridge_extract||rob_semifrg_extract)// VQ水印提取代码
            { 
			  index[comp]= m_getbits.getbits(8);
			   
			  if(rob_semifrg_extract)
			  {
			    frg_bit[*sumbit]=*(CheckPosition(index[comp],8)+3);
				(*sumbit)++;
				tempnumber++;
			  }
			   
			   else if((basicextract||multiextract||robfrgextract||bridge_extract)&&(*sumbit<4096))
			   {
				 if(index[comp]<64)
					markbit[*sumbit]=0;
				 else if((index[comp]-64)%3==0)
					markbit[*sumbit]=1;
				 else if((index[comp]-64)%3==1)
					markbit[*sumbit]=2;
				 else if((index[comp]-64)%3==2)
					markbit[*sumbit]=3;
				 (*sumbit)++;
			   }

			}
        }  
	   else //正常解码块
	   { if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
		{// Intra 
	         bp = ld->block[comp];//传递地址 结果在ld->block中
	         if(syntax_arith_coding)
			 {
	            INTRADC_index = m_sacode.decode_a_symbol(cumf_INTRADC);
	            bp[0] = intradctab[INTRADC_index];
			 } 
	         else 
		        bp[0] = m_getbits.getbits(8);//直流分量
	         if (bp[0] == 128)
	            if(!quiet)
	               AfxMessageBox("Illegal DC-coeff: 1000000!");
	         if (bp[0] == 255)//标准中所定
	             bp[0] = 128;
	         bp[0] *= 8; // Iquant
			 //解码交流分量
	         if( (CBP & (1<<(blk_cnt-1-comp))) ) 
			 {   //从比特流中得到数据放入ld->block[]中
	             if(!syntax_arith_coding)
	                m_getblk.getblock(comp,0);
	             else 
	                m_getblk.get_sac_block(comp,0);
			 } 
		}//end of (Intra) 
	     else 
		 { // Inter 没有直流系数 直接解码交流系数
	        if ( (CBP & (1<<(blk_cnt-1-comp))) ) 
			{    //从比特流中得到数据放入ld->block[]中
	           if(!syntax_arith_coding)
	              m_getblk.getblock(comp,1);
	           else
	              m_getblk.get_sac_block(comp,1);
			} 
		 }
		}
		}
	     if (fault)
			 goto resync;
	  }//end of comp
      
	  if((dctparity||robfrgextract||rob_semifrg_extract)&&(pict_type == PCT_INTRA)&&(robcounter<2304)) //提取密钥水印
	  {  
		 if(dctparity)
		 KeyMark_Extract(keybit,key,qc,sum,sumbit);
	     if(robfrgextract||rob_semifrg_extract)
         KeyMark_Extract(keybit,key2,qc,sum,sumbit2);
         sum=0;
		 robcounter=robcounter+6;
	  }

      // Decode B blocks 
     if (pb_frame) 
	  {
	     for (comp=6; comp<blk_cnt+6; comp++)
		 { 
	        clearblock(comp);
	        if(h263) // 263模式
		   {  // Inter 没有直流系数 直接解码交流系数
			  if ( (CBP & (1<<(blk_cnt-1-comp))) ) 
			{    //从比特流中得到数据放入ld->block[]中
	           if(!syntax_arith_coding)
	              m_getblk.getblock(comp,1);
	           else
	              m_getblk.get_sac_block(comp,1);
			
			}
			}
		   else  if((vqdecode)&&(CBP & (1<<(blk_cnt-1-comp))))// VQ模式
            index[comp]= m_getbits.getbits(10);

           }
		 //end block decode 
	     if (fault)
			 goto resync;
     
      }//end of(pb_frame) 
    }//end of if(!COD)
    //进行下一MB解码
    MBA++;
    num_key++;
    pCBP  = CBP;
	pCBPB = CBPB; 
	pCOD  = COD;
    pmvdbx = mvdbx; 
	pmvdby = mvdby;
	pfast=fast;
	P_Authen=false;
    fflush(stdout);
    if (MBA >= MBAmax && !last_done)
	{
      COD = 1;
      xpos = 0;
      ypos++;
      last_done = 1;
      goto reconstruct_mb;
    }

  }//end of(for)
}

void CGetpic::clearblock(int comp)
{
  int *bp;
  int i;

  bp = (int *)ld->block[comp];

  for (i=0; i<8; i++)
  {
    bp[0] = bp[1] = bp[2] = bp[3] = 0;
    bp += 4;
  }

}

int CGetpic::motion_decode(int vec, int pmv)
{
  if (vec > 31) 
	  vec -= 64;
  vec += pmv;
  if (!long_vectors) 
  {
    if (vec > 31)
      vec -= 64;
    if (vec < -32)
      vec += 64;
  }
  else 
  {
    if (pmv < -31 && vec < -63)
      vec += 64;
    if (pmv > 32 && vec > 63)
      vec -= 64;
  }
  return vec;

}

int CGetpic::find_pmv(int x, int y, int block, int comp)
{
  int p1,p2,p3;
  int xin1,xin2,xin3;
  int yin1,yin2,yin3;
  int vec1,vec2,vec3;
  int l8,o8,or8;

  x++;y++;

  l8  = (modemap[y][x-1] == MODE_INTER4V ? 1 : 0);
  o8  = (modemap[y-1][x] == MODE_INTER4V ? 1 : 0);
  or8 = (modemap[y-1][x+1] == MODE_INTER4V ? 1 : 0);

  switch (block) 
  {
  case 0: 
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 1:
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 2:
    vec1 = 1            ; yin1 = y  ; xin1 = x;
    vec2 = (o8 ? 4 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 3:
    vec1 = (l8 ? 4 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  case 4:
    vec1 = 3            ; yin1 = y  ; xin1 = x;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  default:
    fprintf(stderr,"Illegal block number in find_pmv (getpic.cpp)\n");
    exit(1);
    break;
  }
  p1 = MV[comp][vec1][yin1][xin1];
  p2 = MV[comp][vec2][yin2][xin2];
  p3 = MV[comp][vec3][yin3][xin3];

  if (newgob && (block == 0 || block == 1 || block == 2))
    p2 = NO_VEC;

  if (p2 == NO_VEC) 
    p2 = p3 = p1; 

  return p1+p2+p3 - mmax(p1,mmax(p2,p3)) - mmin(p1,mmin(p2,p3));

}

void CGetpic::addblock(int comp, int bx, int by, int addflag)
{
  int cc,i,j, iincr, P = 1;
  unsigned char *rfp;
  short *bp;
  bp = ld->block[comp];
  if (comp >= 6) 
  {
    // This is a component for B-frame forward prediction 
    P = 0;
    addflag = 1;
    comp -= 6;
  }

  cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */
  if (cc==0)  // luminance 
  {
    // frame DCT coding 
    if (P)
      rfp = newframe[0]
	      + coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    else //P=0 B 帧
      rfp = bframe[0]
	      + coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    iincr = coded_picture_width;
  }
  else  // chrominance 
  {
    // scale coordinates 
    bx >>= 1;
    by >>= 1;
    //frame DCT coding 
    if (P)
      rfp = newframe[cc] + chrom_width*by + bx;
    else
      rfp = bframe[cc] + chrom_width*by + bx;
    iincr = chrom_width;
  }
  if (addflag) //帧间编码要加上预测
  {
    for (i=0; i<8; i++)
	{
      for(j=0;j<8;j++)
		  rfp[j]=clp[bp[j]+rfp[j]];
      bp += 8;
      rfp+= iincr;
    }
  }
  else//帧内编码无须加 
  {
    for (i=0; i<8; i++) 
	{
      for(j=0;j<8;j++)
	  {
		  rfp[j]=(unsigned char)bp[j];
		  if(rfp[j]>255)
		  rfp[j]=255;
		  else if(rfp[j]<0)
		  rfp[j]=0;
	  }
      bp += 8;
      rfp += iincr;
    }
  }//end of (addflag)
}

void CGetpic::reconblock_b(int comp, int bx, int by, int mode, int bdx, int bdy)
{
 int cc,i,j,k, ii;
  unsigned char *bfr, *ffr;
  int BMVx, BMVy;
  int xa,xb,ya,yb,x,y,xvec,yvec,mvx,mvy;
  int xint,xhalf,yint,yhalf,pel;

  x = bx/16+1;y=by/16+1;

  if (mode == MODE_INTER4V)
  {
    if (comp < 4) 
	{
      /* luma */
      mvx = MV[0][comp+1][y][x];
      mvy = MV[1][comp+1][y][x];
      BMVx = (bdx == 0 ? (trb-trd)* mvx/trd : trb * mvx/trd + bdx - mvx);
      BMVy = (bdy == 0 ? (trb-trd)* mvy/trd : trb * mvy/trd + bdy - mvy);
    }
    else 
	{
      /* chroma */
      xvec = yvec = 0;
      for (k = 1; k <= 4; k++) 
	  {
	     mvx = MV[0][k][y][x];
	     mvy = MV[1][k][y][x];
	     xvec += (bdx == 0 ? (trb-trd)* mvx/trd : trb * mvx/trd + bdx - mvx);
	     yvec += (bdy == 0 ? (trb-trd)* mvy/trd : trb * mvy/trd + bdy - mvy);
      }
      
      /* chroma rounding (table 16/H.263) */
      BMVx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
      BMVy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);
    }
  }
  else {
    if (comp < 4) 
	{
      /* luma */
      mvx = MV[0][0][y][x];
      mvy = MV[1][0][y][x];
      BMVx = (bdx == 0 ? (trb-trd)* mvx/trd : trb * mvx/trd + bdx - mvx);
      BMVy = (bdy == 0 ? (trb-trd)* mvy/trd : trb * mvy/trd + bdy - mvy);
    }
    else 
	{
      /* chroma */
      mvx = MV[0][0][y][x];
      mvy = MV[1][0][y][x];
      xvec = (bdx == 0 ? (trb-trd)* mvx/trd : trb * mvx/trd + bdx - mvx);
      yvec = (bdy == 0 ? (trb-trd)* mvy/trd : trb * mvy/trd + bdy - mvy);
      xvec *= 4;
      yvec *= 4;
      
      /* chroma rounding (table 16/H.263) */
      BMVx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
      BMVy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);
    }
  }

  cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */

  if (cc==0) 
  {
    /* luminance */
    find_bidir_limits(BMVx,&xa,&xb,comp&1);
    find_bidir_limits(BMVy,&ya,&yb,(comp&2)>>1);
    bfr = bframe[0] +
      coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    ffr = newframe[0] +
      coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    ii = coded_picture_width;
  }
  else 
  {
    /* chrominance */
    /* scale coordinates and vectors*/
    bx >>= 1;
    by >>= 1;

    find_bidir_chroma_limits(BMVx,&xa,&xb);
    find_bidir_chroma_limits(BMVy,&ya,&yb);

    bfr = bframe[cc]      + chrom_width*(by+((comp&2)<<2)) + bx + (comp&8);
    ffr = newframe[cc]    + chrom_width*(by+((comp&2)<<2)) + bx + (comp&8);
    ii = chrom_width;
  }

  xint = BMVx>>1;
  xhalf = BMVx - 2*xint;
  yint = BMVy>>1;
  yhalf = BMVy - 2*yint;

  ffr += xint + (yint+ya)*ii;
  bfr += ya*ii;
  
  if (!xhalf && !yhalf)
  {
    for (j = ya; j < yb; j++)
	{
      for (i = xa; i < xb; i++) 
	  {
	      pel = ffr[i];
	      bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else if (xhalf && !yhalf)
  {
    for (j = ya; j < yb; j++)
	{
      for (i = xa; i < xb; i++) 
	  {
	     pel = (ffr[i]+ffr[i+1]+1)>>1;
	     bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else if (!xhalf && yhalf)
  {
    for (j = ya; j < yb; j++)
	{
      for (i = xa; i < xb; i++)
	  {
	     pel = (ffr[i]+ffr[ii+i]+1)>>1;
	     bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else 
  { /* if (xhalf && yhalf) */
    for (j = ya; j < yb; j++) 
	{
      for (i = xa; i < xb; i++) 
	  {
	      pel = ((unsigned int)(ffr[i]+ffr[i+1]+ffr[ii+i]+ffr[ii+i+1]+2))>>2;
	      bfr[i] = (pel + bfr[i])>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  return;

}

void CGetpic::find_bidir_limits(int vec, int *start, int *stop, int nhv)
{
  /* limits taken from C loop in section G5 in H.263 */
  *start = mmax(0,(-vec+1)/2 - nhv*8);
  *stop = mmin(7,15-(vec+1)/2 - nhv*8);

  (*stop)++; /* I use < and not <= in the loop */

}

void CGetpic::find_bidir_chroma_limits(int vec, int *start, int *stop)
{

  /* limits taken from C loop in section G5 in H.263 */
  *start = mmax(0,(-vec+1)/2);
  *stop = mmin(7,7-(vec+1)/2);

  (*stop)++; /* I use < and not <= in the loop */
  return;

}

void CGetpic::make_edge_image(unsigned char *src, unsigned char *dst, int width, int height, int edge)
{
  int i,j;
  unsigned char *p1,*p2,*p3,*p4;
  unsigned char *o1,*o2,*o3,*o4;

  /* center image */
  p1 = dst;
  o1 = src;
  for (j = 0; j < height;j++)
  {
    for (i = 0; i < width; i++) 
	{
      *(p1 + i) = *(o1 + i);
    }
    p1 += width + (edge<<1);
    o1 += width;
  }

  /* left and right edges */
  p1 = dst-1;
  o1 = src;
  for (j = 0; j < height;j++) 
  {
    for (i = 0; i < edge; i++) 
	{
      *(p1 - i) = *o1;
      *(p1 + width + i + 1) = *(o1 + width - 1);
    }
    p1 += width + (edge<<1);
    o1 += width;
  }    
    
  /* top and bottom edges */
  p1 = dst;
  p2 = dst + (width + (edge<<1))*(height-1);
  o1 = src;
  o2 = src + width*(height-1);
  for (j = 0; j < edge;j++) 
  {
    p1 = p1 - (width + (edge<<1));
    p2 = p2 + (width + (edge<<1));
    for (i = 0; i < width; i++) 
	{
      *(p1 + i) = *(o1 + i);
      *(p2 + i) = *(o2 + i);
    }
  }    

  /* corners */
  p1 = dst - (width+(edge<<1)) - 1;
  p2 = p1 + width + 1;
  p3 = dst + (width+(edge<<1))*(height)-1;
  p4 = p3 + width + 1;

  o1 = src;
  o2 = o1 + width - 1;
  o3 = src + width*(height-1);
  o4 = o3 + width - 1;
  for (j = 0; j < edge; j++) {
    for (i = 0; i < edge; i++) {
      *(p1 - i) = *o1;
      *(p2 + i) = *o2;
      *(p3 - i) = *o3;
      *(p4 + i) = *o4; 
    }
    p1 = p1 - (width + (edge<<1));
    p2 = p2 - (width + (edge<<1));
    p3 = p3 + width + (edge<<1);
    p4 = p4 + width + (edge<<1);
  }

}
// only used for displayed interpolated frames, not reconstructed ones 
void CGetpic::interpolate_image(unsigned char *in, unsigned char *out, int width, int height)
{
  int x,xx,y,w2;
  unsigned char *pp,*ii;
  w2 = 2*width;
  // Horizontally 
  pp = out;
  ii = in;
  for (y = 0; y < height-1; y++) 
  {
    for (x = 0,xx=0; x < width-1; x++,xx+=2) 
	{
      *(pp + xx) = *(ii + x);
      *(pp + xx+1) = ((unsigned int)(*(ii + x)  + *(ii + x + 1)))>>1;
      *(pp + w2 + xx) = ((unsigned int)(*(ii + x) + *(ii + x + width)))>>1;
      *(pp + w2 + xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 
			              *(ii + x + width) + *(ii + x + width + 1)))>>2;
      
    }
    *(pp + w2 - 2) = *(ii + width - 1);
    *(pp + w2 - 1) = *(ii + width - 1);
    *(pp + w2 + w2 - 2) = *(ii + width + width - 1);
    *(pp + w2 + w2 - 1) = *(ii + width + width - 1);
    pp += w2<<1;
    ii += width;
  }

  // last lines 
  for (x = 0,xx=0; x < width-1; x++,xx+=2) 
  {
    *(pp+ xx) = *(ii + x);    
    *(pp+ xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 1))>>1;
    *(pp+ w2+ xx) = *(ii + x);    
    *(pp+ w2+ xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 1))>>1;
  }
  
  // bottom right corner pels 
  *(pp + (width<<1) - 2) = *(ii + width -1);
  *(pp + (width<<1) - 1) = *(ii + width -1);
  *(pp + (width<<2) - 2) = *(ii + width -1);
  *(pp + (width<<2) - 1) = *(ii + width -1);

  return;

}

void CGetpic::deVQ(int *codeword[], int index, short *block)
{
 for(int i=0;i<8;i++)
 {for(int j=0;j<8;j++)
 {*(block+i*8+j)=codeword[index][8*i+j];
 }
 }	
}

void CGetpic::addVQblock(int comp, int bx, int by, int addflag,int ind,int *codeword[MAXBOOKSIZE])
{
  int cc,i,j, iincr, P = 1;
  unsigned char *rfp;
  short *bp;

  bp = ld->block[comp];
  if (comp >= 6) 
  {
    // This is a component for B-frame forward prediction 
    P = 0;
    addflag = 1;
    comp -= 6;
  }

  cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */
  if (cc==0)  // luminance 
  {
    // frame DCT coding 
    if (P)
      rfp = newframe[0]
	      + coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    else //P=0 B 帧
      rfp = bframe[0]
	      + coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    iincr = coded_picture_width;
  }
  else  // chrominance 
  {
    // scale coordinates 
    bx >>= 1;
    by >>= 1;
    //frame DCT coding 
    if (P)
      rfp = newframe[cc] + chrom_width*by + bx;
    else
      rfp = bframe[cc] + chrom_width*by + bx;
    iincr = chrom_width;
  }
  if (addflag) //帧间编码要加上预测
  {
    for (i=0; i<8; i++)
	{
      for(j=0;j<8;j++)
	  { 
		  if((*(*(codeword+ind)+8*i+j)+rfp[j])>255)
		  rfp[j]=255;
		  else if((*(*(codeword+ind)+8*i+j)+rfp[j])<0)
		  rfp[j]=0;
		  else
		  rfp[j]=(*(*(codeword+ind)+8*i+j)+rfp[j]);
	  }       
        rfp+= iincr;
	}
 
  }
 else
  {
  for (i=0; i<8; i++) 
	{
      for(j=0;j<8;j++)
	    rfp[j]=clp[codeword[ind][8*i+j]];
      bp += 8;
      rfp += iincr;
    }
  }
}

int CGetpic::GetAngle(int x, int y)
{
	int a;
	if((x>0)&&(y==0))//0
		a=0;
	if((x>0)&&(y>0))//45
		a=1;
	if((x==0)&&(y>0))//90
		a=2;
	if((x<0)&&(y>0))//135
		a=3;
	if((x<0)&&(y==0))//180
		a=4;
	if((x<0)&&(y<0))//225
		a=5;
	if((x==0)&&(y<0))//270
		a=6;
	if((x>0)&&(y<0))//315
		a=7;
	return a;
}

void CGetpic::ExtractVMinMV(int mvx, int mvy, int *sumbit, unsigned char mark[1500])
{
              int mx,my,x_half,y_half;
              int one_angle,half_angle;
			   mx=mvx/2;
			   my=mvy/2;
			  
               //得到半象素的符号及数值
			   if(mvx%2==0)
			   x_half=0;
			   if(mvy%2==0)
			   y_half=0;
              
			   if(mvx%2!=0)
			   {
				 if(mx%2==0)
				 x_half=-1;
				 else
			     x_half=1;
			   }

               if(mvy%2!=0)
			   {
				 if(my%2==0)
				 y_half=-1;
				 else
				 y_half=1;
			   }
			  //得到特征值
			  one_angle=GetAngle(mvx,mvy);
			  half_angle=GetAngle(x_half,y_half);
			  //提取水印位
              if(one_angle==half_angle)
			  mark[*sumbit]=0;
			  if(fabs(one_angle-half_angle)==4)
			  mark[*sumbit]=7;
			  
			  if((one_angle-half_angle==2)||(one_angle-half_angle==-6))
			  mark[*sumbit]=3;
			  if((half_angle-one_angle==2)||(half_angle-one_angle==-6))
			  mark[*sumbit]=4;
			  
			  if((one_angle-half_angle==3)||(one_angle-half_angle==-5))
			  mark[*sumbit]=5;
			  if((half_angle-one_angle==3)||(half_angle-one_angle==-5))
			  mark[*sumbit]=6;
			  
			  if((one_angle-half_angle==1)||(one_angle-half_angle==-7))
			  mark[*sumbit]=1;
			  if((half_angle-one_angle==1)||(half_angle-one_angle==-7))
			  mark[*sumbit]=2;
              
              (*sumbit)++;
			   
}

//提取密钥水印
void CGetpic::KeyMark_Extract(unsigned char *filebit, unsigned char *waterbit, int coeff[], int sum, int *times)
{
	int aver=sum/6;
	int feature[6];
	for(int i=0;i<6;i++)
	{
		feature[i]=(coeff[i]>=aver)?1:0;
		*(waterbit+*(times)+i)=*(filebit+*(times)+i)^feature[i];
	}
	*(times)+=6;
}


unsigned char * CGetpic::CheckPosition(int index,int bitnumber)
{
   int i;
   unsigned char *index_bit;
   index_bit=new unsigned char [bitnumber];
   for(i=0;i<bitnumber;i++)
   {
	 *(index_bit+i)=index%2;
	 index=index>>1;
   }
  return index_bit;
}


void CGetpic::MB_DCTVQ_Extract(int index[6][3], unsigned char *robust, unsigned char *fngprt, int *ex_time)
{
  for(int comp=0;comp<4;comp++)
   {
	 if(index[comp][0]%2048<1024)
	  fngprt[*ex_time+comp]=0;
	 else
	  fngprt[*ex_time+comp]=1;
	 if(index[comp][1]%1024<512)
	  robust[*ex_time+comp]=0;
	 else
	  robust[*ex_time+comp]=1;
   }
   (*ex_time)+=4;
}

void CGetpic::Authen_Ifrmae(int index[6][3],unsigned char *Ifrg_ex,int *number)
{
   int c=0;
   int pos=8;
   for(c=0;c<6;c++) 
    if((c<2)||(c>3))
     {
      Ifrg_ex[*number]=*(CheckPosition(index[c][0],11)+pos)^*(CheckPosition(index[c][1],10)+pos)^*(CheckPosition(index[c][2],10)+pos);//认证位
      (*number)++;
    }
   
}
