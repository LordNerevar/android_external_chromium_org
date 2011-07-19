// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//  Accepts all other UTF-8 codes 0000..10FFFF
//  Space optimized
// ** ASSUMES INPUT IS STRUCTURALLY VALID UTF-8 **
//  Table entries are absolute statetable subscripts

#ifndef EXPERIMENTAL_USERS_DSITES_LANGDET_UTF8SCANNOTJUSTLETTERSPECIAL_H__
#define EXPERIMENTAL_USERS_DSITES_LANGDET_UTF8SCANNOTJUSTLETTERSPECIAL_H__

#include "encodings/compact_lang_det/win/cld_utf8statetable.h"

#define X__ (kExitIllegalStructure)
#define RJ_ (kExitReject)
#define S1_ (kExitReplace1)
#define S2_ (kExitReplace2)
#define S3_ (kExitReplace3)
#define S21 (kExitReplace21)
#define S31 (kExitReplace31)
#define S32 (kExitReplace32)
#define T1_ (kExitReplaceOffset1)
#define T2_ (kExitReplaceOffset2)
#define S11 (kExitReplace1S0)
#define SP_ (kExitSpecial)
#define D__ (kExitDoAgain)
#define RJA (kExitRejectAlt)

//  Entire table has 180 state blocks of 64 entries each

static const unsigned int utf8scannotjustletterspecial_STATE0 = 0;		// state[0]
static const unsigned int utf8scannotjustletterspecial_STATE0_SIZE = 64;	// =[1]
static const unsigned int utf8scannotjustletterspecial_TOTAL_SIZE = 11520;
static const unsigned int utf8scannotjustletterspecial_MAX_EXPAND_X4 = 0;
static const unsigned int utf8scannotjustletterspecial_SHIFT = 6;
static const unsigned int utf8scannotjustletterspecial_BYTES = 1;
static const unsigned int utf8scannotjustletterspecial_LOSUB = 0x27272727;
static const unsigned int utf8scannotjustletterspecial_HIADD = 0x44444444;

