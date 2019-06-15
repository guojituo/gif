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

void gifReadExtension(GIF_HANDLE gifFp)
{
	int i,ret;
	u8 blockSize;
	u8 lzwSize;
	u8 label;
	u8 tmpBuf[255];

	ret = gifRead(gifFp, (u8*)&label, 1);
	if(ret!=1){
		printf("read extension label fail.\n");
		return ;
	}
	
	switch(label){
	case _PLAIN_TEXT_LABEL:
		printf("## Plain Text Label(0x%x) ##\n", label);

		break;

	case _GRAPHIC_CONTROL_LABEL:
		printf("## Graphic Control Label(0x%x) ##\n", label);
		ret = gifRead(gifFp, (u8*)&blockSize, 1);
		if(ret!=1){
			printf("gif read fail.\n");
			return ;
		}
		printf("Block Size:%d\n", blockSize);

		ret = gifRead(gifFp, tmpBuf, blockSize);
		if(ret != blockSize) {
			printf("gif read fail.\n");
			return ;
		}

		printf("DisposalMethod:0x%x\n", (tmpBuf[0]>>2)&0xf);
		printf("I:0x%x\n", (tmpBuf[0]>>1)&0x01);
		printf("T:0x%x\n", (tmpBuf[0]>>0)&0x01);
		printf("DelayTime:%d\n", (tmpBuf[2]<<8)|tmpBuf[1]);
		printf("Transparent Color Index:%d\n", tmpBuf[3]);

		//Block Terminator 0x0
		ret = gifRead(gifFp, (u8*)tmpBuf, 1);
		if(ret!=1){
			printf("gif read fail.\n");
			return ;
		}
		printf("Block Terminator:0x%x\n", tmpBuf[0]);
		break;

	case _COMMENT_LABEL:
		printf("## Comment Label(0x%x) ##\n", label);
		break;

	case _APPLICATION_EXTENSION_LABEL:
		printf("## Application Extension Label(0x%x) ##\n", label);
		ret = gifRead(gifFp, (u8*)&blockSize, 1);
		if(ret!=1){
			printf("gif read fail.\n");
			return ;
		}
		printf("Block Size:%d\n", blockSize);
		
		ret = gifRead(gifFp, tmpBuf, blockSize);
		if(ret != blockSize) {
			printf("gif read fail.\n");
			return ;
		}
		printf("Application Identifier:");
		for(i=0;i<8;i++){
			printf("%c",tmpBuf[i]);
		}
		printf("\n");

		printf("Application Authentication Code:");
		for(i=0;i<3;i++){
			printf("%c", tmpBuf[i+8]);
		}
		printf("\n");

		ret = gifRead(gifFp, (u8*)&lzwSize, 1);
		if(ret!=1){
			printf("gif read fail.\n");
			return ;
		}
		printf("lzwSize:%d\n", lzwSize);
		ret = gifRead(gifFp, (u8*)tmpBuf, lzwSize);
		if(ret!=lzwSize){
			printf("gif read fail.\n");
			return; 
		}
		for(i=0;i<lzwSize;i++){
			printf("0x%02x ", tmpBuf[i]);
			if((i+1)%8==0) printf("  ");
			if((i+1)%16==0) printf("\n");
		}
		printf("\n");

		//Block Terminator 0x0
		ret = gifRead(gifFp, (u8*)tmpBuf, 1);
		if(ret!=1){
			printf("gif read fail.\n");
			return ;
		}
		printf("Block Terminator:0x%x\n", tmpBuf[0]);
		break;

	default:
		printf("not support label(0x%x)\n", label);
		break;
	}

	return ;
}

int gifDecode(const unsigned char *fileName)
{
	GIF_HANDLE gifFp;
	gifImage *gif;
	int ret, i;
	int ncolors;
	u8 intro;
	u8 blockSize;
	u8 tmpBuf[512];

	if(!fileName) return -1;

	gif = (gifImage*)malloc(sizeof(gifImage));
	if(!gif) {
		printf("gif image malloc fail.\n");
		return -1;
	}
	memset(gif, 0x0, sizeof(gifImage));

	gifFp = gifOpen(fileName);
	if(gifFp<=0) {
		printf("gif file open fail.(%s)\n", fileName);
		free(gif);
		return -1;
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
			printf("get global color table fail.\n");
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
		printf("\n");
		printf(">> Descriptor:\n");
		printf("intro:0x%x\n", intro);

		switch(intro){
		case GIF_INTRO_IMAGE:
			ret = gifRead(gifFp, (u8*)&gif->isd, GIF_ISD_BYTES);
			if(ret!=GIF_ISD_BYTES){
				printf("read isd fail.\n");
				goto ErrOut;
			}
			printf("Image Descriptor:\n");			
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
			
			//Local Color Table
			if(_ISD_M(gif->isd.flag)){
				ncolors = 2<<(_ISD_PIXEL(gif->isd.flag));	//颜色索引数=2^(pixel+1)
				gif->lctSize = ncolors * 3;
				gif->lct = (u8*)malloc(gif->lctSize);
				if(!gif->lct) {
					printf(" gif Local Color Table malloc fail.\n");
					goto ErrOut;
				}

				ret = gifRead(gifFp, gif->lct, gif->lctSize);
				if(ret != gif->lctSize) {
					printf("get local color table fail.\n");
					goto ErrOut;
				}

				printf("\nLocal Color Table:\n");
				printf("ColorTableSize: %d\n", gif->lctSize);
				for(i=0; i<ncolors; i++){
					printf("# %d #r:%d g:%d b:%d\n", i, gif->lct[i*3],gif->lct[i*3+1],gif->lct[i*3+2]);
				}
			}

			ret = gifRead(gifFp, (u8*)&blockSize, 1);
			if(ret!=1){
				printf("gif read fail.\n");
				goto ErrOut;

			}
			
			printf("blockSize:%d\n", blockSize);
			ret = gifRead(gifFp, (u8*)tmpBuf, blockSize);
			if(ret!=blockSize){
				printf("gif read fail.\n");
				goto ErrOut;
			}
			for(i=0;i<blockSize;i++){
				printf("0x%02x ", tmpBuf[i]);
				if((i+1)%8==0) printf("  ");
				if((i+1)%16==0) printf("\n");
			}
			printf("\n");

			break;
		
		case GIF_INTRO_EXTENSION:
			printf("Extension Descriptor  ");
			gifReadExtension(gifFp);
			break;

		case GIF_INTRO_TRAILER:
			printf("Trailer.\n");
			ret = 0;
			break;
		}

	}while(ret!=0);

ErrOut:
	
	if(gif && gif->gct) free(gif->gct);
	if(gif && gif->lct) free(gif->lct);
	if(gif) free(gif);
	gifClose(gifFp);
	return ret;
}
