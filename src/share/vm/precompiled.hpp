/*
 * Copyright (c) 2010, 2011, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

// Precompiled headers are turned off for Sun Studion,
// or if the user passes USE_PRECOMPILED_HEADER=0 to the makefiles.
#ifndef DONT_USE_PRECOMPILED_HEADER

# include "asm/assembler.hpp"
# include "asm/assembler.inline.hpp"
# include "asm/codeBuffer.hpp"
# include "asm/register.hpp"
# include "ci/ciArray.hpp"
# include "ci/ciArrayKlass.hpp"
# include "ci/ciClassList.hpp"
# include "ci/ciConstant.hpp"
# include "ci/ciConstantPoolCache.hpp"
# include "ci/ciEnv.hpp"
# include "ci/ciExceptionHandler.hpp"
# include "ci/ciField.hpp"
# include "ci/ciFlags.hpp"
# include "ci/ciInstance.hpp"
# include "ci/ciInstanceKlass.hpp"
# include "ci/ciInstanceKlassKlass.hpp"
# include "ci/ciKlass.hpp"
# include "ci/ciKlassKlass.hpp"
# include "ci/ciMethod.hpp"
# include "ci/ciNullObject.hpp"
# include "ci/ciObjArrayKlass.hpp"
# include "ci/ciObject.hpp"
# include "ci/ciObjectFactory.hpp"
# include "ci/ciSignature.hpp"
# include "ci/ciStreams.hpp"
# include "ci/ciSymbol.hpp"
# include "ci/ciType.hpp"
# include "ci/ciTypeArrayKlass.hpp"
# include "ci/ciUtilities.hpp"
# include "ci/compilerInterface.hpp"
# include "classfile/classFileParser.hpp"
# include "classfile/classFileStream.hpp"
# include "classfile/classLoader.hpp"
# include "classfile/javaClasses.hpp"
# include "classfile/symbolTable.hpp"
# include "classfile/systemDictionary.hpp"
# include "classfile/vmSymbols.hpp"
# include "code/codeBlob.hpp"
# include "code/codeCache.hpp"
# include "code/compressedStream.hpp"
# include "code/debugInfo.hpp"
# include "code/debugInfoRec.hpp"
# include "code/dependencies.hpp"
# include "code/exceptionHandlerTable.hpp"
# include "code/jvmticmlr.h"
# include "code/location.hpp"
# include "code/nmethod.hpp"
# include "code/oopRecorder.hpp"
# include "code/pcDesc.hpp"
# include "code/relocInfo.hpp"
# include "code/stubs.hpp"
# include "code/vmreg.hpp"
# include "compiler/disassembler.hpp"
# include "compiler/methodLiveness.hpp"
# include "compiler/oopMap.hpp"
# include "gc_implementation/shared/adaptiveSizePolicy.hpp"
# include "gc_implementation/shared/ageTable.hpp"
# include "gc_implementation/shared/allocationStats.hpp"
# include "gc_implementation/shared/cSpaceCounters.hpp"
# include "gc_implementation/shared/collectorCounters.hpp"
# include "gc_implementation/shared/gSpaceCounters.hpp"
# include "gc_implementation/shared/gcStats.hpp"
# include "gc_implementation/shared/gcUtil.hpp"
# include "gc_implementation/shared/generationCounters.hpp"
# include "gc_implementation/shared/immutableSpace.hpp"
# include "gc_implementation/shared/markSweep.hpp"
# include "gc_implementation/shared/markSweep.inline.hpp"
# include "gc_implementation/shared/mutableSpace.hpp"
# include "gc_implementation/shared/spaceCounters.hpp"
# include "gc_implementation/shared/spaceDecorator.hpp"
# include "gc_interface/collectedHeap.hpp"
# include "gc_interface/collectedHeap.inline.hpp"
# include "gc_interface/gcCause.hpp"
# include "interpreter/abstractInterpreter.hpp"
# include "interpreter/bytecode.hpp"
# include "interpreter/bytecodeHistogram.hpp"
# include "interpreter/bytecodeInterpreter.hpp"
# include "interpreter/bytecodeInterpreter.inline.hpp"
# include "interpreter/bytecodeTracer.hpp"
# include "interpreter/bytecodes.hpp"
# include "interpreter/cppInterpreter.hpp"
# include "interpreter/interpreter.hpp"
# include "interpreter/invocationCounter.hpp"
# include "interpreter/linkResolver.hpp"
# include "interpreter/templateInterpreter.hpp"
# include "interpreter/templateTable.hpp"
# include "jvmtifiles/jvmti.h"
# include "memory/allocation.hpp"
# include "memory/allocation.inline.hpp"
# include "memory/barrierSet.hpp"
# include "memory/barrierSet.inline.hpp"
# include "memory/blockOffsetTable.hpp"
# include "memory/blockOffsetTable.inline.hpp"
# include "memory/cardTableModRefBS.hpp"
# include "memory/collectorPolicy.hpp"
# include "memory/compactingPermGenGen.hpp"
# include "memory/defNewGeneration.hpp"
# include "memory/gcLocker.hpp"
# include "memory/genCollectedHeap.hpp"
# include "memory/genOopClosures.hpp"
# include "memory/genRemSet.hpp"
# include "memory/generation.hpp"
# include "memory/generation.inline.hpp"
# include "memory/heap.hpp"
# include "memory/iterator.hpp"
# include "memory/memRegion.hpp"
# include "memory/modRefBarrierSet.hpp"
# include "memory/oopFactory.hpp"
# include "memory/permGen.hpp"
# include "memory/referencePolicy.hpp"
# include "memory/referenceProcessor.hpp"
# include "memory/resourceArea.hpp"
# include "memory/sharedHeap.hpp"
# include "memory/space.hpp"
# include "memory/space.inline.hpp"
# include "memory/specialized_oop_closures.hpp"
# include "memory/threadLocalAllocBuffer.hpp"
# include "memory/threadLocalAllocBuffer.inline.hpp"
# include "memory/universe.hpp"
# include "memory/universe.inline.hpp"
# include "memory/watermark.hpp"
# include "oops/arrayKlass.hpp"
# include "oops/arrayOop.hpp"
# include "oops/constMethodOop.hpp"
# include "oops/constantPoolOop.hpp"
# include "oops/cpCacheOop.hpp"
# include "oops/instanceKlass.hpp"
# include "oops/instanceOop.hpp"
# include "oops/instanceRefKlass.hpp"
# include "oops/klass.hpp"
# include "oops/klassOop.hpp"
# include "oops/klassPS.hpp"
# include "oops/klassVtable.hpp"
# include "oops/markOop.hpp"
# include "oops/markOop.inline.hpp"
# include "oops/methodDataOop.hpp"
# include "oops/methodOop.hpp"
# include "oops/objArrayKlass.hpp"
# include "oops/objArrayOop.hpp"
# include "oops/oop.hpp"
# include "oops/oop.inline.hpp"
# include "oops/oop.inline2.hpp"
# include "oops/oopsHierarchy.hpp"
# include "oops/symbol.hpp"
# include "oops/typeArrayKlass.hpp"
# include "oops/typeArrayOop.hpp"
# include "prims/jni.h"
# include "prims/jvm.h"
# include "prims/jvmtiExport.hpp"
# include "prims/methodHandles.hpp"
# include "runtime/arguments.hpp"
# include "runtime/atomic.hpp"
# include "runtime/deoptimization.hpp"
# include "runtime/extendedPC.hpp"
# include "runtime/fieldDescriptor.hpp"
# include "runtime/fieldType.hpp"
# include "runtime/frame.hpp"
# include "runtime/frame.inline.hpp"
# include "runtime/globals.hpp"
# include "runtime/globals_extension.hpp"
# include "runtime/handles.hpp"
# include "runtime/handles.inline.hpp"
# include "runtime/icache.hpp"
# include "runtime/init.hpp"
# include "runtime/interfaceSupport.hpp"
# include "runtime/java.hpp"
# include "runtime/javaCalls.hpp"
# include "runtime/javaFrameAnchor.hpp"
# include "runtime/jniHandles.hpp"
# include "runtime/monitorChunk.hpp"
# include "runtime/mutex.hpp"
# include "runtime/mutexLocker.hpp"
# include "runtime/objectMonitor.hpp"
# include "runtime/orderAccess.hpp"
# include "runtime/os.hpp"
# include "runtime/osThread.hpp"
# include "runtime/perfData.hpp"
# include "runtime/perfMemory.hpp"
# include "runtime/prefetch.hpp"
# include "runtime/reflection.hpp"
# include "runtime/reflectionCompat.hpp"
# include "runtime/reflectionUtils.hpp"
# include "runtime/registerMap.hpp"
# include "runtime/safepoint.hpp"
# include "runtime/sharedRuntime.hpp"
# include "runtime/signature.hpp"
# include "runtime/stackValue.hpp"
# include "runtime/stackValueCollection.hpp"
# include "runtime/stubCodeGenerator.hpp"
# include "runtime/stubRoutines.hpp"
# include "runtime/synchronizer.hpp"
# include "runtime/thread.hpp"
# include "runtime/threadLocalStorage.hpp"
# include "runtime/timer.hpp"
# include "runtime/unhandledOops.hpp"
# include "runtime/vframe.hpp"
# include "runtime/virtualspace.hpp"
# include "runtime/vmThread.hpp"
# include "runtime/vm_operations.hpp"
# include "runtime/vm_version.hpp"
# include "services/lowMemoryDetector.hpp"
# include "services/memoryPool.hpp"
# include "services/memoryService.hpp"
# include "services/memoryUsage.hpp"
# include "utilities/accessFlags.hpp"
# include "utilities/array.hpp"
# include "utilities/bitMap.hpp"
# include "utilities/bitMap.inline.hpp"
# include "utilities/constantTag.hpp"
# include "utilities/copy.hpp"
# include "utilities/debug.hpp"
# include "utilities/exceptions.hpp"
# include "utilities/globalDefinitions.hpp"
# include "utilities/growableArray.hpp"
# include "utilities/hashtable.hpp"
# include "utilities/histogram.hpp"
# include "utilities/macros.hpp"
# include "utilities/numberSeq.hpp"
# include "utilities/ostream.hpp"
# include "utilities/preserveException.hpp"
# include "utilities/sizes.hpp"
# include "utilities/taskqueue.hpp"
# include "utilities/top.hpp"
# include "utilities/utf8.hpp"
# include "utilities/workgroup.hpp"
# include "utilities/yieldingWorkgroup.hpp"
#ifdef COMPILER2
# include "libadt/dict.hpp"
# include "libadt/port.hpp"
# include "libadt/set.hpp"
# include "libadt/vectset.hpp"
# include "opto/addnode.hpp"
# include "opto/adlcVMDeps.hpp"
# include "opto/block.hpp"
# include "opto/c2_globals.hpp"
# include "opto/callnode.hpp"
# include "opto/cfgnode.hpp"
# include "opto/compile.hpp"
# include "opto/connode.hpp"
# include "opto/idealGraphPrinter.hpp"
# include "opto/loopnode.hpp"
# include "opto/machnode.hpp"
# include "opto/matcher.hpp"
# include "opto/memnode.hpp"
# include "opto/mulnode.hpp"
# include "opto/multnode.hpp"
# include "opto/node.hpp"
# include "opto/opcodes.hpp"
# include "opto/optoreg.hpp"
# include "opto/phase.hpp"
# include "opto/phaseX.hpp"
# include "opto/regalloc.hpp"
# include "opto/regmask.hpp"
# include "opto/runtime.hpp"
# include "opto/subnode.hpp"
# include "opto/type.hpp"
# include "opto/vectornode.hpp"
#endif // COMPILER2
#ifdef COMPILER1
# include "c1/c1_Compilation.hpp"
# include "c1/c1_Defs.hpp"
# include "c1/c1_FrameMap.hpp"
# include "c1/c1_LIR.hpp"
# include "c1/c1_MacroAssembler.hpp"
# include "c1/c1_ValueType.hpp"
# include "c1/c1_globals.hpp"
#endif // COMPILER1
#ifndef SERIALGC
# include "gc_implementation/concurrentMarkSweep/binaryTreeDictionary.hpp"
# include "gc_implementation/concurrentMarkSweep/cmsOopClosures.hpp"
# include "gc_implementation/concurrentMarkSweep/compactibleFreeListSpace.hpp"
# include "gc_implementation/concurrentMarkSweep/concurrentMarkSweepGeneration.hpp"
# include "gc_implementation/concurrentMarkSweep/freeBlockDictionary.hpp"
# include "gc_implementation/concurrentMarkSweep/freeChunk.hpp"
# include "gc_implementation/concurrentMarkSweep/freeList.hpp"
# include "gc_implementation/concurrentMarkSweep/promotionInfo.hpp"
# include "gc_implementation/g1/dirtyCardQueue.hpp"
# include "gc_implementation/g1/g1BlockOffsetTable.hpp"
# include "gc_implementation/g1/g1BlockOffsetTable.inline.hpp"
# include "gc_implementation/g1/g1OopClosures.hpp"
# include "gc_implementation/g1/g1_globals.hpp"
# include "gc_implementation/g1/g1_specialized_oop_closures.hpp"
# include "gc_implementation/g1/ptrQueue.hpp"
# include "gc_implementation/g1/satbQueue.hpp"
# include "gc_implementation/parNew/parGCAllocBuffer.hpp"
# include "gc_implementation/parNew/parOopClosures.hpp"
# include "gc_implementation/parallelScavenge/objectStartArray.hpp"
# include "gc_implementation/parallelScavenge/parMarkBitMap.hpp"
# include "gc_implementation/parallelScavenge/parallelScavengeHeap.hpp"
# include "gc_implementation/parallelScavenge/psAdaptiveSizePolicy.hpp"
# include "gc_implementation/parallelScavenge/psCompactionManager.hpp"
# include "gc_implementation/parallelScavenge/psGCAdaptivePolicyCounters.hpp"
# include "gc_implementation/parallelScavenge/psGenerationCounters.hpp"
# include "gc_implementation/parallelScavenge/psOldGen.hpp"
# include "gc_implementation/parallelScavenge/psParallelCompact.hpp"
# include "gc_implementation/parallelScavenge/psPermGen.hpp"
# include "gc_implementation/parallelScavenge/psVirtualspace.hpp"
# include "gc_implementation/parallelScavenge/psYoungGen.hpp"
# include "gc_implementation/shared/gcAdaptivePolicyCounters.hpp"
# include "gc_implementation/shared/gcPolicyCounters.hpp"
#endif // SERIALGC

#endif // !DONT_USE_PRECOMPILED_HEADER
