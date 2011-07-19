/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
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

#include "draw/draw_context.h"

#include "util/u_framebuffer.h"
#include "util/u_math.h"
#include "util/u_mm.h"
#include "util/u_memory.h"
#include "util/u_pack_color.h"

#include "tgsi/tgsi_parse.h"

#include "pipe/p_config.h"

#include "r300_cb.h"
#include "r300_context.h"
#include "r300_emit.h"
#include "r300_reg.h"
#include "r300_screen.h"
#include "r300_screen_buffer.h"
#include "r300_state_inlines.h"
#include "r300_fs.h"
#include "r300_texture.h"
#include "r300_vs.h"
#include "r300_winsys.h"
#include "r300_hyperz.h"

/* r300_state: Functions used to intialize state context by translating
 * Gallium state objects into semi-native r300 state objects. */

#define UPDATE_STATE(cso, atom) \
    if (cso != atom.state) { \
        atom.state = cso;    \
        atom.dirty = TRUE;   \
    }

static boolean blend_discard_if_src_alpha_0(unsigned srcRGB, unsigned srcA,
                                            unsigned dstRGB, unsigned dstA)
{
    /* If the blend equation is ADD or REVERSE_SUBTRACT,
     * SRC_ALPHA == 0, and the following state is set, the colorbuffer
     * will not be changed.
     * Notice that the dst factors are the src factors inverted. */
    return (srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
            srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE ||
            srcRGB == PIPE_BLENDFACTOR_ZERO) &&
           (srcA == PIPE_BLENDFACTOR_SRC_COLOR ||
            srcA == PIPE_BLENDFACTOR_SRC_ALPHA ||
            srcA == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE ||
            srcA == PIPE_BLENDFACTOR_ZERO) &&
           (dstRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            dstRGB == PIPE_BLENDFACTOR_ONE) &&
           (dstA == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            dstA == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            dstA == PIPE_BLENDFACTOR_ONE);
}

static boolean blend_discard_if_src_alpha_1(unsigned srcRGB, unsigned srcA,
                                            unsigned dstRGB, unsigned dstA)
{
    /* If the blend equation is ADD or REVERSE_SUBTRACT,
     * SRC_ALPHA == 1, and the following state is set, the colorbuffer
     * will not be changed.
     * Notice that the dst factors are the src factors inverted. */
    return (srcRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            srcRGB == PIPE_BLENDFACTOR_ZERO) &&
           (srcA == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            srcA == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            srcA == PIPE_BLENDFACTOR_ZERO) &&
           (dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
            dstRGB == PIPE_BLENDFACTOR_ONE) &&
           (dstA == PIPE_BLENDFACTOR_SRC_COLOR ||
            dstA == PIPE_BLENDFACTOR_SRC_ALPHA ||
            dstA == PIPE_BLENDFACTOR_ONE);
}

static boolean blend_discard_if_src_color_0(unsigned srcRGB, unsigned srcA,
                                            unsigned dstRGB, unsigned dstA)
{
    /* If the blend equation is ADD or REVERSE_SUBTRACT,
     * SRC_COLOR == (0,0,0), and the following state is set, the colorbuffer
     * will not be changed.
     * Notice that the dst factors are the src factors inverted. */
    return (srcRGB == PIPE_BLENDFACTOR_SRC_COLOR ||
            srcRGB == PIPE_BLENDFACTOR_ZERO) &&
           (srcA == PIPE_BLENDFACTOR_ZERO) &&
           (dstRGB == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            dstRGB == PIPE_BLENDFACTOR_ONE) &&
           (dstA == PIPE_BLENDFACTOR_ONE);
}

static boolean blend_discard_if_src_color_1(unsigned srcRGB, unsigned srcA,
                                            unsigned dstRGB, unsigned dstA)
{
    /* If the blend equation is ADD or REVERSE_SUBTRACT,
     * SRC_COLOR == (1,1,1), and the following state is set, the colorbuffer
     * will not be changed.
     * Notice that the dst factors are the src factors inverted. */
    return (srcRGB == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            srcRGB == PIPE_BLENDFACTOR_ZERO) &&
           (srcA == PIPE_BLENDFACTOR_ZERO) &&
           (dstRGB == PIPE_BLENDFACTOR_SRC_COLOR ||
            dstRGB == PIPE_BLENDFACTOR_ONE) &&
           (dstA == PIPE_BLENDFACTOR_ONE);
}

static boolean blend_discard_if_src_alpha_color_0(unsigned srcRGB, unsigned srcA,
                                                  unsigned dstRGB, unsigned dstA)
{
    /* If the blend equation is ADD or REVERSE_SUBTRACT,
     * SRC_ALPHA_COLOR == (0,0,0,0), and the following state is set,
     * the colorbuffer will not be changed.
     * Notice that the dst factors are the src factors inverted. */
    return (srcRGB == PIPE_BLENDFACTOR_SRC_COLOR ||
            srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
            srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE ||
            srcRGB == PIPE_BLENDFACTOR_ZERO) &&
           (srcA == PIPE_BLENDFACTOR_SRC_COLOR ||
            srcA == PIPE_BLENDFACTOR_SRC_ALPHA ||
            srcA == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE ||
            srcA == PIPE_BLENDFACTOR_ZERO) &&
           (dstRGB == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            dstRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            dstRGB == PIPE_BLENDFACTOR_ONE) &&
           (dstA == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            dstA == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            dstA == PIPE_BLENDFACTOR_ONE);
}

static boolean blend_discard_if_src_alpha_color_1(unsigned srcRGB, unsigned srcA,
                                                  unsigned dstRGB, unsigned dstA)
{
    /* If the blend equation is ADD or REVERSE_SUBTRACT,
     * SRC_ALPHA_COLOR == (1,1,1,1), and the following state is set,
     * the colorbuffer will not be changed.
     * Notice that the dst factors are the src factors inverted. */
    return (srcRGB == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            srcRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            srcRGB == PIPE_BLENDFACTOR_ZERO) &&
           (srcA == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
            srcA == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
            srcA == PIPE_BLENDFACTOR_ZERO) &&
           (dstRGB == PIPE_BLENDFACTOR_SRC_COLOR ||
            dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
            dstRGB == PIPE_BLENDFACTOR_ONE) &&
           (dstA == PIPE_BLENDFACTOR_SRC_COLOR ||
            dstA == PIPE_BLENDFACTOR_SRC_ALPHA ||
            dstA == PIPE_BLENDFACTOR_ONE);
}

static unsigned bgra_cmask(unsigned mask)
{
    /* Gallium uses RGBA color ordering while R300 expects BGRA. */

    return ((mask & PIPE_MASK_R) << 2) |
           ((mask & PIPE_MASK_B) >> 2) |
           (mask & (PIPE_MASK_G | PIPE_MASK_A));
}

/* Create a new blend state based on the CSO blend state.
 *
 * This encompasses alpha blending, logic/raster ops, and blend dithering. */
