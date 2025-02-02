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

#include "shl_thead_rvv.h"

/*************************************************************
    note: VLEN = 128/256
*************************************************************/
int shl_rvv_avgpool2x2s2_fp16(struct csinn_tensor *input, struct csinn_tensor *output,
                              struct csinn_pool_params *params)
{
    __fp16 *input_data = (__fp16 *)input->data;
    __fp16 *output_data = (__fp16 *)output->data;

    int batch = input->dim[0];
    int in_c = input->dim[1];
    int in_h = input->dim[2];
    int in_w = input->dim[3];
    int input_size = in_c * in_h * in_w;

    int out_h = output->dim[2];
    int out_w = output->dim[3];
    int out_hw = out_h * out_w;
    int output_size = in_c * out_h * out_w;

    int extend_h = 0;
    int extend_w = 0;

    if (in_h % 2 == 1 && params->pad_down == 1) {
        extend_h = 1;
        out_h--;
    }
    if (in_w % 2 == 1 && params->pad_right == 1) {
        extend_w = 1;
        out_w--;
    }

    int remain_w = in_w - 2 * out_w;
    __fp16 ratio = 0.25f;
    int vl;

    for (int b = 0; b < batch; b++) {
        for (int c = 0; c < in_c; c++) {
            const __fp16 *line0 = input_data + c * in_h * in_w;
            const __fp16 *line1 = line0 + in_w;
            __fp16 *outptr = output_data + c * out_hw;

            for (int h = 0; h < out_h; h++) {
                ratio = 0.25f;
                int w = out_w;
                while (w > 0) {
                    vl = vsetvl_e16m1(w);
                    vfloat16m1_t _line0_0_6, _line0_1_7;
                    vfloat16m1_t _line1_0_6, _line1_1_7;

                    vlseg2e16_v_f16m1(&_line0_0_6, &_line0_1_7, line0, vl);
                    vlseg2e16_v_f16m1(&_line1_0_6, &_line1_1_7, line1, vl);

                    vfloat16m1_t _sum0 = vfadd_vv_f16m1(_line0_0_6, _line0_1_7, vl);
                    vfloat16m1_t _sum1 = vfadd_vv_f16m1(_line1_0_6, _line1_1_7, vl);
                    vfloat16m1_t _sum = vfadd_vv_f16m1(_sum0, _sum1, vl);
                    vfloat16m1_t _avg = vfmul_vf_f16m1(_sum, ratio, vl);

                    vse16_v_f16m1(outptr, _avg, vl);
                    line0 += 2 * vl;
                    line1 += 2 * vl;
                    outptr += vl;
                    w -= vl;
                }
                if (extend_w) {
                    ratio = (params->count_include_pad) ? 0.25f : 0.5f;
                    outptr[0] = (line0[0] + line1[0]) * ratio;
                    outptr++;
                }
                line0 += remain_w + in_w;
                line1 += remain_w + in_w;
            }
            if (extend_h) {
                ratio = (params->count_include_pad) ? 0.25f : 0.5f;
                int w = out_w;
                while (w > 0) {
                    vl = vsetvl_e16m1(w);
                    vfloat16m1_t _line0_0_6, _line0_1_7;
                    vlseg2e16_v_f16m1(&_line0_0_6, &_line0_1_7, line0, vl);

                    vfloat16m1_t _sum0 = vfadd_vv_f16m1(_line0_0_6, _line0_1_7, vl);
                    vfloat16m1_t _avg = vfmul_vf_f16m1(_sum0, ratio, vl);

                    vse16_v_f16m1(outptr, _avg, vl);
                    line0 += 2 * vl;
                    outptr += vl;
                    w -= vl;
                }
                if (extend_w) {
                    ratio = (params->count_include_pad) ? 0.25f : 1.0f;
                    outptr[0] = line0[0] * ratio;
                    outptr++;
                }
            }
        }
        input_data += input_size;
        output_data += output_size;
    }
    return CSINN_TRUE;
}

int shl_rvv_avgpool2x2s2_p1_fp16(struct csinn_tensor *input, struct csinn_tensor *output,
                                 struct csinn_pool_params *params)
{
    __fp16 *input_data = (__fp16 *)input->data;
    __fp16 *output_data = (__fp16 *)output->data;

    int batch = input->dim[0];
    int in_c = input->dim[1];
    int in_h = input->dim[2];
    int in_w = input->dim[3];
    int input_size = in_c * in_h * in_w;

    int out_h = output->dim[2];
    int out_w = output->dim[3];
    int out_hw = out_h * out_w;
    int output_size = in_c * out_h * out_w;

    int extend_h = 0;
    int extend_w = 0;

