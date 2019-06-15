#ifndef __GIF_H__
#define __GIF_H__

#define GIF_DATA_TYPE

#if defined(GIF_DATA_TYPE)
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
#endif


#define GIF_HEADER_BYTES	(6)	//Signature and Version
#define GIF_LSD_BYTES		(7)	//LogicalScreenDescriptor
#define GIF_ISD_BYTES		(9)	//ImageDescriptor


enum {
	GIF_ERR_NONE = 0,
	GIF_ERR_FORMAT
};

#define GIF_INTRO_TRAILER	0x3B	//GIF Trailer
#define GIF_INTRO_EXTENSION	0x21	//'!'	Extension Introducer
#define GIF_INTRO_IMAGE		0x2C	//','	Image Descriptor Introducer 

// extension label
#define _PLAIN_TEXT_LABEL				(0x01)	//Plain Text Label
#define _GRAPHIC_CONTROL_LABEL			(0xf9)	//Graphic Control Label
#define _COMMENT_LABEL					(0xfe)	//Comment Label
#define _APPLICATION_EXTENSION_LABEL	(0xff)	//Application Extension Label


#if defined(_WIN32)
#pragma pack(1)
#endif

//逻辑屏幕描述块
typedef struct{
	u16 width;		//GIF宽度
	u16 height;		//GIF高度
#define _LSD_M(f)	  ((f>>7)&0x01)
#define _LSD_CR(f)	  ((f>>4)&0x07)
#define _LSD_S(f)	  ((f>>3)&0x01)
#define _LSD_PIXEL(f) ((f)&0x07)
	u8 flag;		//标识符[1:3:1:3] m[7]全局颜色表标志; cr[6:4]颜色深度; s[3]分类标志; pixel[2:0]全局颜色表大小
	u8 bgIndex;		//背景色在全局颜色表中的索引(仅当存在全局颜色表时有效)
	u8 pixelAspect;	//像素宽高比
}gifLogicalScreenDescriptor;

//图像描述块
typedef struct {
	u16 xoffset;	//X方向偏移
	u16 yoffset;	//Y方向偏移
	u16 width;		//宽度
	u16 height;		//高度
#define _ISD_M(f)		((f>>7)&0x01)
#define _ISD_I(f)		((f>>6)&0x01)
#define _ISD_S(f)		((f>>5)&0x01)
#define _ISD_R(f)		((f>>3)&0x03)
#define _ISD_PIXEL(f)	((f)&0x07)
	u8 flag;		//标识符[1:1:1:2:3] m[7]局部颜色列表标志 i[6]交织标志 s[5]分类标志 r[4:3]保留，必须初始化为0 pixel[2:0]局部颜色列表大小
}gifImageScreenDescriptor;

//图形控制扩展(Graphic Control Extension)
typedef struct {
	u8 blockSize;
	u8 flag;	//
	u8 delay;	//Delay Time
	u8 tcIndex;	//Transparent Color Index
}gifGraphicControlExtension;

//注释扩展(Comment Extension)
typedef struct {
	u8 blockSize;
}gifCommentExtension;

//图形文本扩展(Plain Text Extension)
typedef struct {
	u8 blockSize;	//固定值为 12
	u16 tgLeftPos;	//Text Glid Left Position
	u16 tgTopPos;	//Text Glid Top Position
	u16 tgWidth;	//Text Glid Width
	u16 tgHeight;	//Text Glid　Height
	u8 ccWidth;		//Character Cell Width
	u8 ccHeight;	//Character Cell Height
	u8 tfci;		//Text Foreground Color Index
	u8 tbci;		//Text Blackground Color Index
	u8 *data;
}gifPlainTextExtension;

//应用程序扩展(Application Extension)
typedef struct {
	u8 blockSize;
	u8 appIdentifier[8];			//Application Identifier (ASCII Code)
	u8 appAuthenticationCode[3];	//Application Authentication Code (ASCII Code)
}gifApplicationExtension;

//
typedef struct {
	u32 size;	//file size
	u8 header[GIF_HEADER_BYTES];	//Signature and Version
	gifLogicalScreenDescriptor lsd;	//Logical Screen Descriptor
	gifImageScreenDescriptor isd;	//Image Descriptor

	//Global Color Table
	u8 *gct;
	u32 gctSize;

	//Local Color Table
	u8 *lct;
	u32 lctSize;

}gifImage;

#if defined(_WIN32)
#pragma pack()
#endif

int gifDecode(const unsigned char *fileName);

#endif
