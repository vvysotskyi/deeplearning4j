/*******************************************************************************
 * Copyright (c) 2015-2018 Skymind, Inc.
 * Copyright (c) 2019-2020 Konduit K.K.
 *
/* ******************************************************************************
 *
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 *  See the NOTICE file distributed with this work for additional
 *  information regarding copyright ownership.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
//  @author raver119@gmail.com
//

#ifndef ND4J_CONTEXT_PROTOTYPE_H
#define ND4J_CONTEXT_PROTOTYPE_H

#include <vector>
#include <system/Environment.h>
#include <array/DataType.h>
#include <system/dll.h>
#include <graph/RandomGenerator.h>
#include <ops/declarable/OpDescriptor.h>
#include <execution/Engine.h>
#include <execution/ExecutionMode.h>

#ifndef __STANDALONE_BUILD__
#include <config.h>
#endif

namespace sd {
    namespace graph {

        class ND4J_EXPORT ContextPrototype {
        protected:
            // int ids of the input nodes
            std::vector<std::pair<int, int>> _inputs;
            int _nodeId;
            std::vector<double> _tArgs;
            std::vector<int> _iArgs;
            std::vector<bool> _bArgs;
            std::vector<int> _axis;
            std::vector<sd::DataType> _dArgs;

            // TODO: remove this field
			sd::DataType _dataType = sd::DataType::FLOAT32;
			bool _isInplace;

            // opNum for legacy XYZ ops
            int _opNum = -1;
            uint64_t _rootSeed;
            RandomGenerator _randomGenerator;

            std::vector<sd::DataType> _dataTypes;

            sd::ops::OpDescriptor* _opDescriptor;
            bool _useONEDNN = sd::Environment::getInstance().isUseONEDNN();

            // target engine for execution
            samediff::Engine _engine = DEFAULT_ENGINE;

            samediff::ExecutionMode _execMode = samediff::ExecutionMode::MODE_UNDEFINED;
        public:
            explicit ContextPrototype(sd::ops::OpDescriptor* opDescriptor = nullptr, int nodeId = 1, bool inPlace = false);
            ~ContextPrototype() = default;

            int getNodeId();
            int nodeId();

            // this method returns true, if inputs are defined
            bool hasVariablesFilled();

            void setOpDescriptor(sd::ops::OpDescriptor* opDescriptor);

            virtual sd::DataType dataType();
            virtual sd::DataType dataType(int index);
            virtual void setDataType(int index, sd::DataType type);

            bool isInplace();
            void markInplace(bool reallyInplace);

            void pickInput(int input);
            void pickInput(int input, int index);
            void pickInput(std::pair<int, int>& p);
            void fillInputs(std::initializer_list<int> inputs);
            void fillInputs(std::vector<int>& inputs);
            std::vector<std::pair<int, int>>* inputs();

            std::vector<double>* getTArguments();
            std::vector<int>* getIArguments();
            std::vector<bool>* getBArguments();
            std::vector<sd::DataType>* getDArguments();
            std::vector<int>* getAxis();

            samediff::Engine engine();

            size_t numT();
            size_t numI();
            size_t numB();
            size_t numD();

            std::pair<int, int>* input(int idx);

            int opNum();
            void setOpNum(int opNum);

            bool isUseONEDNN() { return _useONEDNN; }
            void setUseONEDNN(bool useONEDNN) { _useONEDNN = useONEDNN; }

            /**
             * This method returns number of inputs available in this block
             * @return
             */
            virtual unsigned long width();

            // just a clone
            ContextPrototype* clone();

            template <typename N>
            ContextPrototype* asT();

            RandomGenerator& randomGenerator() {return _randomGenerator;}
            RandomGenerator const& getRng()const { return _randomGenerator; }
            void setRng(RandomGenerator const& anotherRng) { _randomGenerator = anotherRng; }
            void setRandomGenerator(RandomGenerator const& anotherRng) { _randomGenerator = anotherRng; }
            uint64_t randomSeed() const { return _rootSeed; }
            void setRandomSeed(uint64_t seed) { _rootSeed = seed; }
        };
    }
}

#endif //ND4J_CONTEXT_PROTOTYPE_H
