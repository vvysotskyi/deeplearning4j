/*
 *  ******************************************************************************
 *  *
 *  *
 *  * This program and the accompanying materials are made available under the
 *  * terms of the Apache License, Version 2.0 which is available at
 *  * https://www.apache.org/licenses/LICENSE-2.0.
 *  *
 *  * See the NOTICE file distributed with this work for additional
 *  * information regarding copyright ownership.
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  * License for the specific language governing permissions and limitations
 *  * under the License.
 *  *
 *  * SPDX-License-Identifier: Apache-2.0
 *  *****************************************************************************
 */

 //
 //  @author Oleg Semeniv <oleg.semeniv@gmail.com>
 //
 //

#include <ops/declarable/PlatformHelper.h>
#include <ops/declarable/OpRegistrator.h>
#include <system/platform_boilerplate.h>
#include <helpers/MKLDNNStream.h>
#include "mkldnnUtils.h"

using namespace dnnl;

namespace sd {
    namespace ops {
        namespace platforms {


            //////////////////////////////////////////////////////////////////////
            static void softmaxMKLDNN(const NDArray* x, NDArray* z, const int axis) {

                dnnl::memory::dims shape = x->getShapeAsFlatVector();

                const int xRank = x->rankOf();

                dnnl::memory::format_tag xFormat = onednnUtils::getFormat(*x);
                dnnl::memory::format_tag zFormat = onednnUtils::getFormat(*z);

                // optimized cases
                if (2 == xRank && 0 == axis) {
                    if(x->ews() == 1)
                        xFormat = dnnl::memory::format_tag::ba;
                    if(z->ews() == 1)
                        zFormat = dnnl::memory::format_tag::ba;
                }
                else if (4 == xRank && 1 == axis && (x->sizeAt(2) * x->sizeAt(3)) > 1) {
                    if(x->ews() == 1)
                        xFormat = dnnl::memory::format_tag::acdb;
                    if(z->ews() == 1)
                        zFormat = dnnl::memory::format_tag::acdb;
                }

                dnnl::memory::data_type xType = dnnl::memory::data_type::f32;

                dnnl::memory::desc x_mkl_md, x_user_md, z_mkl_md, z_user_md;

                x_user_md = x_mkl_md = dnnl::memory::desc(shape, xType, xFormat);
                onednnUtils::setBlockStrides(*x, x_user_md);

                // z
                z_user_md = z_mkl_md = dnnl::memory::desc(shape, xType, zFormat);
                onednnUtils::setBlockStrides(*z, z_user_md);

                auto engine = onednnUtils::getEngine(LaunchContext::defaultContext()->engine());

                // Create attributes (to handle alpha and beta if necessary)
                dnnl::primitive_attr attr; // it is empty since we have usual values for alpha (=1) and beta (=0)

                // operation primitive description
                dnnl::softmax_forward::desc op_desc(dnnl::prop_kind::forward_inference, x_mkl_md, axis);

                dnnl::softmax_forward::primitive_desc op_prim_desc(op_desc, attr, engine);

                // arguments (memory buffers) necessary for calculations
                std::unordered_map<int, dnnl::memory> args;

                dnnl::stream stream(engine);

                // provide memory buffers and check whether reorder is required

                // input
                onednnUtils::loadDataToMklStream(*x, engine, stream, x_user_md, op_prim_desc.src_desc(), args[DNNL_ARG_SRC]);

                // z
                auto z_user_mem = onednnUtils::loadDataToMklStream(*z, engine, stream, z_user_md, op_prim_desc.dst_desc(), args[DNNL_ARG_DST]);

                // run calculations
                dnnl::softmax_forward(op_prim_desc).execute(stream, args);

                // reorder outputs if necessary
                if (op_prim_desc.dst_desc() != z_user_mem.get_desc())
                    dnnl::reorder(args[DNNL_ARG_DST], z_user_mem).execute(stream, args[DNNL_ARG_DST], z_user_mem);

                stream.wait();
            }