static void* r300_create_blend_state(struct pipe_context* pipe,
                                     const struct pipe_blend_state* state)
{
    struct r300_screen* r300screen = r300_screen(pipe->screen);
    struct r300_blend_state* blend = CALLOC_STRUCT(r300_blend_state);
    uint32_t blend_control = 0;       /* R300_RB3D_CBLEND: 0x4e04 */
    uint32_t alpha_blend_control = 0; /* R300_RB3D_ABLEND: 0x4e08 */
    uint32_t color_channel_mask = 0;  /* R300_RB3D_COLOR_CHANNEL_MASK: 0x4e0c */
    uint32_t rop = 0;                 /* R300_RB3D_ROPCNTL: 0x4e18 */
    uint32_t dither = 0;              /* R300_RB3D_DITHER_CTL: 0x4e50 */
    CB_LOCALS;

    if (state->rt[0].blend_enable)
    {
        unsigned eqRGB = state->rt[0].rgb_func;
        unsigned srcRGB = state->rt[0].rgb_src_factor;
        unsigned dstRGB = state->rt[0].rgb_dst_factor;

        unsigned eqA = state->rt[0].alpha_func;
        unsigned srcA = state->rt[0].alpha_src_factor;
        unsigned dstA = state->rt[0].alpha_dst_factor;

        /* despite the name, ALPHA_BLEND_ENABLE has nothing to do with alpha,
         * this is just the crappy D3D naming */
        blend_control = R300_ALPHA_BLEND_ENABLE |
            r300_translate_blend_function(eqRGB) |
            ( r300_translate_blend_factor(srcRGB) << R300_SRC_BLEND_SHIFT) |
            ( r300_translate_blend_factor(dstRGB) << R300_DST_BLEND_SHIFT);

        /* Optimization: some operations do not require the destination color.
         *
         * When SRC_ALPHA_SATURATE is used, colorbuffer reads must be enabled,
         * otherwise blending gives incorrect results. It seems to be
         * a hardware bug. */
        if (eqRGB == PIPE_BLEND_MIN || eqA == PIPE_BLEND_MIN ||
            eqRGB == PIPE_BLEND_MAX || eqA == PIPE_BLEND_MAX ||
            dstRGB != PIPE_BLENDFACTOR_ZERO ||
            dstA != PIPE_BLENDFACTOR_ZERO ||
            srcRGB == PIPE_BLENDFACTOR_DST_COLOR ||
            srcRGB == PIPE_BLENDFACTOR_DST_ALPHA ||
            srcRGB == PIPE_BLENDFACTOR_INV_DST_COLOR ||
            srcRGB == PIPE_BLENDFACTOR_INV_DST_ALPHA ||
            srcA == PIPE_BLENDFACTOR_DST_COLOR ||
            srcA == PIPE_BLENDFACTOR_DST_ALPHA ||
            srcA == PIPE_BLENDFACTOR_INV_DST_COLOR ||
            srcA == PIPE_BLENDFACTOR_INV_DST_ALPHA ||
            srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE) {
            /* Enable reading from the colorbuffer. */
            blend_control |= R300_READ_ENABLE;

            if (r300screen->caps.is_r500) {
                /* Optimization: Depending on incoming pixels, we can
                 * conditionally disable the reading in hardware... */
                if (eqRGB != PIPE_BLEND_MIN && eqA != PIPE_BLEND_MIN &&
                    eqRGB != PIPE_BLEND_MAX && eqA != PIPE_BLEND_MAX) {
                    /* Disable reading if SRC_ALPHA == 0. */
                    if ((dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
                         dstRGB == PIPE_BLENDFACTOR_ZERO) &&
                        (dstA == PIPE_BLENDFACTOR_SRC_COLOR ||
                         dstA == PIPE_BLENDFACTOR_SRC_ALPHA ||
                         dstA == PIPE_BLENDFACTOR_ZERO)) {
                         blend_control |= R500_SRC_ALPHA_0_NO_READ;
                    }

                    /* Disable reading if SRC_ALPHA == 1. */
                    if ((dstRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
                         dstRGB == PIPE_BLENDFACTOR_ZERO) &&
                        (dstA == PIPE_BLENDFACTOR_INV_SRC_COLOR ||
                         dstA == PIPE_BLENDFACTOR_INV_SRC_ALPHA ||
                         dstA == PIPE_BLENDFACTOR_ZERO)) {
                         blend_control |= R500_SRC_ALPHA_1_NO_READ;
                    }
                }
            }
        }

        /* Optimization: discard pixels which don't change the colorbuffer.
         *
         * The code below is non-trivial and some math is involved.
         *
         * Discarding pixels must be disabled when FP16 AA is enabled.
         * This is a hardware bug. Also, this implementation wouldn't work
         * with FP blending enabled and equation clamping disabled.
         *
         * Equations other than ADD are rarely used and therefore won't be
         * optimized. */
        if ((eqRGB == PIPE_BLEND_ADD || eqRGB == PIPE_BLEND_REVERSE_SUBTRACT) &&
            (eqA == PIPE_BLEND_ADD || eqA == PIPE_BLEND_REVERSE_SUBTRACT)) {
            /* ADD: X+Y
             * REVERSE_SUBTRACT: Y-X
             *
             * The idea is:
             * If X = src*srcFactor = 0 and Y = dst*dstFactor = 1,
             * then CB will not be changed.
             *
             * Given the srcFactor and dstFactor variables, we can derive
             * what src and dst should be equal to and discard appropriate
             * pixels.
             */
            if (blend_discard_if_src_alpha_0(srcRGB, srcA, dstRGB, dstA)) {
                blend_control |= R300_DISCARD_SRC_PIXELS_SRC_ALPHA_0;
            } else if (blend_discard_if_src_alpha_1(srcRGB, srcA,
                                                    dstRGB, dstA)) {
                blend_control |= R300_DISCARD_SRC_PIXELS_SRC_ALPHA_1;
            } else if (blend_discard_if_src_color_0(srcRGB, srcA,
                                                    dstRGB, dstA)) {
                blend_control |= R300_DISCARD_SRC_PIXELS_SRC_COLOR_0;
            } else if (blend_discard_if_src_color_1(srcRGB, srcA,
                                                    dstRGB, dstA)) {
                blend_control |= R300_DISCARD_SRC_PIXELS_SRC_COLOR_1;
            } else if (blend_discard_if_src_alpha_color_0(srcRGB, srcA,
                                                          dstRGB, dstA)) {
                blend_control |=
                    R300_DISCARD_SRC_PIXELS_SRC_ALPHA_COLOR_0;
            } else if (blend_discard_if_src_alpha_color_1(srcRGB, srcA,
                                                          dstRGB, dstA)) {
                blend_control |=
                    R300_DISCARD_SRC_PIXELS_SRC_ALPHA_COLOR_1;
            }
        }

        /* separate alpha */
        if (srcA != srcRGB || dstA != dstRGB || eqA != eqRGB) {
            blend_control |= R300_SEPARATE_ALPHA_ENABLE;
            alpha_blend_control =
                r300_translate_blend_function(eqA) |
                (r300_translate_blend_factor(srcA) << R300_SRC_BLEND_SHIFT) |
                (r300_translate_blend_factor(dstA) << R300_DST_BLEND_SHIFT);
        }
    }

    /* PIPE_LOGICOP_* don't need to be translated, fortunately. */
    if (state->logicop_enable) {
        rop = R300_RB3D_ROPCNTL_ROP_ENABLE |
                (state->logicop_func) << R300_RB3D_ROPCNTL_ROP_SHIFT;
    }

    /* Color channel masks for all MRTs. */
    color_channel_mask = bgra_cmask(state->rt[0].colormask);
    if (r300screen->caps.is_r500 && state->independent_blend_enable) {
        if (state->rt[1].blend_enable) {
            color_channel_mask |= bgra_cmask(state->rt[1].colormask) << 4;
        }
        if (state->rt[2].blend_enable) {
            color_channel_mask |= bgra_cmask(state->rt[2].colormask) << 8;
        }
        if (state->rt[3].blend_enable) {
            color_channel_mask |= bgra_cmask(state->rt[3].colormask) << 12;
        }
    }

    /* Neither fglrx nor classic r300 ever set this, regardless of dithering
     * state. Since it's an optional implementation detail, we can leave it
     * out and never dither.
     *
     * This could be revisited if we ever get quality or conformance hints.
     *
    if (state->dither) {
        dither = R300_RB3D_DITHER_CTL_DITHER_MODE_LUT |
                        R300_RB3D_DITHER_CTL_ALPHA_DITHER_MODE_LUT;
    }
    */

    /* Build a command buffer. */
    BEGIN_CB(blend->cb, 8);
    OUT_CB_REG(R300_RB3D_ROPCNTL, rop);
    OUT_CB_REG_SEQ(R300_RB3D_CBLEND, 3);
    OUT_CB(blend_control);
    OUT_CB(alpha_blend_control);
    OUT_CB(color_channel_mask);
    OUT_CB_REG(R300_RB3D_DITHER_CTL, dither);
    END_CB;

    /* The same as above, but with no colorbuffer reads and writes. */
    BEGIN_CB(blend->cb_no_readwrite, 8);
    OUT_CB_REG(R300_RB3D_ROPCNTL, rop);
    OUT_CB_REG_SEQ(R300_RB3D_CBLEND, 3);
    OUT_CB(0);
    OUT_CB(0);
    OUT_CB(0);
    OUT_CB_REG(R300_RB3D_DITHER_CTL, dither);
    END_CB;

    return (void*)blend;
}

/* Bind blend state. */
static void r300_bind_blend_state(struct pipe_context* pipe,
                                  void* state)
{
    struct r300_context* r300 = r300_context(pipe);

    UPDATE_STATE(state, r300->blend_state);
}

/* Free blend state. */
static void r300_delete_blend_state(struct pipe_context* pipe,
                                    void* state)
{
    FREE(state);
}

/* Convert float to 10bit integer */
static unsigned float_to_fixed10(float f)
{
    return CLAMP((unsigned)(f * 1023.9f), 0, 1023);
}

/* Set blend color.
 * Setup both R300 and R500 registers, figure out later which one to write. */
static void r300_set_blend_color(struct pipe_context* pipe,
                                 const struct pipe_blend_color* color)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_blend_color_state* state =
        (struct r300_blend_color_state*)r300->blend_color_state.state;
    CB_LOCALS;

    if (r300->screen->caps.is_r500) {
        /* XXX if FP16 blending is enabled, we should use the FP16 format */
        BEGIN_CB(state->cb, 3);
        OUT_CB_REG_SEQ(R500_RB3D_CONSTANT_COLOR_AR, 2);
        OUT_CB(float_to_fixed10(color->color[0]) |
               (float_to_fixed10(color->color[3]) << 16));
        OUT_CB(float_to_fixed10(color->color[2]) |
               (float_to_fixed10(color->color[1]) << 16));
        END_CB;
    } else {
        union util_color uc;
        util_pack_color(color->color, PIPE_FORMAT_B8G8R8A8_UNORM, &uc);

        BEGIN_CB(state->cb, 2);
        OUT_CB_REG(R300_RB3D_BLEND_COLOR, uc.ui);
        END_CB;
    }

    r300->blend_color_state.dirty = TRUE;
}

