/*===================== begin_copyright_notice ==================================

Copyright (c) 2017 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


======================= end_copyright_notice ==================================*/

//===- SPIRVOpCode.h - Class to represent SPIR-V Operation Codes -*- C++ -*-===//
//
//                     The LLVM/SPIRV Translator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimers.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimers in the documentation
// and/or other materials provided with the distribution.
// Neither the names of Advanced Micro Devices, Inc., nor the names of its
// contributors may be used to endorse or promote products derived from this
// Software without specific prior written permission.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
// THE SOFTWARE.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file defines Operation Code class for SPIR-V.
///
//===----------------------------------------------------------------------===//

#ifndef SPIRVOPCODE_HPP_
#define SPIRVOPCODE_HPP_

#include "SPIRVUtil.h"
#include "spirv.hpp"
#include <string>


namespace spv{

template<> inline void
SPIRVMap<Op, std::string>::init() {
#define _SPIRV_OP(x, ...) add(Op##x, #x);
#include "SPIRVOpCodeEnum.h"
#undef _SPIRV_OP
}
SPIRV_DEF_NAMEMAP(Op, OpCodeNameMap)

inline bool isAtomicOpCode(Op OpCode) {
  assert(OpAtomicLoad < OpAtomicXor);
  return ((unsigned)OpCode >= OpAtomicLoad
      && (unsigned)OpCode <= OpAtomicXor)
      || OpCode == OpAtomicFlagTestAndSet
      || OpCode == OpAtomicFlagClear;
}
inline bool isBinaryOpCode(Op OpCode) {
  return ((unsigned)OpCode >= OpIAdd &&
      (unsigned)OpCode <= OpFMod) ||
      OpCode == OpDot;
}

inline bool isShiftOpCode(Op OpCode) {
  return (unsigned)OpCode >= OpShiftRightLogical &&
      (unsigned)OpCode <= OpShiftLeftLogical;
}

inline bool isLogicalOpCode(Op OpCode) {
  return (unsigned)OpCode >= OpLogicalEqual &&
      (unsigned)OpCode <= OpLogicalNot;
}

inline bool isBitwiseOpCode(Op OpCode) {
  return (unsigned)OpCode >= OpBitwiseOr &&
      (unsigned)OpCode <= OpBitwiseAnd;
}

inline bool isBinaryShiftLogicalBitwiseOpCode(Op OpCode) {
  return (((unsigned)OpCode >= OpShiftRightLogical &&
      (unsigned)OpCode <= OpBitwiseAnd) ||
      isBinaryOpCode(OpCode));
}

inline bool isCmpOpCode(Op OpCode) {
  return ((unsigned)OpCode >= OpIEqual &&
      (unsigned)OpCode <= OpFUnordGreaterThanEqual) ||
      (OpCode >= OpLessOrGreater && OpCode <= OpLogicalNotEqual);
}

inline bool isCvtOpCode(Op OpCode) {
  return ((unsigned)OpCode >= OpConvertFToU &&
      (unsigned)OpCode <= OpBitcast) ||
      OpCode == OpSatConvertSToU ||
      OpCode == OpSatConvertUToS;
}

inline bool isCvtToUnsignedOpCode(Op OpCode) {
  return OpCode == OpConvertFToU ||
      OpCode == OpUConvert ||
      OpCode == OpSatConvertSToU;
}

inline bool isCvtFromUnsignedOpCode(Op OpCode) {
  return OpCode == OpConvertUToF ||
      OpCode == OpUConvert ||
      OpCode == OpSatConvertUToS;
}

inline bool isOpaqueGenericTypeOpCode(Op OpCode) {
  return (unsigned)OpCode >= OpTypeEvent &&
      (unsigned)OpCode <= OpTypePipe;
}

inline bool isGenericNegateOpCode(Op OpCode) {
  return (unsigned)OpCode == OpSNegate ||
      (unsigned)OpCode == OpFNegate ||
      (unsigned)OpCode == OpNot;
}

inline bool isAccessChainOpCode(Op OpCode) {
  return OpCode == OpAccessChain ||
      OpCode == OpInBoundsAccessChain;
}

inline bool hasExecScope(Op OpCode) {
  unsigned OC = OpCode;
  return (OpGroupWaitEvents <= OC &&
            OC <= OpGroupSMax) ||
      (OpGroupReserveReadPipePackets <= OC &&
          OC <= OpGroupCommitWritePipe);
}

inline bool hasGroupOperation(Op OpCode) {
  unsigned OC = OpCode;
  return OpGroupIAdd <= OC && OC <= OpGroupSMax;
}

inline bool isTypeOpCode(Op OpCode) {
  unsigned OC = OpCode;
  return OpTypeVoid <= OC && OC <= OpTypePipe;
}

inline bool isConstantOpCode(Op OpCode) {
  unsigned OC = OpCode;
  return (OpConstantTrue <= OC
      && OC <= OpSpecConstantOp)
      || OC == OpUndef;
}

inline bool isModuleScopeAllowedOpCode(Op OpCode) {
  return OpCode == OpVariable ||
      isConstantOpCode(OpCode);
}

inline bool isImageOpCode(Op OpCode) {
    return (OpCode >= OpImageSampleImplicitLod) && (OpCode <= OpImageWrite);
}

inline bool isIntelSubgroupOpCode(Op OpCode) {
  unsigned OC = OpCode;
  return OpSubgroupShuffleINTEL <= OC && OC <= OpSubgroupImageMediaBlockWriteINTEL;
}

}

#endif /* SPIRVOPCODE_HPP_ */
