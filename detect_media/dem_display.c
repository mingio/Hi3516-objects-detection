/******************************************************************************
Copyright (C)
******************************************************************************
File Name     : dem_display.c
Version       : V1.0
Author        :
Created       : 2018/09/02
Last Modified :
Description   : source file for display alg result
Function List :
History       :
 1.Date        : 2018/09/02
   Author      : kapoo pai
   Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>


#include "dem_globals.h"
#include "dem_config.h"
#include "dem_display.h"
#include "dem_hi_wrap.h"

#define OBJ_BOXES_ELEMENTSNUM           (8)
#define MAX_BOXES_NUM                   (10)

#define DATA_MEMSIZE                    (100000)


#define OBJDETECTS_CLASS_NUM       (21)     // all class number of TINY YOLO V2

typedef struct DISPLAY_HANDLE
{
    int init_flag;

    pthread_t thr_id;           // thread id
    pthread_mutex_t mutex;      // thread mutex

    char *data_addr;            // address for save capture picture
    DEM_DISPLAY_TYPE type;		// type
    int data_set_flag;          // flag

    char *label_bmp[OBJDETECTS_CLASS_NUM];  //bmp address for save labels
}DEM_DISPLAY_HANDLE;

DEM_DISPLAY_HANDLE g_dis_handle;

char *g_objs_class_name[OBJDETECTS_CLASS_NUM] =
{
    "bg",
    "aeroplane",
    "bicycle",
    "bird",
    "boat",
    "bottle",
    "bus",
    "car",
    "cat",
    "chair",
    "cow",
    "diningtable",
    "dog",
    "horse",
    "motorbike",
    "person",
    "pottedplant",
    "sheep",
    "sofa",
    "train",
    "tvmonitor"
};

#define FONT_WIDTH          (16)    //(8)
#define FONT_SIZE           (32)    //(16)
#define CHAR_MAX            (26)
// the alphabet for create bmp lables  8 x 16
char g_alphabet_8x16[CHAR_MAX][FONT_SIZE] =
// char g_alphabet[CHAR_MAX][FONT_SIZE] =
{
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x1E,0x22,0x42,0x42,0x3F,0x00,0x00},/*"a",0*/
    {0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x64,0x58,0x00,0x00},/*"b",1*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x22,0x40,0x40,0x40,0x22,0x1C,0x00,0x00},/*"c",2*/
    {0x00,0x00,0x00,0x06,0x02,0x02,0x02,0x1E,0x22,0x42,0x42,0x42,0x26,0x1B,0x00,0x00},/*"d",3*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x7E,0x40,0x40,0x42,0x3C,0x00,0x00},/*"e",4*/
    {0x00,0x00,0x00,0x0F,0x11,0x10,0x10,0x7E,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00},/*"f",5*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x44,0x44,0x38,0x40,0x3C,0x42,0x42,0x3C},/*"g",6*/
    {0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x42,0xE7,0x00,0x00},/*"h",7*/
    {0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00},/*"i",8*/
    {0x00,0x00,0x00,0x0C,0x0C,0x00,0x00,0x1C,0x04,0x04,0x04,0x04,0x04,0x04,0x44,0x78},/*"j",9*/
    {0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x4E,0x48,0x50,0x68,0x48,0x44,0xEE,0x00,0x00},/*"k",10*/
    {0x00,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00},/*"l",11*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x49,0x49,0x49,0x49,0x49,0xED,0x00,0x00},/*"m",12*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xDC,0x62,0x42,0x42,0x42,0x42,0xE7,0x00,0x00},/*"n",13*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00},/*"o",14*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD8,0x64,0x42,0x42,0x42,0x44,0x78,0x40,0xE0},/*"p",15*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x22,0x42,0x42,0x42,0x22,0x1E,0x02,0x07},/*"q",16*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0x32,0x20,0x20,0x20,0x20,0xF8,0x00,0x00},/*"r",17*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x42,0x40,0x3C,0x02,0x42,0x7C,0x00,0x00},/*"s",18*/
    {0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x0C,0x00,0x00},/*"t",19*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0x42,0x42,0x42,0x42,0x46,0x3B,0x00,0x00},/*"u",20*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE7,0x42,0x24,0x24,0x28,0x10,0x10,0x00,0x00},/*"v",21*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD7,0x92,0x92,0xAA,0xAA,0x44,0x44,0x00,0x00},/*"w",22*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,0x24,0x18,0x18,0x18,0x24,0x76,0x00,0x00},/*"x",23*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE7,0x42,0x24,0x24,0x28,0x18,0x10,0x10,0xE0},/*"y",24*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x44,0x08,0x10,0x10,0x22,0x7E,0x00,0x00},/*"z",25*/
};

