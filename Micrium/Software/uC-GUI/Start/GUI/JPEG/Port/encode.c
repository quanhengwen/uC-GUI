﻿/**
  ******************************************************************************
  * @file    LibJPEG/LibJPEG_Encoding/Src/encode.c
  * @author  MCD Application Team
  * @brief   This file contain the compress method.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "encode.h"

/* Private typedef -----------------------------------------------------------*/
   /* This struct contains the JPEG compression parameters */
   static struct jpeg_compress_struct cinfo; 
   /* This struct represents a JPEG error handler */
   static struct jpeg_error_mgr jerr; 
  
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/*
 * BMP 文件头
 * 使用结构体读取文件头时候不读取前两个字节是因为编译器默认是 4 字节对齐的（一次读取 4 个字节）,
 * 结构体中添加 vbfType 会导致后面的数据错误. 读的时候偏移两个字节就可以了。
 */
typedef struct BITMAP_FILE_HEADER {
    //UINT16 bfType;        // 2 Byte 位图文件类型(BMP 格式图片,固定为 "BM" ).
    DWORD  bfSize;        // 4 Byte 位图文件大小,单位 字节.
    UINT16 bfReserved1;   // 2 Byte 保留字 1.
    UINT16 bfReserved2;   // 2 Byte 保留字 2.
    DWORD  bfOffBits;     // 4 Byte 位图数据偏移量,从文件头开始到实际图像数据之间的字节偏移量,单位 字节.
} BITMAP_FILE_HEADER;

/*
 * 位图信息头
 */
typedef struct BITMAP_INFO_HEADER {
     DWORD biSize;           // 4 Byte BITMAP_INFO_HEADER 结构所需字节数.
     DWORD biWidth;          // 4 Byte 图像宽度,以像素为单位.
     DWORD biHeight;         // 4 Byte 图像高度,以像素为单位(正数代表图象是倒向,负数代表图像正向).
     WORD  biPlanes;         // 2 Byte 为目标设备说明颜色平面数目,其值总是被加一.
     WORD  biBitCount;       // 2 Byte 比特数/像素.
     DWORD biCompression;    // 4 Byte 图像数据压缩类型.
     DWORD biSizeImage;      // 4 Byte 图像大小.
     DWORD biXPelsPerMeter;  // 4 Byte 水平分辨率.
     DWORD biYPelsPerMeter;  // 4 Byte 垂直分辨率.
     DWORD biClrUsed;        // 4 Byte 位图使用的颜色表中的颜色索引数.
     DWORD biClrImportant;   // 4 Byte 对图像显示有重要影响的颜色索引数目.
 } BITMAP_INFO_HEADER;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Jpeg Encode
  * @param  file:          pointer to the bmp file
  * @param  file1:         pointer to the jpg file  
  * @param  image_quality: image quality
  * @retval None
  */
void jpeg_encode(JFILE *file, JFILE *file1, uint32_t image_quality)
{
  /* Encode BMP Image to JPEG */  
  JSAMPROW row_pointer;    /* Pointer to a single row */
  uint32_t bytesread;
  BITMAP_FILE_HEADER bmpFileHeader;
  BITMAP_INFO_HEADER bmpInfoHeader;
  JSAMPROW bmp_ptr;
  DWORD data_size;

  /* Get bitmap File Header */
  (void) SetFilePointer((HANDLE) file, 2, NULL, FILE_BEGIN);
  (void) ReadFile((HANDLE) file, &bmpFileHeader, sizeof(bmpFileHeader), (UINT*)&bytesread, NULL);

  /* Get bitmap Info Header */
  (void) ReadFile((HANDLE) file, &bmpInfoHeader, sizeof(bmpInfoHeader), (UINT*)&bytesread, NULL);

  /* Get bitmap image data */
  data_size = bmpInfoHeader.biWidth * bmpInfoHeader.biHeight * (bmpInfoHeader.biBitCount / 8);
  bmp_ptr = JMALLOC(data_size);
  (void) ReadFile((HANDLE) file, bmp_ptr, data_size, (UINT*)&bytesread, NULL);

  /* Step 1: allocate and initialize JPEG compression object */
  /* Set up the error handler */
  cinfo.err = jpeg_std_error(&jerr);
  
  /* Initialize the JPEG compression object */  
  jpeg_create_compress(&cinfo);
  
  /* Step 2: specify data destination */
  jpeg_stdio_dest(&cinfo, file1);
  
  /* Step 3: set parameters for compression */
  cinfo.image_width = bmpInfoHeader.biWidth;
  cinfo.image_height = bmpInfoHeader.biHeight;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  
  /* Set default compression parameters */
  jpeg_set_defaults(&cinfo);
  
  cinfo.dct_method  = JDCT_FLOAT;    
  
  jpeg_set_quality(&cinfo, image_quality, TRUE);
  
  /* Step 4: start compressor */
  jpeg_start_compress(&cinfo, TRUE);
  
  while (cinfo.next_scanline < cinfo.image_height)
  {          
    /* In this application, the input file is a BMP, which first encodes the bottom of the picture */
    /* JPEG encodes the highest part of the picture first. We need to read the lines upside down   */
    /* Move the read pointer to 'last line of the picture - next_scanline'    */
    row_pointer = (JSAMPROW)&bmp_ptr[((cinfo.image_height - 1 - cinfo.next_scanline) * cinfo.image_width * 3)];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }

  /* Step 5: finish compression */
  jpeg_finish_compress(&cinfo);
  
  /* Step 6: release JPEG compression object */
  jpeg_destroy_compress(&cinfo);
    
  JFREE(bmp_ptr);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
