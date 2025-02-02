/*
 * Copyright (C) 2016-2022 T-Head Semiconductor Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* CSI-NN2 version 2.0.x */

#include "shl_c908.h"

void gemm_int8_ncxhwx_12xpackn(int8_t *output, const int8_t *kernel, const int8_t *input,
                               const int32_t *bias, int m, int k, int n, int32_t out_zp,
                               int32_t *mult, int32_t *shift);
void gemm_int8_ncxhwx_8xpackn(int8_t *output, const int8_t *kernel, const int8_t *input,
                              const int32_t *bias, int m, int k, int n, int32_t out_zp,
                              int32_t *mult, int32_t *shift);

void shl_c908_ncxhwx_gemm_12xpackn_int8(int8_t *dst, const int8_t *sa, const int8_t *sb,
                                        const int32_t *bias, int m, int k, int n, int32_t out_zp,
                                        int32_t *mult, int32_t *shift)
{
    const int packn = csrr_vlenb() / sizeof(int8_t) / 2;

    int oc = 0;
    for (; oc + packn - 1 < m; oc += packn) {
        gemm_int8_ncxhwx_12xpackn(dst, sa, sb, bias, packn, k, n, out_zp, mult + oc, shift + oc);
        sa += packn * k;
        dst += packn * n;
        // please use fuse_zp2bias option in hhb, thus bias_data wont be NULL
        bias += packn;
    }
    if (oc < m) {
        gemm_int8_ncxhwx_12xpackn(dst, sa, sb, bias, m - oc, k, n, out_zp, mult + oc, shift + oc);
    }
}
