#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gif.h"

typedef FILE * GIF_HANDLE;

static __inline GIF_HANDLE gifOpen(const char *fileName)
{
	GIF_HANDLE gifFp;

	gifFp = fopen(fileName, "rb");
	if(!gifFp) return 0;

	return gifFp;
}

static __inline int gifClose(GIF_HANDLE gifFp)
{
	if(!gifFp) return -1;
	fclose(gifFp);
	return 0;
}

static __inline int gifRead(GIF_HANDLE gifFp, u8 *buf, u32 len)
{
	int ret;
	if(!gifFp) return -1;
	ret = fread(buf,1,len,gifFp);
	return ret;
}

static __inline int gifWrite(GIF_HANDLE gifFp, const u8 *buf, u32 len)
{
	int ret;
	if(!gifFp) return -1;
	ret = fwrite(buf,1,len,gifFp);
	return ret;
}

static int gifCalcFileSize(GIF_HANDLE gifFp)
{
	int p1,p2,size;

	//计算文件大小
	fseek(gifFp, 0, SEEK_SET);
	p1=ftell(gifFp);
	fseek(gifFp,0,SEEK_END);
	p2=ftell(gifFp);
	fseek(gifFp,0,SEEK_SET);
	size = p2-p1;

	return (size);
}

static int gifCheckHeader(u8 *headBuf, u8 headLen)
{
	if(strncmp((char*)headBuf, "GIF89a", GIF_HEADER_BYTES)==0 || \
		strncmp((char*)headBuf, "GIF79a", GIF_HEADER_BYTES)==0) {
		
		return 1;
	}

	return 0;
}

int gifDecode(const unsigned char *fileName)
{
	GIF_HANDLE gifFp;
	gifImage *gif;
	int ret, i;
	int ncolors;
	u8 intro,label;

	if(!fileName) return -1;
	gif = (gifImage*)malloc(sizeof(gifImage));
	if(!gif) {
		printf("gif image malloc fail.\n");
		return -1;
	}

	gifFp = gifOpen(fileName);
	if(gifFp<=0) {
		printf("gif file open fail.(%s)\n", fileName);
	}

	gif->size = gifCalcFileSize(gifFp);
	printf("gif size: %d\n", gif->size);

	ret = gifRead(gifFp, (u8*)&gif->header, GIF_HEADER_BYTES);
	if(ret != GIF_HEADER_BYTES) {
		printf("gif read header data fail.(%d)\n", ret);
		goto ErrOut;
	}

	ret = gifCheckHeader((u8*)&gif->header, GIF_HEADER_BYTES);
	if(!ret) goto ErrOut;

	//Logical Screen Descriptor.
	ret = gifRead(gifFp, (u8*)&gif->lsd, GIF_LSD_BYTES);
	
	printf("\nLogical Screen Descriptor:\n");
	printf("width: %d\n", gif->lsd.width);
	printf("height: %d\n", gif->lsd.height);
	printf("flag: %d ## m(%d) cr(%d) s(%d) pixel(%d)\n", gif->lsd.flag, \
		_LSD_M(gif->lsd.flag), \
		_LSD_CR(gif->lsd.flag), \
		_LSD_S(gif->lsd.flag), \
		_LSD_PIXEL(gif->lsd.flag));
	printf("bgIndex: %d\n", gif->lsd.bgIndex);
	printf("pixelAspect: %d\n", gif->lsd.pixelAspect);
	
	//Global Color Table
	if(_LSD_M(gif->lsd.flag)){
		ncolors = 2<<(_LSD_PIXEL(gif->lsd.flag));	//颜色索引数=2^(pixel+1)
		gif->gctSize = ncolors * 3;
		gif->gct = (u8*)malloc(gif->gctSize);
		if(!gif->gct) {
			printf(" gif Global Color Table malloc fail.\n");
			goto ErrOut;
		}

		ret = gifRead(gifFp, gif->gct, gif->gctSize);
		if(ret != gif->gctSize) {
			printf("get gobal color table fail.\n");
			goto ErrOut;
		}

		printf("\nGlobal Color Table:\n");
		printf("ColorTableSize: %d\n", gif->gctSize);
		for(i=0; i<ncolors; i++){
			printf("# %d #r:%d g:%d b:%d\n", i, gif->gct[i*3],gif->gct[i*3+1],gif->gct[i*3+2]);
		}
	}

	do{		
		//Image Descriptor	
		ret = gifRead(gifFp, &intro, 1);
		if(ret != 1) {
			printf("intro read fail.\n");
			goto ErrOut;
		}
		printf("intro:0x%x >> %d\n", intro);

		switch(intro){
		case GIF_INTRO_IMAGE:
			ret = gifRead(gifFp, (u8*)&gif->isd, GIF_ISD_BYTES);
			if(ret!=GIF_ISD_BYTES){
				printf("read isd fail.\n");
				goto ErrOut;
			}
			printf("\nImage Descriptor:\n");			
			printf("xoffset: %d\n", gif->isd.xoffset);
			printf("yoffset: %d\n", gif->isd.yoffset);
			printf("width: %d\n", gif->isd.width);
			printf("height: %d\n", gif->isd.height);
			printf("flag: 0x%x m(%d) i(%d) s(%d) r(%d) pixel(%d)\n", gif->isd.flag,\
				_ISD_M(gif->isd.flag),\
				_ISD_I(gif->isd.flag),\
				_ISD_S(gif->isd.flag),\
				_ISD_R(gif->isd.flag),\
				_ISD_PIXEL(gif->isd.flag));
			ret = 0;
			break;
		
		case GIF_INTRO_EXTENSION:
			ret = gifRead(gifFp, (u8*)&label, 1);
			if(ret!=1){
				printf("read extension label fail.\n");
				goto ErrOut;
			}
			printf("label:0x%x\n",label);

			break;

		case GIF_INTRO_TRAILER:
			ret = 0;
			break;
		}

	}while(ret!=0);

ErrOut:
	
	if(gif && gif->gct) free(gif->gct);
	if(gif) free(gif);
	gifClose(gifFp);
	return ret;
}