static void r300_set_clip_state(struct pipe_context* pipe,
                                const struct pipe_clip_state* state)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_clip_state *clip =
            (struct r300_clip_state*)r300->clip_state.state;
    CB_LOCALS;

    clip->clip = *state;

    if (r300->screen->caps.has_tcl) {
        r300->clip_state.size = 2 + !!state->nr * 3 + state->nr * 4;

        BEGIN_CB(clip->cb, r300->clip_state.size);
        if (state->nr) {
           OUT_CB_REG(R300_VAP_PVS_VECTOR_INDX_REG,
                   (r300->screen->caps.is_r500 ?
                    R500_PVS_UCP_START : R300_PVS_UCP_START));
           OUT_CB_ONE_REG(R300_VAP_PVS_UPLOAD_DATA, state->nr * 4);
           OUT_CB_TABLE(state->ucp, state->nr * 4);
        }
        OUT_CB_REG(R300_VAP_CLIP_CNTL, ((1 << state->nr) - 1) |
                R300_PS_UCP_MODE_CLIP_AS_TRIFAN |
                (state->depth_clamp ? R300_CLIP_DISABLE : 0));
        END_CB;

        r300->clip_state.dirty = TRUE;
    } else {
        draw_set_clip_state(r300->draw, state);
    }
}

static void
r300_set_sample_mask(struct pipe_context *pipe,
                     unsigned sample_mask)
{
}


/* Create a new depth, stencil, and alpha state based on the CSO dsa state.
 *
 * This contains the depth buffer, stencil buffer, alpha test, and such.
 * On the Radeon, depth and stencil buffer setup are intertwined, which is
 * the reason for some of the strange-looking assignments across registers. */
static void*
        r300_create_dsa_state(struct pipe_context* pipe,
                              const struct pipe_depth_stencil_alpha_state* state)
{
    struct r300_capabilities *caps = &r300_screen(pipe->screen)->caps;
    struct r300_dsa_state* dsa = CALLOC_STRUCT(r300_dsa_state);
    CB_LOCALS;

    dsa->dsa = *state;

    /* Depth test setup. - separate write mask depth for decomp flush */
    if (state->depth.writemask) {
        dsa->z_buffer_control |= R300_Z_WRITE_ENABLE;
    }

    if (state->depth.enabled) {
        dsa->z_buffer_control |= R300_Z_ENABLE;

        dsa->z_stencil_control |=
            (r300_translate_depth_stencil_function(state->depth.func) <<
                R300_Z_FUNC_SHIFT);
    }

    /* Stencil buffer setup. */
    if (state->stencil[0].enabled) {
        dsa->z_buffer_control |= R300_STENCIL_ENABLE;
        dsa->z_stencil_control |=
            (r300_translate_depth_stencil_function(state->stencil[0].func) <<
                R300_S_FRONT_FUNC_SHIFT) |
            (r300_translate_stencil_op(state->stencil[0].fail_op) <<
                R300_S_FRONT_SFAIL_OP_SHIFT) |
            (r300_translate_stencil_op(state->stencil[0].zpass_op) <<
                R300_S_FRONT_ZPASS_OP_SHIFT) |
            (r300_translate_stencil_op(state->stencil[0].zfail_op) <<
                R300_S_FRONT_ZFAIL_OP_SHIFT);

        dsa->stencil_ref_mask =
                (state->stencil[0].valuemask << R300_STENCILMASK_SHIFT) |
                (state->stencil[0].writemask << R300_STENCILWRITEMASK_SHIFT);

        if (state->stencil[1].enabled) {
            dsa->two_sided = TRUE;

            dsa->z_buffer_control |= R300_STENCIL_FRONT_BACK;
            dsa->z_stencil_control |=
            (r300_translate_depth_stencil_function(state->stencil[1].func) <<
                R300_S_BACK_FUNC_SHIFT) |
            (r300_translate_stencil_op(state->stencil[1].fail_op) <<
                R300_S_BACK_SFAIL_OP_SHIFT) |
            (r300_translate_stencil_op(state->stencil[1].zpass_op) <<
                R300_S_BACK_ZPASS_OP_SHIFT) |
            (r300_translate_stencil_op(state->stencil[1].zfail_op) <<
                R300_S_BACK_ZFAIL_OP_SHIFT);

            dsa->stencil_ref_bf =
                (state->stencil[1].valuemask << R300_STENCILMASK_SHIFT) |
                (state->stencil[1].writemask << R300_STENCILWRITEMASK_SHIFT);

            if (caps->is_r500) {
                dsa->z_buffer_control |= R500_STENCIL_REFMASK_FRONT_BACK;
            } else {
                dsa->two_sided_stencil_ref =
                  (state->stencil[0].valuemask != state->stencil[1].valuemask ||
                   state->stencil[0].writemask != state->stencil[1].writemask);
            }
        }
    }

    /* Alpha test setup. */
    if (state->alpha.enabled) {
        dsa->alpha_function =
            r300_translate_alpha_function(state->alpha.func) |
            R300_FG_ALPHA_FUNC_ENABLE;

        /* We could use 10bit alpha ref but who needs that? */
        dsa->alpha_function |= float_to_ubyte(state->alpha.ref_value);

        if (caps->is_r500)
            dsa->alpha_function |= R500_FG_ALPHA_FUNC_8BIT;
    }

    BEGIN_CB(&dsa->cb_begin, 8);
    OUT_CB_REG(R300_FG_ALPHA_FUNC, dsa->alpha_function);
    OUT_CB_REG_SEQ(R300_ZB_CNTL, 3);
    OUT_CB(dsa->z_buffer_control);
    OUT_CB(dsa->z_stencil_control);
    OUT_CB(dsa->stencil_ref_mask);
    OUT_CB_REG(R500_ZB_STENCILREFMASK_BF, dsa->stencil_ref_bf);
    END_CB;

    BEGIN_CB(dsa->cb_no_readwrite, 8);
    OUT_CB_REG(R300_FG_ALPHA_FUNC, dsa->alpha_function);
    OUT_CB_REG_SEQ(R300_ZB_CNTL, 3);
    OUT_CB(0);
    OUT_CB(0);
    OUT_CB(0);
    OUT_CB_REG(R500_ZB_STENCILREFMASK_BF, 0);
    END_CB;

    return (void*)dsa;
}

static void r300_dsa_inject_stencilref(struct r300_context *r300)
{
    struct r300_dsa_state *dsa =
            (struct r300_dsa_state*)r300->dsa_state.state;

    if (!dsa)
        return;

    dsa->stencil_ref_mask =
        (dsa->stencil_ref_mask & ~R300_STENCILREF_MASK) |
        r300->stencil_ref.ref_value[0];
    dsa->stencil_ref_bf =
        (dsa->stencil_ref_bf & ~R300_STENCILREF_MASK) |
        r300->stencil_ref.ref_value[1];
}

/* Bind DSA state. */
static void r300_bind_dsa_state(struct pipe_context* pipe,
                                void* state)
{
    struct r300_context* r300 = r300_context(pipe);

    if (!state) {
        return;
    }

    UPDATE_STATE(state, r300->dsa_state);

    r300->hyperz_state.dirty = TRUE; /* Will be updated before the emission. */
    r300_dsa_inject_stencilref(r300);
}

/* Free DSA state. */
static void r300_delete_dsa_state(struct pipe_context* pipe,
                                  void* state)
{
    FREE(state);
}

static void r300_set_stencil_ref(struct pipe_context* pipe,
                                 const struct pipe_stencil_ref* sr)
{
    struct r300_context* r300 = r300_context(pipe);

    r300->stencil_ref = *sr;

    r300_dsa_inject_stencilref(r300);
    r300->dsa_state.dirty = TRUE;
}

static void r300_tex_set_tiling_flags(struct r300_context *r300,
                                      struct r300_texture *tex, unsigned level)
{
    /* Check if the macrotile flag needs to be changed.
     * Skip changing the flags otherwise. */
    if (tex->desc.macrotile[tex->surface_level] !=
        tex->desc.macrotile[level]) {
        /* Tiling determines how DRM treats the buffer data.
         * We must flush CS when changing it if the buffer is referenced. */
        if (r300->rws->cs_is_buffer_referenced(r300->cs,
                                               tex->buffer, R300_REF_CS))
            r300->context.flush(&r300->context, 0, NULL);

        r300->rws->buffer_set_tiling(r300->rws, tex->buffer,
                tex->desc.microtile, tex->desc.macrotile[level],
                tex->desc.stride_in_bytes[0]);

        tex->surface_level = level;
    }
}

/* This switcheroo is needed just because of goddamned MACRO_SWITCH. */
static void r300_fb_set_tiling_flags(struct r300_context *r300,
                               const struct pipe_framebuffer_state *state)
{
    unsigned i;

    /* Set tiling flags for new surfaces. */
    for (i = 0; i < state->nr_cbufs; i++) {
        r300_tex_set_tiling_flags(r300,
                                  r300_texture(state->cbufs[i]->texture),
                                  state->cbufs[i]->level);
    }
    if (state->zsbuf) {
        r300_tex_set_tiling_flags(r300,
                                  r300_texture(state->zsbuf->texture),
                                  state->zsbuf->level);
    }
}

static void r300_print_fb_surf_info(struct pipe_surface *surf, unsigned index,
                                    const char *binding)
{
    struct pipe_resource *tex = surf->texture;
    struct r300_texture *rtex = r300_texture(tex);

    fprintf(stderr,
            "r300:   %s[%i] Dim: %ix%i, Offset: %i, ZSlice: %i, "
            "Face: %i, Level: %i, Format: %s\n"

            "r300:     TEX: Macro: %s, Micro: %s, Pitch: %i, "
            "Dim: %ix%ix%i, LastLevel: %i, Format: %s\n",

            binding, index, surf->width, surf->height, surf->offset,
            surf->zslice, surf->face, surf->level,
            util_format_short_name(surf->format),

            rtex->desc.macrotile[0] ? "YES" : " NO",
            rtex->desc.microtile ? "YES" : " NO",
            rtex->desc.stride_in_pixels[0],
            tex->width0, tex->height0, tex->depth0,
            tex->last_level, util_format_short_name(tex->format));
}