static const uint8 utf8scannotjustletterspecial[] = {
// state[0] 0x000000 Byte 1
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,RJ_,  0,RJ_,  0,

  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,

X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,

X__,X__,  6,  7,  8,  8,  8,  8,   8,  8,  8,  9,  8, 10, 11, 12,
  8,  8, 13,  8, 14, 15, 16, 17,  18, 19,  8, 20, 21, 22, 23, 24,
 25, 54, 92,106,113,115,115,115, 115,116,118,115,115,123,  2,125,
142,  4,  4,175,  5,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,

// state[4 - 2] 0x00e000 Byte 2 of 3
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[5 - 2] 0x000800 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[6 - 2] 0x040000 Byte 2 of 4
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,

// state[7 - 2] 0x100000 Byte 2 of 4
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,

// state[8 - 2] 0x000080 Byte 2 of 2
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,RJ_,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,RJ_,  0,  0,   0,  0,RJ_,  0,  0,  0,  0,  0,

// state[9 - 2] 0x0000c0 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[10 - 2] 0x000100 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[11 - 2] 0x0002c0 Byte 2 of 2
RJ_,RJ_,  0,  0,  0,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[12 - 2] 0x000340 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,

// state[13 - 2] 0x000380 Byte 2 of 2
  0,  0,  0,  0,  0,  0,RJ_,  0, RJ_,RJ_,RJ_,  0,RJ_,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[14 - 2] 0x0003c0 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[15 - 2] 0x000480 Byte 2 of 2
RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[16 - 2] 0x000500 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[17 - 2] 0x000540 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,RJ_,  0,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[18 - 2] 0x000580 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,

// state[19 - 2] 0x0005c0 Byte 2 of 2
  0,RJ_,RJ_,  0,RJ_,RJ_,  0,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[20 - 2] 0x000600 Byte 2 of 2
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,

// state[21 - 2] 0x000640 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[22 - 2] 0x0006c0 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,RJ_,RJ_,RJ_,  0,  0,RJ_,

// state[23 - 2] 0x000700 Byte 2 of 2
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[24 - 2] 0x000740 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[25 - 2] 0x000780 Byte 2 of 2
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[26 - 2] 0x0007c0 Byte 2 of 2
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,RJ_,  0,  0,  0,  0,  0,

// state[27 - 2] 0x000000 Byte 2 of 3
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
  3,  3,  3,  3, 26, 27, 28, 29,  30, 31, 32, 33, 34, 35, 36, 37,
 38, 39, 40, 41, 42, 43, 44, 45,  46, 47, 48, 49, 50, 51, 52, 53,

// state[28 - 2] 0x000900 Byte 3 of 3
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,

// state[29 - 2] 0x000940 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[30 - 2] 0x000980 Byte 3 of 3
  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_,
RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,  0,  0,  0,RJ_,RJ_, RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,

// state[31 - 2] 0x0009c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_, RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,RJ_,   0,  0,  0,  0,RJ_,RJ_,  0,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[32 - 2] 0x000a00 Byte 3 of 3
  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,RJ_,
RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,  0,RJ_,RJ_,  0, RJ_,RJ_,  0,  0,RJ_,  0,RJ_,RJ_,

// state[33 - 2] 0x000a40 Byte 3 of 3
RJ_,RJ_,RJ_,  0,  0,  0,  0,RJ_, RJ_,  0,  0,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[34 - 2] 0x000a80 Byte 3 of 3
  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,
RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,

// state[35 - 2] 0x000ac0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_, RJ_,RJ_,  0,RJ_,RJ_,RJ_,  0,  0,
RJ_,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[36 - 2] 0x000b00 Byte 3 of 3
  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_,
RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,

// state[37 - 2] 0x000b40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,RJ_, RJ_,  0,  0,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,RJ_,RJ_,   0,  0,  0,  0,RJ_,RJ_,  0,RJ_,
RJ_,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[38 - 2] 0x000b80 Byte 3 of 3
  0,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,RJ_,RJ_,  0,RJ_,  0,RJ_,RJ_,
  0,  0,  0,RJ_,RJ_,  0,  0,  0, RJ_,RJ_,RJ_,  0,  0,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,RJ_,RJ_,

// state[39 - 2] 0x000bc0 Byte 3 of 3
RJ_,RJ_,RJ_,  0,  0,  0,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[40 - 2] 0x000c00 Byte 3 of 3
  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,RJ_,RJ_,

// state[41 - 2] 0x000c40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[42 - 2] 0x000c80 Byte 3 of 3
  0,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,

// state[43 - 2] 0x000cc0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[44 - 2] 0x000d00 Byte 3 of 3
  0,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,RJ_,RJ_,

// state[45 - 2] 0x000d40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[46 - 2] 0x000d80 Byte 3 of 3
  0,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0,  0,

// state[47 - 2] 0x000dc0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,  0,RJ_,  0,  0,  0,  0,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[48 - 2] 0x000e00 Byte 3 of 3
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,

// state[49 - 2] 0x000e40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[50 - 2] 0x000e80 Byte 3 of 3
  0,RJ_,RJ_,  0,RJ_,  0,  0,RJ_, RJ_,  0,RJ_,  0,  0,RJ_,  0,  0,
  0,  0,  0,  0,RJ_,RJ_,RJ_,RJ_,   0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,RJ_,RJ_,RJ_,  0,RJ_,  0,RJ_,   0,  0,RJ_,RJ_,  0,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,RJ_,RJ_,RJ_,  0,  0,

// state[51 - 2] 0x000ec0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[52 - 2] 0x000f00 Byte 3 of 3
RJ_,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,RJ_,  0,RJ_,   0,RJ_,  0,  0,  0,  0,RJ_,RJ_,

// state[53 - 2] 0x000f40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[54 - 2] 0x000f80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,

// state[55 - 2] 0x000fc0 Byte 3 of 3
  0,  0,  0,  0,  0,  0,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[56 - 2] 0x001000 Byte 2 of 3
 55, 56, 57, 58,  8, 59, 60, 61,   8, 62, 63, 64, 65, 66, 67, 68,
 69,  8,  8,  8,  8,  8,  8,  8,   8, 70, 71, 72, 73, 74, 75, 76,
 77, 78, 79,  3, 80, 81, 82, 83,  84,  3,  3,  3,  8, 85,  3,  3,
  3,  3,  3,  3,  8,  8,  8, 86,   8,  8, 87, 61, 88, 89, 90, 91,

// state[57 - 2] 0x001000 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,   0,RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,  0,  0,  0,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,

// state[58 - 2] 0x001040 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[59 - 2] 0x001080 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[60 - 2] 0x0010c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,RJ_,  0,  0,  0,

// state[61 - 2] 0x001140 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[62 - 2] 0x001180 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,  0,  0,  0,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[63 - 2] 0x0011c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,

// state[64 - 2] 0x001240 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[65 - 2] 0x001280 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,

// state[66 - 2] 0x0012c0 Byte 3 of 3
RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[67 - 2] 0x001300 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[68 - 2] 0x001340 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[69 - 2] 0x001380 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[70 - 2] 0x0013c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[71 - 2] 0x001400 Byte 3 of 3
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[72 - 2] 0x001640 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[73 - 2] 0x001680 Byte 3 of 3
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[74 - 2] 0x0016c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[75 - 2] 0x001700 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[76 - 2] 0x001740 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[77 - 2] 0x001780 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[78 - 2] 0x0017c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,RJ_,   0,  0,  0,  0,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[79 - 2] 0x001800 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[80 - 2] 0x001840 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,

// state[81 - 2] 0x001880 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[82 - 2] 0x001900 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,

// state[83 - 2] 0x001940 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[84 - 2] 0x001980 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[85 - 2] 0x0019c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[86 - 2] 0x001a00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[87 - 2] 0x001b40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[88 - 2] 0x001dc0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,RJ_,RJ_,

// state[89 - 2] 0x001e80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[90 - 2] 0x001f00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[91 - 2] 0x001f40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,RJ_,  0,RJ_,  0,RJ_,  0,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,

// state[92 - 2] 0x001f80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0,

// state[93 - 2] 0x001fc0 Byte 3 of 3
  0,  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,  0,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,
  0,  0,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,

// state[94 - 2] 0x002000 Byte 2 of 3
  3, 93, 94, 95, 96, 97, 98,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
 99,100,  8,101,102,103,104,105,   3,  3,  3,  3,  3,  3,  3,  3,

// state[95 - 2] 0x002040 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,RJ_,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,RJ_,

// state[96 - 2] 0x002080 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[97 - 2] 0x0020c0 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[98 - 2] 0x002100 Byte 3 of 3
  0,  0,RJ_,  0,  0,  0,  0,RJ_,   0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0,  0,   0,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,RJ_,  0,RJ_,  0, RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,RJ_,

// state[99 - 2] 0x002140 Byte 3 of 3
  0,  0,  0,  0,  0,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[100 - 2] 0x002180 Byte 3 of 3
  0,  0,  0,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[101 - 2] 0x002c00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[102 - 2] 0x002c40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,
  0,  0,  0,  0,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,

// state[103 - 2] 0x002cc0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[104 - 2] 0x002d00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[105 - 2] 0x002d40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,  0,  0,  0,  0,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[106 - 2] 0x002d80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,

// state[107 - 2] 0x002dc0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[108 - 2] 0x003000 Byte 2 of 3
107, 69,108,109,110,  8,111,112,   3,  3,  3,  3,  3,  3,  3,  3,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,

// state[109 - 2] 0x003000 Byte 3 of 3
  0,  0,  0,  0,  0,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,RJ_,RJ_,  0,  0,  0,

// state[110 - 2] 0x003080 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[111 - 2] 0x0030c0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,

// state[112 - 2] 0x003100 Byte 3 of 3
  0,  0,  0,  0,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[113 - 2] 0x003180 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,

// state[114 - 2] 0x0031c0 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[115 - 2] 0x004000 Byte 2 of 3
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,114,  3,   8,  8,  8,  8,  8,  8,  8,  8,

// state[116 - 2] 0x004d80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[117 - 2] 0x005000 Byte 2 of 3
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,

// state[118 - 2] 0x009000 Byte 2 of 3
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,117,  3,

// state[119 - 2] 0x009f80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,

// state[120 - 2] 0x00a000 Byte 2 of 3
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,119,  3,  3,  3,  3,  3,   3,  3,  3,  3,120,  3,  3,  3,
121,122,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,

// state[121 - 2] 0x00a480 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[122 - 2] 0x00a700 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[123 - 2] 0x00a800 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[124 - 2] 0x00a840 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[125 - 2] 0x00d000 Byte 2 of 3
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,124,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[126 - 2] 0x00d780 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[127 - 2] 0x00f000 Byte 2 of 3
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  8,  8,  8,  8, 126,127,  8,128,129,130, 23,131,
  8,  8,  8,  8,132, 21,133,134, 135,136,  8,137,138,139,140,141,

// state[128 - 2] 0x00fa00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[129 - 2] 0x00fa40 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[130 - 2] 0x00fac0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[131 - 2] 0x00fb00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0,

// state[132 - 2] 0x00fb40 Byte 3 of 3
RJ_,RJ_,  0,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[133 - 2] 0x00fbc0 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[134 - 2] 0x00fd00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,

// state[135 - 2] 0x00fd80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[136 - 2] 0x00fdc0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,

// state[137 - 2] 0x00fe00 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[138 - 2] 0x00fe40 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[139 - 2] 0x00fec0 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,  0,

// state[140 - 2] 0x00ff00 Byte 3 of 3
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,

// state[141 - 2] 0x00ff40 Byte 3 of 3
  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[142 - 2] 0x00ff80 Byte 3 of 3
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,

// state[143 - 2] 0x00ffc0 Byte 3 of 3
  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,   0,  0,RJ_,RJ_,RJ_,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[144 - 2] 0x000000 Byte 2 of 4
X__,X__,X__,X__,X__,X__,X__,X__, X__,X__,X__,X__,X__,X__,X__,X__,
143,  2,155,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,157,  2,  2,
115,115,115,115,115,115,115,115, 115,115,172,  2,  2,  2,  2,174,
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,

// state[145 - 2] 0x010000 Byte 3 of 4
144,145,  8,146,  3,  3,  3,  3,   3,  3,  3,  3,147,148,149,150,
  8,  8,151,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
152,  3,  3,  3,153,  3,  3,  3, 154,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[146 - 2] 0x010000 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,RJ_,RJ_,  0,RJ_,

// state[147 - 2] 0x010040 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[148 - 2] 0x0100c0 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,

// state[149 - 2] 0x010300 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[150 - 2] 0x010340 Byte 4 of 4
RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[151 - 2] 0x010380 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[152 - 2] 0x0103c0 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[153 - 2] 0x010480 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[154 - 2] 0x010800 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_, RJ_,  0,  0,  0,RJ_,  0,  0,RJ_,

// state[155 - 2] 0x010900 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[156 - 2] 0x010a00 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,  0,   0,  0,  0,  0,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_,   0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0, RJ_,RJ_,RJ_,  0,  0,  0,  0,RJ_,

// state[157 - 2] 0x012000 Byte 3 of 4
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,156,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[158 - 2] 0x012340 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[159 - 2] 0x01d000 Byte 3 of 4
  3,  3,  3,  3,  3,158,159,  3,   3,160,  3,  3,  3,  3,  3,  3,
  8,161,162,163,164,165,  8,  8,   8,  8,166,167,168,169,170,171,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[160 - 2] 0x01d140 Byte 4 of 4
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,RJ_,RJ_,RJ_, RJ_,RJ_,  0,  0,  0,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,  0,  0,  0,  0,  0,   0,  0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[161 - 2] 0x01d180 Byte 4 of 4
RJ_,RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,RJ_,RJ_,RJ_,RJ_,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[162 - 2] 0x01d240 Byte 4 of 4
  0,  0,RJ_,RJ_,RJ_,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[163 - 2] 0x01d440 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[164 - 2] 0x01d480 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
  0,  0,RJ_,  0,  0,RJ_,RJ_,  0,   0,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,RJ_,  0,RJ_,RJ_,RJ_,

// state[165 - 2] 0x01d4c0 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[166 - 2] 0x01d500 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_, RJ_,RJ_,RJ_,  0,  0,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,  0,

// state[167 - 2] 0x01d540 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,  0,   0,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[168 - 2] 0x01d680 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,  0, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[169 - 2] 0x01d6c0 Byte 4 of 4
RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_,

// state[170 - 2] 0x01d700 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,  0,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[171 - 2] 0x01d740 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[172 - 2] 0x01d780 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,  0,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,

// state[173 - 2] 0x01d7c0 Byte 4 of 4
RJ_,RJ_,RJ_,  0,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[174 - 2] 0x02a000 Byte 3 of 4
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,   8,  8,  8,173,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[175 - 2] 0x02a6c0 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

// state[176 - 2] 0x02f000 Byte 3 of 4
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  8,  8,  8,  8,  8,  8,  8,  8, 151,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[177 - 2] 0x0c0000 Byte 2 of 4
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
176,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,   2,  2,  2,  2,  2,  2,  2,  2,

// state[178 - 2] 0x0e0000 Byte 3 of 4
  3,  3,  3,  3,  8,  8,  8,177,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,   3,  3,  3,  3,  3,  3,  3,  3,

// state[179 - 2] 0x0e01c0 Byte 4 of 4
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_, RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,RJ_,
  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0,  0,

};