// the alphabet for create bmp lables  16 x 32
// char g_alphabet_16x32[CHAR_MAX][64] =
char g_alphabet[CHAR_MAX][64] =
{
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xC0,0x18,0x60,0x30,0x30,
        0x30,0x30,0x00,0x30,0x01,0xF0,0x0E,0x30,0x38,0x30,0x30,0x30,0x60,0x30,0x60,0x30,
        0x60,0x32,0x30,0xF2,0x1F,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"a",0*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x78,0x00,0x18,0x00,
        0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0xF0,0x1B,0x18,0x1C,0x0C,
        0x1C,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x04,
        0x1C,0x0C,0x1E,0x18,0x13,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"b",1*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x0E,0x30,0x18,0x18,
        0x18,0x18,0x30,0x18,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x04,0x18,0x04,
        0x18,0x08,0x0C,0x10,0x03,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"c",2*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x78,0x00,0x18,
        0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x07,0xD8,0x0C,0x38,0x18,0x18,
        0x18,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x30,0x18,0x10,0x18,
        0x18,0x38,0x0C,0x5E,0x07,0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"d",3*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x0C,0x30,0x18,0x18,
        0x10,0x08,0x30,0x0C,0x30,0x0C,0x3F,0xFC,0x30,0x00,0x30,0x00,0x30,0x00,0x18,0x04,
        0x18,0x08,0x0E,0x18,0x03,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"e",4*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x01,0xC3,
        0x01,0x03,0x03,0x03,0x03,0x00,0x03,0x00,0x03,0x00,0x3F,0xF8,0x03,0x00,0x03,0x00,
        0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,
        0x03,0x00,0x03,0x00,0x1F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"f",5*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xEE,0x0C,0x36,0x08,0x18,
        0x18,0x18,0x18,0x18,0x18,0x18,0x08,0x18,0x0C,0x30,0x0F,0xE0,0x18,0x00,0x18,0x00,
        0x0F,0xF0,0x0F,0xFC,0x10,0x0E,0x30,0x06,0x30,0x06,0x30,0x06,0x1C,0x1C,0x07,0xF0
    },/*"g",6*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x78,0x00,0x18,0x00,
        0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x19,0xF0,0x1B,0x18,0x1C,0x0C,
        0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,
        0x18,0x0C,0x18,0x0C,0x7E,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"h",7*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x01,0xC0,
        0x01,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x1F,0x80,0x01,0x80,0x01,0x80,
        0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
        0x01,0x80,0x01,0x80,0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"i",8*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x1C,
        0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x01,0xF8,0x00,0x18,0x00,0x18,
        0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,
        0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x18,0x30,0x18,0x60,0x0F,0xC0
    },/*"j",9*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x78,0x00,0x18,0x00,
        0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x7C,0x18,0x30,0x18,0x60,
        0x18,0xC0,0x18,0x80,0x19,0x80,0x1B,0x80,0x1C,0xC0,0x18,0xE0,0x18,0x60,0x18,0x30,
        0x18,0x38,0x18,0x18,0x7E,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"k",10*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x1F,0x80,0x01,0x80,
        0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
        0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
        0x01,0x80,0x01,0x80,0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"l",11*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x77,0x38,0x39,0xCC,0x31,0x8C,
        0x31,0x8C,0x31,0x8C,0x31,0x8C,0x31,0x8C,0x31,0x8C,0x31,0x8C,0x31,0x8C,0x31,0x8C,
        0x31,0x8C,0x31,0x8C,0x7B,0xDE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"m",12*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x78,0xF0,0x1B,0x18,0x1C,0x0C,
        0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,
        0x18,0x0C,0x18,0x0C,0x7E,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"n",13*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x0E,0x38,0x08,0x0C,
        0x18,0x0C,0x30,0x06,0x30,0x06,0x30,0x06,0x30,0x06,0x30,0x06,0x30,0x06,0x18,0x0C,
        0x18,0x0C,0x0C,0x18,0x03,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"o",14*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x79,0xF0,0x1A,0x18,0x1C,0x0C,
        0x18,0x04,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x06,0x18,0x0C,
        0x1C,0x0C,0x1E,0x18,0x19,0xE0,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x7E,0x00
    },/*"p",15*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xC4,0x0C,0x3C,0x18,0x1C,
        0x18,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x10,0x0C,
        0x18,0x1C,0x0C,0x3C,0x07,0xCC,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x3F
    },/*"q",16*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x7E,0x3C,0x06,0x66,0x06,0x86,
        0x07,0x00,0x07,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x06,0x00,
        0x06,0x00,0x06,0x00,0x7F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"r",17*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xE4,0x0C,0x1C,0x18,0x0C,
        0x18,0x04,0x18,0x00,0x0E,0x00,0x07,0xC0,0x01,0xF0,0x00,0x38,0x20,0x0C,0x20,0x0C,
        0x30,0x0C,0x38,0x18,0x37,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"s",18*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x01,0x00,0x01,0x00,0x01,0x00,0x03,0x00,0x07,0x00,0x3F,0xF8,0x03,0x00,0x03,0x00,
        0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x04,
        0x03,0x04,0x01,0x88,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"t",19*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x04,0x78,0x3C,0x18,0x0C,0x18,0x0C,
        0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,
        0x18,0x1C,0x0C,0x2F,0x07,0xC8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"u",20*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x3C,0x18,0x18,0x18,0x10,
        0x1C,0x10,0x0C,0x20,0x0C,0x20,0x0E,0x40,0x06,0x40,0x06,0x40,0x07,0x80,0x03,0x80,
        0x03,0x80,0x03,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"v",21*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFB,0xEF,0x71,0xC6,0x30,0xC4,
        0x31,0xC4,0x31,0xC4,0x19,0xC8,0x19,0xC8,0x1A,0x68,0x1A,0x68,0x0E,0x70,0x0E,0x70,
        0x0E,0x70,0x04,0x20,0x04,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"w",22*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x7C,0x0E,0x10,0x0E,0x20,
        0x07,0x20,0x03,0x40,0x03,0x80,0x01,0xC0,0x01,0xC0,0x02,0xE0,0x06,0x60,0x04,0x30,
        0x08,0x30,0x18,0x18,0x7C,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"x",23*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x3E,0x18,0x18,0x18,0x10,
        0x0C,0x10,0x0C,0x10,0x0C,0x20,0x06,0x20,0x06,0x20,0x03,0x40,0x03,0x40,0x03,0x40,
        0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x00,0x01,0x00,0x01,0x00,0x32,0x00,0x3C,0x00
    },/*"y",24*/
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,0x30,0x30,0x20,0x70,
        0x20,0x60,0x00,0xC0,0x01,0xC0,0x01,0x80,0x03,0x00,0x07,0x00,0x0E,0x04,0x0C,0x04,
        0x1C,0x0C,0x38,0x18,0x3F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },/*"z",25*/
};

