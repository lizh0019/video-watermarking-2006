#include "stdafx.h"
#include "Global.h"
#include <MATH.H>
#include "Getpic.h"

int VQ_Encode(int *vect,int **codeword,int Low,int High, int dimension)
{
      int index=Low; 
      double dis=0,mind;
      int k,n;
      index=Low; //ตอฦต
	  mind=VectorDistance(dimension,vect,codeword[index]);
	  for(n=Low;n<High;n++)
	  {
	    dis=0.0;
		for(k=0;k<dimension;k++)
        {			  
	       dis+=pow((double)(vect[k]-codeword[n][k]),2);
		   if(dis>=mind)
           break;
		 }
        if(dis<mind)
		{
		   mind=dis;
		   index=n;
		}
      }
     return index;
}

int VQ_Constrain(int *vect,int **codeword,int Low,int High, int dimension,int pos, unsigned char wm_bit)
{
      int index; 
      double dis=0,mind;
      CGetpic pic;
      int k,n;
      int index_len;
      index_len=log(High-Low)/log(2);
      index=(int)(pow(2,pos)*wm_bit); //ตอฦต
	  mind=VectorDistance(dimension,vect,codeword[index]);
	  for(n=Low;n<High;n++)
	  {		  
	    if(*(pic.CheckPosition(n,index_len)+pos)==wm_bit)
         {
            dis=0.0;
            for(k=0;k<dimension;k++)
            {			  
	          dis+=pow((double)(vect[k]-codeword[n][k]),2);
		      if(dis>=mind)
              break;
            }
            if(dis<mind)
            {
		      mind=dis;
		      index=n;
            }
         }
      }
     return index;
}