void r300_mark_fb_state_dirty(struct r300_context *r300,
                              enum r300_fb_state_change change)
{
    struct pipe_framebuffer_state *state = r300->fb_state.state;
    boolean has_hyperz = r300->rws->get_value(r300->rws, R300_CAN_HYPERZ);

    /* What is marked as dirty depends on the enum r300_fb_state_change. */
    r300->gpu_flush.dirty = TRUE;
    r300->fb_state.dirty = TRUE;
    r300->hyperz_state.dirty = TRUE;

    if (change == R300_CHANGED_FB_STATE) {
        r300->aa_state.dirty = TRUE;
        r300->fb_state_pipelined.dirty = TRUE;
    }

    /* Now compute the fb_state atom size. */
    r300->fb_state.size = 2 + (8 * state->nr_cbufs);

    if (r300->cbzb_clear)
        r300->fb_state.size += 10;
    else if (state->zsbuf) {
        r300->fb_state.size += 10;
        if (has_hyperz)
            r300->fb_state.size += r300->screen->caps.hiz_ram ? 8 : 4;
    }

    /* The size of the rest of atoms stays the same. */
}

static void
    r300_set_framebuffer_state(struct pipe_context* pipe,
                               const struct pipe_framebuffer_state* state)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_aa_state *aa = (struct r300_aa_state*)r300->aa_state.state;
    struct pipe_framebuffer_state *old_state = r300->fb_state.state;
    boolean has_hyperz = r300->rws->get_value(r300->rws, R300_CAN_HYPERZ);
    unsigned max_width, max_height, i;
    uint32_t zbuffer_bpp = 0;
    int blocksize;

    if (r300->screen->caps.is_r500) {
        max_width = max_height = 4096;
    } else if (r300->screen->caps.is_r400) {
        max_width = max_height = 4021;
    } else {
        max_width = max_height = 2560;
    }

    if (state->width > max_width || state->height > max_height) {
        fprintf(stderr, "r300: Implementation error: Render targets are too "
        "big in %s, refusing to bind framebuffer state!\n", __FUNCTION__);
        return;
    }

    /* If nr_cbufs is changed from zero to non-zero or vice versa... */
    if (!!old_state->nr_cbufs != !!state->nr_cbufs) {
        r300->blend_state.dirty = TRUE;
    }
    /* If zsbuf is set from NULL to non-NULL or vice versa.. */
    if (!!old_state->zsbuf != !!state->zsbuf) {
        r300->dsa_state.dirty = TRUE;
    }

    /* The tiling flags are dependent on the surface miplevel, unfortunately. */
    r300_fb_set_tiling_flags(r300, state);

    util_copy_framebuffer_state(r300->fb_state.state, state);

    r300_mark_fb_state_dirty(r300, R300_CHANGED_FB_STATE);

    r300->z_compression = false;
    
    if (state->zsbuf) {
        blocksize = util_format_get_blocksize(state->zsbuf->texture->format);
        switch (blocksize) {
        case 2:
            zbuffer_bpp = 16;
            break;
        case 4:
            zbuffer_bpp = 24;
            break;
        }
        if (has_hyperz) {
            struct r300_surface *zs_surf = r300_surface(state->zsbuf);
            struct r300_texture *tex;
            int compress = r300->screen->caps.is_rv350 ? RV350_Z_COMPRESS_88 : R300_Z_COMPRESS_44;
            int level = zs_surf->base.level;

            tex = r300_texture(zs_surf->base.texture);

            /* work out whether we can support hiz on this buffer */
            r300_hiz_alloc_block(r300, zs_surf);
        
            /* work out whether we can support zmask features on this buffer */
            r300_zmask_alloc_block(r300, zs_surf, compress);

            if (tex->zmask_mem[level]) {
                /* compression causes hangs on 16-bit */
                if (zbuffer_bpp == 24)
                    r300->z_compression = compress;
            }
            DBG(r300, DBG_HYPERZ,
                "hyper-z features: hiz: %d @ %08x z-compression: %d z-fastfill: %d @ %08x\n", tex->hiz_mem[level] ? 1 : 0,
                tex->hiz_mem[level] ? tex->hiz_mem[level]->ofs : 0xdeadbeef,
                r300->z_compression, tex->zmask_mem[level] ? 1 : 0,
                tex->zmask_mem[level] ? tex->zmask_mem[level]->ofs : 0xdeadbeef);
        }

        /* Polygon offset depends on the zbuffer bit depth. */
        if (r300->zbuffer_bpp != zbuffer_bpp) {
            r300->zbuffer_bpp = zbuffer_bpp;

            if (r300->polygon_offset_enabled)
                r300->rs_state.dirty = TRUE;
        }
    }

    /* Set up AA config. */
    if (r300->rws->get_value(r300->rws, R300_VID_DRM_2_3_0)) {
        if (state->nr_cbufs && state->cbufs[0]->texture->nr_samples > 1) {
            aa->aa_config = R300_GB_AA_CONFIG_AA_ENABLE;

            switch (state->cbufs[0]->texture->nr_samples) {
                case 2:
                    aa->aa_config |= R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_2;
                    break;
                case 3:
                    aa->aa_config |= R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_3;
                    break;
                case 4:
                    aa->aa_config |= R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_4;
                    break;
                case 6:
                    aa->aa_config |= R300_GB_AA_CONFIG_NUM_AA_SUBSAMPLES_6;
                    break;
            }
        } else {
            aa->aa_config = 0;
        }
    }

    if (DBG_ON(r300, DBG_FB)) {
        fprintf(stderr, "r300: set_framebuffer_state:\n");
        for (i = 0; i < state->nr_cbufs; i++) {
            r300_print_fb_surf_info(state->cbufs[i], i, "CB");
        }
        if (state->zsbuf) {
            r300_print_fb_surf_info(state->zsbuf, 0, "ZB");
        }
    }
}

/* Create fragment shader state. */
static void* r300_create_fs_state(struct pipe_context* pipe,
                                  const struct pipe_shader_state* shader)
{
    struct r300_fragment_shader* fs = NULL;

    fs = (struct r300_fragment_shader*)CALLOC_STRUCT(r300_fragment_shader);

    /* Copy state directly into shader. */
    fs->state = *shader;
    fs->state.tokens = tgsi_dup_tokens(shader->tokens);

    return (void*)fs;
}

void r300_mark_fs_code_dirty(struct r300_context *r300)
{
    struct r300_fragment_shader* fs = r300_fs(r300);

    r300->fs.dirty = TRUE;
    r300->fs_rc_constant_state.dirty = TRUE;
    r300->fs_constants.dirty = TRUE;
    r300->fs.size = fs->shader->cb_code_size;

    if (r300->screen->caps.is_r500) {
        r300->fs_rc_constant_state.size = fs->shader->rc_state_count * 7;
        r300->fs_constants.size = fs->shader->externals_count * 4 + 3;
    } else {
        r300->fs_rc_constant_state.size = fs->shader->rc_state_count * 5;
        r300->fs_constants.size = fs->shader->externals_count * 4 + 1;
    }

    ((struct r300_constant_buffer*)r300->fs_constants.state)->remap_table =
            fs->shader->code.constants_remap_table;
}

/* Bind fragment shader state. */
static void r300_bind_fs_state(struct pipe_context* pipe, void* shader)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_fragment_shader* fs = (struct r300_fragment_shader*)shader;

    if (fs == NULL) {
        r300->fs.state = NULL;
        return;
    }

    r300->fs.state = fs;
    r300_pick_fragment_shader(r300);
    r300_mark_fs_code_dirty(r300);

    r300->rs_block_state.dirty = TRUE; /* Will be updated before the emission. */
}

/* Delete fragment shader state. */
static void r300_delete_fs_state(struct pipe_context* pipe, void* shader)
{
    struct r300_fragment_shader* fs = (struct r300_fragment_shader*)shader;
    struct r300_fragment_shader_code *tmp, *ptr = fs->first;

    while (ptr) {
        tmp = ptr;
        ptr = ptr->next;
        rc_constants_destroy(&tmp->code.constants);
        FREE(tmp->cb_code);
        FREE(tmp);
    }
    FREE((void*)fs->state.tokens);
    FREE(shader);
}

static void r300_set_polygon_stipple(struct pipe_context* pipe,
                                     const struct pipe_poly_stipple* state)
{
    /* XXX no idea how to set this up, but not terribly important */
}

/* Create a new rasterizer state based on the CSO rasterizer state.
 *
 * This is a very large chunk of state, and covers most of the graphics
 * backend (GB), geometry assembly (GA), and setup unit (SU) blocks.
 *
 * In a not entirely unironic sidenote, this state has nearly nothing to do
 * with the actual block on the Radeon called the rasterizer (RS). */
