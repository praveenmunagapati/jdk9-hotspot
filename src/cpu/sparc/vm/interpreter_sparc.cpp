/*
 * Copyright (c) 1997, 2014, Oracle and/or its affiliates. All rights reserved.
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

#include "precompiled.hpp"
#include "asm/macroAssembler.hpp"
#include "interpreter/bytecodeHistogram.hpp"
#include "interpreter/interpreter.hpp"
#include "interpreter/interpreterGenerator.hpp"
#include "interpreter/interpreterRuntime.hpp"
#include "interpreter/interp_masm.hpp"
#include "interpreter/templateTable.hpp"
#include "oops/arrayOop.hpp"
#include "oops/methodData.hpp"
#include "oops/method.hpp"
#include "oops/oop.inline.hpp"
#include "prims/jvmtiExport.hpp"
#include "prims/jvmtiThreadState.hpp"
#include "prims/methodHandles.hpp"
#include "runtime/arguments.hpp"
#include "runtime/deoptimization.hpp"
#include "runtime/frame.inline.hpp"
#include "runtime/sharedRuntime.hpp"
#include "runtime/stubRoutines.hpp"
#include "runtime/synchronizer.hpp"
#include "runtime/timer.hpp"
#include "runtime/vframeArray.hpp"
#include "utilities/debug.hpp"
#ifdef COMPILER1
#include "c1/c1_Runtime1.hpp"
#endif



// Generation of Interpreter
//
// The InterpreterGenerator generates the interpreter into Interpreter::_code.


#define __ _masm->


//----------------------------------------------------------------------------------------------------




int AbstractInterpreter::BasicType_as_index(BasicType type) {
  int i = 0;
  switch (type) {
    case T_BOOLEAN: i = 0; break;
    case T_CHAR   : i = 1; break;
    case T_BYTE   : i = 2; break;
    case T_SHORT  : i = 3; break;
    case T_INT    : i = 4; break;
    case T_LONG   : i = 5; break;
    case T_VOID   : i = 6; break;
    case T_FLOAT  : i = 7; break;
    case T_DOUBLE : i = 8; break;
    case T_OBJECT : i = 9; break;
    case T_ARRAY  : i = 9; break;
    default       : ShouldNotReachHere();
  }
  assert(0 <= i && i < AbstractInterpreter::number_of_result_handlers, "index out of bounds");
  return i;
}


#ifndef _LP64
address AbstractInterpreterGenerator::generate_slow_signature_handler() {
  address entry = __ pc();
  Argument argv(0, true);

  // We are in the jni transition frame. Save the last_java_frame corresponding to the
  // outer interpreter frame
  //
  __ set_last_Java_frame(FP, noreg);
  // make sure the interpreter frame we've pushed has a valid return pc
  __ mov(O7, I7);
  __ mov(Lmethod, G3_scratch);
  __ mov(Llocals, G4_scratch);
  __ save_frame(0);
  __ mov(G2_thread, L7_thread_cache);
  __ add(argv.address_in_frame(), O3);
  __ mov(G2_thread, O0);
  __ mov(G3_scratch, O1);
  __ call(CAST_FROM_FN_PTR(address, InterpreterRuntime::slow_signature_handler), relocInfo::runtime_call_type);
  __ delayed()->mov(G4_scratch, O2);
  __ mov(L7_thread_cache, G2_thread);
  __ reset_last_Java_frame();

  // load the register arguments (the C code packed them as varargs)
  for (Argument ldarg = argv.successor(); ldarg.is_register(); ldarg = ldarg.successor()) {
      __ ld_ptr(ldarg.address_in_frame(), ldarg.as_register());
  }
  __ ret();
  __ delayed()->
     restore(O0, 0, Lscratch);  // caller's Lscratch gets the result handler
  return entry;
}


#else
// LP64 passes floating point arguments in F1, F3, F5, etc. instead of
// O0, O1, O2 etc..
// Doubles are passed in D0, D2, D4
// We store the signature of the first 16 arguments in the first argument
// slot because it will be overwritten prior to calling the native
// function, with the pointer to the JNIEnv.
// If LP64 there can be up to 16 floating point arguments in registers
// or 6 integer registers.
address AbstractInterpreterGenerator::generate_slow_signature_handler() {

  enum {
    non_float  = 0,
    float_sig  = 1,
    double_sig = 2,
    sig_mask   = 3
  };

  address entry = __ pc();
  Argument argv(0, true);

  // We are in the jni transition frame. Save the last_java_frame corresponding to the
  // outer interpreter frame
  //
  __ set_last_Java_frame(FP, noreg);
  // make sure the interpreter frame we've pushed has a valid return pc
  __ mov(O7, I7);
  __ mov(Lmethod, G3_scratch);
  __ mov(Llocals, G4_scratch);
  __ save_frame(0);
  __ mov(G2_thread, L7_thread_cache);
  __ add(argv.address_in_frame(), O3);
  __ mov(G2_thread, O0);
  __ mov(G3_scratch, O1);
  __ call(CAST_FROM_FN_PTR(address, InterpreterRuntime::slow_signature_handler), relocInfo::runtime_call_type);
  __ delayed()->mov(G4_scratch, O2);
  __ mov(L7_thread_cache, G2_thread);
  __ reset_last_Java_frame();


  // load the register arguments (the C code packed them as varargs)
  Address Sig = argv.address_in_frame();        // Argument 0 holds the signature
  __ ld_ptr( Sig, G3_scratch );                   // Get register argument signature word into G3_scratch
  __ mov( G3_scratch, G4_scratch);
  __ srl( G4_scratch, 2, G4_scratch);             // Skip Arg 0
  Label done;
  for (Argument ldarg = argv.successor(); ldarg.is_float_register(); ldarg = ldarg.successor()) {
    Label NonFloatArg;
    Label LoadFloatArg;
    Label LoadDoubleArg;
    Label NextArg;
    Address a = ldarg.address_in_frame();
    __ andcc(G4_scratch, sig_mask, G3_scratch);
    __ br(Assembler::zero, false, Assembler::pt, NonFloatArg);
    __ delayed()->nop();

    __ cmp(G3_scratch, float_sig );
    __ br(Assembler::equal, false, Assembler::pt, LoadFloatArg);
    __ delayed()->nop();

    __ cmp(G3_scratch, double_sig );
    __ br(Assembler::equal, false, Assembler::pt, LoadDoubleArg);
    __ delayed()->nop();

    __ bind(NonFloatArg);
    // There are only 6 integer register arguments!
    if ( ldarg.is_register() )
      __ ld_ptr(ldarg.address_in_frame(), ldarg.as_register());
    else {
    // Optimization, see if there are any more args and get out prior to checking
    // all 16 float registers.  My guess is that this is rare.
    // If is_register is false, then we are done the first six integer args.
      __ br_null_short(G4_scratch, Assembler::pt, done);
    }
    __ ba(NextArg);
    __ delayed()->srl( G4_scratch, 2, G4_scratch );

    __ bind(LoadFloatArg);
    __ ldf( FloatRegisterImpl::S, a, ldarg.as_float_register(), 4);
    __ ba(NextArg);
    __ delayed()->srl( G4_scratch, 2, G4_scratch );

    __ bind(LoadDoubleArg);
    __ ldf( FloatRegisterImpl::D, a, ldarg.as_double_register() );
    __ ba(NextArg);
    __ delayed()->srl( G4_scratch, 2, G4_scratch );

    __ bind(NextArg);

  }

  __ bind(done);
  __ ret();
  __ delayed()->
     restore(O0, 0, Lscratch);  // caller's Lscratch gets the result handler
  return entry;
}
#endif

void InterpreterGenerator::generate_counter_overflow(Label& Lcontinue) {

  // Generate code to initiate compilation on the counter overflow.

  // InterpreterRuntime::frequency_counter_overflow takes two arguments,
  // the first indicates if the counter overflow occurs at a backwards branch (NULL bcp)
  // and the second is only used when the first is true.  We pass zero for both.
  // The call returns the address of the verified entry point for the method or NULL
  // if the compilation did not complete (either went background or bailed out).
  __ set((int)false, O2);
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::frequency_counter_overflow), O2, O2, true);
  // returns verified_entry_point or NULL
  // we ignore it in any case
  __ ba_short(Lcontinue);

}


// End of helpers

// Various method entries

address InterpreterGenerator::generate_jump_to_normal_entry(void) {
  address entry = __ pc();
  assert(Interpreter::entry_for_kind(Interpreter::zerolocals) != NULL, "should already be generated");
  AddressLiteral al(Interpreter::entry_for_kind(Interpreter::zerolocals));
  __ jump_to(al, G3_scratch);
  __ delayed()->nop();
  return entry;
}

// Abstract method entry
// Attempt to execute abstract method. Throw exception
//
address InterpreterGenerator::generate_abstract_entry(void) {
  address entry = __ pc();
  // abstract method entry
  // throw exception
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_AbstractMethodError));
  // the call_VM checks for exception, so we should never return here.
  __ should_not_reach_here();
  return entry;

}

bool AbstractInterpreter::can_be_compiled(methodHandle m) {
  // No special entry points that preclude compilation
  return true;
}

void Deoptimization::unwind_callee_save_values(frame* f, vframeArray* vframe_array) {

  // This code is sort of the equivalent of C2IAdapter::setup_stack_frame back in
  // the days we had adapter frames. When we deoptimize a situation where a
  // compiled caller calls a compiled caller will have registers it expects
  // to survive the call to the callee. If we deoptimize the callee the only
  // way we can restore these registers is to have the oldest interpreter
  // frame that we create restore these values. That is what this routine
  // will accomplish.

  // At the moment we have modified c2 to not have any callee save registers
  // so this problem does not exist and this routine is just a place holder.

  assert(f->is_interpreted_frame(), "must be interpreted");
}


//----------------------------------------------------------------------------------------------------
// Exceptions
