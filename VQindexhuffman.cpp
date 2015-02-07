#include"stdafx.h"

int VQ_index_Huff(int index_num,int word_num,int *VQindex)
{
   double   *proba;     //码字出现的概率
   CString  *m_strCode; // Huffman 码表
   double   *dTempt;
   int      *turn,      //节点
		    dT;
   int i=0,j=0,k=0;
   int codelength=0;
   
   m_strCode=new CString[word_num];
   dTempt=new double[word_num];
   proba=new double[word_num];
   turn=new int[word_num];
   
   //统计码字索引出现的概率
   
   //初始化概率值
   for(i=0;i<word_num;i++)
     proba[i]=0;
   
   for(i=0;i<index_num;i++)
     proba[VQindex[i]]++;

   //赋初始值
   for(int n=0;n<word_num;n++)
   {
	dTempt[n]=proba[n];
    turn[n]=n;
   }

   //冒泡法排序
  for(j=0;j<word_num-1;j++)
  {
	 for(i=0;i<word_num-1-j;i++)
	 {
		 if(dTempt[i]>dTempt[i+1])
		 {
			 dT=dTempt[i];
             dTempt[i]=dTempt[i+1];
             dTempt[i+1]=dT;
            for(int k=0;k<word_num;k++)
            {
             if(turn[k]==i)
             turn[k]=i+1;
             else if(turn[k]==i+1)
             turn[k]=i;
            }
         } 
     }
   }
  for(i=0;i<word_num-1;i++)//如果概率为0,则跳出
  {
   if(dTempt[i]>0)
	   break;
  }
   
  for(;i<word_num-1;i++)
  {
	for(k=0;k<word_num+1;k++)
	{
     if(turn[k]==i)
     m_strCode[k]="1"+m_strCode[k];
     else if(turn[k]==i+1)
     m_strCode[k]="0"+m_strCode[k];
	}

   dTempt[i+1]+=dTempt[i];
   for(k=0;k<word_num;k++)
   {
	if(turn[k]==i)
    turn[k]=i+1;
   }
  
  for(j=i+1;j<word_num-1;j++)
  {
    if(dTempt[j]>dTempt[j+1])
	{
      dT=dTempt[j];
      dTempt[j]=dTempt[j+1];
      dTempt[j+1]=dT;
      for( k=0;k<word_num;k++)
	  {
       if(turn[k]==j)
        turn[k]=j+1;
       else if(turn[k]==j+1)
        turn[k]=j;
	  }
  }	  
  else
   break;
  }
 }
  
     //计算平均码字长度
   for(i=1;i<index_num;i++)
   {
     int l=m_strCode[VQindex[i]].GetLength(); 
     codelength=codelength+l;
   }

   delete [] m_strCode;
   delete [] dTempt;
   delete [] proba;
   delete [] turn;

  return codelength;
}