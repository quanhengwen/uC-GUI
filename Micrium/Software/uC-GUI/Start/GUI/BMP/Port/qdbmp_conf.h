/**
  ******************************************************************************
  * @file    qdbmp_conf.h
  * @author  MCD Application Team
  * @brief    this file contains the libJPEG abstruction defines for the memory
  *          management routines and IO File operations
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
#include <windows.h>
#include <stdint.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#define BFILE                      HANDLE

#define BMALLOC                    malloc
#define BCALLOC                    calloc
#define BFREE                      free

#define BFCLOSE                    bclose_file
#define BFREAD                     bread_file
#define BFWRITE                    bwrite_file

size_t bread_file (uint8_t *buf, uint32_t sizeofbuf, BFILE  *file);
size_t bwrite_file (uint8_t *buf, uint32_t sizeofbuf, BFILE  *file);
size_t bclose_file(BFILE  *file);

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