            PLATFORM_IMPL(softmax, ENGINE_CPU) {

                auto input = INPUT_VARIABLE(0);
                auto output = OUTPUT_VARIABLE(0);

                const int rank = input->rankOf();
                int dim = block.getIArguments()->size() > 0 ? INT_ARG(0) : rank - 1;

                if (dim < 0) {
                    dim += rank;
                }

                REQUIRE_TRUE(dim < rank && dim >= 0, 0, "SOFTMAX_MKLDNN OP: the value of input integer parameter (dimension) must be less than input array rank %i, but got dimension = %i instead !", rank, dim);

                REQUIRE_TRUE(rank <= 6, 0, "SOFTMAX_MKLDNN OP: the rank of input must be less or qual 6, but got rank = %i instead !", rank);

                // mkldnnSoftMax
                softmaxMKLDNN(input, output, dim);

                return Status::OK();
            }

            PLATFORM_CHECK(softmax, ENGINE_CPU) {

                auto x = INPUT_VARIABLE(0);
                auto z = OUTPUT_VARIABLE(0);

                Requirements req("ONEDNN SOFTMAX OP");
                req.expectTrue(block.isUseONEDNN(), IS_USE_ONEDNN_MSG) &&
                req.expectFalse(makeInfoVariable(x->isEmpty(), IS_EMPTY_MSG_INPUT), EXPECTED_FALSE) &&
                req.expectLess(makeInfoVariable(x->rankOf(), RANK_MSG_INPUT), 7) &&
                req.expectGreater(makeInfoVariable(x->rankOf(), RANK_MSG_INPUT), 2) &&
                req.expectEq(makeInfoVariable(x->dataType(), TYPE_MSG_INPUT), DataType::FLOAT32) &&
                req.expectEq(makeInfoVariable(z->dataType(), TYPE_MSG_OUTPUT), DataType::FLOAT32);
                req.logTheSuccess();
                return req;

            }

            //////////////////////////////////////////////////////////////////////
            static void softmaxBpMKLDNN(const NDArray* x, const NDArray* dLdz, NDArray* dLdx, const int axis) {

                dnnl::memory::desc x_user_md, x_mkl_md, dLdx_mkl_md, dLdx_user_md, dLdz_mkl_md, dLdz_user_md;

                // x
                x_mkl_md = x_user_md = dnnl::memory::desc(x->getShapeAsFlatVector(), dnnl::memory::data_type::f32, onednnUtils::getFormat(*x));
                onednnUtils::setBlockStrides(*x, x_user_md);

                // dLdx
                dLdx_mkl_md = dLdx_user_md = dnnl::memory::desc(dLdx->getShapeAsFlatVector(), dnnl::memory::data_type::f32, onednnUtils::getFormat(*dLdx));
                onednnUtils::setBlockStrides(*dLdx, dLdx_user_md);

                // dLdz
                dLdz_mkl_md = dLdz_user_md = dnnl::memory::desc(dLdz->getShapeAsFlatVector(), dnnl::memory::data_type::f32, onednnUtils::getFormat(*dLdz));
                onednnUtils::setBlockStrides(*dLdz, dLdz_user_md);

                auto engine = onednnUtils::getEngine(LaunchContext::defaultContext()->engine());

                // operation primitive description
                // forward description
                dnnl::softmax_forward::desc op_ff_desc(dnnl::prop_kind::forward_inference, x_mkl_md, axis);
                dnnl::softmax_forward::primitive_desc op_ff_prim_desc(op_ff_desc, engine);

                // backward description
                dnnl::softmax_backward::desc op_bp_desc(dLdz_mkl_md, dLdx_mkl_md, axis);
                dnnl::softmax_backward::primitive_desc op_bp_prim_desc(op_bp_desc, engine, op_ff_prim_desc);

                // arguments (memory buffers) necessary for calculations
                std::unordered_map<int, dnnl::memory> argsbp, argsff;

                dnnl::stream stream(engine);

                // provide memory buffers and check whether reorder is required for forward
                // input
                onednnUtils::loadDataToMklStream(*x, engine, stream, x_user_md, op_ff_prim_desc.src_desc(), argsff[DNNL_ARG_SRC]);

                // dLdz
                onednnUtils::loadDataToMklStream(*dLdz, engine, stream, dLdz_user_md, op_bp_prim_desc.diff_dst_desc(), argsbp[DNNL_ARG_DIFF_DST]);

                // dLdx
                auto dLdx_user_mem = onednnUtils::loadDataToMklStream(*dLdx, engine, stream, dLdx_user_md, op_ff_prim_desc.src_desc(), argsff[DNNL_ARG_DST]);

                // check and arg set for backprob
                argsbp[DNNL_ARG_DIFF_SRC] = argsff[DNNL_ARG_DST];
                argsbp[DNNL_ARG_DST]      = argsff[DNNL_ARG_DST];


                // run calculations forward
                dnnl::softmax_forward(op_ff_prim_desc).execute(stream, argsff);

                // run calculations backward
                dnnl::softmax_backward(op_bp_prim_desc).execute(stream, argsbp);

                // reorder outputs if necessary
                if (op_ff_prim_desc.dst_desc() != dLdx_user_mem.get_desc())
                    dnnl::reorder(argsff[DNNL_ARG_DST], dLdx_user_mem).execute(stream, argsff[DNNL_ARG_DST], dLdx_user_mem);

                stream.wait();
            }