static void* r300_create_rs_state(struct pipe_context* pipe,
                                  const struct pipe_rasterizer_state* state)
{
    struct r300_rs_state* rs = CALLOC_STRUCT(r300_rs_state);
    int i;
    float psiz;
    uint32_t vap_control_status;    /* R300_VAP_CNTL_STATUS: 0x2140 */
    uint32_t point_size;            /* R300_GA_POINT_SIZE: 0x421c */
    uint32_t point_minmax;          /* R300_GA_POINT_MINMAX: 0x4230 */
    uint32_t line_control;          /* R300_GA_LINE_CNTL: 0x4234 */
    uint32_t polygon_offset_enable; /* R300_SU_POLY_OFFSET_ENABLE: 0x42b4 */
    uint32_t cull_mode;             /* R300_SU_CULL_MODE: 0x42b8 */
    uint32_t line_stipple_config;   /* R300_GA_LINE_STIPPLE_CONFIG: 0x4328 */
    uint32_t line_stipple_value;    /* R300_GA_LINE_STIPPLE_VALUE: 0x4260 */
    uint32_t polygon_mode;          /* R300_GA_POLY_MODE: 0x4288 */
    uint32_t clip_rule;             /* R300_SC_CLIP_RULE: 0x43D0 */

    /* Specifies top of Raster pipe specific enable controls,
     * i.e. texture coordinates stuffing for points, lines, triangles */
    uint32_t stuffing_enable;       /* R300_GB_ENABLE: 0x4008 */

    /* Point sprites texture coordinates, 0: lower left, 1: upper right */
    float point_texcoord_left = 0;  /* R300_GA_POINT_S0: 0x4200 */
    float point_texcoord_bottom = 0;/* R300_GA_POINT_T0: 0x4204 */
    float point_texcoord_right = 1; /* R300_GA_POINT_S1: 0x4208 */
    float point_texcoord_top = 0;   /* R300_GA_POINT_T1: 0x420c */
    CB_LOCALS;

    /* Copy rasterizer state. */
    rs->rs = *state;
    rs->rs_draw = *state;

    /* Generate point sprite texture coordinates in GENERIC0
     * if point_quad_rasterization is TRUE. */
    rs->rs.sprite_coord_enable = state->point_quad_rasterization *
                                 (state->sprite_coord_enable | 1);

    /* Override some states for Draw. */
    rs->rs_draw.sprite_coord_enable = 0; /* We can do this in HW. */

#ifdef PIPE_ARCH_LITTLE_ENDIAN
    vap_control_status = R300_VC_NO_SWAP;
#else
    vap_control_status = R300_VC_32BIT_SWAP;
#endif

    /* If no TCL engine is present, turn off the HW TCL. */
    if (!r300_screen(pipe->screen)->caps.has_tcl) {
        vap_control_status |= R300_VAP_TCL_BYPASS;
    }

    /* Point size width and height. */
    point_size =
        pack_float_16_6x(state->point_size) |
        (pack_float_16_6x(state->point_size) << R300_POINTSIZE_X_SHIFT);

    /* Point size clamping. */
    if (state->point_size_per_vertex) {
        /* Per-vertex point size.
         * Clamp to [0, max FB size] */
        psiz = pipe->screen->get_paramf(pipe->screen,
                                        PIPE_CAP_MAX_POINT_WIDTH);
        point_minmax =
            pack_float_16_6x(psiz) << R300_GA_POINT_MINMAX_MAX_SHIFT;
    } else {
        /* We cannot disable the point-size vertex output,
         * so clamp it. */
        psiz = state->point_size;
        point_minmax =
            (pack_float_16_6x(psiz) << R300_GA_POINT_MINMAX_MIN_SHIFT) |
            (pack_float_16_6x(psiz) << R300_GA_POINT_MINMAX_MAX_SHIFT);
    }

    /* Line control. */
    line_control = pack_float_16_6x(state->line_width) |
        R300_GA_LINE_CNTL_END_TYPE_COMP;

    /* Enable polygon mode */
    polygon_mode = 0;
    if (state->fill_front != PIPE_POLYGON_MODE_FILL ||
        state->fill_back != PIPE_POLYGON_MODE_FILL) {
        polygon_mode = R300_GA_POLY_MODE_DUAL;
    }

    /* Front face */
    if (state->front_ccw) 
        cull_mode = R300_FRONT_FACE_CCW;
    else
        cull_mode = R300_FRONT_FACE_CW;

    /* Polygon offset */
    polygon_offset_enable = 0;
    if (util_get_offset(state, state->fill_front)) {
       polygon_offset_enable |= R300_FRONT_ENABLE;
    }
    if (util_get_offset(state, state->fill_back)) {
       polygon_offset_enable |= R300_BACK_ENABLE;
    }

    rs->polygon_offset_enable = polygon_offset_enable != 0;

    /* Polygon mode */
    if (polygon_mode) {
       polygon_mode |=
          r300_translate_polygon_mode_front(state->fill_front);
       polygon_mode |=
          r300_translate_polygon_mode_back(state->fill_back);
    }

    if (state->cull_face & PIPE_FACE_FRONT) {
        cull_mode |= R300_CULL_FRONT;
    }
    if (state->cull_face & PIPE_FACE_BACK) {
        cull_mode |= R300_CULL_BACK;
    }

    if (state->line_stipple_enable) {
        line_stipple_config =
            R300_GA_LINE_STIPPLE_CONFIG_LINE_RESET_LINE |
            (fui((float)state->line_stipple_factor) &
                R300_GA_LINE_STIPPLE_CONFIG_STIPPLE_SCALE_MASK);
        /* XXX this might need to be scaled up */
        line_stipple_value = state->line_stipple_pattern;
    } else {
        line_stipple_config = 0;
        line_stipple_value = 0;
    }

    if (state->flatshade) {
        rs->color_control = R300_SHADE_MODEL_FLAT;
    } else {
        rs->color_control = R300_SHADE_MODEL_SMOOTH;
    }

    clip_rule = state->scissor ? 0xAAAA : 0xFFFF;

    /* Point sprites */
    stuffing_enable = 0;
    if (rs->rs.sprite_coord_enable) {
        stuffing_enable = R300_GB_POINT_STUFF_ENABLE;
        for (i = 0; i < 8; i++) {
            if (rs->rs.sprite_coord_enable & (1 << i))
                stuffing_enable |=
                    R300_GB_TEX_ST << (R300_GB_TEX0_SOURCE_SHIFT + (i*2));
        }

        switch (state->sprite_coord_mode) {
            case PIPE_SPRITE_COORD_UPPER_LEFT:
                point_texcoord_top = 0.0f;
                point_texcoord_bottom = 1.0f;
                break;
            case PIPE_SPRITE_COORD_LOWER_LEFT:
                point_texcoord_top = 1.0f;
                point_texcoord_bottom = 0.0f;
                break;
        }
    }

    /* Build the main command buffer. */
    BEGIN_CB(rs->cb_main, 25);
    OUT_CB_REG(R300_VAP_CNTL_STATUS, vap_control_status);
    OUT_CB_REG(R300_GA_POINT_SIZE, point_size);
    OUT_CB_REG_SEQ(R300_GA_POINT_MINMAX, 2);
    OUT_CB(point_minmax);
    OUT_CB(line_control);
    OUT_CB_REG_SEQ(R300_SU_POLY_OFFSET_ENABLE, 2);
    OUT_CB(polygon_offset_enable);
    rs->cull_mode_index = 9;
    OUT_CB(cull_mode);
    OUT_CB_REG(R300_GA_LINE_STIPPLE_CONFIG, line_stipple_config);
    OUT_CB_REG(R300_GA_LINE_STIPPLE_VALUE, line_stipple_value);
    OUT_CB_REG(R300_GA_POLY_MODE, polygon_mode);
    OUT_CB_REG(R300_SC_CLIP_RULE, clip_rule);
    OUT_CB_REG(R300_GB_ENABLE, stuffing_enable);
    OUT_CB_REG_SEQ(R300_GA_POINT_S0, 4);
    OUT_CB_32F(point_texcoord_left);
    OUT_CB_32F(point_texcoord_bottom);
    OUT_CB_32F(point_texcoord_right);
    OUT_CB_32F(point_texcoord_top);
    END_CB;

    /* Build the two command buffers for polygon offset setup. */
    if (polygon_offset_enable) {
        float scale = state->offset_scale * 12;
        float offset = state->offset_units * 4;

        BEGIN_CB(rs->cb_poly_offset_zb16, 5);
        OUT_CB_REG_SEQ(R300_SU_POLY_OFFSET_FRONT_SCALE, 4);
        OUT_CB_32F(scale);
        OUT_CB_32F(offset);
        OUT_CB_32F(scale);
        OUT_CB_32F(offset);
        END_CB;

        offset = state->offset_units * 2;

        BEGIN_CB(rs->cb_poly_offset_zb24, 5);
        OUT_CB_REG_SEQ(R300_SU_POLY_OFFSET_FRONT_SCALE, 4);
        OUT_CB_32F(scale);
        OUT_CB_32F(offset);
        OUT_CB_32F(scale);
        OUT_CB_32F(offset);
        END_CB;
    }

    return (void*)rs;
}

