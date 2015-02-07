// AttackMethod.cpp: implementation of the AttackMethod class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "视频编解码器.h"
#include "AttackMethod.h"
#include "Global.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include"math.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AttackMethod::AttackMethod()
{

}

AttackMethod::~AttackMethod()
{

}

void AttackMethod::ContrastEn(PictImage *org,PictImage *enhance,int width,int height)
{
   int m=0,n=0,i=0,j=0,nTemp;
   int *count;
   count=new int [256];
   unsigned char *byMap;
   byMap=new unsigned char [256];
   for(m=0;m<256;m++)
     count[m]=0;
   for(m=0;m<height;m++)
    for(n=0;n<width;n++)
      count[*(org->lum+m*width+n)]++;
    for(i=0;i<256;i++)
    {
      nTemp=0;
      for(j=0;j<=i;j++)
      nTemp+=count[j];
      byMap[i]=(unsigned char)(nTemp*255/(width*height));
    }
    for(m=0;m<height;m++)
     for(n=0;n<width;n++)
    *(enhance->lum+m*width+n)=byMap[*(org->lum+m*width+n)];
    delete [] count;
    delete [] byMap;
}

void AttackMethod::Blur(PictImage *org, PictImage *blur, int width, int height, int nTotLen, int nTotTime)
{
    long iColumn;
	long jRow;
	int n,m;
    double temp,temp_cb,temp_cr;
	int nTime;
    int B;
    for(jRow=0;jRow<height;jRow++)
	{
		for(iColumn=0;iColumn<width;iColumn++)
		{
            *(blur->lum+width*jRow+iColumn)=*(org->lum+width*jRow+iColumn);
		}
	}
    
    for(jRow=0;jRow<height/2;jRow++)
	{
		for(iColumn=0;iColumn<width/2;iColumn++)
		{
            *(blur->Cb+width*jRow/2+iColumn)=*(org->Cb+width*jRow/2+iColumn);
            *(blur->Cr+width*jRow/2+iColumn)=*(org->Cr+width*jRow/2+iColumn);
		}
	}
    
    for(jRow=0;jRow<height;jRow++)
	{
		for(iColumn=1;iColumn<width-2;iColumn++)
		{
			temp=0;
            for(n=-1;n<=2;n++)
            temp+=*(org->lum+width*jRow+iColumn+n);
		    temp=temp/4;
			if(temp<0)
			  temp=0;
			if(temp>255)
		      temp=255;
		    *(blur->lum+width*jRow+iColumn)=(unsigned char)temp;
		}
	}
    
    for(jRow=0;jRow<height/2;jRow++)
	{
		for(iColumn=1;iColumn<width/2-2;iColumn++)
		{
			temp_cb=0;
            temp_cr=0;
            for(n=-1;n<=2;n++)
            {
              temp_cb+=*(org->Cb+width/2*jRow+iColumn+n);
              temp_cr+=*(org->Cr+width/2*jRow+iColumn+n);
            }
		    temp_cb=temp_cb/4;
            temp_cr=temp_cr/4;
			if(temp_cb<0)
			  temp_cb=0;
			if(temp_cb>255)
		      temp_cb=255;
            if(temp_cr<0)
			  temp_cr=0;
			if(temp_cr>255)
		      temp_cr=255;
		    *(blur->Cb+width/2*jRow+iColumn)=(unsigned char)temp_cb;
            *(blur->Cr+width/2*jRow+iColumn)=(unsigned char)temp_cr;
		}
	}
}

void AttackMethod::Crop(PictImage *org, PictImage *cropped, int width, int height)
{
  int col,row;
  
  for(row=0;row<height;row++)
    for(col=0;col<width;col++)
    {
     *(cropped->lum+row*width+col)=*(org->lum+row*width+col);
     if(row<32||row>height-33)
     *(cropped->lum+row*width+col)=0;
    }
    
  for(row=0;row<height/2;row++)
    for(col=0;col<width/2;col++)
    {
     *(cropped->Cb+row*width/2+col)=*(org->Cb+row*width/2+col);
     *(cropped->Cr+row*width/2+col)=*(org->Cr+row*width/2+col);
     if(row<16||row>height/2-17)
     {
       *(cropped->Cb+row*width/2+col)=0;
       *(cropped->Cr+row*width/2+col)=0;
     }
    }
}