// color of labels foreground
char g_class_bmp_fgcolor[OBJDETECTS_CLASS_NUM][2] =
{
    {0x01, 0xc0},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
    {0x7f, 0xff},
};

// color of labels background
char g_class_bmp_bgcolor[OBJDETECTS_CLASS_NUM][2] =
{
    {0x1f, 0xff},
    {0x2f, 0xef},
    {0x3f, 0xdf},
    {0x4f, 0xcf},
    {0x5f, 0xbf},
    {0x6f, 0xaf},
    {0x7f, 0x9f},
    {0x1f, 0x8f},
    {0x2a, 0x7f},
    {0x3f, 0x6f},
    {0x4f, 0x5f},
    {0x53, 0x4f},
    {0x6f, 0x3f},
    {0x7d, 0x2f},
    {0x1f, 0x1f},
    {0x2c, 0x0f},
    {0x3f, 0xff},
    {0x43, 0xdf},
    {0x5f, 0xaf},
    {0x6a, 0x8f},
    {0x7f, 0x5f},
};

/*******************************************************************************
 * name    : DEM_display_classlabel_bmp_get
 * function: get the address of class label bmp.
 * input   : class_idx:  class name index
 * output  :
 * return  : address or NULL
 *******************************************************************************/
char *display_classlabel_bmp_get(int class_idx, DEM_RES_ST *res)
{
    if (NULL == res)
    {
        printf("[%s: %d]params res is NULL!\n", __func__, __LINE__);
        return NULL;
    }

    if (class_idx < 0 || class_idx >= OBJDETECTS_CLASS_NUM)
    {
        printf("[%s: %d]class index is overlimit, index: %d!\n", __func__, __LINE__, class_idx);
        return NULL;
    }

    char *name = g_objs_class_name[class_idx];
    if (NULL == name)
    {
        printf("[%s: %d]data is NULL!\n", __func__, __LINE__);
        return NULL;
    }
    int name_len = strlen(name);

    // bmp format: 1555, size 8 x 16
    res->width  = sizeof(char) * FONT_WIDTH * name_len;
    res->height = FONT_SIZE;

    // printf("[%s: %d]Get bmp resolution of class \'%s\' success, width: %d, height: %d!\n",
    //          __func__, __LINE__, name, res->width, res->height);

    return g_dis_handle.label_bmp[class_idx];
}

