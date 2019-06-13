#include <stdio.h>

int gifTest(const char *filePath)
{
	FILE *fp;
	int p1,p2,size;
	unsigned char tmpBuf[512];

	if(!filePath) return -1;

	fp = fopen(filePath, "rb");
	if(!fp) {
		printf(" file open fail(%s).\n", filePath);
		return -1;
	}

	//计算文件大小
	fseek(fp, 0, SEEK_SET);
	p1=ftell(fp);
	fseek(fp,0,SEEK_END);
	p2=ftell(fp);
	fseek(fp,0,SEEK_SET);
	size = p2-p1;
	printf("file size: %d\n", size);

	if(!size || size<6){
		printf("文件不完整.\n");
		goto ErrOut;
	}

	
ErrOut:
	fclose(fp);

	return 0;
}