/* Bind rasterizer state. */
static void r300_bind_rs_state(struct pipe_context* pipe, void* state)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_rs_state* rs = (struct r300_rs_state*)state;
    int last_sprite_coord_enable = r300->sprite_coord_enable;
    boolean last_two_sided_color = r300->two_sided_color;

    if (r300->draw && rs) {
        draw_set_rasterizer_state(r300->draw, &rs->rs_draw, state);
    }

    if (rs) {
        r300->polygon_offset_enabled = rs->polygon_offset_enable;
        r300->sprite_coord_enable = rs->rs.sprite_coord_enable;
        r300->two_sided_color = rs->rs.light_twoside;
    } else {
        r300->polygon_offset_enabled = FALSE;
        r300->sprite_coord_enable = 0;
        r300->two_sided_color = FALSE;
    }

    UPDATE_STATE(state, r300->rs_state);
    r300->rs_state.size = 25 + (r300->polygon_offset_enabled ? 5 : 0);

    if (last_sprite_coord_enable != r300->sprite_coord_enable ||
        last_two_sided_color != r300->two_sided_color) {
        r300->rs_block_state.dirty = TRUE;
    }
}

/* Free rasterizer state. */
static void r300_delete_rs_state(struct pipe_context* pipe, void* state)
{
    FREE(state);
}

static void*
        r300_create_sampler_state(struct pipe_context* pipe,
                                  const struct pipe_sampler_state* state)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_sampler_state* sampler = CALLOC_STRUCT(r300_sampler_state);
    boolean is_r500 = r300->screen->caps.is_r500;
    int lod_bias;

    sampler->state = *state;

    /* r300 doesn't handle CLAMP and MIRROR_CLAMP correctly when either MAG
     * or MIN filter is NEAREST. Since texwrap produces same results
     * for CLAMP and CLAMP_TO_EDGE, we use them instead. */
    if (sampler->state.min_img_filter == PIPE_TEX_FILTER_NEAREST ||
        sampler->state.mag_img_filter == PIPE_TEX_FILTER_NEAREST) {
        /* Wrap S. */
        if (sampler->state.wrap_s == PIPE_TEX_WRAP_CLAMP)
            sampler->state.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        else if (sampler->state.wrap_s == PIPE_TEX_WRAP_MIRROR_CLAMP)
            sampler->state.wrap_s = PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE;

        /* Wrap T. */
        if (sampler->state.wrap_t == PIPE_TEX_WRAP_CLAMP)
            sampler->state.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        else if (sampler->state.wrap_t == PIPE_TEX_WRAP_MIRROR_CLAMP)
            sampler->state.wrap_t = PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE;

        /* Wrap R. */
        if (sampler->state.wrap_r == PIPE_TEX_WRAP_CLAMP)
            sampler->state.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
        else if (sampler->state.wrap_r == PIPE_TEX_WRAP_MIRROR_CLAMP)
            sampler->state.wrap_r = PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE;
    }

    sampler->filter0 |=
        (r300_translate_wrap(sampler->state.wrap_s) << R300_TX_WRAP_S_SHIFT) |
        (r300_translate_wrap(sampler->state.wrap_t) << R300_TX_WRAP_T_SHIFT) |
        (r300_translate_wrap(sampler->state.wrap_r) << R300_TX_WRAP_R_SHIFT);

    sampler->filter0 |= r300_translate_tex_filters(state->min_img_filter,
                                                   state->mag_img_filter,
                                                   state->min_mip_filter,
                                                   state->max_anisotropy > 0);

    sampler->filter0 |= r300_anisotropy(state->max_anisotropy);

    /* Unfortunately, r300-r500 don't support floating-point mipmap lods. */
    /* We must pass these to the merge function to clamp them properly. */
    sampler->min_lod = (unsigned)MAX2(state->min_lod, 0);
    sampler->max_lod = (unsigned)MAX2(ceilf(state->max_lod), 0);

    lod_bias = CLAMP((int)(state->lod_bias * 32 + 1), -(1 << 9), (1 << 9) - 1);

    sampler->filter1 |= (lod_bias << R300_LOD_BIAS_SHIFT) & R300_LOD_BIAS_MASK;

    /* This is very high quality anisotropic filtering for R5xx.
     * It's good for benchmarking the performance of texturing but
     * in practice we don't want to slow down the driver because it's
     * a pretty good performance killer. Feel free to play with it. */
    if (DBG_ON(r300, DBG_ANISOHQ) && is_r500) {
        sampler->filter1 |= r500_anisotropy(state->max_anisotropy);
    }

    /* R500-specific fixups and optimizations */
    if (r300->screen->caps.is_r500) {
        sampler->filter1 |= R500_BORDER_FIX;
    }

    return (void*)sampler;
}

static void r300_bind_sampler_states(struct pipe_context* pipe,
                                     unsigned count,
                                     void** states)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_textures_state* state =
        (struct r300_textures_state*)r300->textures_state.state;
    unsigned tex_units = r300->screen->caps.num_tex_units;

    if (count > tex_units) {
        return;
    }

    memcpy(state->sampler_states, states, sizeof(void*) * count);
    state->sampler_state_count = count;

    r300->textures_state.dirty = TRUE;
}

static void r300_lacks_vertex_textures(struct pipe_context* pipe,
                                       unsigned count,
                                       void** states)
{
}

static void r300_delete_sampler_state(struct pipe_context* pipe, void* state)
{
    FREE(state);
}

static uint32_t r300_assign_texture_cache_region(unsigned index, unsigned num)
{
    /* This looks like a hack, but I believe it's suppose to work like
     * that. To illustrate how this works, let's assume you have 5 textures.
     * From docs, 5 and the successive numbers are:
     *
     * FOURTH_1     = 5
     * FOURTH_2     = 6
     * FOURTH_3     = 7
     * EIGHTH_0     = 8
     * EIGHTH_1     = 9
     *
     * First 3 textures will get 3/4 of size of the cache, divived evenly
     * between them. The last 1/4 of the cache must be divided between
     * the last 2 textures, each will therefore get 1/8 of the cache.
     * Why not just to use "5 + texture_index" ?
     *
     * This simple trick works for all "num" <= 16.
     */
    if (num <= 1)
        return R300_TX_CACHE(R300_TX_CACHE_WHOLE);
    else
        return R300_TX_CACHE(num + index);
}

static void r300_set_fragment_sampler_views(struct pipe_context* pipe,
                                            unsigned count,
                                            struct pipe_sampler_view** views)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_textures_state* state =
        (struct r300_textures_state*)r300->textures_state.state;
    struct r300_texture *texture;
    unsigned i, real_num_views = 0, view_index = 0;
    unsigned tex_units = r300->screen->caps.num_tex_units;
    boolean dirty_tex = FALSE;

    if (count > tex_units) {
        return;
    }

    /* Calculate the real number of views. */
    for (i = 0; i < count; i++) {
        if (views[i])
            real_num_views++;
    }

    for (i = 0; i < count; i++) {
        if (&state->sampler_views[i]->base != views[i]) {
            pipe_sampler_view_reference(
                    (struct pipe_sampler_view**)&state->sampler_views[i],
                    views[i]);

            if (!views[i]) {
                continue;
            }

            /* A new sampler view (= texture)... */
            dirty_tex = TRUE;

            /* Set the texrect factor in the fragment shader.
             * Needed for RECT and NPOT fallback. */
            texture = r300_texture(views[i]->texture);
            if (texture->desc.is_npot) {
                r300->fs_rc_constant_state.dirty = TRUE;
            }

            state->sampler_views[i]->texcache_region =
                r300_assign_texture_cache_region(view_index, real_num_views);
            view_index++;
        }
    }

    for (i = count; i < tex_units; i++) {
        if (state->sampler_views[i]) {
            pipe_sampler_view_reference(
                    (struct pipe_sampler_view**)&state->sampler_views[i],
                    NULL);
        }
    }

    state->sampler_view_count = count;

    r300->textures_state.dirty = TRUE;

    if (dirty_tex) {
        r300->texture_cache_inval.dirty = TRUE;
    }
}

static struct pipe_sampler_view *
r300_create_sampler_view(struct pipe_context *pipe,
                         struct pipe_resource *texture,
                         const struct pipe_sampler_view *templ)
{
    struct r300_sampler_view *view = CALLOC_STRUCT(r300_sampler_view);
    struct r300_texture *tex = r300_texture(texture);
    boolean is_r500 = r300_screen(pipe->screen)->caps.is_r500;

    if (view) {
        view->base = *templ;
        view->base.reference.count = 1;
        view->base.context = pipe;
        view->base.texture = NULL;
        pipe_resource_reference(&view->base.texture, texture);

        view->swizzle[0] = templ->swizzle_r;
        view->swizzle[1] = templ->swizzle_g;
        view->swizzle[2] = templ->swizzle_b;
        view->swizzle[3] = templ->swizzle_a;

        view->format = tex->tx_format;
        view->format.format1 |= r300_translate_texformat(templ->format,
                                                         view->swizzle,
                                                         is_r500);
        if (is_r500) {
            view->format.format2 |= r500_tx_format_msb_bit(templ->format);
        }
    }

    return (struct pipe_sampler_view*)view;
}

static void
r300_sampler_view_destroy(struct pipe_context *pipe,
                          struct pipe_sampler_view *view)
{
   pipe_resource_reference(&view->texture, NULL);
   FREE(view);
}

static void r300_set_scissor_state(struct pipe_context* pipe,
                                   const struct pipe_scissor_state* state)
{
    struct r300_context* r300 = r300_context(pipe);

    memcpy(r300->scissor_state.state, state,
        sizeof(struct pipe_scissor_state));

    r300->scissor_state.dirty = TRUE;
}

