//lable LBG.cpp
#include"stdafx.h"
#include"global.h"
#include <math.h>

short int DCTVQLLBGCodebookDesign(unsigned short int dimension, unsigned long int blocks, unsigned short int booksize,
								  int *TrainVector[MAXIMAGESIZE], int *CodeVector[MAXBOOKSIZE])

{
   unsigned long int i,l,t,vectornum;
   unsigned long int *posi;
   unsigned short int j,k,p;
   double d0,d1,d,min,par;
   double *dd,*dc;
   unsigned long int pppp;
   unsigned short int count,n;
   int *index;
   unsigned  int *number;
   unsigned char *label;
   double e=0.00001;
 
   void *hg;  
   hg=GlobalAlloc(GMEM_FIXED,sizeof(unsigned long int)*blocks);
   if(hg==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   posi=(unsigned long int *)hg;
    
   void *hdd,*hdc,*hnum,*hlab,*hindex;
   hdd=GlobalAlloc(GMEM_FIXED,sizeof(double)*dimension);
   if(hg==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   dd=(double*)hdd;
   
   hdc=GlobalAlloc(GMEM_FIXED,sizeof(double)*booksize);
   if(hdc==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   dc=(double*)hdc;

   hnum=GlobalAlloc(GMEM_FIXED,sizeof(unsigned  int)*booksize);
   if(hnum==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   number=(unsigned  int*)hnum;

   hlab=GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*booksize);
   if(hlab==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   label=(unsigned char*)hlab;

   hindex=GlobalAlloc(GMEM_FIXED,sizeof(int)*blocks);
   if(hindex==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   index=(int*)hindex;
   
   booksize=booksize/2;
   
   par=(double)(blocks-1)/0x7fff;
   srand((unsigned)time(NULL));
   for(i=0;i<booksize;i++)
   {
     l=(int)((double)rand()*par);
 	 memcpy(CodeVector[i],TrainVector[l],dimension*sizeof(int));
   }
   d0=9.99e+60;   
   
   //生成大小为(booksize/2)的码书
   for(;;)
   {  
     d1=0.0;
 	 for(j=0;j<booksize;j++)
		 number[j]=0;
	 for(i=0;i<blocks;i++)
	 {
	    index[i]=0;
		min=0.0;
        for(k=0;k<dimension;k++)
            min+=pow((double)TrainVector[i][k]-(double)CodeVector[0][k],2);
		for(j=1;j<booksize;j++)
		{
		  d=0.0;
		  for(k=0;k<dimension;k++)
		  {			  
		    d+=pow((double)TrainVector[i][k]-(double)CodeVector[j][k],2);
		    if(d>=min)break;
		  }
		  if(d<min)
		  {
		    min=d;
		    index[i]=j;
		  }
		}
	    number[index[i]]++;
		d1+=min;
	 }
	 if(fabs(d0-d1)/d1<=e)break;
	 d0=d1;

     //计算质心
	 for(j=0;j<booksize;j++)
	 {
	    count=0;
		if(number[j]!=0)
		{
		  for(k=0;k<dimension;k++)
			  dd[k]=0.0;
		  for(l=0;l<blocks;l++)
		  {
		    if(count<number[j])
			{
				if(index[l]==j)
				{
			       for(k=0;k<dimension;k++)
			           dd[k]+=TrainVector[l][k];
				   count++;
				}
			}
			else
				break;
		  }
		  for(k=0;k<dimension;k++)
		      CodeVector[j][k]=(int)(dd[k]/(double)number[j]);
		}
		else
		{
		  l=(int)((double)rand()*par);
          memcpy(CodeVector[j],TrainVector[l],dimension*sizeof(int));
		}
	 }
   }   
   
   //码字扩展
   for(p=0;p<booksize;p++)
   {
     //Looking for the training vectors in cluster p
	 pppp=0;
     count=0;
	 for(i=0;i<blocks;i++)
	 {
		if(count<number[p])
		{
		   if(index[i]==p)
		   {
			   count++;
		       posi[pppp]=i;//对码书中的第p个码字，记录第pppp(0<pppp<number[p])个
			                //对应的TrainVectors在训练矢量集中的索引
		       pppp++;
		   }
		}
		else
			break;
	 }// end of i

     d0=9.99e+60;  
	 vectornum=number[p]; //胞腔中训练矢量的个数
	 
     label[0]=(unsigned char)(1+pow(-1,(double)n))/2; //实现了0/1标记的伪随机化
	 label[1]=1-label[0];
     if(vectornum==0)
     {
       for(k=0;k<dimension;k++)
       {
        CodeVector[p+booksize][k]=CodeVector[posi[0]][k]*0.8;
       }
     }
     if(vectornum==1)
     {
       for(k=0;k<dimension;k++)
       {
        CodeVector[p+booksize*label[0]][k]=TrainVector[posi[0]][k];
        CodeVector[p+booksize*label[1]][k]=TrainVector[posi[0]][k]*0.8;
       }
     }
     if(vectornum==2)
     {
       memcpy(CodeVector[p+booksize*label[0]],TrainVector[posi[0]],dimension*sizeof(int));
       memcpy(CodeVector[p+booksize*label[1]],TrainVector[posi[1]],dimension*sizeof(int));
     }
     if(vectornum>2)
     {
      par=(double)(vectornum-1)/0x7fff;
      srand((unsigned)time(NULL));
     
	  t=(int)((double)rand()*par);
      memcpy(CodeVector[p],TrainVector[posi[t]],dimension*sizeof(int));
	  
      do
      {
        l=(int)((double)rand()*par);
        memcpy(CodeVector[p+booksize],TrainVector[posi[l]],dimension*sizeof(int));
      }while(l==t);

	 for(n=0;;n++)
	 {   
	   for(k=0;k<2;k++)
	   {
		 number[k*booksize+p]=0;
		 dc[k*booksize+p]=0;
	   }
	   d1=0.0;
	   //Partioning stage
	   for(i=0;i<vectornum;i++)
	   {
	     min=0.0;
	   	 d=0.0;
         for(k=0;k<dimension;k++)
		   min+=pow((double)TrainVector[posi[i]][k]-(double)CodeVector[p][k],2);
		 for(k=0;k<dimension;k++)
		   d+=pow((double)TrainVector[posi[i]][k]-(double)CodeVector[p+booksize][k],2);
		 
		 index[posi[i]]=p;  
		 if(d<min)
		 {
		   min=d;
		   index[posi[i]]=p+booksize;
		 }
		 number[index[posi[i]]]++;
         dc[index[posi[i]]]+=min;
		 d1+=min;
	   }// end of (i)
 
	   label[0]=(unsigned char)(1+pow(-1,(double)n))/2; //实现了0／1标记的伪随机化
	   label[1]=1-label[0];
	   
	   if(fabs(d0-d1)/d1<=e)
	      break;
	   d0=d1;

	   for(j=0;j<2;j++)
	   { 
	     for(k=0;k<dimension;k++)
		     dd[k]=0.0;
		 for(l=0;l<vectornum;l++)
		 {
		     if(index[posi[l]]==p+booksize*j)
			 {
		        for(k=0;k<dimension;k++)
			        dd[k]+=(double)TrainVector[posi[l]][k];
			 }
		 }
		 for(k=0;k<dimension;k++)
		     CodeVector[p+booksize*label[j]][k]=(int)(dd[k]/number[p+booksize*j]);
	   }//end of (j)
	 }//end of (n)
     } //end of (vectnum>2)
   }// end of (p)
   d1=0.0;
   for(j=0;j<booksize*2;j++)
   	 d1+=dc[j];

   GlobalFree(hg);
   GlobalFree(hdd);
   GlobalFree(hdc);
   GlobalFree(hnum);
   GlobalFree(hindex);
   
   return 1;
}

short int DCTVQLLBGCodebookDesign_II(unsigned short int dimension, unsigned long int blocks, unsigned short int booksize,
								  int *TrainVector[MAXIMAGESIZE], int *CodeVector[MAXBOOKSIZE])
{
unsigned long int i,l,vectornum;
   unsigned long int *posi;
   unsigned short int j,k,p;
   double d0,d1,d,min,par;
   double *dd,*dc;
   unsigned long int pppp;
   unsigned short int count,n;
   int *index;
   unsigned  int *number;
   unsigned char *label;
   double e=0.0001;

   void *hg;  
   hg=GlobalAlloc(GMEM_FIXED,sizeof(unsigned long int)*blocks);
   if(hg==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   posi=(unsigned long int *)hg;
    
   void *hdd,*hdc,*hnum,*hlab,*hindex;
   hdd=GlobalAlloc(GMEM_FIXED,sizeof(double)*dimension);
   if(hg==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   dd=(double*)hdd;
   
   hdc=GlobalAlloc(GMEM_FIXED,sizeof(double)*booksize);
   if(hdc==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   dc=(double*)hdc;

   hnum=GlobalAlloc(GMEM_FIXED,sizeof(unsigned  int)*booksize);
   if(hnum==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   number=(unsigned  int*)hnum;

   hlab=GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*booksize);
   if(hlab==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   label=(unsigned char*)hlab;

   hindex=GlobalAlloc(GMEM_FIXED,sizeof(int)*blocks);
   if(hindex==NULL)
   {
	   MessageBox(NULL,"Not enough memory!","System information",MB_ICONSTOP|MB_OK);
       return 0;
   }
   index=(int*)hindex;
   
   booksize=booksize/2;
   
  
   par=(double)(blocks-1)/0x7fff;
   srand((unsigned)time(NULL));
   for(i=0;i<booksize;i++)
   {
     l=(int)((double)rand()*par);
 	 memcpy(CodeVector[i],TrainVector[l],dimension*sizeof(float));
   }
   d0=9.99e+60;   
   
   //生成大小为(booksize/2)的码书
   for(;;)
   {  
     d1=0.0;
 	 for(j=0;j<booksize;j++)
		 number[j]=0;
	 for(i=0;i<blocks;i++)
	 {
	    index[i]=0;
		min=0.0;
        for(k=0;k<dimension;k++)
            min+=pow((double)TrainVector[i][k]-(double)CodeVector[0][k],2);
		for(j=1;j<booksize;j++)
		{
		  d=0.0;
		  for(k=0;k<dimension;k++)
		  {			  
		    d+=pow((double)TrainVector[i][k]-(double)CodeVector[j][k],2);
		    if(d>=min)break;
		  }
		  if(d<min)
		  {
		    min=d;
		    index[i]=j;
		  }
		}
	    number[index[i]]++;
		d1+=min;
	 }
	 if((d0-d1)/d1<=e)break;
	 d0=d1;
     //计算质心
	 for(j=0;j<booksize;j++)
	 {
	    count=0;
		if(number[j]!=0)
		{
		  for(k=0;k<dimension;k++)
			  dd[k]=0.0;
		  for(l=0;l<blocks;l++)
		  {
		    if(count<number[j])
			{
				if(index[l]==j)
				{
			       for(k=0;k<dimension;k++)
			           dd[k]+=(double)TrainVector[l][k];
				   count++;
				}
			}
			else
				break;
		  }
		  for(k=0;k<dimension;k++)
		      CodeVector[j][k]=(float)(dd[k]/(double)number[j]);

		}
		else
		{
		  l=(int)((double)rand()*par);
          memcpy(CodeVector[j],TrainVector[l],dimension*sizeof(float));
		}
	 }
   }   
   
   //码字扩展
   for(p=0;p<booksize;p++)
   {
     //Looking for the training vectors in cluster p
	 pppp=0;
     count=0;
	 for(i=0;i<blocks;i++)
	 {
		if(count<number[p])
		{
		   if(index[i]==p)
		   {
			   count++;
		       posi[pppp]=i;//对码书中的第p个码字，记录第pppp(0<pppp<number[p])个
			                //对应的TrainVectors在训练矢量集中的索引
		       pppp++;
		   }
		}
		else
			break;
	 }

     d0=9.99e+60;  
	 vectornum=number[p]; //胞腔中训练矢量的个数
	 
	 par=(double)(vectornum-1)/0x7fff;
     srand((unsigned)time(NULL));
     
	 l=(int)((double)rand()*par);
     memcpy(CodeVector[p],TrainVector[posi[l]],dimension*sizeof(float));
	 
	 l=(int)((double)rand()*par);
 	 memcpy(CodeVector[p+booksize],TrainVector[posi[l]],dimension*sizeof(float));

	 for(n=0;;n++)
	 {   
	   for(k=0;k<2;k++)
	   {
		 number[k*booksize+p]=0;
		 dc[k*booksize+p]=0;
	   }
	   d1=0.0;
	   //Partioning stage
	   for(i=0;i<vectornum;i++)
	   {
	     min=0.0;
	   	 d=0.0;
         for(k=0;k<dimension;k++)
		   min+=pow((double)TrainVector[posi[i]][k]-(double)CodeVector[p][k],2);
		 for(k=0;k<dimension;k++)
		   d+=pow((double)TrainVector[posi[i]][k]-(double)CodeVector[p+booksize][k],2);
		 
		 index[posi[i]]=p;  
		 if(d<min)
		 {
		   min=d;
		   index[posi[i]]=p+booksize;
		 }
		 number[index[posi[i]]]++;
         dc[index[posi[i]]]+=min;
		 d1+=min;
	   }
 
	   label[0]=(unsigned char)(1+pow(-1,(double)n))/2; //实现了0/1标记的伪随机化
	   label[1]=1-label[0];
	   
	   if((d0-d1)/d1<=e)
	      break;
	   d0=d1;

	   for(j=0;j<2;j++)
	   { 
	     for(k=0;k<dimension;k++)
		     dd[k]=0.0;
		 for(l=0;l<vectornum;l++)
		 {
		     if(index[posi[l]]==p+booksize*j)
			 {
		        for(k=0;k<dimension;k++)
			        dd[k]+=(double)TrainVector[posi[l]][k];
			 }
		 }
		 for(k=0;k<dimension;k++)
		     CodeVector[p+booksize*label[j]][k]=(float)(dd[k]/(double)number[p+booksize*j]);
	   }

	 }
   }
   d1=0.0;
   for(j=0;j<booksize*2;j++)
   	 d1+=dc[j];

   GlobalFree(hg);
   GlobalFree(hdd);
   GlobalFree(hdc);
   GlobalFree(hnum);
   GlobalFree(hindex);
   
   return 1;

}