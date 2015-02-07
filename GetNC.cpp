#include"stdafx.h"
#include"Global.h"

NCDATA GetNC(int width, int height, unsigned char *Org, unsigned char *Wm)
{ 
    NCDATA nc_data;
    int i,m_total;
    m_total=width*height;
    nc_data.Corrct=0;
    nc_data.Wrong=0;
    nc_data.NC=0.0;
    ///////////////  Calculate NC /////////////////////////
    for(i=0;i<width*height;i++)
	 {
        if(Org[i]==Wm[i])
		 nc_data.Corrct++;
		else
		 nc_data.Wrong++;
	 }
     nc_data.NC=(double)(nc_data.Corrct)/m_total;
     return nc_data;
}

BOOL  OpenWm(int m_width,int m_height,unsigned char *orgbit, CString &name)
{
    CFile imp;
	int filesize;
	LPBITMAPFILEHEADER bitfile;
    LPBITMAPINFOHEADER bitinfo;
	char temp[50];
	CFileDialog openkey(TRUE,NULL,NULL,OFN_HIDEREADONLY,"Bitmap(*.bmp)|*.bmp|");
	openkey.m_ofn.lStructSize=88;
	unsigned char t;
    unsigned char * ImageBytes;
    int i,j,k,round,Bytesoff;
    int width,height;
    CWnd win;
    if(openkey.DoModal()==IDOK)
	{
	    name=openkey.GetFileName();
		if(imp.Open(name,CFile::modeRead)==NULL)//Can not open watermar iage
		{
            sprintf(temp,"Can not open the selcted bitmap!");
			win.MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
			return false;
		}
		else//Read pixels
		{   filesize=imp.GetLength();
		    ImageBytes=(unsigned char *)GlobalAlloc(GMEM_FIXED,sizeof(unsigned char)*(filesize));
	     	imp.Read(ImageBytes,filesize);	
		    bitfile=(LPBITMAPFILEHEADER)ImageBytes;
		    bitinfo=(LPBITMAPINFOHEADER)(ImageBytes+sizeof(BITMAPFILEHEADER));
			int m,l;
			m=sizeof(BITMAPFILEHEADER);
			l=sizeof(BITMAPINFOHEADER);
			BITMAPINFO* image=(BITMAPINFO *)ImageBytes+sizeof(BITMAPFILEHEADER);
            width=bitinfo->biWidth;
			height=bitinfo->biHeight;
            round=(width%64)?(width/64+1)*64:width;
			Bytesoff=bitfile->bfOffBits;
			int p=sizeof(BITMAPINFO);
			if(width!=m_width||height!=m_height)
			{
				sprintf(temp,"You have open an image with improper size!");
				win.MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				return false;
			}
			
			if(bitfile->bfType!=0x4d42)
			{ 
				sprintf(temp,"You have open a file that it not supported by this software!");
				win.MessageBox(temp,"Open bitmap",MB_ICONSTOP|MB_OK);
				filesize=0;
				return false;
			} 
			
			if(bitinfo->biBitCount!=1)
			{ 
				filesize=0;
				return false; 
			} 
			
			//If the image is compressed or not
			if(bitinfo->biCompression!=0)
			{ 
				filesize=0;
				return false;
			}
	   
           for(i=0;i<height;i++)
           {
			for(j=0;j<width/8;j++)
			{
				t=*(ImageBytes+Bytesoff+i*round/8+j);
				for(k=0;k<8;k++)
				{   
					orgbit[i*width+j*8+7-k]=t%2;
					t=t>>1;
				}
			}
       	}

	}//end of else
	}//end of if open
   return true;
}