static void r300_set_viewport_state(struct pipe_context* pipe,
                                    const struct pipe_viewport_state* state)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_viewport_state* viewport =
        (struct r300_viewport_state*)r300->viewport_state.state;

    r300->viewport = *state;

    if (r300->draw) {
        draw_set_viewport_state(r300->draw, state);
        viewport->vte_control = R300_VTX_XY_FMT | R300_VTX_Z_FMT;
        return;
    }

    /* Do the transform in HW. */
    viewport->vte_control = R300_VTX_W0_FMT;

    if (state->scale[0] != 1.0f) {
        viewport->xscale = state->scale[0];
        viewport->vte_control |= R300_VPORT_X_SCALE_ENA;
    }
    if (state->scale[1] != 1.0f) {
        viewport->yscale = state->scale[1];
        viewport->vte_control |= R300_VPORT_Y_SCALE_ENA;
    }
    if (state->scale[2] != 1.0f) {
        viewport->zscale = state->scale[2];
        viewport->vte_control |= R300_VPORT_Z_SCALE_ENA;
    }
    if (state->translate[0] != 0.0f) {
        viewport->xoffset = state->translate[0];
        viewport->vte_control |= R300_VPORT_X_OFFSET_ENA;
    }
    if (state->translate[1] != 0.0f) {
        viewport->yoffset = state->translate[1];
        viewport->vte_control |= R300_VPORT_Y_OFFSET_ENA;
    }
    if (state->translate[2] != 0.0f) {
        viewport->zoffset = state->translate[2];
        viewport->vte_control |= R300_VPORT_Z_OFFSET_ENA;
    }

    r300->viewport_state.dirty = TRUE;
    if (r300->fs.state && r300_fs(r300)->shader->inputs.wpos != ATTR_UNUSED) {
        r300->fs_rc_constant_state.dirty = TRUE;
    }
}

static void r300_set_vertex_buffers(struct pipe_context* pipe,
                                    unsigned count,
                                    const struct pipe_vertex_buffer* buffers)
{
    struct r300_context* r300 = r300_context(pipe);
    struct pipe_vertex_buffer *vbo;
    unsigned i, max_index = (1 << 24) - 1;
    boolean any_user_buffer = FALSE;

    if (count == r300->vertex_buffer_count &&
        memcmp(r300->vertex_buffer, buffers,
            sizeof(struct pipe_vertex_buffer) * count) == 0) {
        return;
    }

    if (r300->screen->caps.has_tcl) {
        /* HW TCL. */
        r300->incompatible_vb_layout = FALSE;

        /* Check if the strides and offsets are aligned to the size of DWORD. */
        for (i = 0; i < count; i++) {
            if (buffers[i].buffer) {
                if (buffers[i].stride % 4 != 0 ||
                    buffers[i].buffer_offset % 4 != 0) {
                    r300->incompatible_vb_layout = TRUE;
                    break;
                }
            }
        }

        for (i = 0; i < count; i++) {
            /* Why, yes, I AM casting away constness. How did you know? */
            vbo = (struct pipe_vertex_buffer*)&buffers[i];

            /* Skip NULL buffers */
            if (!buffers[i].buffer) {
                continue;
            }

            if (r300_buffer_is_user_buffer(vbo->buffer)) {
                any_user_buffer = TRUE;
            }

            if (vbo->max_index == ~0) {
                /* if no VBO stride then only one vertex value so max index is 1 */
                /* should think about converting to VS constants like svga does */
                if (!vbo->stride)
                    vbo->max_index = 1;
                else
                    vbo->max_index =
                             (vbo->buffer->width0 - vbo->buffer_offset) / vbo->stride;
            }

            max_index = MIN2(vbo->max_index, max_index);
        }

        r300->any_user_vbs = any_user_buffer;
        r300->vertex_buffer_max_index = max_index;

    } else {
        /* SW TCL. */
        draw_set_vertex_buffers(r300->draw, count, buffers);
    }

    /* Common code. */
    for (i = 0; i < count; i++) {
        /* Reference our buffer. */
        pipe_resource_reference(&r300->vertex_buffer[i].buffer, buffers[i].buffer);
    }
    for (; i < r300->vertex_buffer_count; i++) {
        /* Dereference any old buffers. */
        pipe_resource_reference(&r300->vertex_buffer[i].buffer, NULL);
    }

    memcpy(r300->vertex_buffer, buffers,
        sizeof(struct pipe_vertex_buffer) * count);
    r300->vertex_buffer_count = count;
}

static void r300_set_index_buffer(struct pipe_context* pipe,
                                  const struct pipe_index_buffer *ib)
{
    struct r300_context* r300 = r300_context(pipe);

    if (ib) {
        pipe_resource_reference(&r300->index_buffer.buffer, ib->buffer);
        memcpy(&r300->index_buffer, ib, sizeof(r300->index_buffer));
    }
    else {
        pipe_resource_reference(&r300->index_buffer.buffer, NULL);
        memset(&r300->index_buffer, 0, sizeof(r300->index_buffer));
    }

    if (r300->screen->caps.has_tcl) {
       /* TODO make this more like a state */
    }
    else {
       draw_set_index_buffer(r300->draw, ib);
    }
}

/* Initialize the PSC tables. */
static void r300_vertex_psc(struct r300_vertex_element_state *velems)
{
    struct r300_vertex_stream_state *vstream = &velems->vertex_stream;
    uint16_t type, swizzle;
    enum pipe_format format;
    unsigned i;

    if (velems->count > 16) {
        fprintf(stderr, "r300: More than 16 vertex elements are not supported,"
                " requested %i, using 16.\n", velems->count);
        velems->count = 16;
    }

    /* Vertex shaders have no semantics on their inputs,
     * so PSC should just route stuff based on the vertex elements,
     * and not on attrib information. */
    for (i = 0; i < velems->count; i++) {
        format = velems->hw_format[i];

        type = r300_translate_vertex_data_type(format);
        if (type == R300_INVALID_FORMAT) {
            fprintf(stderr, "r300: Bad vertex format %s.\n",
                    util_format_short_name(format));
            assert(0);
            abort();
        }

        type |= i << R300_DST_VEC_LOC_SHIFT;
        swizzle = r300_translate_vertex_data_swizzle(format);

        if (i & 1) {
            vstream->vap_prog_stream_cntl[i >> 1] |= type << 16;
            vstream->vap_prog_stream_cntl_ext[i >> 1] |= swizzle << 16;
        } else {
            vstream->vap_prog_stream_cntl[i >> 1] |= type;
            vstream->vap_prog_stream_cntl_ext[i >> 1] |= swizzle;
        }
    }

    /* Set the last vector in the PSC. */
    if (i) {
        i -= 1;
    }
    vstream->vap_prog_stream_cntl[i >> 1] |=
        (R300_LAST_VEC << (i & 1 ? 16 : 0));

    vstream->count = (i >> 1) + 1;
}

#define FORMAT_REPLACE(what, withwhat) \
    case PIPE_FORMAT_##what: *format = PIPE_FORMAT_##withwhat; break

static void* r300_create_vertex_elements_state(struct pipe_context* pipe,
                                               unsigned count,
                                               const struct pipe_vertex_element* attribs)
{
    struct r300_vertex_element_state *velems;
    unsigned i;
    enum pipe_format *format;

    assert(count <= PIPE_MAX_ATTRIBS);
    velems = CALLOC_STRUCT(r300_vertex_element_state);
    if (velems != NULL) {
        velems->count = count;
        memcpy(velems->velem, attribs, sizeof(struct pipe_vertex_element) * count);

        if (r300_screen(pipe->screen)->caps.has_tcl) {
            /* Set the best hw format in case the original format is not
             * supported by hw. */
            for (i = 0; i < count; i++) {
                velems->hw_format[i] = velems->velem[i].src_format;
                format = &velems->hw_format[i];

                /* This is basically the list of unsupported formats.
                 * For now we don't care about the alignment, that's going to
                 * be sorted out after the PSC setup. */
                switch (*format) {
                    FORMAT_REPLACE(R64_FLOAT,           R32_FLOAT);
                    FORMAT_REPLACE(R64G64_FLOAT,        R32G32_FLOAT);
                    FORMAT_REPLACE(R64G64B64_FLOAT,     R32G32B32_FLOAT);
                    FORMAT_REPLACE(R64G64B64A64_FLOAT,  R32G32B32A32_FLOAT);

                    FORMAT_REPLACE(R32_UNORM,           R32_FLOAT);
                    FORMAT_REPLACE(R32G32_UNORM,        R32G32_FLOAT);
                    FORMAT_REPLACE(R32G32B32_UNORM,     R32G32B32_FLOAT);
                    FORMAT_REPLACE(R32G32B32A32_UNORM,  R32G32B32A32_FLOAT);

                    FORMAT_REPLACE(R32_USCALED,         R32_FLOAT);
                    FORMAT_REPLACE(R32G32_USCALED,      R32G32_FLOAT);
                    FORMAT_REPLACE(R32G32B32_USCALED,   R32G32B32_FLOAT);
                    FORMAT_REPLACE(R32G32B32A32_USCALED,R32G32B32A32_FLOAT);

                    FORMAT_REPLACE(R32_SNORM,           R32_FLOAT);
                    FORMAT_REPLACE(R32G32_SNORM,        R32G32_FLOAT);
                    FORMAT_REPLACE(R32G32B32_SNORM,     R32G32B32_FLOAT);
                    FORMAT_REPLACE(R32G32B32A32_SNORM,  R32G32B32A32_FLOAT);

                    FORMAT_REPLACE(R32_SSCALED,         R32_FLOAT);
                    FORMAT_REPLACE(R32G32_SSCALED,      R32G32_FLOAT);
                    FORMAT_REPLACE(R32G32B32_SSCALED,   R32G32B32_FLOAT);
                    FORMAT_REPLACE(R32G32B32A32_SSCALED,R32G32B32A32_FLOAT);

                    FORMAT_REPLACE(R32_FIXED,           R32_FLOAT);
                    FORMAT_REPLACE(R32G32_FIXED,        R32G32_FLOAT);
                    FORMAT_REPLACE(R32G32B32_FIXED,     R32G32B32_FLOAT);
                    FORMAT_REPLACE(R32G32B32A32_FIXED,  R32G32B32A32_FLOAT);

                    default:;
                }

                velems->incompatible_layout =
                        velems->incompatible_layout ||
                        velems->velem[i].src_format != velems->hw_format[i] ||
                        velems->velem[i].src_offset % 4 != 0;
            }

            /* Now setup PSC.
             * The unused components will be replaced by (..., 0, 1). */
            r300_vertex_psc(velems);

            /* Align the formats to the size of DWORD.
             * We only care about the blocksizes of the formats since
             * swizzles are already set up.
             * Also compute the vertex size. */
            for (i = 0; i < count; i++) {
                /* This is OK because we check for aligned strides too. */
                velems->hw_format_size[i] =
                    align(util_format_get_blocksize(velems->hw_format[i]), 4);
                velems->vertex_size_dwords += velems->hw_format_size[i] / 4;
            }
        }
    }
    return velems;
}