void AttackMethod::Filter(PictImage *org, PictImage *filter, int pels, int lines)
{
    int m,n;
    for(m=0;m<lines;m++)
		{
			for(n=0;n<pels;n++)
			{
			   int min=0,max=0,media=0,c=0,r=0;
			   unsigned char a[10],t=0;

              if((m==0)||(m==lines-1)||(n==0)||(n==pels-1))
                *(filter->lum+m*pels+n)=*(org->lum+m*pels+n);
              
			  else{
			  for(r=0;r<3;r++)
			   for(c=0;c<3;c++)
			   {
				 a[3*r+c+1]=*(org->lum+(m+r-1)*pels+n+c-1);
			   }
              for(int j=1;j<=8;j++)
                 for(int i=1;i<=9-j;i++)
				 {
					 if(a[i]>a[i+1])
					 {
						t=a[i];a[i]=a[i+1];a[i+1]=t;
					 }
				 }
			  *(filter->lum+m*pels+n)=a[5];
			  }
			}
		}

		for(m=0;m<lines/2;m++)
		{
			for(n=0;n<pels/2;n++)
			{
				int min=0,max=0,media=0,c=0,r=0;
				unsigned char a[10],t=0;
				
				if((m==0)||(m==lines/2-1)||(n==0)||(n==pels/2-1))
					*(filter->Cb+m*pels/2+n)=*(org->Cb+m*pels/2+n);
				
				else{
					for(r=0;r<3;r++)
						for(c=0;c<3;c++)
						{
							a[3*r+c+1]=*(org->Cb+(m+r-1)*pels/2+n+c-1);
						}
						for(int j=1;j<=8;j++)
							for(int i=1;i<=9-j;i++)
							{
								if(a[i]>a[i+1])
								{
									t=a[i];a[i]=a[i+1];a[i+1]=t;
								}
							}
							*(filter->Cb+m*pels/2+n)=a[5];
				}
			}
		}
	    
		for( m=0;m<lines/2;m++)
		{
			for(int n=0;n<pels/2;n++)
			{
				int min=0,max=0,media=0,c=0,r=0;
				unsigned char a[10],t=0;
				
				if((m==0)||(m==lines/2-1)||(n==0)||(n==pels/2-1))
					*(filter->Cr+m*pels/2+n)=*(org->Cr+m*pels/2+n);
				
				else{
					for(r=0;r<3;r++)
						for(c=0;c<3;c++)
						{
							a[3*r+c+1]=*(org->Cr+(m+r-1)*pels/2+n+c-1);
						}
						for(int j=1;j<=8;j++)
							for(int i=1;i<=9-j;i++)
							{
								if(a[i]>a[i+1])
								{
									t=a[i];a[i]=a[i+1];a[i+1]=t;
								}
							}
							*(filter->Cr+m*pels/2+n)=a[5];
				}
			}
		}

}

void AttackMethod::Noise(PictImage *org, PictImage *noise, int pels,int lines, double percent)
{
        double par;
		int bitnum,bitnum2,k;
        int m(0),n(0),i(0);
        if(percent<0)
        percent=0;
        if(percent>100)
        percent=100;
		bitnum=(int)pels*lines*percent/100;
		for(m=0;m<lines;m++)
		{
			for(n=0;n<pels;n++)
			{
			  *(noise->lum+m*pels+n)=*(org->lum+m*pels+n);
			}
		}
		
		for( m=0;m<lines/2;m++)
		{
			for(int n=0;n<pels/2;n++)
			{
				*(noise->Cb+m*pels/2+n)=*(org->Cb+m*pels/2+n);
				*(noise->Cr+m*pels/2+n)=*(org->Cr+m*pels/2+n);
			}
		}

		par=(double)(pels*lines-1)/(double)0x7fff;
	    //srand((unsigned)time(NULL));

		for(i=0;i<bitnum;i++)
		{
			int col,row;
	        //产生随即数 
			k=(int)((double)rand()*par);
			row=k/pels;
			col=k%pels;
            if(k%2==0)
            *(noise->lum+row*pels+col)=255;
            else
            *(noise->lum+row*pels+col)=0;
		}

}

void AttackMethod::Rotate(PictImage *org, PictImage *rot, int pels, int lines, double angle)
{
    int m,n;
    int newn, newm;
    double temp;
	 for(m=0;m<lines;m++)
		{
			for( n=0;n<pels;n++)
			{
				*(rot->lum+m*pels+n)=0;
				if((m<lines/2)&&(n<pels/2))
                {
                 *(rot->Cb+m*pels/2+n)=0;
                 *(rot->Cr+m*pels/2+n)=0;
                }
			}
		}

	
      for( m=0;m<lines;m++)
		{
			for( n=0;n<pels;n++)
			{
				temp=sqrt(m*m+n*n);
				newm=(int)(sin(asin((double)m/temp)+3.1415926*angle/180)*temp);
				newn=(int)(cos(acos((double)n/temp)+3.1415926*angle/180)*temp);
               if((newm>=0)&&(newm<lines)&&(newn>0)&&(newn<pels))
			   {
				 *(rot->lum+newm*pels+newn)=*(org->lum+m*pels+n);
                 /**(rot->Cb+newm*pels/4+newn/2)=*(org->Cb+m*pels/4+n/2);
                 *(rot->Cr+newm*pels/4+newn/2)=*(org->Cr+m*pels/4+n/2);*/
			   }
			}
		}
        
       for( m=0;m<lines/2;m++)
		{
			for( n=0;n<pels/2;n++)
			{
				temp=sqrt(m*m+n*n);
				newm=(int)(sin(asin((double)m/temp)+3.1415926*angle/180)*temp);
				newn=(int)(cos(acos((double)n/temp)+3.1415926*angle/180)*temp);
               if((newm>=0)&&(newm<lines/2)&&(newn>=0)&&(newn<pels/2))
			   {
                 *(rot->Cb+newm*pels/2+newn)=*(org->Cb+m*pels/2+n);
                 *(rot->Cr+newm*pels/2+newn)=*(org->Cr+m*pels/2+n);
			   }
			}
		}
}