    if (in_h % 2 == 0 && params->pad_down == 1) {
        extend_h = 1;
        out_h--;
    }
    if (in_w % 2 == 0 && params->pad_right == 1) {
        extend_w = 1;
        out_w--;
    }

    int remain_w = in_w - 2 * out_w + 1;
    __fp16 ratio = 0.25f;
    int vl;

    for (int b = 0; b < batch; b++) {
        for (int c = 0; c < in_c; c++) {
            const __fp16 *line00 = input_data + c * in_h * in_w;
            __fp16 *outptr = output_data + c * out_hw;

            // h top ---- w left
            ratio = (params->count_include_pad) ? 0.25f : 1.0f;
            outptr[0] = line00[0] * ratio;
            outptr++;
            line00++;
            // h top ---- w mid
            ratio = (params->count_include_pad) ? 0.25f : 0.5f;
            int w = out_w - 1;
            while (w > 0) {
                vl = vsetvl_e16m1(w);
                vfloat16m1_t _line0_0_6, _line0_1_7;
                vlseg2e16_v_f16m1(&_line0_0_6, &_line0_1_7, line00, vl);

                vfloat16m1_t _sum0 = vfadd_vv_f16m1(_line0_0_6, _line0_1_7, vl);
                vfloat16m1_t _avg = vfmul_vf_f16m1(_sum0, ratio, vl);

                vse16_v_f16m1(outptr, _avg, vl);
                line00 += 2 * vl;
                outptr += vl;
                w -= vl;
            }

            // h top ---- w right
            ratio = (params->count_include_pad) ? 0.25f : 1.0f;
            if (extend_w) {
                outptr[0] = line00[0] * ratio;
                outptr++;
            }
            line00 += remain_w;

            // h mid
            const __fp16 *line0 = line00;
            const __fp16 *line1 = line0 + in_w;
            for (int h = 0; h < out_h - 1; h++) {
                // h mid ---- w left
                ratio = (params->count_include_pad) ? 0.25f : 0.5f;
                outptr[0] = (line0[0] + line1[0]) * ratio;
                outptr++;
                line0++;
                line1++;
                // h mid ---- w mid
                ratio = 0.25f;
                int w = out_w - 1;
                while (w > 0) {
                    vl = vsetvl_e16m1(w);
                    vfloat16m1_t _line0_0_6, _line0_1_7;
                    vfloat16m1_t _line1_0_6, _line1_1_7;

                    vlseg2e16_v_f16m1(&_line0_0_6, &_line0_1_7, line0, vl);
                    vlseg2e16_v_f16m1(&_line1_0_6, &_line1_1_7, line1, vl);

                    vfloat16m1_t _sum0 = vfadd_vv_f16m1(_line0_0_6, _line0_1_7, vl);
                    vfloat16m1_t _sum1 = vfadd_vv_f16m1(_line1_0_6, _line1_1_7, vl);
                    vfloat16m1_t _sum = vfadd_vv_f16m1(_sum0, _sum1, vl);
                    vfloat16m1_t _avg = vfmul_vf_f16m1(_sum, ratio, vl);

                    vse16_v_f16m1(outptr, _avg, vl);
                    line0 += 2 * vl;
                    line1 += 2 * vl;
                    outptr += vl;
                    w -= vl;
                }
                // h mid ---- w right
                ratio = (params->count_include_pad) ? 0.25f : 0.5f;
                if (extend_w) {
                    outptr[0] = (line0[0] + line1[0]) * ratio;
                    outptr++;
                }
                line0 += remain_w + in_w;
                line1 += remain_w + in_w;
            }
            // h bottom
            if (extend_h) {
                // h bottom ---- w left
                ratio = (params->count_include_pad) ? 0.25f : 1.0f;
                outptr[0] = line0[0] * ratio;
                outptr++;
                line0++;
                // h bottom ---- w mid
                ratio = (params->count_include_pad) ? 0.25f : 0.5f;
                int w = out_w - 1;
                while (w > 0) {
                    vl = vsetvl_e16m1(w);
                    vfloat16m1_t _line0_0_6, _line0_1_7;
                    vlseg2e16_v_f16m1(&_line0_0_6, &_line0_1_7, line0, vl);

                    vfloat16m1_t _sum0 = vfadd_vv_f16m1(_line0_0_6, _line0_1_7, vl);
                    vfloat16m1_t _avg = vfmul_vf_f16m1(_sum0, ratio, vl);

                    vse16_v_f16m1(outptr, _avg, vl);
                    line0 += 2 * vl;
                    outptr += vl;
                    w -= vl;
                }
                // h bottom ---- w right
                ratio = (params->count_include_pad) ? 0.25f : 1.0f;
                if (extend_w) {
                    outptr[0] = line0[0] * ratio;
                }
            }
        }
        input_data += input_size;
        output_data += output_size;
    }
    return CSINN_TRUE;
}