/*******************************************************************************
 * name    : display_task_func
 * function: task function for jdisplay
 * input   :
 * output  :
 * return  :
 *******************************************************************************/
void *display_task_func(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)"display_task", 0, 0, 0);

    printf("[%s: %d]tread display_task start process!\n", __func__, __LINE__);
    while (DEM_TRUE)
    {
        // printf("[%s: %d]start to get picture!\n", __func__, __LINE__);

        // lock
        pthread_mutex_lock(&g_dis_handle.mutex);
        DEM_DISPLAY_TYPE type = g_dis_handle.type;
		switch (type)
		{
            case DIS_YOLOV2_OBJDTCS:
            {
                int num = 0;
                num = *(int *)g_dis_handle.data_addr;

                char *class_bmp[num];
                DEM_RES_ST bmp_res[num];
                int act_num = 0;
                float act_boxes[num][4];         // X1, Y1, X2, Y2


                if (0 < num && MAX_BOXES_NUM > num)
                {
                    // printf("[%s: %d]box number is: %d!\n", __func__, __LINE__, num);
                    int idx = -1;
                    float *box_addr = (float *)((char *)g_dis_handle.data_addr + sizeof(num));
                    int loop = 0;

                    for (loop = 0; loop < num; ++loop)
                    {
                        idx = (int)(*(box_addr + 4));

                        #if 0
                        printf(">>>>> [%s: %d]idx: %d, box info: %f, %f, %f, %f, %f, %f, %f, %f\n",
                            __func__, __LINE__, idx,
                            *(box_addr    ), *(box_addr + 1), *(box_addr + 2), *(box_addr + 3),
                            *(box_addr + 4), *(box_addr + 5), *(box_addr + 6), *(box_addr + 7));
                        #endif

                        if (idx >= 0 && idx < OBJDETECTS_CLASS_NUM)
                        {

                            class_bmp[loop]    = display_classlabel_bmp_get(idx, &bmp_res[loop]);
                            act_boxes[loop][0] = (*(box_addr))     - (*(box_addr + 2)) / 2.0;
                            act_boxes[loop][1] = (*(box_addr + 1)) - (*(box_addr + 3)) / 2.0;
                            act_boxes[loop][2] = (*(box_addr))     + (*(box_addr + 2)) / 2.0;
                            act_boxes[loop][3] = (*(box_addr + 1)) + (*(box_addr + 3)) / 2.0;

                            act_boxes[loop][0] = act_boxes[loop][0] > 0 ? act_boxes[loop][0] : 0;
                            act_boxes[loop][1] = act_boxes[loop][1] > 0 ? act_boxes[loop][1] : 0;
                            act_boxes[loop][2] = act_boxes[loop][2] < 1 ? act_boxes[loop][2] : 1;
                            act_boxes[loop][3] = act_boxes[loop][3] < 1 ? act_boxes[loop][3] : 1;

                            act_num++;
                        }
                        else
                        {
                            printf("[%s: %d]ERROR!! class idx is overlimit: %d!\n", __func__, __LINE__, idx);
                        }

                        #if 0
                        printf(">>>>> [%s: %d]idx: %d, box info: %f, %f, %f, %f\n", __func__, __LINE__, idx,
                            act_boxes[loop][0], act_boxes[loop][1], act_boxes[loop][2], act_boxes[loop][3]);
                        #endif

                        box_addr += OBJ_BOXES_ELEMENTSNUM;
                    }
                }
                #if 0
                int idx = 0;
                for (idx = 0; idx < act_num; ++idx)
                {
                    printf("[%s: %d]resoltion width: %d, height: %d!\n", __func__, __LINE__, bmp_res[idx].width, bmp_res[idx].height);
                }
                #endif

                // clear the type flag
                g_dis_handle.type = DIS_INVALID;
                // unlock
                pthread_mutex_unlock(&g_dis_handle.mutex);

                // printf("[%s: %d]start to call HI_wrap_label_box_show, box number is: %d!\n", __func__, __LINE__, act_num);
                HI_wrap_label_box_show(act_num, class_bmp, &bmp_res[0], &act_boxes[0][0]);
                break;
            }
            default:
                // unlock
                pthread_mutex_unlock(&g_dis_handle.mutex);
			break;
		}

        // printf("[%s: %d]finish to get picture!\n", __func__, __LINE__);
        usleep(50000);
    }

    printf("[%s: %d]tread display_task end process!\n", __func__, __LINE__);
}