static void r300_bind_vertex_elements_state(struct pipe_context *pipe,
                                            void *state)
{
    struct r300_context *r300 = r300_context(pipe);
    struct r300_vertex_element_state *velems = state;

    if (velems == NULL) {
        return;
    }

    r300->velems = velems;

    if (r300->draw) {
        draw_set_vertex_elements(r300->draw, velems->count, velems->velem);
        return;
    }

    UPDATE_STATE(&velems->vertex_stream, r300->vertex_stream_state);
    r300->vertex_stream_state.size = (1 + velems->vertex_stream.count) * 2;
}

static void r300_delete_vertex_elements_state(struct pipe_context *pipe, void *state)
{
   FREE(state);
}

static void* r300_create_vs_state(struct pipe_context* pipe,
                                  const struct pipe_shader_state* shader)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_vertex_shader* vs = CALLOC_STRUCT(r300_vertex_shader);

    /* Copy state directly into shader. */
    vs->state = *shader;
    vs->state.tokens = tgsi_dup_tokens(shader->tokens);

    if (r300->screen->caps.has_tcl) {
        r300_init_vs_outputs(vs);
        r300_translate_vertex_shader(r300, vs);
    } else {
        r300_draw_init_vertex_shader(r300->draw, vs);
    }

    return vs;
}

static void r300_bind_vs_state(struct pipe_context* pipe, void* shader)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_vertex_shader* vs = (struct r300_vertex_shader*)shader;

    if (vs == NULL) {
        r300->vs_state.state = NULL;
        return;
    }
    if (vs == r300->vs_state.state) {
        return;
    }
    r300->vs_state.state = vs;

    /* The majority of the RS block bits is dependent on the vertex shader. */
    r300->rs_block_state.dirty = TRUE; /* Will be updated before the emission. */

    if (r300->screen->caps.has_tcl) {
        unsigned fc_op_dwords = r300->screen->caps.is_r500 ? 3 : 2;
        r300->vs_state.dirty = TRUE;
        r300->vs_state.size =
                vs->code.length + 9 +
                (vs->immediates_count ? vs->immediates_count * 4 + 3 : 0) +
        (vs->code.num_fc_ops ? vs->code.num_fc_ops * fc_op_dwords + 4 : 0);

        if (vs->externals_count) {
            r300->vs_constants.dirty = TRUE;
            r300->vs_constants.size = vs->externals_count * 4 + 3;
        } else {
            r300->vs_constants.size = 0;
        }

        ((struct r300_constant_buffer*)r300->vs_constants.state)->remap_table =
                vs->code.constants_remap_table;

        r300->pvs_flush.dirty = TRUE;
    } else {
        draw_bind_vertex_shader(r300->draw,
                (struct draw_vertex_shader*)vs->draw_vs);
    }
}

static void r300_delete_vs_state(struct pipe_context* pipe, void* shader)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_vertex_shader* vs = (struct r300_vertex_shader*)shader;

    if (r300->screen->caps.has_tcl) {
        rc_constants_destroy(&vs->code.constants);
        if (vs->code.constants_remap_table)
            FREE(vs->code.constants_remap_table);
    } else {
        draw_delete_vertex_shader(r300->draw,
                (struct draw_vertex_shader*)vs->draw_vs);
    }

    FREE((void*)vs->state.tokens);
    FREE(shader);
}

static void r300_set_constant_buffer(struct pipe_context *pipe,
                                     uint shader, uint index,
                                     struct pipe_resource *buf)
{
    struct r300_context* r300 = r300_context(pipe);
    struct r300_constant_buffer *cbuf;
    uint32_t *mapped = r300_buffer(buf)->user_buffer;

    switch (shader) {
        case PIPE_SHADER_VERTEX:
            cbuf = (struct r300_constant_buffer*)r300->vs_constants.state;
            break;
        case PIPE_SHADER_FRAGMENT:
            cbuf = (struct r300_constant_buffer*)r300->fs_constants.state;
            break;
        default:
            assert(0);
            return;
    }

    if (buf == NULL || buf->width0 == 0 ||
        (mapped = r300_buffer(buf)->constant_buffer) == NULL) {
        return;
    }

    if (shader == PIPE_SHADER_FRAGMENT ||
        (shader == PIPE_SHADER_VERTEX && r300->screen->caps.has_tcl)) {
        assert((buf->width0 % (4 * sizeof(float))) == 0);
        cbuf->ptr = mapped + index*4;
    }

    if (shader == PIPE_SHADER_VERTEX) {
        if (r300->screen->caps.has_tcl) {
            if (r300->vs_constants.size) {
                r300->vs_constants.dirty = TRUE;
            }
            r300->pvs_flush.dirty = TRUE;
        } else if (r300->draw) {
            draw_set_mapped_constant_buffer(r300->draw, PIPE_SHADER_VERTEX,
                0, mapped, buf->width0);
        }
    } else if (shader == PIPE_SHADER_FRAGMENT) {
        r300->fs_constants.dirty = TRUE;
    }
}

void r300_init_state_functions(struct r300_context* r300)
{
    r300->context.create_blend_state = r300_create_blend_state;
    r300->context.bind_blend_state = r300_bind_blend_state;
    r300->context.delete_blend_state = r300_delete_blend_state;

    r300->context.set_blend_color = r300_set_blend_color;

    r300->context.set_clip_state = r300_set_clip_state;
    r300->context.set_sample_mask = r300_set_sample_mask;

    r300->context.set_constant_buffer = r300_set_constant_buffer;

    r300->context.create_depth_stencil_alpha_state = r300_create_dsa_state;
    r300->context.bind_depth_stencil_alpha_state = r300_bind_dsa_state;
    r300->context.delete_depth_stencil_alpha_state = r300_delete_dsa_state;

    r300->context.set_stencil_ref = r300_set_stencil_ref;

    r300->context.set_framebuffer_state = r300_set_framebuffer_state;

    r300->context.create_fs_state = r300_create_fs_state;
    r300->context.bind_fs_state = r300_bind_fs_state;
    r300->context.delete_fs_state = r300_delete_fs_state;

    r300->context.set_polygon_stipple = r300_set_polygon_stipple;

    r300->context.create_rasterizer_state = r300_create_rs_state;
    r300->context.bind_rasterizer_state = r300_bind_rs_state;
    r300->context.delete_rasterizer_state = r300_delete_rs_state;

    r300->context.create_sampler_state = r300_create_sampler_state;
    r300->context.bind_fragment_sampler_states = r300_bind_sampler_states;
    r300->context.bind_vertex_sampler_states = r300_lacks_vertex_textures;
    r300->context.delete_sampler_state = r300_delete_sampler_state;

    r300->context.set_fragment_sampler_views = r300_set_fragment_sampler_views;
    r300->context.create_sampler_view = r300_create_sampler_view;
    r300->context.sampler_view_destroy = r300_sampler_view_destroy;

    r300->context.set_scissor_state = r300_set_scissor_state;

    r300->context.set_viewport_state = r300_set_viewport_state;

    r300->context.set_vertex_buffers = r300_set_vertex_buffers;
    r300->context.set_index_buffer = r300_set_index_buffer;

    r300->context.create_vertex_elements_state = r300_create_vertex_elements_state;
    r300->context.bind_vertex_elements_state = r300_bind_vertex_elements_state;
    r300->context.delete_vertex_elements_state = r300_delete_vertex_elements_state;

    r300->context.create_vs_state = r300_create_vs_state;
    r300->context.bind_vs_state = r300_bind_vs_state;
    r300->context.delete_vs_state = r300_delete_vs_state;
}
