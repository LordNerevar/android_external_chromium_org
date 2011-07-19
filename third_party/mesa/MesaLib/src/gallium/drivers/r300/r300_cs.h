/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

/**
 * This file contains macros for immediate command submission.
 */

#ifndef R300_CS_H
#define R300_CS_H

#include "r300_reg.h"
#include "r300_context.h"
#include "r300_winsys.h"

/* Yes, I know macros are ugly. However, they are much prettier than the code
 * that they neatly hide away, and don't have the cost of function setup,so
 * we're going to use them. */

#ifdef DEBUG
#define CS_DEBUG(x) x
#else
#define CS_DEBUG(x)
#endif

/**
 * Command submission setup.
 */

#define CS_LOCALS(context) \
    struct r300_winsys_cs *cs_copy = (context)->cs; \
    struct r300_winsys_screen *cs_winsys = (context)->rws; \
    int cs_count = 0; (void) cs_count; (void) cs_winsys;

#define BEGIN_CS(size) do { \
    assert(size <= (cs_copy->ndw - cs_copy->cdw)); \
    CS_DEBUG(cs_count = size;) \
} while (0)

#ifdef DEBUG
#define END_CS do { \
    if (cs_count != 0) \
        debug_printf("r300: Warning: cs_count off by %d at (%s, %s:%i)\n", \
                     cs_count, __FUNCTION__, __FILE__, __LINE__); \
    cs_count = 0; \
} while (0)
#else
#define END_CS
#endif


/**
 * Writing pure DWORDs.
 */

#define OUT_CS(value) do { \
    cs_copy->ptr[cs_copy->cdw++] = (value); \
    CS_DEBUG(cs_count--;) \
} while (0)

#define OUT_CS_32F(value) \
    OUT_CS(fui(value))

#define OUT_CS_REG(register, value) do { \
    OUT_CS(CP_PACKET0(register, 0)); \
    OUT_CS(value); \
} while (0)

/* Note: This expects count to be the number of registers,
 * not the actual packet0 count! */
#define OUT_CS_REG_SEQ(register, count) \
    OUT_CS(CP_PACKET0((register), ((count) - 1)))

#define OUT_CS_ONE_REG(register, count) \
    OUT_CS(CP_PACKET0((register), ((count) - 1)) | RADEON_ONE_REG_WR)

#define OUT_CS_PKT3(op, count) \
    OUT_CS(CP_PACKET3(op, count))

#define OUT_CS_TABLE(values, count) do { \
    memcpy(cs_copy->ptr + cs_copy->cdw, values, count * 4); \
    cs_copy->cdw += count; \
    CS_DEBUG(cs_count -= count;) \
} while (0)


/**
 * Writing relocations.
 */

#define OUT_CS_RELOC(bo, offset, rd, wd) do { \
    assert(bo); \
    OUT_CS(offset); \
    cs_winsys->cs_write_reloc(cs_copy, bo, rd, wd); \
    CS_DEBUG(cs_count -= 2;) \
} while (0)

#define OUT_CS_BUF_RELOC(bo, offset, rd, wd) do { \
    assert(bo); \
    OUT_CS_RELOC(r300_buffer(bo)->buf, offset, rd, wd); \
} while (0)

#define OUT_CS_TEX_RELOC(tex, offset, rd, wd) do { \
    assert(tex); \
    OUT_CS_RELOC(tex->buffer, offset, rd, wd); \
} while (0)

#define OUT_CS_BUF_RELOC_NO_OFFSET(bo, rd, wd) do { \
    assert(bo); \
    cs_winsys->cs_write_reloc(cs_copy, r300_buffer(bo)->buf, rd, wd); \
    CS_DEBUG(cs_count -= 2;) \
} while (0)


/**
 * Command buffer emission.
 */

#define WRITE_CS_TABLE(values, count) do { \
    CS_DEBUG(assert(cs_count == 0);) \
    memcpy(cs_copy->ptr + cs_copy->cdw, (values), (count) * 4); \
    cs_copy->cdw += (count); \
} while (0)

#endif /* R300_CS_H */