/*******************************************************************************
 * name    : display_process_start
 * function: start to display.
 * input   :
 * output  :
 * return  : success: DEM_SUCCESS
 *           fail: error number
 *******************************************************************************/
int display_process_start(void)
{
    pthread_attr_t attr;
    int ret = DEM_COMMONFAIL;

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 0x96000);     /* 600K */

    printf("[%s: %d]pthread_create display_task_func start!\n", __func__, __LINE__);
    ret = pthread_create(&g_dis_handle.thr_id, &attr, display_task_func, NULL);
    if (DEM_SUCCESS != ret)
    {
        printf("[%s: %d]pthread_create display_task_func fail!\n", __func__, __LINE__);
        return DEM_CTHRFAIL;
    }

    return DEM_SUCCESS;
}

/*******************************************************************************
 * name    : display_classname_bmp_destory
 * function: destory all bmp for the class name.
 * input   :
 * output  :
 * return  : success: DEM_SUCCESS
 *           fail: error number
 *******************************************************************************/
int display_classname_bmp_destory(void)
{
    int class_id = 0;
    for (class_id = 0; class_id < OBJDETECTS_CLASS_NUM; ++class_id)
    {
        if (NULL != g_dis_handle.label_bmp[class_id])
        {
            free(g_dis_handle.label_bmp[class_id]);
            g_dis_handle.label_bmp[class_id] = NULL;
        }
    }

    return DEM_SUCCESS;
}