            PLATFORM_IMPL(softmax_bp, ENGINE_CPU) {

                auto input = INPUT_VARIABLE(0);
                auto dLdz = INPUT_VARIABLE(1);
                auto dLdx = OUTPUT_VARIABLE(0);

                const int rank = input->rankOf();
                const int dLdzRank = dLdz->rankOf();
                int dim = block.getIArguments()->size() > 0 ? INT_ARG(0) : rank - 1;

                if (dim < 0) {
                    dim += rank;
                }

                REQUIRE_TRUE(dim < rank && dim >= 0, 0, "SOFTMAX_MKLDNN_BP OP: the value of input integer parameter (dimension) must be less than input array rank %i, but got dimension = %i instead !", rank, dim);

                REQUIRE_TRUE(rank <= 6 && dLdzRank <= 6, 0, "SOFTMAX_MKLDNN_BP OP: the rank of input and dLdz must be less or qual 6, but got input rank = %i and dLdz rank rank = %i instead !", rank, dLdzRank);

                // mkldnnSoftMax
                softmaxBpMKLDNN(input, dLdz, dLdx, dim);

                return Status::OK();
            }

            PLATFORM_CHECK(softmax_bp, ENGINE_CPU) {

                auto x = INPUT_VARIABLE(0);
                auto dLdz = INPUT_VARIABLE(1);
                auto dLdx = OUTPUT_VARIABLE(0);

                Requirements req("ONEDNN SOFTMAX_BP OP");
                req.expectTrue(block.isUseONEDNN(), IS_USE_ONEDNN_MSG) &&
                req.expectFalse(makeInfoVariable(x->isEmpty(), IS_EMPTY_MSG_INPUT0), EXPECTED_FALSE) &&
                req.expectFalse(makeInfoVariable(dLdz->isEmpty(), IS_EMPTY_MSG_INPUT1), EXPECTED_FALSE) &&
                req.expectLess(makeInfoVariable(x->rankOf(), RANK_MSG_INPUT0), 7) &&
                req.expectEq(makeInfoVariable(x->rankOf(), RANK_MSG_INPUT0), makeInfoVariable(dLdz->rankOf(), RANK_MSG_INPUT1)) &&
                req.expectGreater(makeInfoVariable(x->rankOf(), RANK_MSG_INPUT), 0) &&
                req.expectEq(makeInfoVariable(x->dataType(), TYPE_MSG_INPUT0), DataType::FLOAT32) &&
                req.expectEq(makeInfoVariable(dLdz->dataType(), TYPE_MSG_INPUT1), DataType::FLOAT32) &&
                req.expectEq(makeInfoVariable(dLdx->dataType(), TYPE_MSG_OUTPUT), DataType::FLOAT32) &&
                req.expect(makeShapeInfoVariable(x, SHAPE_MSG_INPUT0), makeShapeInfoVariable(dLdz, SHAPE_MSG_INPUT1),
                    [](const decltype(x)& l, const decltype(dLdz)& r){
                        for (int i = 0; i < l->rankOf(); i++) {
                            if (l->sizeAt(i) != r->sizeAt(i)) {
                                return false;
                            }
                        }
                        return true;
                    }
                 , EXPECTED_EQ_MSG);
                req.logTheSuccess();
                return req;

            }

        }
    }
}