// Remap base[0] = (del, add, string_offset)
static const RemapEntry utf8scannotjustletterspecial_remap_base[] = {
{0,0,0} };

// Remap string[0]
static const unsigned char utf8scannotjustletterspecial_remap_string[] = {
0 };

static const unsigned char utf8scannotjustletterspecial_fast[256] = {
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,1,0,1,0,

0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,0,0,0,0,0,
0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,0,0,0,0,0,

1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

};

static const UTF8ScanObj utf8scannotjustletterspecial_obj = {
  utf8scannotjustletterspecial_STATE0,
  utf8scannotjustletterspecial_STATE0_SIZE,
  utf8scannotjustletterspecial_TOTAL_SIZE,
  utf8scannotjustletterspecial_MAX_EXPAND_X4,
  utf8scannotjustletterspecial_SHIFT,
  utf8scannotjustletterspecial_BYTES,
  utf8scannotjustletterspecial_LOSUB,
  utf8scannotjustletterspecial_HIADD,
  utf8scannotjustletterspecial,
  utf8scannotjustletterspecial_remap_base,
  utf8scannotjustletterspecial_remap_string,
  utf8scannotjustletterspecial_fast
};


#undef X__
#undef RJ_
#undef S1_
#undef S2_
#undef S3_
#undef S21
#undef S31
#undef S32
#undef T1_
#undef T2_
#undef S11
#undef SP_
#undef D__
#undef RJA

// Table has 11776 bytes, Hash = 395A-230B

#endif  // EXPERIMENTAL_USERS_DSITES_LANGDET_UTF8SCANNOTJUSTLETTERSPECIAL_H__
