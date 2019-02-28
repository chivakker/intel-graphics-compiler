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

#include "Compiler/Optimizer/OpenCLPasses/CorrectlyRoundedDivSqrt/CorrectlyRoundedDivSqrt.hpp"
#include "Compiler/MetaDataApi/IGCMetaDataHelper.h"
#include "Compiler/IGCPassSupport.h"
#include "GenISAIntrinsics/GenIntrinsicInst.h"

#include "common/LLVMWarningsPush.hpp"
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>
#include "common/LLVMWarningsPop.hpp"

using namespace llvm;
using namespace IGC;
using namespace IGC::IGCMD;

// Register pass to igc-opt
#define PASS_FLAG "igc-correctly-rounded-div-sqrt"
#define PASS_DESCRIPTION "Ensures single precision divide and sqrt are correctly rounded"
#define PASS_CFG_ONLY false
#define PASS_ANALYSIS false
IGC_INITIALIZE_PASS_BEGIN(CorrectlyRoundedDivSqrt, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)
IGC_INITIALIZE_PASS_DEPENDENCY(MetaDataUtilsWrapper)
IGC_INITIALIZE_PASS_END(CorrectlyRoundedDivSqrt, PASS_FLAG, PASS_DESCRIPTION, PASS_CFG_ONLY, PASS_ANALYSIS)

char CorrectlyRoundedDivSqrt::ID = 0;

CorrectlyRoundedDivSqrt::CorrectlyRoundedDivSqrt() : ModulePass(ID),
    m_forceCR(false), m_hasHalfTy(false), m_IsCorrectlyRounded(false)
{
    initializeCorrectlyRoundedDivSqrtPass(*PassRegistry::getPassRegistry());
}

CorrectlyRoundedDivSqrt::CorrectlyRoundedDivSqrt(bool forceCR, bool HasHalf) : ModulePass(ID),
    m_forceCR(forceCR), m_hasHalfTy(HasHalf), m_IsCorrectlyRounded(false)
{
    initializeCorrectlyRoundedDivSqrtPass(*PassRegistry::getPassRegistry());
}

bool CorrectlyRoundedDivSqrt::runOnModule(Module &M) 
{
    // Was the module compiled with the CR flag on?
    m_IsCorrectlyRounded = getAnalysis<MetaDataUtilsWrapper>().getModuleMetaData()->compOpt.CorrectlyRoundedDivSqrt;

    // Even if it wasn't, it's possible that CR was requested through a build-time option
    // (This is relevant at least for SPIR)
    if( !m_IsCorrectlyRounded && !m_forceCR )
    {
        return false;
    }

    m_changed = false;

    for( Function &F : M )
    {
        if( F.isDeclaration() )
        {
            if (!m_hasHalfTy)
                m_changed |= processDeclaration(F);
        }
        else
        {
            visit(F);
        }
    }
    return m_changed;
}

bool CorrectlyRoundedDivSqrt::processDeclaration(Function &F)
{
    StringRef name = F.getName();
    if( name.startswith("_Z4sqrt") )
    {
        std::string newName = name.str();
        newName[2] = '7';
        newName.insert(7, "_cr");
        F.setName(newName);
        return true;
    }
    else if (name.startswith("__builtin_spirv_OpenCL_sqrt_") &&
             !name.startswith("__builtin_spirv_OpenCL_sqrt_cr"))
    {
        std::string newName = name.str();
        newName.insert(28, "cr_");
        F.setName(newName);
        return true;
    }

    // not sqrt function
    return false;
}

Value *CorrectlyRoundedDivSqrt::emitIEEEDivide(BinaryOperator *I, Value *Op0, Value *Op1)
{
    Type *Ty = Op0->getType();
    IRBuilder<> IRB(I);

    auto* IEEEDivide = GenISAIntrinsic::getDeclaration(
        I->getModule(),
        GenISAIntrinsic::GenISA_IEEE_Divide,
        Ty->getScalarType());

    Value *Divide = nullptr;
    if (!isa<VectorType>(Ty))
    {
        Value *Args[] = { Op0, Op1 };
        Divide = IRB.CreateCall(IEEEDivide, Args);
    }
    else
    {
        unsigned VecLen = Ty->getVectorNumElements();
        Divide = UndefValue::get(Ty);
        for (unsigned i = 0; i < VecLen; i++)
        {
            auto *SOp0 = IRB.CreateExtractElement(Op0, i);
            auto *SOp1 = IRB.CreateExtractElement(Op1, i);
            Value *Args[] = { SOp0, SOp1 };
            auto *ScalarDivide = IRB.CreateCall(IEEEDivide, Args);
            Divide = IRB.CreateInsertElement(Divide, ScalarDivide, i);
        }
    }

    return Divide;
}

void CorrectlyRoundedDivSqrt::visitFDiv(BinaryOperator &I)
{
    Type *Ty = I.getType();

    if (Ty->getScalarType()->isFloatTy())
    {
        auto *Divide = emitIEEEDivide(&I, I.getOperand(0), I.getOperand(1));

        I.replaceAllUsesWith(Divide);
        I.eraseFromParent();
        m_changed = true;
    }
}
