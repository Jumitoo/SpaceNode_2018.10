/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: lib_ctype.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2008 年 08 月 30 日
**
** 描        述: ctype.h.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#define  _CTYPE_PRIVATE                                                 /*  private                     */
#include "ctype.h"
/*********************************************************************************************************
  ctype array
*********************************************************************************************************/
const uint8_t   _C_ctype_[1 + _CTYPE_NUM_CHARS] = {
    0,
    _C,    _C,    _C,    _C,    _C,    _C,    _C,    _C,
    _C,    _C|_S, _C|_S, _C|_S, _C|_S, _C|_S, _C,    _C,
    _C,    _C,    _C,    _C,    _C,    _C,    _C,    _C,
    _C,    _C,    _C,    _C,    _C,    _C,    _C,    _C,
    _S|_B, _P,    _P,    _P,    _P,    _P,    _P,    _P,
    _P,    _P,    _P,    _P,    _P,    _P,    _P,    _P,
    _N,    _N,    _N,    _N,    _N,    _N,    _N,    _N,
    _N,    _N,    _P,    _P,    _P,    _P,    _P,    _P,
    _P,    _U|_X, _U|_X, _U|_X, _U|_X, _U|_X, _U|_X, _U,
    _U,    _U,    _U,    _U,    _U,    _U,    _U,    _U,
    _U,    _U,    _U,    _U,    _U,    _U,    _U,    _U,
    _U,    _U,    _U,    _P,    _P,    _P,    _P,    _P,
    _P,    _L|_X, _L|_X, _L|_X, _L|_X, _L|_X, _L|_X, _L,
    _L,    _L,    _L,    _L,    _L,    _L,    _L,    _L,
    _L,    _L,    _L,    _L,    _L,    _L,    _L,    _L,
    _L,    _L,    _L,    _P,    _P,    _P,    _P,    _C
};

const unsigned char *_ctype_ = _C_ctype_;                               /* Backward compatibility.      */
/*********************************************************************************************************
  libc functions
*********************************************************************************************************/
int lib_isalnum(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & (_U | _L | _N)));
}

int lib_isalpha(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & (_U | _L)));
}

int lib_iscntrl(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _C));
}

int lib_isdigit(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _N));
}

int lib_isgraph(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & (_P | _U | _L | _N)));
}

int lib_islower(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _L));
}

int lib_isprint(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & (_P | _U | _L | _N | _B)));
}

int lib_ispunct(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _P));
}

int lib_isspace(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _S));
}

int lib_isupper(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & _U));
}

int lib_isxdigit(int c)
{
    return (c == -1 ? 0 : ((_ctype_ + 1)[(unsigned char)c] & (_N | _X)));
}

int lib_isascii(int c)
{
    return ((c >= 0x00 && c <= 0x7f) ? 1 : 0);
}

int lib_toascii(int c)
{
    return (c & 0x7f);
}

int lib_isblank(int c)
{
    return (c == ' ' || c == '\t');
}

/*********************************************************************************************************
  _C_toupper_ & _C_tolower_
*********************************************************************************************************/
#ifndef EOF
#define EOF (-1)
#endif

#if EOF != -1
#error "EOF != -1"
#endif
/*********************************************************************************************************
  _C_toupper_ table
*********************************************************************************************************/
const int16_t   _C_toupper_[1 + 256] = {
    EOF,
    0x00,    0x01,    0x02,    0x03,    0x04,    0x05,    0x06,    0x07,
    0x08,    0x09,    0x0a,    0x0b,    0x0c,    0x0d,    0x0e,    0x0f,
    0x10,    0x11,    0x12,    0x13,    0x14,    0x15,    0x16,    0x17,
    0x18,    0x19,    0x1a,    0x1b,    0x1c,    0x1d,    0x1e,    0x1f,
    0x20,    0x21,    0x22,    0x23,    0x24,    0x25,    0x26,    0x27,
    0x28,    0x29,    0x2a,    0x2b,    0x2c,    0x2d,    0x2e,    0x2f,
    0x30,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,    0x37,
    0x38,    0x39,    0x3a,    0x3b,    0x3c,    0x3d,    0x3e,    0x3f,
    0x40,    0x41,    0x42,    0x43,    0x44,    0x45,    0x46,    0x47,
    0x48,    0x49,    0x4a,    0x4b,    0x4c,    0x4d,    0x4e,    0x4f,
    0x50,    0x51,    0x52,    0x53,    0x54,    0x55,    0x56,    0x57,
    0x58,    0x59,    0x5a,    0x5b,    0x5c,    0x5d,    0x5e,    0x5f,
    0x60,    'A',     'B',     'C',     'D',     'E',     'F',     'G',
    'H',     'I',     'J',     'K',     'L',     'M',     'N',     'O',
    'P',     'Q',     'R',     'S',     'T',     'U',     'V',     'W',
    'X',     'Y',     'Z',     0x7b,    0x7c,    0x7d,    0x7e,    0x7f,
    0x80,    0x81,    0x82,    0x83,    0x84,    0x85,    0x86,    0x87,
    0x88,    0x89,    0x8a,    0x8b,    0x8c,    0x8d,    0x8e,    0x8f,
    0x90,    0x91,    0x92,    0x93,    0x94,    0x95,    0x96,    0x97,
    0x98,    0x99,    0x9a,    0x9b,    0x9c,    0x9d,    0x9e,    0x9f,
    0xa0,    0xa1,    0xa2,    0xa3,    0xa4,    0xa5,    0xa6,    0xa7,
    0xa8,    0xa9,    0xaa,    0xab,    0xac,    0xad,    0xae,    0xaf,
    0xb0,    0xb1,    0xb2,    0xb3,    0xb4,    0xb5,    0xb6,    0xb7,
    0xb8,    0xb9,    0xba,    0xbb,    0xbc,    0xbd,    0xbe,    0xbf,
    0xc0,    0xc1,    0xc2,    0xc3,    0xc4,    0xc5,    0xc6,    0xc7,
    0xc8,    0xc9,    0xca,    0xcb,    0xcc,    0xcd,    0xce,    0xcf,
    0xd0,    0xd1,    0xd2,    0xd3,    0xd4,    0xd5,    0xd6,    0xd7,
    0xd8,    0xd9,    0xda,    0xdb,    0xdc,    0xdd,    0xde,    0xdf,
    0xe0,    0xe1,    0xe2,    0xe3,    0xe4,    0xe5,    0xe6,    0xe7,
    0xe8,    0xe9,    0xea,    0xeb,    0xec,    0xed,    0xee,    0xef,
    0xf0,    0xf1,    0xf2,    0xf3,    0xf4,    0xf5,    0xf6,    0xf7,
    0xf8,    0xf9,    0xfa,    0xfb,    0xfc,    0xfd,    0xfe,    0xff
};