/*******************************************************************************
 * name    : display_classname_bmp_create
 * function: create all bmp for the class name.
 * input   :
 * output  :
 * return  : success: DEM_SUCCESS
 *           fail: error number
 *******************************************************************************/
int display_classname_bmp_create(void)
{
    int class_id = 0;
    for (class_id = 0; class_id < OBJDETECTS_CLASS_NUM; ++class_id)
    {
        char *name = g_objs_class_name[class_id];
        if (NULL == name)
        {
            printf("[%s: %d]data is NULL!\n", __func__, __LINE__);
            return DEM_COMMONFAIL;
        }

        // printf("[%s: %d]create bmp %s start!\n", __func__, __LINE__, name);

        int name_len = strlen(name);
        printf("[%s: %d]class name is: %s, len: %d!\n", __func__, __LINE__, name, name_len);

        // bmp format: 1555, size 8 x 16
        int bmp_mem_size = 2 * sizeof(char) * FONT_SIZE * FONT_WIDTH * name_len;
        g_dis_handle.label_bmp[class_id] = malloc(bmp_mem_size);
        if (NULL == g_dis_handle.label_bmp[class_id])
        {
            printf("[%s: %d]malloc label_bmp fail!\n", __func__, __LINE__);
            goto FREE_MEM;
        }

        //get the font model
        char *font_model_addr = NULL;
        char *bmp_fill_addr = g_dis_handle.label_bmp[class_id];
        // printf("[%s: %d]start to create bmp, address is 0x%x!\n", __func__, __LINE__, (unsigned int)bmp_fill_addr);

        int char_idx = 0;
        for (char_idx = 0; char_idx < name_len; ++char_idx)
        {
            int alphabet_idx = (int)(*(name + char_idx) - 'a');
            // printf("[%s: %d]alphabet_idx is %d!\n", __func__, __LINE__, alphabet_idx);
            font_model_addr = &g_alphabet[alphabet_idx][0];
            bmp_fill_addr = g_dis_handle.label_bmp[class_id] + (char_idx * FONT_WIDTH << 1);

            int row_idx = 0;
            int column_idx = 0;
            int bit = 0;
            for (row_idx = 0; row_idx < FONT_SIZE; ++row_idx)
            {
                char *row_addr_begin = bmp_fill_addr;
                for (column_idx = 0; column_idx < (FONT_WIDTH / 8); ++column_idx)
                {
                    char word_module = *(font_model_addr + row_idx * (FONT_WIDTH / 8) + column_idx);
                    for (bit = 0; bit < 8; ++bit)
                    {
                        if (1 == ((word_module >> (7 - bit % 8)) & 0x1))
                        {
                            // set for foreground
                            *(row_addr_begin++) = g_class_bmp_fgcolor[class_id][0];
                            *(row_addr_begin++) = g_class_bmp_fgcolor[class_id][1];
                        }
                        else
                        {
                            // set for background
                            *(row_addr_begin++) = g_class_bmp_bgcolor[class_id][0];
                            *(row_addr_begin++) = g_class_bmp_bgcolor[class_id][1];
                        }
                    }
                }

                bmp_fill_addr += 2 * sizeof(char) * FONT_WIDTH * name_len;
            }
        }
        #if 0
        if (0 == class_id)
        {
            int width = 0;
            int height = 0;
            bmp_fill_addr = g_dis_handle.label_bmp[class_id];
            for (height = 0; height < FONT_SIZE; ++height)
            {
                for (width = 0; width < FONT_WIDTH * 2 * 2; ++width)
                {
                    printf("0x%x, ", *(bmp_fill_addr++));
                }
                printf("\n");
            }
        }
        #endif

        printf("[%s: %d]create bmp %s success!\n", __func__, __LINE__, name);
    }

    return DEM_SUCCESS;

FREE_MEM:
    display_classname_bmp_destory();

    return DEM_COMMONFAIL;
}

