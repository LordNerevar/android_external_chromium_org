/*
 * fontconfig/fc-case/fccase.tmpl.h
 *
 * Copyright © 2003 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#define FC_NUM_CASE_FOLD	264
#define FC_NUM_CASE_FOLD_CHARS	471
#define FC_MAX_CASE_FOLD_CHARS	6
#define FC_MAX_CASE_FOLD_EXPAND	4
#define FC_MIN_FOLD_CHAR	0x00000041
#define FC_MAX_FOLD_CHAR	0x00010427

static const FcCaseFold    fcCaseFold[FC_NUM_CASE_FOLD] = {
    { 0x00000041, FC_CASE_FOLD_RANGE,    0x001a,     32 },
    { 0x000000b5, FC_CASE_FOLD_RANGE,    0x0001,    775 },
    { 0x000000c0, FC_CASE_FOLD_RANGE,    0x0017,     32 },
    { 0x000000d8, FC_CASE_FOLD_RANGE,    0x0007,     32 },
    { 0x000000df, FC_CASE_FOLD_FULL,     0x0002,      0 },
    { 0x00000100, FC_CASE_FOLD_EVEN_ODD, 0x002f,      1 },
    { 0x00000130, FC_CASE_FOLD_FULL,     0x0003,      2 },
    { 0x00000132, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x00000139, FC_CASE_FOLD_EVEN_ODD, 0x000f,      1 },
    { 0x00000149, FC_CASE_FOLD_FULL,     0x0003,      5 },
    { 0x0000014a, FC_CASE_FOLD_EVEN_ODD, 0x002d,      1 },
    { 0x00000178, FC_CASE_FOLD_RANGE,    0x0001,   -121 },
    { 0x00000179, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x0000017f, FC_CASE_FOLD_RANGE,    0x0001,   -268 },
    { 0x00000181, FC_CASE_FOLD_RANGE,    0x0001,    210 },
    { 0x00000182, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x00000186, FC_CASE_FOLD_RANGE,    0x0001,    206 },
    { 0x00000187, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000189, FC_CASE_FOLD_RANGE,    0x0002,    205 },
    { 0x0000018b, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000018e, FC_CASE_FOLD_RANGE,    0x0001,     79 },
    { 0x0000018f, FC_CASE_FOLD_RANGE,    0x0001,    202 },
    { 0x00000190, FC_CASE_FOLD_RANGE,    0x0001,    203 },
    { 0x00000191, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000193, FC_CASE_FOLD_RANGE,    0x0001,    205 },
    { 0x00000194, FC_CASE_FOLD_RANGE,    0x0001,    207 },
    { 0x00000196, FC_CASE_FOLD_RANGE,    0x0001,    211 },
    { 0x00000197, FC_CASE_FOLD_RANGE,    0x0001,    209 },
    { 0x00000198, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000019c, FC_CASE_FOLD_RANGE,    0x0001,    211 },
    { 0x0000019d, FC_CASE_FOLD_RANGE,    0x0001,    213 },
    { 0x0000019f, FC_CASE_FOLD_RANGE,    0x0001,    214 },
    { 0x000001a0, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x000001a6, FC_CASE_FOLD_RANGE,    0x0001,    218 },
    { 0x000001a7, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001a9, FC_CASE_FOLD_RANGE,    0x0001,    218 },
    { 0x000001ac, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001ae, FC_CASE_FOLD_RANGE,    0x0001,    218 },
    { 0x000001af, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001b1, FC_CASE_FOLD_RANGE,    0x0002,    217 },
    { 0x000001b3, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x000001b7, FC_CASE_FOLD_RANGE,    0x0001,    219 },
    { 0x000001b8, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001bc, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001c4, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001c5, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001c7, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001c8, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001ca, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001cb, FC_CASE_FOLD_EVEN_ODD, 0x0011,      1 },
    { 0x000001de, FC_CASE_FOLD_EVEN_ODD, 0x0011,      1 },
    { 0x000001f0, FC_CASE_FOLD_FULL,     0x0003,      8 },
    { 0x000001f1, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001f2, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x000001f6, FC_CASE_FOLD_RANGE,    0x0001,    -97 },
    { 0x000001f7, FC_CASE_FOLD_RANGE,    0x0001,    -56 },
    { 0x000001f8, FC_CASE_FOLD_EVEN_ODD, 0x0027,      1 },
    { 0x00000220, FC_CASE_FOLD_RANGE,    0x0001,   -130 },
    { 0x00000222, FC_CASE_FOLD_EVEN_ODD, 0x0011,      1 },
    { 0x0000023a, FC_CASE_FOLD_RANGE,    0x0001,  10795 },
    { 0x0000023b, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000023d, FC_CASE_FOLD_RANGE,    0x0001,   -163 },
    { 0x0000023e, FC_CASE_FOLD_RANGE,    0x0001,  10792 },
    { 0x00000241, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000243, FC_CASE_FOLD_RANGE,    0x0001,   -195 },
    { 0x00000244, FC_CASE_FOLD_RANGE,    0x0001,     69 },
    { 0x00000245, FC_CASE_FOLD_RANGE,    0x0001,     71 },
    { 0x00000246, FC_CASE_FOLD_EVEN_ODD, 0x0009,      1 },
    { 0x00000345, FC_CASE_FOLD_RANGE,    0x0001,    116 },
    { 0x00000370, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x00000376, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000386, FC_CASE_FOLD_RANGE,    0x0001,     38 },
    { 0x00000388, FC_CASE_FOLD_RANGE,    0x0003,     37 },
    { 0x0000038c, FC_CASE_FOLD_RANGE,    0x0001,     64 },
    { 0x0000038e, FC_CASE_FOLD_RANGE,    0x0002,     63 },
    { 0x00000390, FC_CASE_FOLD_FULL,     0x0006,     11 },
    { 0x00000391, FC_CASE_FOLD_RANGE,    0x0011,     32 },
    { 0x000003a3, FC_CASE_FOLD_RANGE,    0x0009,     32 },
    { 0x000003b0, FC_CASE_FOLD_FULL,     0x0006,     17 },
    { 0x000003c2, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000003cf, FC_CASE_FOLD_RANGE,    0x0001,      8 },
    { 0x000003d0, FC_CASE_FOLD_RANGE,    0x0001,    -30 },
    { 0x000003d1, FC_CASE_FOLD_RANGE,    0x0001,    -25 },
    { 0x000003d5, FC_CASE_FOLD_RANGE,    0x0001,    -15 },
    { 0x000003d6, FC_CASE_FOLD_RANGE,    0x0001,    -22 },
    { 0x000003d8, FC_CASE_FOLD_EVEN_ODD, 0x0017,      1 },
    { 0x000003f0, FC_CASE_FOLD_RANGE,    0x0001,    -54 },
    { 0x000003f1, FC_CASE_FOLD_RANGE,    0x0001,    -48 },
    { 0x000003f4, FC_CASE_FOLD_RANGE,    0x0001,    -60 },
    { 0x000003f5, FC_CASE_FOLD_RANGE,    0x0001,    -64 },
    { 0x000003f7, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000003f9, FC_CASE_FOLD_RANGE,    0x0001,     -7 },
    { 0x000003fa, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000003fd, FC_CASE_FOLD_RANGE,    0x0003,   -130 },
    { 0x00000400, FC_CASE_FOLD_RANGE,    0x0010,     80 },
    { 0x00000410, FC_CASE_FOLD_RANGE,    0x0020,     32 },
    { 0x00000460, FC_CASE_FOLD_EVEN_ODD, 0x0021,      1 },
    { 0x0000048a, FC_CASE_FOLD_EVEN_ODD, 0x0035,      1 },
    { 0x000004c0, FC_CASE_FOLD_RANGE,    0x0001,     15 },
    { 0x000004c1, FC_CASE_FOLD_EVEN_ODD, 0x000d,      1 },
    { 0x000004d0, FC_CASE_FOLD_EVEN_ODD, 0x0057,      1 },
    { 0x00000531, FC_CASE_FOLD_RANGE,    0x0026,     48 },
    { 0x00000587, FC_CASE_FOLD_FULL,     0x0004,     23 },
    { 0x000010a0, FC_CASE_FOLD_RANGE,    0x0026,   7264 },
    { 0x000010c7, FC_CASE_FOLD_RANGE,    0x0001,   7264 },
    { 0x000010cd, FC_CASE_FOLD_RANGE,    0x0001,   7264 },
    { 0x00001e00, FC_CASE_FOLD_EVEN_ODD, 0x0095,      1 },
    { 0x00001e96, FC_CASE_FOLD_FULL,     0x0003,     27 },
    { 0x00001e97, FC_CASE_FOLD_FULL,     0x0003,     30 },
    { 0x00001e98, FC_CASE_FOLD_FULL,     0x0003,     33 },
    { 0x00001e99, FC_CASE_FOLD_FULL,     0x0003,     36 },
    { 0x00001e9a, FC_CASE_FOLD_FULL,     0x0003,     39 },
    { 0x00001e9b, FC_CASE_FOLD_RANGE,    0x0001,    -58 },
    { 0x00001e9e, FC_CASE_FOLD_FULL,     0x0002,     42 },
    { 0x00001ea0, FC_CASE_FOLD_EVEN_ODD, 0x005f,      1 },
    { 0x00001f08, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f18, FC_CASE_FOLD_RANGE,    0x0006,     -8 },
    { 0x00001f28, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f38, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f48, FC_CASE_FOLD_RANGE,    0x0006,     -8 },
    { 0x00001f50, FC_CASE_FOLD_FULL,     0x0004,     44 },
    { 0x00001f52, FC_CASE_FOLD_FULL,     0x0006,     48 },
    { 0x00001f54, FC_CASE_FOLD_FULL,     0x0006,     54 },
    { 0x00001f56, FC_CASE_FOLD_FULL,     0x0006,     60 },
    { 0x00001f59, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f5b, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f5d, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f5f, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f68, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f80, FC_CASE_FOLD_FULL,     0x0005,     66 },
    { 0x00001f81, FC_CASE_FOLD_FULL,     0x0005,     71 },
    { 0x00001f82, FC_CASE_FOLD_FULL,     0x0005,     76 },
    { 0x00001f83, FC_CASE_FOLD_FULL,     0x0005,     81 },
    { 0x00001f84, FC_CASE_FOLD_FULL,     0x0005,     86 },
    { 0x00001f85, FC_CASE_FOLD_FULL,     0x0005,     91 },
    { 0x00001f86, FC_CASE_FOLD_FULL,     0x0005,     96 },
    { 0x00001f87, FC_CASE_FOLD_FULL,     0x0005,    101 },
    { 0x00001f88, FC_CASE_FOLD_FULL,     0x0005,    106 },
    { 0x00001f89, FC_CASE_FOLD_FULL,     0x0005,    111 },
    { 0x00001f8a, FC_CASE_FOLD_FULL,     0x0005,    116 },
    { 0x00001f8b, FC_CASE_FOLD_FULL,     0x0005,    121 },
    { 0x00001f8c, FC_CASE_FOLD_FULL,     0x0005,    126 },
    { 0x00001f8d, FC_CASE_FOLD_FULL,     0x0005,    131 },
    { 0x00001f8e, FC_CASE_FOLD_FULL,     0x0005,    136 },
    { 0x00001f8f, FC_CASE_FOLD_FULL,     0x0005,    141 },
    { 0x00001f90, FC_CASE_FOLD_FULL,     0x0005,    146 },
    { 0x00001f91, FC_CASE_FOLD_FULL,     0x0005,    151 },
    { 0x00001f92, FC_CASE_FOLD_FULL,     0x0005,    156 },
    { 0x00001f93, FC_CASE_FOLD_FULL,     0x0005,    161 },
    { 0x00001f94, FC_CASE_FOLD_FULL,     0x0005,    166 },
    { 0x00001f95, FC_CASE_FOLD_FULL,     0x0005,    171 },
    { 0x00001f96, FC_CASE_FOLD_FULL,     0x0005,    176 },
    { 0x00001f97, FC_CASE_FOLD_FULL,     0x0005,    181 },
    { 0x00001f98, FC_CASE_FOLD_FULL,     0x0005,    186 },
    { 0x00001f99, FC_CASE_FOLD_FULL,     0x0005,    191 },
    { 0x00001f9a, FC_CASE_FOLD_FULL,     0x0005,    196 },
    { 0x00001f9b, FC_CASE_FOLD_FULL,     0x0005,    201 },
    { 0x00001f9c, FC_CASE_FOLD_FULL,     0x0005,    206 },
    { 0x00001f9d, FC_CASE_FOLD_FULL,     0x0005,    211 },
    { 0x00001f9e, FC_CASE_FOLD_FULL,     0x0005,    216 },
    { 0x00001f9f, FC_CASE_FOLD_FULL,     0x0005,    221 },
    { 0x00001fa0, FC_CASE_FOLD_FULL,     0x0005,    226 },
    { 0x00001fa1, FC_CASE_FOLD_FULL,     0x0005,    231 },
    { 0x00001fa2, FC_CASE_FOLD_FULL,     0x0005,    236 },
    { 0x00001fa3, FC_CASE_FOLD_FULL,     0x0005,    241 },
    { 0x00001fa4, FC_CASE_FOLD_FULL,     0x0005,    246 },
    { 0x00001fa5, FC_CASE_FOLD_FULL,     0x0005,    251 },
    { 0x00001fa6, FC_CASE_FOLD_FULL,     0x0005,    256 },
    { 0x00001fa7, FC_CASE_FOLD_FULL,     0x0005,    261 },
    { 0x00001fa8, FC_CASE_FOLD_FULL,     0x0005,    266 },
    { 0x00001fa9, FC_CASE_FOLD_FULL,     0x0005,    271 },
    { 0x00001faa, FC_CASE_FOLD_FULL,     0x0005,    276 },
    { 0x00001fab, FC_CASE_FOLD_FULL,     0x0005,    281 },
    { 0x00001fac, FC_CASE_FOLD_FULL,     0x0005,    286 },
    { 0x00001fad, FC_CASE_FOLD_FULL,     0x0005,    291 },
    { 0x00001fae, FC_CASE_FOLD_FULL,     0x0005,    296 },
    { 0x00001faf, FC_CASE_FOLD_FULL,     0x0005,    301 },
    { 0x00001fb2, FC_CASE_FOLD_FULL,     0x0005,    306 },
    { 0x00001fb3, FC_CASE_FOLD_FULL,     0x0004,    311 },
    { 0x00001fb4, FC_CASE_FOLD_FULL,     0x0004,    315 },
    { 0x00001fb6, FC_CASE_FOLD_FULL,     0x0004,    319 },
    { 0x00001fb7, FC_CASE_FOLD_FULL,     0x0006,    323 },
    { 0x00001fb8, FC_CASE_FOLD_RANGE,    0x0002,     -8 },
    { 0x00001fba, FC_CASE_FOLD_RANGE,    0x0002,    -74 },
    { 0x00001fbc, FC_CASE_FOLD_FULL,     0x0004,    329 },
    { 0x00001fbe, FC_CASE_FOLD_RANGE,    0x0001,  -7173 },
    { 0x00001fc2, FC_CASE_FOLD_FULL,     0x0005,    333 },
    { 0x00001fc3, FC_CASE_FOLD_FULL,     0x0004,    338 },
    { 0x00001fc4, FC_CASE_FOLD_FULL,     0x0004,    342 },
    { 0x00001fc6, FC_CASE_FOLD_FULL,     0x0004,    346 },
    { 0x00001fc7, FC_CASE_FOLD_FULL,     0x0006,    350 },
    { 0x00001fc8, FC_CASE_FOLD_RANGE,    0x0004,    -86 },
    { 0x00001fcc, FC_CASE_FOLD_FULL,     0x0004,    356 },
    { 0x00001fd2, FC_CASE_FOLD_FULL,     0x0006,    360 },
    { 0x00001fd3, FC_CASE_FOLD_FULL,     0x0006,    366 },
    { 0x00001fd6, FC_CASE_FOLD_FULL,     0x0004,    372 },
    { 0x00001fd7, FC_CASE_FOLD_FULL,     0x0006,    376 },
    { 0x00001fd8, FC_CASE_FOLD_RANGE,    0x0002,     -8 },
    { 0x00001fda, FC_CASE_FOLD_RANGE,    0x0002,   -100 },
    { 0x00001fe2, FC_CASE_FOLD_FULL,     0x0006,    382 },
    { 0x00001fe3, FC_CASE_FOLD_FULL,     0x0006,    388 },
    { 0x00001fe4, FC_CASE_FOLD_FULL,     0x0004,    394 },
    { 0x00001fe6, FC_CASE_FOLD_FULL,     0x0004,    398 },
    { 0x00001fe7, FC_CASE_FOLD_FULL,     0x0006,    402 },
    { 0x00001fe8, FC_CASE_FOLD_RANGE,    0x0002,     -8 },
    { 0x00001fea, FC_CASE_FOLD_RANGE,    0x0002,   -112 },
    { 0x00001fec, FC_CASE_FOLD_RANGE,    0x0001,     -7 },
    { 0x00001ff2, FC_CASE_FOLD_FULL,     0x0005,    408 },
    { 0x00001ff3, FC_CASE_FOLD_FULL,     0x0004,    413 },
    { 0x00001ff4, FC_CASE_FOLD_FULL,     0x0004,    417 },
    { 0x00001ff6, FC_CASE_FOLD_FULL,     0x0004,    421 },
    { 0x00001ff7, FC_CASE_FOLD_FULL,     0x0006,    425 },
    { 0x00001ff8, FC_CASE_FOLD_RANGE,    0x0002,   -128 },
    { 0x00001ffa, FC_CASE_FOLD_RANGE,    0x0002,   -126 },
    { 0x00001ffc, FC_CASE_FOLD_FULL,     0x0004,    431 },
    { 0x00002126, FC_CASE_FOLD_RANGE,    0x0001,  -7517 },
    { 0x0000212a, FC_CASE_FOLD_RANGE,    0x0001,  -8383 },
    { 0x0000212b, FC_CASE_FOLD_RANGE,    0x0001,  -8262 },
    { 0x00002132, FC_CASE_FOLD_RANGE,    0x0001,     28 },
    { 0x00002160, FC_CASE_FOLD_RANGE,    0x0010,     16 },
    { 0x00002183, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000024b6, FC_CASE_FOLD_RANGE,    0x001a,     26 },
    { 0x00002c00, FC_CASE_FOLD_RANGE,    0x002f,     48 },
    { 0x00002c60, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00002c62, FC_CASE_FOLD_RANGE,    0x0001, -10743 },
    { 0x00002c63, FC_CASE_FOLD_RANGE,    0x0001,  -3814 },
    { 0x00002c64, FC_CASE_FOLD_RANGE,    0x0001, -10727 },
    { 0x00002c67, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x00002c6d, FC_CASE_FOLD_RANGE,    0x0001, -10780 },
    { 0x00002c6e, FC_CASE_FOLD_RANGE,    0x0001, -10749 },
    { 0x00002c6f, FC_CASE_FOLD_RANGE,    0x0001, -10783 },
    { 0x00002c70, FC_CASE_FOLD_RANGE,    0x0001, -10782 },
    { 0x00002c72, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00002c75, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00002c7e, FC_CASE_FOLD_RANGE,    0x0002, -10815 },
    { 0x00002c80, FC_CASE_FOLD_EVEN_ODD, 0x0063,      1 },
    { 0x00002ceb, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x00002cf2, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000a640, FC_CASE_FOLD_EVEN_ODD, 0x002d,      1 },
    { 0x0000a680, FC_CASE_FOLD_EVEN_ODD, 0x0017,      1 },
    { 0x0000a722, FC_CASE_FOLD_EVEN_ODD, 0x000d,      1 },
    { 0x0000a732, FC_CASE_FOLD_EVEN_ODD, 0x003d,      1 },
    { 0x0000a779, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x0000a77d, FC_CASE_FOLD_RANGE,    0x0001,  30204 },
    { 0x0000a77e, FC_CASE_FOLD_EVEN_ODD, 0x0009,      1 },
    { 0x0000a78b, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000a78d, FC_CASE_FOLD_RANGE,    0x0001,  23256 },
    { 0x0000a790, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x0000a7a0, FC_CASE_FOLD_EVEN_ODD, 0x0009,      1 },
    { 0x0000a7aa, FC_CASE_FOLD_RANGE,    0x0001,  23228 },
    { 0x0000fb00, FC_CASE_FOLD_FULL,     0x0002,    435 },
    { 0x0000fb01, FC_CASE_FOLD_FULL,     0x0002,    437 },
    { 0x0000fb02, FC_CASE_FOLD_FULL,     0x0002,    439 },
    { 0x0000fb03, FC_CASE_FOLD_FULL,     0x0003,    441 },
    { 0x0000fb04, FC_CASE_FOLD_FULL,     0x0003,    444 },
    { 0x0000fb05, FC_CASE_FOLD_FULL,     0x0002,    447 },
    { 0x0000fb06, FC_CASE_FOLD_FULL,     0x0002,    449 },
    { 0x0000fb13, FC_CASE_FOLD_FULL,     0x0004,    451 },
    { 0x0000fb14, FC_CASE_FOLD_FULL,     0x0004,    455 },
    { 0x0000fb15, FC_CASE_FOLD_FULL,     0x0004,    459 },
    { 0x0000fb16, FC_CASE_FOLD_FULL,     0x0004,    463 },
    { 0x0000fb17, FC_CASE_FOLD_FULL,     0x0004,    467 },
    { 0x0000ff21, FC_CASE_FOLD_RANGE,    0x001a,     32 },
    { 0x00010400, FC_CASE_FOLD_RANGE,    0x0028,     40 },
};

static const FcChar8	fcCaseFoldChars[FC_NUM_CASE_FOLD_CHARS] = {
0x73,0x73,0x69,0xcc,0x87,0xca,0xbc,0x6e,0x6a,0xcc,0x8c,0xce,0xb9,0xcc,0x88,0xcc,
0x81,0xcf,0x85,0xcc,0x88,0xcc,0x81,0xd5,0xa5,0xd6,0x82,0x68,0xcc,0xb1,0x74,0xcc,
0x88,0x77,0xcc,0x8a,0x79,0xcc,0x8a,0x61,0xca,0xbe,0x73,0x73,0xcf,0x85,0xcc,0x93,
0xcf,0x85,0xcc,0x93,0xcc,0x80,0xcf,0x85,0xcc,0x93,0xcc,0x81,0xcf,0x85,0xcc,0x93,
0xcd,0x82,0xe1,0xbc,0x80,0xce,0xb9,0xe1,0xbc,0x81,0xce,0xb9,0xe1,0xbc,0x82,0xce,
0xb9,0xe1,0xbc,0x83,0xce,0xb9,0xe1,0xbc,0x84,0xce,0xb9,0xe1,0xbc,0x85,0xce,0xb9,
0xe1,0xbc,0x86,0xce,0xb9,0xe1,0xbc,0x87,0xce,0xb9,0xe1,0xbc,0x80,0xce,0xb9,0xe1,
0xbc,0x81,0xce,0xb9,0xe1,0xbc,0x82,0xce,0xb9,0xe1,0xbc,0x83,0xce,0xb9,0xe1,0xbc,
0x84,0xce,0xb9,0xe1,0xbc,0x85,0xce,0xb9,0xe1,0xbc,0x86,0xce,0xb9,0xe1,0xbc,0x87,
0xce,0xb9,0xe1,0xbc,0xa0,0xce,0xb9,0xe1,0xbc,0xa1,0xce,0xb9,0xe1,0xbc,0xa2,0xce,
0xb9,0xe1,0xbc,0xa3,0xce,0xb9,0xe1,0xbc,0xa4,0xce,0xb9,0xe1,0xbc,0xa5,0xce,0xb9,
0xe1,0xbc,0xa6,0xce,0xb9,0xe1,0xbc,0xa7,0xce,0xb9,0xe1,0xbc,0xa0,0xce,0xb9,0xe1,
0xbc,0xa1,0xce,0xb9,0xe1,0xbc,0xa2,0xce,0xb9,0xe1,0xbc,0xa3,0xce,0xb9,0xe1,0xbc,
0xa4,0xce,0xb9,0xe1,0xbc,0xa5,0xce,0xb9,0xe1,0xbc,0xa6,0xce,0xb9,0xe1,0xbc,0xa7,
0xce,0xb9,0xe1,0xbd,0xa0,0xce,0xb9,0xe1,0xbd,0xa1,0xce,0xb9,0xe1,0xbd,0xa2,0xce,
0xb9,0xe1,0xbd,0xa3,0xce,0xb9,0xe1,0xbd,0xa4,0xce,0xb9,0xe1,0xbd,0xa5,0xce,0xb9,
0xe1,0xbd,0xa6,0xce,0xb9,0xe1,0xbd,0xa7,0xce,0xb9,0xe1,0xbd,0xa0,0xce,0xb9,0xe1,
0xbd,0xa1,0xce,0xb9,0xe1,0xbd,0xa2,0xce,0xb9,0xe1,0xbd,0xa3,0xce,0xb9,0xe1,0xbd,
0xa4,0xce,0xb9,0xe1,0xbd,0xa5,0xce,0xb9,0xe1,0xbd,0xa6,0xce,0xb9,0xe1,0xbd,0xa7,
0xce,0xb9,0xe1,0xbd,0xb0,0xce,0xb9,0xce,0xb1,0xce,0xb9,0xce,0xac,0xce,0xb9,0xce,
0xb1,0xcd,0x82,0xce,0xb1,0xcd,0x82,0xce,0xb9,0xce,0xb1,0xce,0xb9,0xe1,0xbd,0xb4,
0xce,0xb9,0xce,0xb7,0xce,0xb9,0xce,0xae,0xce,0xb9,0xce,0xb7,0xcd,0x82,0xce,0xb7,
0xcd,0x82,0xce,0xb9,0xce,0xb7,0xce,0xb9,0xce,0xb9,0xcc,0x88,0xcc,0x80,0xce,0xb9,
0xcc,0x88,0xcc,0x81,0xce,0xb9,0xcd,0x82,0xce,0xb9,0xcc,0x88,0xcd,0x82,0xcf,0x85,
0xcc,0x88,0xcc,0x80,0xcf,0x85,0xcc,0x88,0xcc,0x81,0xcf,0x81,0xcc,0x93,0xcf,0x85,
0xcd,0x82,0xcf,0x85,0xcc,0x88,0xcd,0x82,0xe1,0xbd,0xbc,0xce,0xb9,0xcf,0x89,0xce,
0xb9,0xcf,0x8e,0xce,0xb9,0xcf,0x89,0xcd,0x82,0xcf,0x89,0xcd,0x82,0xce,0xb9,0xcf,
0x89,0xce,0xb9,0x66,0x66,0x66,0x69,0x66,0x6c,0x66,0x66,0x69,0x66,0x66,0x6c,0x73,
0x74,0x73,0x74,0xd5,0xb4,0xd5,0xb6,0xd5,0xb4,0xd5,0xa5,0xd5,0xb4,0xd5,0xab,0xd5,
0xbe,0xd5,0xb6,0xd5,0xb4,0xd5,0xad
};