const short *_toupper_tab_ = _C_toupper_;
/*********************************************************************************************************
  _C_tolower_ table
*********************************************************************************************************/
const int16_t   _C_tolower_[1 + 256] = {
    EOF,
    0x00,    0x01,    0x02,    0x03,    0x04,    0x05,    0x06,    0x07,
    0x08,    0x09,    0x0a,    0x0b,    0x0c,    0x0d,    0x0e,    0x0f,
    0x10,    0x11,    0x12,    0x13,    0x14,    0x15,    0x16,    0x17,
    0x18,    0x19,    0x1a,    0x1b,    0x1c,    0x1d,    0x1e,    0x1f,
    0x20,    0x21,    0x22,    0x23,    0x24,    0x25,    0x26,    0x27,
    0x28,    0x29,    0x2a,    0x2b,    0x2c,    0x2d,    0x2e,    0x2f,
    0x30,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,    0x37,
    0x38,    0x39,    0x3a,    0x3b,    0x3c,    0x3d,    0x3e,    0x3f,
    0x40,    'a',     'b',     'c',     'd',     'e',     'f',     'g',
    'h',     'i',     'j',     'k',     'l',     'm',     'n',     'o',
    'p',     'q',     'r',     's',     't',     'u',     'v',     'w',
    'x',     'y',     'z',     0x5b,    0x5c,    0x5d,    0x5e,    0x5f,
    0x60,    0x61,    0x62,    0x63,    0x64,    0x65,    0x66,    0x67,
    0x68,    0x69,    0x6a,    0x6b,    0x6c,    0x6d,    0x6e,    0x6f,
    0x70,    0x71,    0x72,    0x73,    0x74,    0x75,    0x76,    0x77,
    0x78,    0x79,    0x7a,    0x7b,    0x7c,    0x7d,    0x7e,    0x7f,
    0x80,    0x81,    0x82,    0x83,    0x84,    0x85,    0x86,    0x87,
    0x88,    0x89,    0x8a,    0x8b,    0x8c,    0x8d,    0x8e,    0x8f,
    0x90,    0x91,    0x92,    0x93,    0x94,    0x95,    0x96,    0x97,
    0x98,    0x99,    0x9a,    0x9b,    0x9c,    0x9d,    0x9e,    0x9f,
    0xa0,    0xa1,    0xa2,    0xa3,    0xa4,    0xa5,    0xa6,    0xa7,
    0xa8,    0xa9,    0xaa,    0xab,    0xac,    0xad,    0xae,    0xaf,
    0xb0,    0xb1,    0xb2,    0xb3,    0xb4,    0xb5,    0xb6,    0xb7,
    0xb8,    0xb9,    0xba,    0xbb,    0xbc,    0xbd,    0xbe,    0xbf,
    0xc0,    0xc1,    0xc2,    0xc3,    0xc4,    0xc5,    0xc6,    0xc7,
    0xc8,    0xc9,    0xca,    0xcb,    0xcc,    0xcd,    0xce,    0xcf,
    0xd0,    0xd1,    0xd2,    0xd3,    0xd4,    0xd5,    0xd6,    0xd7,
    0xd8,    0xd9,    0xda,    0xdb,    0xdc,    0xdd,    0xde,    0xdf,
    0xe0,    0xe1,    0xe2,    0xe3,    0xe4,    0xe5,    0xe6,    0xe7,
    0xe8,    0xe9,    0xea,    0xeb,    0xec,    0xed,    0xee,    0xef,
    0xf0,    0xf1,    0xf2,    0xf3,    0xf4,    0xf5,    0xf6,    0xf7,
    0xf8,    0xf9,    0xfa,    0xfb,    0xfc,    0xfd,    0xfe,    0xff
};

const short *_tolower_tab_ = _C_tolower_;
/*********************************************************************************************************
  END
*********************************************************************************************************/