/*******************************************************************************
 * name    : DEM_display_data_set
 * function: set the alg result data for display.
 * input   : data:  input result data
 *           type:  data type
 * output  :
 * return  : DEM_SUCCESS or errno number
 *******************************************************************************/
int DEM_display_data_set(void *data, DEM_DISPLAY_TYPE type)
{
	if (NULL == data)
	{
        printf("[%s: %d]data is NULL!\n", __func__, __LINE__);
        return DEM_COMMONFAIL;
    }

    int num = 0;
    switch (type)
    {
    	case DIS_YOLOV2_OBJDTCS:
    	num = *(int *)data;
    	printf("[%s: %d]box number is: %d!\n", __func__, __LINE__, num);
    	pthread_mutex_lock(&g_dis_handle.mutex);
    	memcpy(g_dis_handle.data_addr, (char *)data, sizeof(int) + sizeof(float) * num * OBJ_BOXES_ELEMENTSNUM);
    	g_dis_handle.type = type;
    	pthread_mutex_unlock(&g_dis_handle.mutex);
    	break;

    	default:
    	return DEM_COMMONFAIL;
    }

    return DEM_SUCCESS;
}

/*******************************************************************************
 * name    : DEM_display_module_init
 * function: module initlizatin.
 * input   :
 * output  :
 * return  : DEM_SUCCESS or errno number
 *******************************************************************************/
int DEM_display_module_init(void)
{
	if (DEM_MODULE_INIT == g_dis_handle.init_flag)
    {
        printf("[%s: %d]display module has initialized!\n", __func__, __LINE__);
        return DEM_SUCCESS;
    }

    DEM_CONFIG_ST *cfg = DEM_cfg_get_global_params();
    if (NULL == cfg)
    {
        printf("[%s: %d]cfg_get_global_configures fail!\n", __func__, __LINE__);
        return DEM_COMMONFAIL;
    }

    memset(&g_dis_handle, 0, sizeof(g_dis_handle));
    if (NULL == g_dis_handle.data_addr)
    {
    	g_dis_handle.data_addr = (char *)malloc(DATA_MEMSIZE);
    }

    if (NULL == g_dis_handle.data_addr)
    {
    	printf("[%s: %d]g_dis_handle.data_addr malloc fail!\n", __func__, __LINE__);
        return DEM_COMMONFAIL;
    }
    memset(g_dis_handle.data_addr, 0, DATA_MEMSIZE);

    if (DEM_SUCCESS != display_classname_bmp_create())
    {
        printf("[%s: %d]display_classname_bmp_create fail!\n", __func__, __LINE__);
        return DEM_COMMONFAIL;
    }

    // create mutex lock
    if (DEM_SUCCESS != pthread_mutex_init(&g_dis_handle.mutex, NULL))
    {
        printf("[%s: %d]pthread_mutex_init fail!\n", __func__, __LINE__);
        goto FIRST_FAIL;
    }

    // create thread
    if (DEM_SUCCESS != display_process_start())
    {
        printf("[%s: %d]display_process_start fail!\n", __func__, __LINE__);
        goto SECOND_FAIL;
    }
    g_dis_handle.init_flag = DEM_MODULE_INIT;

    return DEM_SUCCESS;
#if 0
THIRD_FAIL:
	if (0 != g_dis_handle.thr_id)
	{
		pthread_join(g_dis_handle.thr_id, 0);
		g_dis_handle.thr_id = 0;
	}
#endif
SECOND_FAIL:
	pthread_mutex_destroy(&g_dis_handle.mutex);

FIRST_FAIL:
	if (NULL == g_dis_handle.data_addr)
	{
		free(g_dis_handle.data_addr);
		g_dis_handle.data_addr = NULL;
	}

    display_classname_bmp_destory();

	return DEM_COMMONFAIL;
}
