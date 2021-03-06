/*
Fracterval
Copyright 2017 Russell Leidich

This collection of files constitutes the Fracterval Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Fracterval Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Fracterval Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Fracterval Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
128-bit Unsigned Fracterval and Fractoid Kernel

Documentation is in the header of fracterval_u128.h.
*/
#include "flag.h"
#include "flag_fracterval_u128.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "bitscan.h"
#include "bitscan_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"

u8
fracterval_u128_divide_fracterval_u128(fru128 *a_base, fru128 p, fru128 q){
/*
Use FRU128_DIVIDE_FRU128() instead of calling here directly.

Divide one fracterval by another.

In:

  *a_base is undefined.

  p is the dividend fracterval.

  q is the divisor fracterval.

Out:

  Returns one if ((p.b+1)/q.a) exceeds one, or q.a is zero; else zero.

  *a_base is (p/q) expressed as a fracterval.
*/
  fru128 a;
  u128 dividend;
  u128 dividend0;
  u128 dividend1;
  u8 divisor_shift;
  u128 divisor_u128;
  u64 divisor_u128_hi;
  u64 divisor_u128_lo;
  u128 divisor_u128_shifted;
  u64 divisor_u64;
  u8 phase;
  u128 product;
  u128 product_shifted;
  u64 quotient;
  u8 status;

  U128_SET_ZERO(a.b);
  phase=0;
  status=0;
  do{
    a.a=a.b;
    if(!phase){
      dividend=p.a;
      U128_INCREMENT(divisor_u128, q.b);
    }else{
      U128_INCREMENT(dividend, p.b);
      divisor_u128=q.a;
    }
    if(U128_IS_NOT_ZERO(divisor_u128)&&U128_IS_LESS_EQUAL(dividend, divisor_u128)){
      U128_SET_ZERO(a.b);
      divisor_shift=128;
      do{
        divisor_shift=(u8)(divisor_shift-U16_BITS);
        U128_SHIFT_RIGHT(divisor_u128_shifted, divisor_shift, divisor_u128);
      }while(U128_IS_ZERO(divisor_u128_shifted));
      divisor_shift=(u8)(U128_BITS-U16_BITS-divisor_shift);
      U128_SET_ZERO(dividend0);
      U128_SHIFT_LEFT(dividend1, divisor_shift, dividend);
      dividend=dividend1;
      U128_SHIFT_LEFT_SELF(divisor_u128, divisor_shift);
      U128_TO_U64_HI(divisor_u128_hi, divisor_u128);
      U128_TO_U64_LO(divisor_u128_lo, divisor_u128);
      divisor_u64=divisor_u128_hi;
      divisor_u64++;
      U128_TO_U64_HI(quotient, dividend);
      if(divisor_u64){
        U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor_u64, status);
      }
      U128_FROM_U64_HI(a.b, quotient);
      U128_FROM_U64_PRODUCT(product, divisor_u128_lo, quotient);
      U128_SHIFT_LEFT(product_shifted, U64_BITS, product);
      if(U128_IS_LESS(dividend0, product_shifted)){
        U128_DECREMENT_SELF(dividend1);
      }
      U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
      U128_SHIFT_RIGHT(product_shifted, U128_BITS-U64_BITS, product);
      U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
      U128_FROM_U64_PRODUCT(product, divisor_u128_hi, quotient);
      U128_SUBTRACT_U128_SELF(dividend1, product);
      U128_FROM_U128_PAIR_BIT_IDX(dividend, U128_BITS-46, dividend0, dividend1);
      U128_TO_U64_HI(quotient, dividend);
      if(divisor_u64){
        U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor_u64, status);
      }
      U128_ADD_U64_SHIFTED_SELF(a.b, U64_BITS-46, quotient);
      U128_FROM_U64_PRODUCT(product, divisor_u128_lo, quotient);
      U128_SHIFT_LEFT(product_shifted, U64_BITS-46, product);
      if(U128_IS_LESS(dividend0, product_shifted)){
        U128_DECREMENT_SELF(dividend1);
      }
      U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
      U128_SHIFT_RIGHT(product_shifted, U128_BITS-(U64_BITS-46), product);
      U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
      U128_FROM_U64_PRODUCT(product, divisor_u128_hi, quotient);
      U128_SHIFT_LEFT(product_shifted, U128_BITS-46, product);
      if(U128_IS_LESS(dividend0, product_shifted)){
        U128_DECREMENT_SELF(dividend1);
      }
      U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
      U128_SHIFT_RIGHT(product_shifted, 46, product);
      U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
      U128_FROM_U128_PAIR_BIT_IDX(dividend, U128_BITS-(46+46), dividend0, dividend1);
      U128_TO_U64_HI(quotient, dividend);
      if(divisor_u64){
        U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor_u64, status);
      }
      quotient>>=46+46-U64_BITS;
      U128_ADD_U64_LO_SELF(a.b, quotient);
      U128_FROM_U64_PRODUCT(product, divisor_u128_lo, quotient);
      if(U128_IS_LESS(dividend0, product)){
        U128_DECREMENT_SELF(dividend1);
      }
      U128_SUBTRACT_U128_SELF(dividend0, product);
      U128_FROM_U64_PRODUCT(product, divisor_u128_hi, quotient);
      U128_SHIFT_LEFT(product_shifted, U64_BITS, product);
      if(U128_IS_LESS(dividend0, product_shifted)){
        U128_DECREMENT_SELF(dividend1);
      }
      U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
      U128_SHIFT_RIGHT(product_shifted, U128_BITS-U64_BITS, product);
      U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
      if(U128_IS_LESS_EQUAL(divisor_u128, dividend0)||U128_IS_NOT_ZERO(dividend1)){
        U128_INCREMENT_SELF(a.b);
        U128_SUBTRACT_U128_SELF(dividend0, divisor_u128);
      }
      if(!phase){
        if(U128_IS_ZERO(a.b)&&U128_IS_NOT_ZERO(p.a)){
          U128_SET_ONES(a.b);
        }
      }else{
        if(U128_IS_ZERO(dividend0)){
          if(U128_IS_ONES(p.b)){
            status=1;
          }
          U128_DECREMENT_SELF(a.b);
        }
      }
    }else{
      if(U128_IS_ZERO(divisor_u128)&&!phase){
        a.b=p.a;
      }else{
        U128_SET_ONES(a.b);
        status=1;
      }
    }
    phase=!phase;
  }while(phase);
  *a_base=a;
  return status;
}

u8
fracterval_u128_divide_u128(fru128 *a_base, fru128 p, u128 q){
/*
Use FRU128_DIVIDE_U128() instead of calling here directly.

Divide a fracterval by a u128.

In:

  *a_base is undefined.

  p is the dividend fracterval.

  q is the divisor.

Out:

  Returns one if q is zero; else zero.

  *a_base is (p/q) expressed as a fracterval.
*/
  fru128 a;
  u64 divisor_hi;
  u64 divisor_lo;
  u8 status;

  U128_TO_U64_HI(divisor_hi, q);
  status=0;
  if(!divisor_hi){
    U128_TO_U64_LO(divisor_lo, q);
    U128_DIVIDE_U64_TO_U128(a.a, p.a, divisor_lo, status);
    U128_DIVIDE_U64_TO_U128(a.b, p.b, divisor_lo, status);
  }else{
    U128_DIVIDE_U128(a.a, p.a, q, status);
    U128_DIVIDE_U128(a.b, p.b, q, status);
  }
  *a_base=a;
  return status;
}

u8
fracterval_u128_divide_u64(fru128 *a_base, fru128 p, u64 v){
/*
Use FRU128_DIVIDE_U64() instead of calling here directly.

Divide a fracterval by a u64.

In:

  *a_base is undefined.

  p is the dividend fracterval.

  v is the divisor.

Out:

  Returns one if v is zero; else zero.

  *a_base is (p/v) expressed as a fracterval.
*/
  fru128 a;
  u8 status;

  status=0;
  U128_DIVIDE_U64_TO_U128(a.a, p.a, v, status);
  U128_DIVIDE_U64_TO_U128(a.b, p.b, v, status);
  *a_base=a;
  return status;
}

void
fracterval_u128_from_fractoid_u128_mantissa_u128_product(fru128 *a_base, u128 p, u128 q){
/*
Use FRU128_FROM_FTD128_MANTISSA_U128_PRODUCT() instead of calling here directly.

Set a fracterval to the product of a fractoid and a mantissa.

In:

  *a_base is undefined.

  p is the fractoid, which is NOT interchangeable with the mantissa because fractoids have a presumed error of one ULP, whereas mantissas are presumed to have none.

  q is the mantissa.

Out:

  *a_base is ({p}*q) expressed as a fracterval.
*/
  fru128 a;
  u8 carry;
  u64 factor0_0;
  u64 factor0_1;
  u64 factor1_0;
  u64 factor1_1;
  u64 product_lo_or;
  u128 product0;
  u128 product1;
  u128 product2;
  u128 product3;

  U128_TO_U64_PAIR(factor0_0, factor0_1, p);
  U128_TO_U64_PAIR(factor1_0, factor1_1, q);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_ADD_U128(a.b, product0, q);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  carry=U128_IS_LESS(a.b, product0);
  U128_TO_U64_LO(product_lo_or, a.b);
  U128_SHIFT_RIGHT(a.a, U64_BITS, product0);
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U128_SELF(a.a, product1);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.a, product2);
  U128_ADD_U128_SELF(a.b, product1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  carry=U128_IS_LESS(a.a, product2);
  U128_ADD_U128_SELF(a.b, product2);
  U128_SHIFT_RIGHT_SELF(a.a, U64_BITS);
  U128_ADD_U8_HI_SELF(a.a, carry);
  carry=U128_IS_LESS(a.b, product2);
  U128_TO_OR_U64_LO(product_lo_or, a.b);
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U128_SELF(a.a, product3);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product3);
  if((!product_lo_or)&&U128_IS_NOT_ZERO(q)){
    U128_DECREMENT_SELF(a.b);
  }
  *a_base=a;
  return;
}

void
fracterval_u128_from_fractoid_u128_mantissa_u64_product(fru128 *a_base, u128 p, u64 v){
/*
Use FRU128_FROM_FTD128_MANTISSA_U64_PRODUCT() instead of calling here directly.

Set a fracterval to the product of a fractoid and a mantissa.

In:

  *a_base is undefined.

  p is the fractoid.

  v is the mantissa.

Out:

  *a_base is ({p}*v) expressed as a fracterval.
*/
  fru128 a;
  u64 factor0;
  u64 factor1;
  u64 product_u64;
  u128 product0;
  u128 product1;

  U128_TO_U64_PAIR(factor0, factor1, p);
  U128_FROM_U64_PRODUCT(product0, factor0, v);
  U128_FROM_U64_PRODUCT(product1, factor1, v);
  U128_TO_U64_HI(product_u64, product0);
  U128_ADD_U64_LO(a.b, product0, v);
  U128_ADD_U64_LO(a.a, product1, product_u64);
  U128_TO_U64_LO(product_u64, a.b);
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U128_SELF(a.b, product1);
  if((!product_u64)&&v){
    U128_DECREMENT_SELF(a.b);
  }
  *a_base=a;
  return;
}

u8
fracterval_u128_from_fractoid_u128_u64_product(fru128 *a_base, u128 p, u64 v){
/*
Use FRU128_FROM_FTD128_U64_PRODUCT(), or FRU128_FROM_FTD128_NONZERO_U64_PRODUCT() if v is guaranteed to be nonzero, instead of calling here directly.

Set a fracterval to the product of a fractoid and a u64.

In:

  *a_base is undefined.

  p is the fractoid.

  v is the u64.

Out:

  Returns one if ({p}*v) could exceed one, else zero.

  *a_base is ({p}*v) expressed as a fracterval.
*/
  fru128 a;
  u64 factor0;
  u64 factor1;
  u64 product_u64;
  u128 product0;
  u128 product1;
  u8 status;

  U128_TO_U64_PAIR(factor0, factor1, p);
  U128_FROM_U64_PRODUCT(product0, factor0, v);
  U128_FROM_U64_PRODUCT(product1, factor1, v);
  U128_TO_U64_LO(product_u64, product1);
  U128_ADD_U64_HI(a.a, product0, product_u64);
  if(U128_IS_LESS_EQUAL(product0, a.a)){
    U128_ADD_U64_LO(a.b, a.a, v);
    if(U128_IS_LESS_EQUAL(a.a, a.b)){
      if(v){
        U128_DECREMENT_SELF(a.b);
      }
      status=0;
    }else{
      U128_SET_ONES(a.b);
      status=1;
    }
  }else{
    FRU128_SET_ONES(a);
    status=1;
  }
  *a_base=a;
  return status;
}

void *
fracterval_u128_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Fracterval U128 calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

u8
fracterval_u128_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of FRU128_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of FRU128_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=FRU128_BUILD_BREAK_COUNT) or (build_feature_count>FRU128_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=FRU128_BUILD_BREAK_COUNT);
  status=(u8)(status|(FRU128_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

fru128 *
fracterval_u128_list_malloc(ULONG fru128_idx_max){
/*
Allocate a list of (fru128)s.

In:

  fru128_idx_max is the number of (fru128)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (fru128_idx_max+1) (fru128)s. It must be freed via fracterval_u128_free().
*/
  ULONG fru128_count;
  fru128 *list_base;
  u64 list_bit_count;
  ULONG list_size;

  list_base=NULL;
  fru128_count=fru128_idx_max+1;
  if(fru128_count){
    list_size=fru128_count<<(U128_SIZE_LOG2+1);
    if((list_size>>(U128_SIZE_LOG2+1))==fru128_count){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
      list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
      if((list_bit_count>>U8_BITS_LOG2)==list_size){
        list_base=DEBUG_MALLOC_PARANOID(list_size);
      }
    }
  }
  return list_base;
}

u8
fracterval_u128_log_delta_u64(fru128 *a_base, u64 v){
/*
Use FRU128_LOG_DELTA_U64() instead of calling here directly.

Compute the difference between successive natural logs in 6.122 fixed-point.

In:

  *a_base is undefined.

  v is the u64 from which to compute (log(v+1)-log(v)), on [1, U64_MAX].

Out:

  Returns one if v is zero, else zero.

  *a_base is (log(v+1)-log(v)) expressed as a 6.122 fixed-point fracterval if v is nonzero, else one.
*/
  fru128 log_delta;
  u64 mantissa0;
  u64 mantissa1;
  u128 mantissa_u128_0;
  u128 mantissa_u128_1;
  u8 msb;
  u8 shift;
  u8 status;
  u64 v_plus_1;

  FRU128_SET_ZERO(log_delta);
  status=0;
  v_plus_1=v+1;
  if(2<v_plus_1){
    BITSCAN_MSB64_SMALL_GET(msb, v);
    shift=(u8)(U64_BIT_MAX-msb);
    mantissa0=v<<shift;
    mantissa1=v_plus_1<<shift;
    U128_FROM_U64_HI(mantissa_u128_0, mantissa0);
    U128_FROM_U64_HI(mantissa_u128_1, mantissa1);
/*
If mantissa_u128_1 is zero, it will be treated as 1.0 by fracterval_u128_log_mantissa_delta_u128(). Otherwise, mantissa_u128_0 and mantissa_u128_1 have their high bits set, with (mantissa_u128_0<mantissa_u128_1), so fracterval_u128_log_mantissa_delta_u128() is guaranteed to return zero status.
*/
    fracterval_u128_log_mantissa_delta_u128(&log_delta, mantissa_u128_0, mantissa_u128_1);
    FRU128_SHIFT_RIGHT_SELF(log_delta, U64_BITS_LOG2);
  }else{
    if(v){
      if(v_plus_1){
        U128_FROM_U64_PAIR(log_delta.a, FTD128_LOG2_FLOOR_LO, FTD128_LOG2_FLOOR_HI);
        U128_SHIFT_RIGHT_SELF(log_delta.a, U64_BITS_LOG2);
      }else{
/*
logdelta(U64_MAX) is actually on the interval [(2^(-64)), (2^(-64))+(2^(-128))]. Load the result shifted appropriately.
*/
        U128_FROM_U64_LO(log_delta.a, 1ULL<<(U64_BITS-U64_BITS_LOG2));
      }
    }else{
      U128_NOT_SELF(log_delta.a);
      status=1;
    }
    log_delta.b=log_delta.a;
  }
  *a_base=log_delta;
  return status;
}

u8
fracterval_u128_log_delta_u64_cached(fru128 *a_base, ULONG log_delta_idx_max, fru128 *log_delta_list_base, u64 *log_delta_parameter_list_base, u64 v){
/*
Use FRU128_LOG_DELTA_U64_CACHED(), or FRU128_LOG_DELTA_U64_NONZERO_CACHED() if v is guaranteed to be nonzero, instead of calling here directly.

Deliver cached results from fracterval_u128_log_delta_u64().

In:

  *a_base is undefined.

  log_delta_idx_max is fracterval_u128_log_u64_cache_init():In:log_delta_idx_max.

  log_delta_list_base is fracterval_u128_log_u64_cache_init():Out:*log_delta_list_base_base.

  log_delta_parameter_list_base is the return value of fracterval_u128_log_u64_cache_init().

  v is as defined in fracterval_u128_log_delta_u64().

Out:

  Returns as defined for fracterval_u128_log_delta_u64(a_base, v).

  *a_base is as defined in fracterval_u128_log_delta_u64().

  *log_delta_list_base contains *a_base at index (log_delta_idx_max&v).

  *log_delta_parameter_list_base contains v at index (log_delta_idx_max&v).
*/
  fru128 a;
  ULONG idx;
  u8 status;

  idx=log_delta_idx_max&(ULONG)(v);
  if(log_delta_parameter_list_base[idx]==v){
    a=log_delta_list_base[idx];
    status=!v;
  }else{
    log_delta_parameter_list_base[idx]=v;
    status=fracterval_u128_log_delta_u64(&a, v);
    log_delta_list_base[idx]=a;
  }
  *a_base=a;
  return status;
}

u8
fracterval_u128_log_mantissa_delta_u128(fru128 *a_base, u128 p, u128 q){
/*
Use FRU128_LOG_MANTISSA_DELTA_U128() instead of calling here directly.

Compute the absolute value of the difference between the natural logs of mantissas on [0.5, 1.0].

In:

  *a_base is undefined.

  p is a mantissa on [U64_SPAN_HALF, U64_MAX-1], corresponding to [0.5, 1.0). Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

  q is analogous to p, but on [p+1, U64_MAX]. It may also be zero, in which case it shall be treated as though it were 1.0.

Out:

  Returns one if (p<U64_SPAN_HALF), else zero.

  *a_base is (log(q)-log(p)) expressed as a fracterval if (U64_SPAN_HALF<=p), else the ambiguous fracterval.
*/
  u64 denominator;
  fru128 log_delta;
  u128 p_minus_q;
  fru128 p_power;
  fru128 q_power;
  u8 status;
  fru128 term;
/*
This is just 2 instances of fracterval_u128_log_mantissa_u64() fused together for speed and accuracy. See that function for comments.
*/
  FRU128_SET_ZERO(log_delta);
  status=0;
  if(U128_IS_SIGNED(p)){
    denominator=1;
    U128_NEGATE_SELF(p);
    U128_NEGATE_SELF(q);
    FRU128_FROM_FTD128(p_power, p);
    FRU128_FROM_FTD128(q_power, q);
    U128_SUBTRACT_U128(p_minus_q, p, q);
    FRU128_FROM_FTD128(log_delta, p_minus_q);
    do{
      FRU128_MULTIPLY_MANTISSA_U128_SELF(p_power, p);
      FRU128_MULTIPLY_MANTISSA_U128_SELF(q_power, q);
      denominator++;
      FRU128_SUBTRACT_FRU128(term, p_power, q_power, status);
      FRU128_DIVIDE_U64_SELF(term, denominator, status);
      FRU128_ADD_FRU128_SELF(log_delta, term, status);
    }while(U128_IS_NOT_ZERO(term.b));
    FRU128_EXPAND_UP_SELF(log_delta, status);
/*
If status is one, it's because underflow occured, which doesn't affect correctness of the result. Forget it.
*/
    status=0;
  }else{
    U128_NOT_SELF(log_delta.b);
    status=1;
  }
  *a_base=log_delta;
  return status;
}

u8
fracterval_u128_log_mantissa_u128(fru128 *a_base, u128 p){
/*
Use FRU128_LOG_MANTISSA_U128() instead of calling here directly.

Compute the negative natural log of a mantissa on [0.5, 1.0).

In:

  *a_base is undefined.

  p is a mantissa which is the operand of the log, on [(2^127), (2^128-1)], corresponding to [0.5, 1.0). Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

Out:

  Returns one if (p<(2^127)), else zero.

  *a_base is (-log(p/(2^128))) expressed as a fracterval if ((2^127)<=p), else the ambiguous fracterval.
*/
  u64 denominator;
  fru128 log;
  fru128 power;
  u8 status;
  fru128 term;

  FRU128_SET_ZERO(log);
  status=0;
  if(U128_IS_SIGNED(p)){
    denominator=1;
/*
Negate the mantissa to produce the (x-1) term in the log series. As stated above, mantissas are assumed to have zero uncertainty, so we can do this with u128 negation instead of fracterval negation.
*/
    U128_NEGATE_SELF(p);
/*
Compute the terms of the log series. We lose some accuracy by converting a mantissa to a fracterval, but so what. On the plus side, mantissa multiplication is more accurate than fractoid multiplication.
*/
    FRU128_FROM_FTD128(power, p);
    FRU128_FROM_FTD128(log, p);
    do{
      FRU128_MULTIPLY_MANTISSA_U128_SELF(power, p);
      denominator++;
      FRU128_DIVIDE_U64(term, power, denominator, status);
      FRU128_ADD_FRU128_SELF(log, term, status);
    }while(U128_IS_NOT_ZERO(term.b));
/*
The error is bounded by the magnitude of the last term computed because terms are scaling by a factor of less than (1/2). The last term spans one ULP because term.b is zero.
*/
    FRU128_EXPAND_UP_SELF(log, status);
/*
If status is one, it's because underflow occured, which doesn't affect correctness of the result. Forget it.
*/
    status=0;
  }else{
    U128_NOT_SELF(log.b);
    status=1;
  }
  *a_base=log;
  return status;
}

u8
fracterval_u128_log_u128(fru128 *a_base, u128 p){
/*
Use FRU128_LOG_U128() instead of calling here directly.

Compute the 7.121 fixed-point natural log of a u128.

In:

  *a_base is undefined.

  p is the u128 whose log to compute.

Out:

  Returns one if p is zero, else zero.

  *a_base is log(p) expressed as a 6.121 fixed-point fracterval if p is nonzero, else zero.
*/
  fru128 log;
  fru128 log_fractoid;
  fru128 log2;
  u64 log2_count;
  u128 mantissa;
  u8 msb;
  u8 shift;
  u8 status;
  u64 v;

  U128_TO_U64_HI(v, p);
  if(!v){
    U128_TO_U64_LO(v, p);
    status=fracterval_u128_log_u64(&log, v);
    FRU128_SHIFT_RIGHT_SELF(log, 1);
  }else{
    BITSCAN_MSB64_SMALL_GET(msb, v);
    msb=(u8)(msb+U64_BITS);
    shift=(u8)(U128_BIT_MAX-msb);
    U128_SHIFT_LEFT(mantissa, shift, p);
/*
mantissa has its high bit set, so fracterval_u128_log_mantissa_u128() is guaranteed to return zero status.
*/
    status=fracterval_u128_log_mantissa_u128(&log_fractoid, mantissa);
    FRU128_SHIFT_RIGHT_SELF(log_fractoid, U128_BITS_LOG2);
    U128_FROM_U64_PAIR(log2.a, FTD128_LOG2_FLOOR_LO, FTD128_LOG2_FLOOR_HI);
    log2.b=log2.a;
    FRU128_SHIFT_RIGHT_SELF(log2, U128_BITS_LOG2);
    log2_count=(u8)(msb+1);
    FRU128_MULTIPLY_U64_SELF(log2, log2_count, status);
    FRU128_SUBTRACT_FRU128(log, log2, log_fractoid, status);
  }
  *a_base=log;
  return status;
}

u128 *
fracterval_u128_log_u128_cache_init(ULONG log_idx_max, fru128 **log_list_base_base){
/*
Allocate a cache for saving previous fracterval_u128_log_u128() results.

In:

  log_idx_max is one less than the number of items to allocate in the cache. It must be one less than a power of 2.

Out:

  Returns NULL on failure, else the base of (log_idx_max+1) u128 zeroes, all but the first of which indicating that the corresponding cache entry is undefined. This list must be freed via fracterval_u128_free().

  *log_list_base_base is NULL on failure, else the base of a list containing (log_idx_max+1) undefined (fru128)s, except for the first one, which corresponds to the log of zero, and is therefore saturated to zero. The list must be freed via fracterval_u128_free().
*/
  fru128 *log_list_base;
  u128 *log_parameter_list_base;
  fru128 zero;

  log_list_base=fracterval_u128_list_malloc(log_idx_max);
  log_parameter_list_base=fracterval_u128_u128_list_malloc(log_idx_max);
  if(UINT_IS_POWER_OF_2_MINUS_1(log_idx_max)&&log_list_base&&log_parameter_list_base){
    fracterval_u128_u128_list_zero(log_idx_max, log_parameter_list_base);
    FRU128_SET_ZERO(zero);
    log_list_base[0]=zero;
  }else{
    log_parameter_list_base=fracterval_u128_free(log_parameter_list_base);
    log_list_base=fracterval_u128_free(log_list_base);
  }
  *log_list_base_base=log_list_base;
  return log_parameter_list_base;
}

u8
fracterval_u128_log_u128_cached(fru128 *a_base, ULONG log_idx_max, fru128 *log_list_base, u128 *log_parameter_list_base, u128 p){
/*
Use FRU128_LOG_U128_CACHED(), or FRU128_LOG_U128_NONZERO_CACHED() if p is guaranteed to be nonzero, instead of calling here directly.

Deliver cached results from fracterval_u128_log_u128().

In:

  *a_base is undefined.

  log_idx_max is fracterval_u128_log_u128_cache_init():In:log_idx_max.

  log_list_base is fracterval_u128_log_u128_cache_init():Out:*log_list_base_base.

  log_parameter_list_base is the return value of fracterval_u128_log_u128_cache_init().

  p is as defined in fracterval_u128_log_u128().

Out:

  Returns as defined for fracterval_u128_log_u128(a_base, p).

  *a_base is as defined in fracterval_u128_log_u128().

  *log_list_base contains *a_base at index (log_idx_max&v).

  *log_parameter_list_base contains v at index (log_idx_max&v).
*/
  fru128 a;
  ULONG idx;
  u8 status;
  u64 v;

  U128_TO_U64_LO(v, p);
  idx=log_idx_max&(ULONG)(v);
  if(U128_IS_EQUAL(log_parameter_list_base[idx], p)){
    a=log_list_base[idx];
    status=U128_IS_ZERO(p);
  }else{
    log_parameter_list_base[idx]=p;
    status=fracterval_u128_log_u128(&a, p);
    log_list_base[idx]=a;
  }
  *a_base=a;
  return status;
}

u8
fracterval_u128_log_u64(fru128 *a_base, u64 v){
/*
Use FRU128_LOG_U64() instead of calling here directly.

Compute the 6.122 fixed-point natural log of a u64.

In:

  *a_base is undefined.

  v is the u64 whose log to compute.

Out:

  Returns one if v is zero, else zero.

  *a_base is log(v) expressed as a 6.122 fixed-point fracterval if v is nonzero, else zero.
*/
  fru128 log;
  fru128 log_fractoid;
  fru128 log2;
  u64 log2_count;
  u64 mantissa;
  u128 mantissa_u128;
  u8 msb;
  u8 shift;
  u8 status;

  status=0;
  if(2<v){
    BITSCAN_MSB64_SMALL_GET(msb, v);
    shift=(u8)(U64_BIT_MAX-msb);
    mantissa=v<<shift;
    U128_FROM_U64_HI(mantissa_u128, mantissa);
/*
mantissa_u128 has its high bit set, so fracterval_u128_log_mantissa_u128() is guaranteed to return zero status.
*/
    fracterval_u128_log_mantissa_u128(&log_fractoid, mantissa_u128);
    FRU128_SHIFT_RIGHT_SELF(log_fractoid, U64_BITS_LOG2);
    U128_FROM_U64_PAIR(log2.a, FTD128_LOG2_FLOOR_LO, FTD128_LOG2_FLOOR_HI);
    log2.b=log2.a;
    FRU128_SHIFT_RIGHT_SELF(log2, U64_BITS_LOG2);
    log2_count=(u8)(msb+1);
    FRU128_MULTIPLY_U64_SELF(log2, log2_count, status);
    FRU128_SUBTRACT_FRU128(log, log2, log_fractoid, status);
  }else{
    if(v==2){
      U128_FROM_U64_PAIR(log.a, FTD128_LOG2_FLOOR_LO, FTD128_LOG2_FLOOR_HI);
      U128_SHIFT_RIGHT_SELF(log.a, U64_BITS_LOG2);
      log.b=log.a;
    }else{
      FRU128_SET_ZERO(log);
      status=!v;
    }
  }
  *a_base=log;
  return status;
}

u64 *
fracterval_u128_log_u64_cache_init(ULONG log_idx_max, fru128 **log_list_base_base){
/*
Allocate a cache for saving previous fracterval_u128_log_delta_u64() or fracterval_u128_log_u64() results. In the former case, the cached result of logdelta(0) will improperly saturate to zero instead of one, but this lookup should be precluded by design.

In:

  log_idx_max is one less than the number of items to allocate in the cache. It must be one less than a power of 2.

Out:

  Returns NULL on failure, else the base of (log_idx_max+1) u64 zeroes, all but the first of which indicating that the corresponding cache entry is undefined. This list must be freed via fracterval_u128_free().

  *log_list_base_base is NULL on failure, else the base of a list containing (log_idx_max+1) undefined (fru128)s, except for the first one, which corresponds to the log of zero, and is therefore saturated to zero. The list must be freed via fracterval_u128_free().
*/
  fru128 *log_list_base;
  u64 *log_parameter_list_base;
  fru128 zero;

  log_list_base=fracterval_u128_list_malloc(log_idx_max);
  log_parameter_list_base=fracterval_u128_u64_list_malloc(log_idx_max);
  if(UINT_IS_POWER_OF_2_MINUS_1(log_idx_max)&&log_list_base&&log_parameter_list_base){
    fracterval_u128_u64_list_zero(log_idx_max, log_parameter_list_base);
    FRU128_SET_ZERO(zero);
    log_list_base[0]=zero;
  }else{
    log_parameter_list_base=fracterval_u128_free(log_parameter_list_base);
    log_list_base=fracterval_u128_free(log_list_base);
  }
  *log_list_base_base=log_list_base;
  return log_parameter_list_base;
}

u8
fracterval_u128_log_u64_cached(fru128 *a_base, ULONG log_idx_max, fru128 *log_list_base, u64 *log_parameter_list_base, u64 v){
/*
Use FRU128_LOG_U64_CACHED(), or FRU128_LOG_U64_NONZERO_CACHED() if v is guaranteed to be nonzero, instead of calling here directly.

Deliver cached results from fracterval_u128_log_u64().

In:

  *a_base is undefined.

  log_idx_max is fracterval_u128_log_u64_cache_init():In:log_idx_max.

  log_list_base is fracterval_u128_log_u64_cache_init():Out:*log_list_base_base.

  log_parameter_list_base is the return value of fracterval_u128_log_u64_cache_init().

  v is as defined in fracterval_u128_log_u64().

Out:

  Returns as defined for fracterval_u128_log_u64(a_base, v).

  *a_base is as defined in fracterval_u128_log_u64().

  *log_list_base contains *a_base at index (log_idx_max&v).

  *log_parameter_list_base contains v at index (log_idx_max&v).
*/
  fru128 a;
  ULONG idx;
  u8 status;

  idx=log_idx_max&(ULONG)(v);
  if(log_parameter_list_base[idx]==v){
    a=log_list_base[idx];
    status=!v;
  }else{
    log_parameter_list_base[idx]=v;
    status=fracterval_u128_log_u64(&a, v);
    log_list_base[idx]=a;
  }
  *a_base=a;
  return status;
}

void
fracterval_u128_multiply_fracterval_u128(fru128 *a_base, fru128 p, fru128 q){
/*
Use FRU128_MULTIPLY_FRU128() instead of calling here directly.

Multiply one fracterval by another.

In:

  *a_base is undefined.

  p is the first factor fracterval.

  q is the second factor fracterval.

Out:

  *a_base is (p*q) expressed as a fracterval.
*/
  fru128 a;
  u8 carry;
  u64 factor0_0;
  u64 factor0_1;
  u64 factor1_0;
  u64 factor1_1;
  u128 product0;
  u128 product1;
  u128 product2;
  u128 product3;

  U128_TO_U64_PAIR(factor0_0, factor0_1, p.a);
  U128_TO_U64_PAIR(factor1_0, factor1_1, q.a);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  U128_SHIFT_RIGHT(a.a, U64_BITS, product0);
  U128_ADD_U128_SELF(a.a, product1);
  U128_ADD_U128_SELF(a.a, product2);
  carry=U128_IS_LESS(a.a, product2);
  U128_SHIFT_RIGHT_SELF(a.a, U64_BITS);
  U128_ADD_U8_HI_SELF(a.a, carry);
  U128_ADD_U128_SELF(a.a, product3);
  U128_TO_U64_PAIR(factor0_0, factor0_1, p.b);
  U128_TO_U64_PAIR(factor1_0, factor1_1, q.b);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  U128_ADD_U128(a.b, p.b, product0);
  carry=U128_IS_LESS(a.b, product0);
  U128_ADD_U128_SELF(a.b, q.b);
  carry=(u8)(carry+U128_IS_LESS(a.b, q.b));
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product1);
  carry=U128_IS_LESS(a.b, product1);
  U128_ADD_U128_SELF(a.b, product2);
  carry=(u8)(carry+U128_IS_LESS(a.b, product2));
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product3);
  *a_base=a;
  return;
}

void
fracterval_u128_multiply_fractoid_u128(fru128 *a_base, fru128 p, u128 q){
/*
Use FRU128_MULTIPLY_FTD128() instead of calling here directly.

Multiply a fracterval by a fractoid.

In:

  *a_base is undefined.

  p is the fracterval.

  q is the fractoid.

Out:

  *a_base is (p*{q}) expressed as a fracterval.
*/
  fru128 a;
  u8 carry;
  u64 factor0_0;
  u64 factor0_1;
  u64 factor1_0;
  u64 factor1_1;
  u128 product0;
  u128 product1;
  u128 product2;
  u128 product3;

  U128_TO_U64_PAIR(factor0_0, factor0_1, p.a);
  U128_TO_U64_PAIR(factor1_0, factor1_1, q);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  U128_SHIFT_RIGHT(a.a, U64_BITS, product0);
  U128_ADD_U128_SELF(a.a, product1);
  U128_ADD_U128_SELF(a.a, product2);
  carry=U128_IS_LESS(a.a, product2);
  U128_SHIFT_RIGHT_SELF(a.a, U64_BITS);
  U128_ADD_U8_HI_SELF(a.a, carry);
  U128_ADD_U128_SELF(a.a, product3);
  U128_TO_U64_PAIR(factor0_0, factor0_1, p.b);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  U128_ADD_U128(a.b, p.b, product0);
  carry=U128_IS_LESS(a.b, product0);
  U128_ADD_U128_SELF(a.b, q);
  carry=(u8)(carry+U128_IS_LESS(a.b, q));
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product1);
  carry=U128_IS_LESS(a.b, product1);
  U128_ADD_U128_SELF(a.b, product2);
  carry=(u8)(carry+U128_IS_LESS(a.b, product2));
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product3);
  *a_base=a;
  return;
}

void
fracterval_u128_multiply_mantissa_u128(fru128 *a_base, fru128 p, u128 q){
/*
Use FRU128_MULTIPLY_MANTISSA_U128() instead of calling here directly.

Multiply a fracterval by a mantissa.

In:

  *a_base is undefined.

  p is the fracterval.

  q is the mantissa. Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

Out:

  *a_base is (p*q) expressed as a fracterval.
*/
  fru128 a;
  u8 carry;
  u64 factor0_0;
  u64 factor0_1;
  u64 factor1_0;
  u64 factor1_1;
  u64 product_lo_or;
  u128 product0;
  u128 product1;
  u128 product2;
  u128 product3;

  U128_TO_U64_PAIR(factor0_0, factor0_1, p.a);
  U128_TO_U64_PAIR(factor1_0, factor1_1, q);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  U128_SHIFT_RIGHT(a.a, U64_BITS, product0);
  U128_ADD_U128_SELF(a.a, product1);
  U128_ADD_U128_SELF(a.a, product2);
  carry=U128_IS_LESS(a.a, product2);
  U128_SHIFT_RIGHT_SELF(a.a, U64_BITS);
  U128_ADD_U8_HI_SELF(a.a, carry);
  U128_ADD_U128_SELF(a.a, product3);
  U128_TO_U64_PAIR(factor0_0, factor0_1, p.b);
  U128_FROM_U64_PRODUCT(product0, factor0_0, factor1_0);
  U128_FROM_U64_PRODUCT(product1, factor0_1, factor1_0);
  U128_FROM_U64_PRODUCT(product2, factor0_0, factor1_1);
  U128_FROM_U64_PRODUCT(product3, factor0_1, factor1_1);
  U128_ADD_U128(a.b, product0, q);
  carry=U128_IS_LESS(a.b, product0);
  U128_TO_U64_LO(product_lo_or, a.b);
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product1);
  U128_ADD_U128_SELF(a.b, product2);
  carry=U128_IS_LESS(a.b, product2);
  U128_TO_OR_U64_LO(product_lo_or, a.b);
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U8_HI_SELF(a.b, carry);
  U128_ADD_U128_SELF(a.b, product3);
  if((!product_lo_or)&&U128_IS_NOT_ZERO(q)){
    U128_DECREMENT_SELF(a.b);
  }
  *a_base=a;
  return;
}

void
fracterval_u128_multiply_mantissa_u64(fru128 *a_base, fru128 p, u64 v){
/*
Use FRU128_MULTIPLY_MANTISSA_U64() instead of calling here directly.

Multiply a fracterval by a mantissa.

In:

  *a_base is undefined.

  p is the fracterval.

  v is the mantissa. Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

Out:

  *a_base is (p*v) expressed as a fracterval.
*/
  fru128 a;
  u64 factor0;
  u64 factor1;
  u64 product_lo;
  u128 product0;
  u128 product1;

  U128_TO_U64_PAIR(factor0, factor1, p.a);
  U128_FROM_U64_PRODUCT(product0, factor0, v);
  U128_FROM_U64_PRODUCT(product1, factor1, v);
  a.a=product0;
  U128_SHIFT_RIGHT_SELF(a.a, U64_BITS);
  U128_ADD_U128_SELF(a.a, product1);
  U128_TO_U64_PAIR(factor0, factor1, p.b);
  U128_FROM_U64_PRODUCT(product0, factor0, v);
  U128_FROM_U64_PRODUCT(product1, factor1, v);
  U128_ADD_U64_LO(a.b, product0, v);
  U128_TO_U64_LO(product_lo, a.b);
  U128_SHIFT_RIGHT_SELF(a.b, U64_BITS);
  U128_ADD_U128_SELF(a.b, product1);
  if((!product_lo)&&v){
    U128_DECREMENT_SELF(a.b);
  }
  *a_base=a;
  return;
}

u8
fracterval_u128_multiply_u64(fru128 *a_base, fru128 p, u64 v){
/*
Use FRU128_MULTIPLY_U64() instead of calling here directly.

Multiply a fracterval by a u64.

In:

  *a_base is undefined.

  p is the fracterval.

  v is the u64 by which to multiply the fracterval.

Out:

  Returns one if ((p.b+1)*v) exceeds one, else zero.

  *a_base is (p*v) expressed as a fracterval.
*/
  fru128 a;
  u8 status;
  u8 wrap_status;

  status=0;
  U128_MULTIPLY_U64(a.a, p.a, v, status);
  U128_MULTIPLY_U64(a.b, p.b, v, status);
  if(v){
    wrap_status=0;
    U128_ADD_U64_LO_SELF_CHECK(a.b, v, wrap_status);
    if(!wrap_status){
      U128_DECREMENT_SELF(a.b);
    }else{
      status=(u8)(status|U128_IS_NOT_ZERO(a.b));
      U128_SET_ONES(a.b);
    }
  }
  *a_base=a;
  return status;
}

void
fracterval_u128_nats_from_bits(fru128 *a_base, fru128 p){
/*
Use FRU128_NATS_FROM_BITS() instead of calling here directly.

Convert a fracterval representing a number of bits (log2 units) to nats (natural log units).

In:

  *a_base is undefined.

  p has units of nats.

Out:

  *a_base is (p*log(2)), which has units of bits.
*/
  fru128 a;
  u128 log2;

  U128_FROM_U64_PAIR(log2, FTD128_LOG2_FLOOR_LO, FTD128_LOG2_FLOOR_HI);
  FRU128_MULTIPLY_FTD128(a, p, log2);
  *a_base=a;
  return;
}

u8
fracterval_u128_nats_to_bits(fru128 *a_base, fru128 p){
/*
Use FRU128_NATS_TO_BITS() instead of calling here directly.

Convert a fracterval representing a number of nats (natural log units) to bits (log2 units).

In:

  *a_base is undefined.

  p has units of bits.

Out:

  Returns one on fracterval saturation, else zero.

  *a_base is (p/log(2)), which has units of nats.
*/
  fru128 a;
  fru128 log2;
  u8 status;

  U128_FROM_U64_PAIR(log2.a, FTD128_LOG2_FLOOR_LO, FTD128_LOG2_FLOOR_HI);
  log2.b=log2.a;
  status=0;
  FRU128_DIVIDE_FRU128(a, p, log2, status);
  *a_base=a;
  return status;
}

u8
fracterval_u128_rank_list_insert_ascending(fru128 p, ULONG *rank_count_base, ULONG *rank_idx_base, ULONG rank_idx_max_max, fru128 *rank_list_base, u128 *threshold_base){
/*
Insert a u128 fracterval into list sorted ascending by mean, such that existing fractervals may only be displaced if the new one is greater and not merely equal.

In:

  p is the fracterval to attempt to insert into the list.

  *rank_count_base is the number of fractervals already in the list.

  *rank_idx_base is undefined.

  rank_idx_max_max is (fracterval_u128_rank_list_malloc():In:rank_idx_max_max).

  rank_list_base is the return value of fracterval_u128_rank_list_malloc().

  *threshold_base is the maximum mean to allow in the list on the first call (usually ((2^128)-1)), then fed back thereafter. The point is to provide a fast way to reject new fractervals that are decreasingly likely to qualify over time. This function merely updates the threshold; it's up to the caller to take advantage of it, but doing so is not required for correctness.

Out:

  Returns zero if the insertion was successful, else one if p didn't qualify.

  *rank_count_base is updated.

  *rank_idx_base is the u128 fracterval index (not the u128 index) at which p was stored.

  *threshold_base is updated and has not increased.
*/
  u128 p_mean;
  u128 q_mean;
  fru128 q;
  ULONG rank_count;
  ULONG rank_idx0;
  ULONG rank_idx1;
  u8 status;
  u128 threshold;

  rank_count=*rank_count_base;
  FRU128_MEAN_TO_FTD128(p_mean, p);
  rank_idx0=rank_count;
  status=1;
  if(rank_count){
    status=0;
    do{
      rank_idx1=rank_idx0-1;
      q=rank_list_base[rank_idx1];
      FRU128_MEAN_TO_FTD128(q_mean, q);
      if(U128_IS_LESS_EQUAL(q_mean, p_mean)){
        status=(rank_idx0==rank_count);
        break;
      }
      rank_list_base[rank_idx0]=q;
      rank_idx0=rank_idx1;
    }while(rank_idx1);
  }
  if(rank_count<=rank_idx_max_max){
    rank_count++;
    status=0;
    *rank_count_base=rank_count;
  }
  if(!status){
    rank_list_base[rank_idx0]=p;
    *rank_idx_base=rank_idx0;
    if(rank_idx0==rank_idx_max_max){
      U128_DECREMENT_SATURATE(threshold, p_mean);
      *threshold_base=threshold;
    }
  }
  return status;
}

u8
fracterval_u128_rank_list_insert_descending(fru128 p, ULONG *rank_count_base, ULONG *rank_idx_base, ULONG rank_idx_max_max, fru128 *rank_list_base, u128 *threshold_base){
/*
Insert a u128 fracterval into list sorted descending by mean, such that existing fractervals may only be displaced if the new one is greater and not merely equal.

In:

  p is the fracterval to attempt to insert into the list.

  *rank_count_base is the number of fractervals already in the list.

  *rank_idx_base is undefined.

  rank_idx_max_max is (fracterval_u128_rank_list_malloc():In:rank_idx_max_max).

  rank_list_base is the return value of fracterval_u128_rank_list_malloc().

  *threshold_base is the minimum mean to allow in the list on the first call (usually zero), then fed back thereafter. The point is to provide a fast way to reject new fractervals that are decreasingly likely to qualify over time. This function merely updates the threshold; it's up to the caller to take advantage of it, but doing so is not required for correctness.

Out:

  Returns zero if the insertion was successful, else one if p didn't qualify.

  *rank_count_base is updated.

  *rank_idx_base is the u128 fracterval index (not the u128 index) at which p was stored.

  *threshold_base is updated and has not increased.
*/
  u128 p_mean;
  u128 q_mean;
  fru128 q;
  ULONG rank_count;
  ULONG rank_idx0;
  ULONG rank_idx1;
  u8 status;
  u128 threshold;

  rank_count=*rank_count_base;
  FRU128_MEAN_TO_FTD128(p_mean, p);
  rank_idx0=rank_count;
  status=1;
  if(rank_count){
    status=0;
    do{
      rank_idx1=rank_idx0-1;
      q=rank_list_base[rank_idx1];
      FRU128_MEAN_TO_FTD128(q_mean, q);
      if(U128_IS_LESS_EQUAL(p_mean, q_mean)){
        status=(rank_idx0==rank_count);
        break;
      }
      rank_list_base[rank_idx0]=q;
      rank_idx0=rank_idx1;
    }while(rank_idx1);
  }
  if(rank_count<=rank_idx_max_max){
    rank_count++;
    status=0;
    *rank_count_base=rank_count;
  }
  if(!status){
    rank_list_base[rank_idx0]=p;
    *rank_idx_base=rank_idx0;
    if(rank_idx0==rank_idx_max_max){
      U128_INCREMENT_SATURATE(threshold, p_mean);
      *threshold_base=threshold;
    }
  }
  return status;
}

fru128 *
fracterval_u128_rank_list_malloc(ULONG rank_idx_max_max){
/*
Allocate a list of u128 fractervals to serve as a list of the greatest or least ranking mean values.

In:

  rank_idx_max_max is one less than the number of fractervals to allocate.

Out:

  Returns NULL on failure, else the base of (rank_idx_max+2) u128 fractervals. The extra fracterval allows for simpler insertion by permitting one item of overflow. (And that, by the way, is why this function is distinct from a plain fracterval list allocator.) It must be freed via fracterval_u128_free().
*/
  fru128 *list_base;
  u64 list_bit_count;
  ULONG list_size;

  list_base=NULL;
  list_size=(rank_idx_max_max+2)<<(U128_SIZE_LOG2+1);
  if(rank_idx_max_max==((list_size>>(U128_SIZE_LOG2+1))-2)){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
    list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
    if((list_bit_count>>U8_BITS_LOG2)==list_size){
      list_base=DEBUG_MALLOC_PARANOID(list_size);
    }
  }
  return list_base;
}

void
fracterval_u128_root_fractoid_u128(fru128 *a_base, u128 p){
/*
Use FRU128_ROOT_FRACTOID_U128() instead of calling here directly.

Set a fracterval to the square root of a fractoid.

In:

  *a_base is undefined.

  p is the fractoid whose square root to compute.

Out:

  *a_base is ({p}^(1/2)) expressed as a fracterval.
*/
  u128 root;
  u128 root_max;
  u128 root_min;
  u128 square;

  U128_SET_ONES(root_max);
  U128_SET_ZERO(root_min);
  do{
    U128_SUBTRACT_U128(root, root_max, root_min);
    U128_SHIFT_RIGHT_SELF(root, 1);
    U128_SUBTRACT_FROM_U128_SELF(root, root_max);
    FTD128_FROM_MANTISSA_U128_PRODUCT(square, root, root);
    if(U128_IS_LESS(square, p)){
      root_min=root;
    }else{
      U128_DECREMENT(root_max, root);
    }
  }while(U128_IS_NOT_EQUAL(root_max, root_min));
  a_base[0].a=root_min;
  U128_SET_ONES(root_max);
  do{
    U128_SUBTRACT_U128(root, root_max, root_min);
    U128_SHIFT_RIGHT_SELF(root, 1);
    U128_SUBTRACT_FROM_U128_SELF(root, root_max);
    FTD128_FROM_MANTISSA_U128_PRODUCT(square, root, root);
    if(U128_IS_LESS_EQUAL(square, p)){
      root_min=root;
    }else{
      U128_DECREMENT(root_max, root);
    }
  }while(U128_IS_NOT_EQUAL(root_max, root_min));
  a_base[0].b=root_max;
  return;
}

u8
fracterval_u128_shift_left(fru128 *a_base, u8 b, fru128 p){
/*
Use FRU128_SHIFT_LEFT() instead of calling here directly.

Multiply a fracterval by a power of 2.

In:

  *a_base is undefined.

  b is the power of 2 by which to multiply the fracterval, on [0, U128_BIT_MAX].

  p is the fracterval.

Out:

  Returns one if ((p.b+1)*(2^b)) exceeds one, else zero.

  *a_base is (p*(2^b)) expressed as a fracterval.
*/
  fru128 a;
  u8 status;

  status=0;
  U128_SHIFT_LEFT_CHECK(a.a, b, p.a, status);
  if(!status){
    U128_SHIFT_LEFT_CHECK(a.b, b, p.b, status);
    if(!status){
      U128_INCREMENT(a.b, p.b);
      U128_SHIFT_LEFT_SELF(a.b, b);
      U128_DECREMENT_SELF(a.b);
    }else{
      U128_SET_ONES(a.b);
    }
  }else{
    U128_SET_ONES(a.a);
    U128_SET_ONES(a.b);
  }
  *a_base=a;
  return status;
}

u32
fracterval_u128_to_fakefloat(fru128 p, u8 units_bit_idx){
/*
Convert a 128-bit fixed-point fracterval to a fakefloat (a u32 masquerading as a float) by applying nearest-or-even rounding, and while accounting for the one-ULP understatement of its upper limit.

In:

  p is a 128-bit fixed-point fracterval -- that is, a fracterval with a binary point which may be somewhere other than at the postterminal bit index.

  units_bit_idx is the number of bits in the fractional part. For an ordinary 128-bit fracterval (or fractoid, for that matter), this would simply be U128_BITS. On [0, U128_BITS].

Out:

  Returns a nonnegative fakefloat of the description in the summary. It may be infinity or denormal.
*/
  u8 exp;
  u8 half_ulp_count;
  u32 fakefloat;
  u32 mantissa;
  u8 msb;
  u8 shift;
  u64 t;
  u64 u;
/*
Given that the upper limit understates the max by a ULP, and given that we're about to shift away the low bits of both limits, we might be disposing of up to (3/2) of a ULP. This might affect the nearest-or-even rounding result of the mean below.
*/
  U128_TO_U64_LO(t, p.a);
  U128_TO_U64_LO(u, p.b);
  half_ulp_count=1;
  half_ulp_count=(u8)(half_ulp_count+(half_ulp_count&t)+(half_ulp_count&u));
/*
Set p.a to the floor of the mean of itself and p.b, or one less.
*/
  FRU128_SHIFT_RIGHT_SELF(p, 1);
  U128_ADD_U128_SELF(p.a, p.b);
/*
Add another ULP if we lost it due to truncation. (It wont't wrap.)
*/
  if(half_ulp_count!=1){
    half_ulp_count&=1;
    U128_INCREMENT_SELF(p.a);
  }
/*
At most, the mean is understated by half a ULP. Now find the MSB of the mean (in p.a) so we can determine the exponent of the fakefloat to output.
*/
  U128_TO_U64_PAIR(t, u, p.a);
  if(u){
    BITSCAN_MSB64_FLAT_GET(msb, u);
    msb=(u8)(msb+U64_BITS);
  }else{
    BITSCAN_MSB64_FLAT_GET(msb, t);
  }
  exp=(u8)(msb+I8_MAX-units_bit_idx);
  if(U24_BIT_MAX<msb){
    shift=(u8)(msb-U24_BITS);
    U128_SHIFT_RIGHT(p.b, shift, p.a);
    U128_TO_U64_LO(t, p.b);
/*
Bit zero of t now contains bit negative one of the prospective mantissa. If bit zero is zero and bit negative one is one, then: (1) we might have an infinitely precise fractional part equal to (1/2) and (2) in that case, nearest-or-even rounding won't behave the same as nearest rounding. So -- _only_ in that case -- and if the fraction is in fact equal to (1/2), then we need to round down instead, resulting in an even number. We can skip this expensive check if half_ulp_count is one because, in that case, we will get the correct nearest-or-even result by simply rounding nearest.
*/
    mantissa=(u32)(t);
    if((!half_ulp_count)&&((mantissa&3)==1)){
      U128_SHIFT_LEFT_SELF(p.b, shift);
      mantissa-=U128_IS_EQUAL(p.a, p.b);
    }
    mantissa=(mantissa>>1)+(mantissa&1);
/*
If we widened the mantissa by a bit due to wrap, then we need to increment the exponent and leave the mantissa to be ANDed back to zero below.
*/
    exp=(u8)(exp+(mantissa==U24_SPAN));
  }else if(msb!=U24_BIT_MAX){
    shift=(u8)(U24_BIT_MAX-1-msb);
    U128_TO_U64_LO(t, p.a);
    mantissa=(((u32)(t)<<1)+half_ulp_count)<<shift;
    if(exp==U8_MAX){
/*
The biased exponent has wrapped to negative one. This can only occur if msb is zero and units_bit_idx is U128_BITS. There are 3 ways this can occur depending on bit zero of the input p.a and the input p.b, all of which being correctly handled below.
*/
      exp=0;
      mantissa>>=2;
    }else if(!exp){
/*
The biased exponent is zero, which means that we have 0.X implicit instead of 1.X because this is a denormal, so we must shift the mantissa right by one. Fortuneately, the result is exact because its bit zero is currently zero.
*/
      mantissa>>=1;
    }else if(!t){
/*
half_ulp_count is guaranteed to be one, while t is zero, so fix the mantissa accordingly.
*/
      exp--;
      if(exp){
        mantissa=0;
      }
    }
  }else{
/*
The MSB is already in the right place. Just check for the corner case in which upward rounding causes the MSB to move left, in which case increment the exponent, but don't worry about the mantissa itself, which will be ANDed back to zero below.
*/
    mantissa=(u32)(t);
    mantissa+=half_ulp_count&mantissa;
    exp=(u8)(exp+(mantissa==U24_SPAN));
  }
  fakefloat=((u32)(exp)<<U24_BIT_MAX)+(mantissa&(U24_MAX>>1));
  return fakefloat;
}

u128 *
fracterval_u128_u128_list_malloc(ULONG u128_idx_max){
/*
Allocate a list of (u128)s.

In:

  u128_idx_max is the number of (u128)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (u128_idx_max+1) (u128)s. It must be freed via fracterval_u128_free().
*/
  u128 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG u128_count;

  list_base=NULL;
  u128_count=u128_idx_max+1;
  if(u128_count){
    list_size=u128_count<<U128_SIZE_LOG2;
    if((list_size>>U128_SIZE_LOG2)==u128_count){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
      list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
      if((list_bit_count>>U8_BITS_LOG2)==list_size){
        list_base=DEBUG_MALLOC_PARANOID(list_size);
      }
    }
  }
  return list_base;
}

void
fracterval_u128_u128_list_zero(ULONG u128_idx_max, u128 *u128_list_base){
/*
Zero a list of (u128)s.

In:

  u128_idx_max is the number of (u128)s to zero, less one.

  u128_list_base is the base of the list to zero.

Out:

  The list is zeroed as specified above.
*/
  ULONG list_size;

  list_size=(u128_idx_max+1)<<U128_SIZE_LOG2;
  memset(u128_list_base, 0, (size_t)(list_size));
  return;
}

u64 *
fracterval_u128_u64_list_malloc(ULONG u64_idx_max){
/*
Allocate a list of (u64)s.

In:

  u64_idx_max is the number of (u64)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (u64_idx_max+1) (u64)s. It must be freed via fracterval_u128_free().
*/
  u64 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG u64_count;

  list_base=NULL;
  u64_count=u64_idx_max+1;
  if(u64_count){
    list_size=u64_count<<U64_SIZE_LOG2;
    if((list_size>>U64_SIZE_LOG2)==u64_count){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
      list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
      if((list_bit_count>>U8_BITS_LOG2)==list_size){
        list_base=DEBUG_MALLOC_PARANOID(list_size);
      }
    }
  }
  return list_base;
}

void
fracterval_u128_u64_list_zero(ULONG u64_idx_max, u64 *u64_list_base){
/*
Zero a list of (u64)s.

In:

  u64_idx_max is the number of (u64)s to zero, less one.

  u64_list_base is the base of the list to zero.

Out:

  The list is zeroed as specified above.
*/
  ULONG list_size;

  list_size=(u64_idx_max+1)<<U64_SIZE_LOG2;
  memset(u64_list_base, 0, (size_t)(list_size));
  return;
}

u128
fractoid_u128_from_mantissa_u128_product(u128 p, u128 q){
/*
Use FTD128_FROM_MANTISSA_U128_PRODUCT() instead of calling here directly.

Set a u128 fractoid to the product of 2 u128 mantissas.

In:

  p is a mantissa factor.

  q is the other mantissa factor.

Out:

  Returns the minimum value of {p*q} which includes the exact value (p*q).
*/
  u128 a0;
  u128 a1;
  u8 carry;
  u64 chunk;
  u64 p0;
  u64 p1;
  u64 q0;
  u64 q1;
  u128 r;

  U128_TO_U64_LO(p0, p);
  U128_TO_U64_HI(p1, p);
  U128_TO_U64_LO(q0, q);
  U128_TO_U64_HI(q1, q);
  U128_FROM_U64_PRODUCT(a0, p0, q0);
  U128_FROM_U64_PRODUCT(a1, p1, q1);
  U128_FROM_U64_PRODUCT(r, p0, q1);
  U128_TO_U64_LO(chunk, r);
  carry=0;
  U128_ADD_U64_HI_SELF_CHECK(a0, chunk, carry);
  U128_TO_U64_HI(chunk, r);
  U128_ADD_U8_SELF(a1, carry);
  U128_ADD_U64_LO_SELF(a1, chunk);
  U128_FROM_U64_PRODUCT(r, p1, q0);
  U128_TO_U64_LO(chunk, r);
  carry=0;
  U128_ADD_U64_HI_SELF_CHECK(a0, chunk, carry);
  U128_TO_U64_HI(chunk, r);
  U128_ADD_U8_SELF(a1, carry);
  U128_ADD_U64_LO_SELF(a1, chunk);
  if(U128_IS_ZERO(a0)){
    U128_DECREMENT_SATURATE_SELF(a1);
  }
  return a1;
}

u8
fractoid_u128_ratio_u128(u128 *a_base, u128 p, u128 q){
/*
Use FTD128_RATIO_U128() instead of calling here directly.

Set a u128 fractoid to the ratio of 2 (u128)s.

In:

  *a_base is undefined.

  p is the numerator.

  q is the denominator.

Out:

  Returns one if (p/q) exceeds one, or q is zero; else zero.

  *a_base is the minimum value of {p/q} which includes the exact value (p/q), saturating to ones.
*/
  u128 a;
  u128 dividend;
  u128 dividend0;
  u128 dividend1;
  u8 divisor_shift;
  u128 divisor_u128;
  u64 divisor_u128_hi;
  u64 divisor_u128_lo;
  u128 divisor_u128_shifted;
  u64 divisor_u64;
  u128 product;
  u128 product_shifted;
  u64 quotient;
  u8 status;

  status=0;
  if(U128_IS_LESS(p, q)){
    U128_SET_ZERO(a);
    dividend=p;
    divisor_shift=128;
    divisor_u128=q;
    do{
      divisor_shift=(u8)(divisor_shift-U16_BITS);
      U128_SHIFT_RIGHT(divisor_u128_shifted, divisor_shift, divisor_u128);
    }while(U128_IS_ZERO(divisor_u128_shifted));
    divisor_shift=(u8)(U128_BITS-U16_BITS-divisor_shift);
    U128_SET_ZERO(dividend0);
    U128_SHIFT_LEFT(dividend1, divisor_shift, dividend);
    dividend=dividend1;
    U128_SHIFT_LEFT_SELF(divisor_u128, divisor_shift);
    U128_TO_U64_HI(divisor_u128_hi, divisor_u128);
    U128_TO_U64_LO(divisor_u128_lo, divisor_u128);
    divisor_u64=divisor_u128_hi;
    divisor_u64++;
    U128_TO_U64_HI(quotient, dividend);
    if(divisor_u64){
      U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor_u64, status);
    }
    U128_FROM_U64_HI(a, quotient);
    U128_FROM_U64_PRODUCT(product, divisor_u128_lo, quotient);
    U128_SHIFT_LEFT(product_shifted, U64_BITS, product);
    if(U128_IS_LESS(dividend0, product_shifted)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
    U128_SHIFT_RIGHT(product_shifted, U128_BITS-U64_BITS, product);
    U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
    U128_FROM_U64_PRODUCT(product, divisor_u128_hi, quotient);
    U128_SUBTRACT_U128_SELF(dividend1, product);
    U128_FROM_U128_PAIR_BIT_IDX(dividend, U128_BITS-46, dividend0, dividend1);
    U128_TO_U64_HI(quotient, dividend);
    if(divisor_u64){
      U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor_u64, status);
    }
    U128_ADD_U64_SHIFTED_SELF(a, U64_BITS-46, quotient);
    U128_FROM_U64_PRODUCT(product, divisor_u128_lo, quotient);
    U128_SHIFT_LEFT(product_shifted, U64_BITS-46, product);
    if(U128_IS_LESS(dividend0, product_shifted)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
    U128_SHIFT_RIGHT(product_shifted, U128_BITS-(U64_BITS-46), product);
    U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
    U128_FROM_U64_PRODUCT(product, divisor_u128_hi, quotient);
    U128_SHIFT_LEFT(product_shifted, U128_BITS-46, product);
    if(U128_IS_LESS(dividend0, product_shifted)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
    U128_SHIFT_RIGHT(product_shifted, 46, product);
    U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
    U128_FROM_U128_PAIR_BIT_IDX(dividend, U128_BITS-(46+46), dividend0, dividend1);
    U128_TO_U64_HI(quotient, dividend);
    if(divisor_u64){
      U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor_u64, status);
    }
    quotient>>=46+46-U64_BITS;
    U128_ADD_U64_LO_SELF(a, quotient);
    U128_FROM_U64_PRODUCT(product, divisor_u128_lo, quotient);
    if(U128_IS_LESS(dividend0, product)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product);
    U128_FROM_U64_PRODUCT(product, divisor_u128_hi, quotient);
    U128_SHIFT_LEFT(product_shifted, U64_BITS, product);
    if(U128_IS_LESS(dividend0, product_shifted)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
    U128_SHIFT_RIGHT(product_shifted, U128_BITS-U64_BITS, product);
    U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
    if(U128_IS_LESS_EQUAL(divisor_u128, dividend0)||U128_IS_NOT_ZERO(dividend1)){
      U128_INCREMENT_SELF(a);
      U128_SUBTRACT_U128_SELF(dividend0, divisor_u128);
    }
    if(U128_IS_ZERO(dividend0)){
      U128_DECREMENT_SATURATE_SELF(a);
    }
  }else{
    U128_SET_ONES(a);
    status=U128_IS_NOT_EQUAL(p, q)||U128_IS_ZERO(q);
  }
  *a_base=a;
  return status;
}

u8
fractoid_u128_ratio_u64(u128 *a_base, u64 v, u64 w){
/*
Use FTD128_RATIO_U64() instead of calling here directly.

Set a u128 fractoid to the ratio of 2 (u64)s.

In:

  *a_base is undefined.

  v is the numerator.

  w is the denominator.

Out:

  Returns one if (v/w) exceeds one, or w is zero; else zero.

  *a_base is the minimum value of {v/w} which includes the exact value (v/w), saturating to ones.
*/
  u128 a;
  u128 dividend;
  u128 dividend0;
  u128 dividend1;
  u8 divisor_shift;
  u128 divisor_u128;
  u64 divisor;
  u64 divisor_shifted;
  u128 product;
  u128 product_shifted;
  u64 quotient;
  u8 status;

  status=0;
  if(v<w){
    U128_SET_ZERO(a);
    U128_FROM_U64_LO(dividend, v);
    divisor=w;
    divisor_shift=U64_BITS;
    do{
      divisor_shift=(u8)(divisor_shift-U16_BITS);
      divisor_shifted=divisor>>divisor_shift;
    }while(!divisor_shifted);
    divisor_shift=(u8)(U128_BITS-U16_BITS-divisor_shift);
    U128_SET_ZERO(dividend0);
    U128_SHIFT_LEFT(dividend1, divisor_shift, dividend);
    dividend=dividend1;
    divisor<<=(divisor_shift-U64_BITS);
    U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor, status);
    U128_FROM_U64_HI(a, quotient);
    U128_FROM_U64_PRODUCT(product, divisor, quotient);
    U128_SUBTRACT_U128_SELF(dividend1, product);
    U128_FROM_U128_PAIR_BIT_IDX(dividend, U128_BITS-46, dividend0, dividend1);
    U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor, status);
    U128_ADD_U64_SHIFTED_SELF(a, U64_BITS-46, quotient);
    U128_FROM_U64_PRODUCT(product, divisor, quotient);
    U128_SHIFT_LEFT(product_shifted, U128_BITS-46, product);
    if(U128_IS_LESS(dividend0, product_shifted)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
    U128_SHIFT_RIGHT(product_shifted, 46, product);
    U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
    U128_FROM_U128_PAIR_BIT_IDX(dividend, U128_BITS-(46+46), dividend0, dividend1);
    U128_DIVIDE_U64_TO_U64(quotient, dividend, divisor, status);
    quotient>>=46+46-U64_BITS;
    U128_ADD_U64_LO_SELF(a, quotient);
    U128_FROM_U64_PRODUCT(product, divisor, quotient);
    U128_SHIFT_LEFT(product_shifted, U64_BITS, product);
    if(U128_IS_LESS(dividend0, product_shifted)){
      U128_DECREMENT_SELF(dividend1);
    }
    U128_SUBTRACT_U128_SELF(dividend0, product_shifted);
    U128_SHIFT_RIGHT(product_shifted, U128_BITS-U64_BITS, product);
    U128_SUBTRACT_U128_SELF(dividend1, product_shifted);
    U128_FROM_U64_HI(divisor_u128, divisor);
    if(U128_IS_LESS_EQUAL(divisor_u128, dividend0)||U128_IS_NOT_ZERO(dividend1)){
      U128_INCREMENT_SELF(a);
      U128_SUBTRACT_U128_SELF(dividend0, divisor_u128);
    }
    if(U128_IS_ZERO(dividend0)){
      U128_DECREMENT_SATURATE_SELF(a);
    }
  }else{
    U128_SET_ONES(a);
    status=(v!=w)||!w;
  }
  *a_base=a;
  return status;
}

u8
fractoid_u128_reciprocal_u128(u128 *a_base, u128 p){
/*
Use FTD128_RECIPROCAL_U128() instead of calling here directly.

Set a u128 fractoid to the reciprocal of a u128.

In:

  *a_base is undefined.

  p is the u128 whose reciprocal to compute.

Out:

  Returns one if p is zero, else zero.

  *a_base is the minimum value of {1/p} which includes the exact value (1/p), saturating to ones.
*/
  u128 a;
  u128 one;
  u8 status;

  U128_SET_ULP(one);
  status=0;
  FTD128_RATIO_U128(a, one, p, status);
  *a_base=a;
  return status;
}

u8
fractoid_u128_reciprocal_u64(u128 *a_base, u64 v){
/*
Use U128_RECIPROCAL_U64() instead of calling here directly.

Set a u128 fractoid to the reciprocal of a u64.

In:

  *a_base is undefined.

  v is the u64 whose reciprocal to compute.

Out:

  Returns one if v is zero, else zero.

  *a_base is the minimum value of {1/v} which includes the exact value (1/v), saturating to ones.
*/
  u128 a;
  u64 one;
  u8 status;

  one=1;
  status=0;
  FTD128_RATIO_U64(a, one, v, status);
  *a_base=a;
  return status;
}

u32
fractoid_u128_to_fakefloat_ceil(u128 p, u8 units_bit_idx){
/*
Convert a 128-bit fixed-point fractoid to a fakefloat by rounding toward positive infinity, and while accounting for its implied width of one ULP.

In:

  p is a 128-bit fixed-point fractoid -- that is, a fractoid with a binary point which may be somewhere other than at the postterminal bit index.

  units_bit_idx is as defined in fracterval_u128_to_fakefloat().

Out:

  Returns a nonnegative fakefloat of the description in the summary. It may be infinity.
*/
  u8 exp;
  u32 fakefloat;
  u32 mantissa;
  u8 msb;
  u8 shift;
  u64 t;
  u64 u;

  U128_INCREMENT_SELF(p);
/*
Find the MSB of p so we can determine the exponent of the fakefloat to output.
*/
  U128_TO_U64_PAIR(t, u, p);
  if(u){
    BITSCAN_MSB64_FLAT_GET(msb, u);
    msb=(u8)(msb+U64_BITS);
  }else{
    BITSCAN_MSB64_FLAT_GET(msb, t);
  }
  exp=(u8)(msb+I8_MAX-units_bit_idx);
  if(U24_BIT_MAX<=msb){
    shift=(u8)(msb-U24_BIT_MAX);
    U128_SHIFT_RIGHT_SELF(p, shift);
    U128_TO_U64_LO(t, p);
    mantissa=(u32)(t);
  }else if(msb!=U24_BIT_MAX){
    shift=(u8)(U24_BIT_MAX-msb);
    U128_TO_U64_LO(t, p);
    mantissa=(u32)(t)<<shift;
    if(mantissa){
      if(exp==U8_MAX){
/*
The biased exponent has wrapped to negative one. This can only occur if mantissa is one (because it's known to be nonzero) and units_bit_idx is U128_BITS, so the correct fakefloat is epsilon.
*/
        exp=0;
        mantissa=1;
      }else if(!exp){
/*
The biased exponent is zero, which means that we have 0.X implicit instead of 1.X because this is a denormal, but we're still assuming the latter, so we must shift the mantissa right by one. Fortuneately, the result is exact because its bit zero is currently zero.
*/
        mantissa>>=1;
      }
    }else{
/*
mantissa is zero due to wrap on the increment of p above, so the ceiling is exactly 2^(U128_BITS-units_bit_idx). Set exp accordingly (and possibly produce infinity).
*/
      exp=(u8)(U8_MAX-units_bit_idx);
    }
  }else{
/*
The MSB is already in the right place.
*/
    mantissa=(u32)(t);
  }
  fakefloat=((u32)(exp)<<U24_BIT_MAX)+(mantissa&(U24_MAX>>1));
  return fakefloat;
}

u32
fractoid_u128_to_fakefloat_floor(u128 p, u8 units_bit_idx){
/*
Convert a 128-bit fixed-point fractoid to a fakefloat by rounding toward negative infinity (but saturating to zero).

In:

  p is as defined in fractoid_u128_to_fakefloat_ceil().

  units_bit_idx is as defined in fracterval_u128_to_fakefloat().

Out:

  Returns a nonnegative fakefloat of the description in the summary. It may be denormal.
*/
  u8 exp;
  u32 fakefloat;
  u32 mantissa;
  u8 msb;
  u8 shift;
  u64 t;
  u64 u;
/*
Find the MSB of p so we can determine the exponent of the fakefloat to output.
*/
  U128_TO_U64_PAIR(t, u, p);
  if(u){
    BITSCAN_MSB64_FLAT_GET(msb, u);
    msb=(u8)(msb+U64_BITS);
  }else{
    BITSCAN_MSB64_FLAT_GET(msb, t);
  }
  exp=(u8)(msb+I8_MAX-units_bit_idx);
  if(U24_BIT_MAX<=msb){
    shift=(u8)(msb-U24_BIT_MAX);
    U128_SHIFT_RIGHT_SELF(p, shift);
    U128_TO_U64_LO(t, p);
    mantissa=(u32)(t);
  }else if(msb!=U24_BIT_MAX){
    shift=(u8)(U24_BIT_MAX-msb);
    U128_TO_U64_LO(t, p);
    mantissa=(u32)(t)<<shift;
    if(exp==U8_MAX){
/*
The biased exponent has wrapped to negative one. This can only occur if msb is zero and units_bit_idx is U128_BITS.
*/
      exp=0;
      mantissa>>=2;
    }else if(!exp){
/*
The biased exponent is zero, which means that we have 0.X implicit instead of 1.X because this is a denormal, so we must shift the mantissa right by one. Fortuneately, the result is exact because its bit zero is currently zero.
*/
      mantissa>>=1;
    }else if(!mantissa){
/*
p is zero, so ensure that fakefloat comes out as the same.
*/
      exp=0;
    }
  }else{
/*
The MSB is already in the right place.
*/
    mantissa=(u32)(t);
  }
  fakefloat=((u32)(exp)<<U24_BIT_MAX)+(mantissa&(U24_MAX>>1));
  return fakefloat;
}

u32
fractoid_u128_to_fakefloat_nearest_or_even(u128 p, u8 units_bit_idx){
/*
Convert a 128-bit fixed-point fractoid to a fakefloat by applying nearest-or-even rounding, and while accounting for its implied width of one ULP.

In:

  p is as defined in fractoid_u128_to_fakefloat_ceil().

  units_bit_idx is as defined in fracterval_u128_to_fakefloat().

Out:

  Returns a nonnegative fakefloat of the description in the summary. It may be infinity or denormal.
*/
  u32 fakefloat;
  fru128 p_fru128;

  p_fru128.a=p;
  p_fru128.b=p;
  fakefloat=fracterval_u128_to_fakefloat(p_fru128, units_bit_idx);
  return fakefloat;
}

u8
u128_divide_u128(u128 *a_base, u128 p, u128 q){
/*
Use U128_DIVIDE_U128() instead of calling here directly.

Divide a u128 by a u128 and saturate the result to (2^128-1).

In:

  *a_base is undefined.

  p is the u128 dividend.

  q is the u128 divisor.

Out:

  Returns one if q is zero, else zero.

  *a_base is MIN((p/q), (2^128-1)).
*/
  u128 a;
  u128 dividend;
  u64 dividend_hi;
  u8 dividend_shift;
  u128 dividend_shifted;
  u128 divisor;
  u64 divisor_hi;
  u64 divisor_lo;
  u8 divisor_shift;
  u128 divisor_shifted;
  u128 product;
  u64 quotient;
  u8 status;

  U128_SET_ZERO(a);
  status=0;
  dividend=p;
  divisor=q;
  U128_TO_U64_HI(divisor_hi, divisor);
  if(!divisor_hi){
    U128_TO_U64_LO(divisor_lo, divisor);
    U128_DIVIDE_U64_TO_U128(a, dividend, divisor_lo, status);
  }else if(U128_IS_LESS(divisor, dividend)){
    divisor_shift=0;
    do{
      divisor_shift++;
      U128_SHIFT_RIGHT(divisor_shifted, divisor_shift, divisor);
      U128_TO_U64_HI(divisor_hi, divisor_shifted);
    }while(!divisor_hi);
    U128_INCREMENT_SELF(divisor_shifted);
    dividend_shift=divisor_shift;
    U128_SHIFT_RIGHT(dividend_shifted, dividend_shift, dividend);
    U128_TO_U64_HI(dividend_hi, dividend_shifted);
    U128_TO_U64_LO(divisor_lo, divisor_shifted);
    if(divisor_lo<=dividend_hi){
      U128_SHIFT_RIGHT_SELF(dividend_shifted, 1);
      dividend_shift++;
    }
    U128_DIVIDE_U64_TO_U64(quotient, dividend_shifted, divisor_lo, status);
    U128_MULTIPLY_U64(product, divisor, quotient, status);
    U128_SUBTRACT_U128_SELF(dividend, product);
    while(U128_IS_LESS_EQUAL(divisor, dividend)){
      U128_SUBTRACT_U128_SELF(dividend, divisor);
      quotient++;
    }
    U128_FROM_U64_LO(a, quotient);
  }else if(U128_IS_EQUAL(divisor, dividend)){
    U128_SET_ULP(a);
  }
  *a_base=a;
  return status;
}

u8
u128_divide_u64_to_u128(u128 *a_base, u128 p, u64 v){
/*
Use U128_DIVIDE_U64_TO_U128() instead of calling here directly.

Divide a u128 by a u64 and saturate the result to (2^128-1).

In:

  *a_base is undefined.

  p is the u128 dividend.

  v is the u64 divisor.

Out:

  Returns one if v is zero, else zero.

  *a_base is MIN((p/v), (2^128-1)).
*/
  u128 a;
  #ifdef _32_
    u64 dividend;
    u8 dividend_shift;
    u64 dividend0;
    u64 dividend1;
    u64 product;
    u64 product_shifted;
    u64 quotient;
    u64 quotient_shifted;
  #endif
  u8 status;

  if(v){
    #ifdef _64_
      a=p/v;
    #else
      U128_SET_ZERO(a);
      dividend0=p.a;
      dividend1=p.b;
      do{
        dividend_shift=U64_BITS;
        if(dividend1){
          dividend_shift=0;
          while(dividend1>>1>>dividend_shift){
            dividend_shift++;
          }
          dividend_shift=(u8)(U64_BIT_MAX-dividend_shift);
        }
        dividend=dividend0;
        if(dividend_shift<=U64_BIT_MAX){
           dividend=dividend1<<dividend_shift;
           if(dividend_shift){
            dividend|=dividend0>>(U64_BITS-dividend_shift);
          }
        }
        if(!(v>>U64_BIT_MAX)){
          quotient=dividend/v;
        }else{
          quotient=0;
          if(dividend<v){
            if(dividend_shift!=U64_BITS){
              dividend_shift++;
              quotient++;
            }
          }else{
            quotient++;
          }
        }
        product=quotient*v;
        if(dividend_shift){
          product_shifted=product<<(U64_BITS-dividend_shift);
          dividend1-=(dividend0<product_shifted);
          dividend0-=product_shifted;
          quotient_shifted=quotient<<(U64_BITS-dividend_shift);
          a.a+=quotient_shifted;
          a.b+=(a.a<quotient_shifted);
          if(dividend_shift<=U64_BIT_MAX){
            product_shifted=product>>dividend_shift;
            dividend1-=product_shifted;
            quotient_shifted=quotient>>dividend_shift;
            a.b+=quotient_shifted;
          }
        }else{
          a.b+=quotient;
          dividend1-=product;
        }
      }while(dividend_shift<=U64_BIT_MAX);
    #endif
    status=0;
  }else{
    U128_SET_ONES(a);
    status=1;
  }
  *a_base=a;
  return status;
}

u8
u128_divide_u64_to_u64(u64 *t_base, u128 p, u64 v){
/*
Use U128_DIVIDE_U64_TO_U64() instead of calling here directly.

Divide a u128 by a u64 and saturate the result to U64_MAX.

In:

  p is the u128 dividend.

  v is the u64 divisor.

Out:

  Returns one if (p/v) exceeds U64_MAX, else zero.

  *t_base is MIN((p/v), U64_MAX).
*/
  #ifdef _32_
    u64 dividend;
    u64 dividend0;
    u64 dividend1;
    u8 divisor_shift;
    u32 divisor_u32;
    u64 divisor_u64;
    u64 product;
    u64 product_shifted;
    u32 quotient;
  #endif
  u64 t;
  u8 status;

  status=0;
  #ifdef _64_
    if(v){
      t=(u64)(p/v);
    }else{
      status=1;
      t=0;
      t--;
    }
  #else
    dividend=p.b;
    divisor_u64=v;
    t=0;
    if(dividend<divisor_u64){
      divisor_shift=0;
      while(!(divisor_u64>>(U64_BITS-8-divisor_shift))){
        divisor_shift=(u8)(divisor_shift+8);
      }
      dividend0=p.a;
      dividend1=dividend;
      if(divisor_shift){
        dividend1=(dividend1<<divisor_shift)|(dividend0>>(U64_BITS-divisor_shift));
        dividend0<<=divisor_shift;
      }
      dividend=dividend1;
      divisor_u64<<=divisor_shift;
      divisor_u32=(u32)(divisor_u64>>U32_BITS);
      divisor_u32++;
      quotient=(u32)(dividend>>U32_BITS);
      if(divisor_u32){
        quotient=(u32)(dividend/divisor_u32);
      }
      t=(u64)(quotient)<<U32_BITS;
      product=(u64)((u32)(divisor_u64))*quotient;
      product_shifted=product<<U32_BITS;
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>(U64_BITS-U32_BITS);
      dividend1-=product_shifted;
      product=(divisor_u64>>U32_BITS)*quotient;
      dividend1-=product;
      dividend=(dividend0>>(U64_BITS-22))|(dividend1<<22);
      quotient=(u32)(dividend>>U32_BITS);
      if(divisor_u32){
        quotient=(u32)(dividend/divisor_u32);
      }
      t+=(u64)(quotient)<<(U32_BITS-22);
      product=(u64)((u32)(divisor_u64))*quotient;
      product_shifted=product<<(U32_BITS-22);
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>(U64_BITS-(U32_BITS-22));
      dividend1-=product_shifted;
      product=(divisor_u64>>U32_BITS)*quotient;
      product_shifted=product<<(U64_BITS-22);
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>22;
      dividend1-=product_shifted;
      dividend=(dividend0>>(U64_BITS-22-22))|(dividend1<<(22+22));
      quotient=(u32)(dividend>>U32_BITS);
      if(divisor_u32){
        quotient=(u32)(dividend/divisor_u32);
      }
      quotient>>=22+22-U32_BITS;
      t+=(u64)(quotient);
      product=(u64)((u32)(divisor_u64))*quotient;
      dividend1-=(dividend0<product);
      dividend0-=product;
      product=(divisor_u64>>U32_BITS)*quotient;
      product_shifted=product<<U32_BITS;
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>(U64_BITS-U32_BITS);
      dividend1-=product_shifted;
      t+=(divisor_u64<=dividend0)||dividend1;
    }else{
      status=1;
      t--;
    }
  #endif
  *t_base=t;
  return status;
}

u128
u128_from_u128_pair_bit_idx(u8 b, u128 p, u128 q){
/*
Use U128_FROM_U128_PAIR_BIT_IDX() instead of calling here directly.

Extract a u128 from a particular bit index into a pair of (u128)s. This function exists just to prevent stupid compiler warnings in 32-bit regimes due to apparent shift overflows which actually can't occur.

In:

  b is the base bit index, on [0, U128_BIT_MAX].

  p is the low u128 of the pair.

  q is the high u128 of the pair.

Out:

  Returns (((q<<128)|p)>>b).
*/
  u128 a;

  #ifdef _64_
    a=p>>b;
    if(b){
      a+=q<<(u8)(U128_BITS-b);
    }
  #else
    if(b<=U64_BIT_MAX){
      a.a=(p.a>>b)|(p.b<<(U64_BITS-b));
      a.b=(p.b>>b)|(q.a<<(U64_BITS-b));
    }else{
      a.a=(p.b>>(b-U64_BITS))|(q.a<<(U64_BITS+U64_BITS-b));
      a.b=(q.a>>(b-U64_BITS))|(q.b<<(U64_BITS+U64_BITS-b));
    }
  #endif
  return a;    
}

u128
u128_from_u64_product(u64 v, u64 w){
/*
Use U128_FROM_U64_PRODUCT() instead of calling here directly.

Multiply a u64 by a u64 to produce a u128.

In:

  v is a u64 factor.

  w is a u64 factor.

Out:

  Returns (v*w).
*/
  u128 a;
  #ifdef _32_
    u64 a_old;
    u64 product0;
    u64 product1;
  #endif

  #ifdef _64_
    a=(u128)(v)*w;
  #else
    a.a=(u64)((u32)(v))*(u32)(w);
    a.b=(u64)((u32)(v>>U32_BITS))*(u32)(w>>U32_BITS);
    product0=(u64)((u32)(v>>U32_BITS))*(u32)(w);
    product1=(u64)((u32)(v))*(u32)(w>>U32_BITS);
    a_old=a.a;
    a.a+=product0<<U32_BITS;
    a.b+=product0>>U32_BITS;
    a.b+=(a.a<a_old);
    a_old=a.a;
    a.a+=product1<<U32_BITS;
    a.b+=product1>>U32_BITS;
    a.b+=(a.a<a_old);
  #endif
  return a;
}

u8
u128_msb_get(u128 p){
/*
Use U128_MSB_GET() instead of calling here directly.

Find the MSB of a u128. Note that MSB means "maximum zero-based bit index at which a one is present, or zero if no such index exists". It is not a bit, despite the name.

In:

  p is the u128 the MSB of which to find.

Out:

  Returns the MSB of p.
*/
  u8 msb;
  u64 uint;

  U128_TO_U64_HI(uint, p);
  if(!uint){
    U128_TO_U64_LO(uint, p);
  }
  BITSCAN_MSB64_FLAT_GET(msb, uint);
  return msb;
}

u8
u128_multiply_u64(u128 *a_base, u128 p, u64 v){
/*
Use U128_MULTIPLY_U64() instead of calling here directly.

Multiply a u128 by a u64 and saturate the result to (2^128-1).

In:

  *a_base is undefined.

  p is the u128 factor.

  v is the u64 factor.

Out:

  Returns one if (p*v) exceeds (2^128-1), else zero.

  *a_base is MIN((p*v), (2^128-1)).
*/
  u128 a;
  u64 carry;
  u64 factor0;
  u64 factor1;
  u128 product0;
  u128 product1;
  u8 status;

  U128_TO_U64_PAIR(factor0, factor1, p);
  U128_FROM_U64_PRODUCT(product0, factor0, v);
  U128_FROM_U64_PRODUCT(product1, factor1, v);
  U128_SHIFT_LEFT(a, U64_BITS, product1);
  U128_ADD_U128_SELF(a, product0);
  U128_TO_U64_HI(carry, product1);
  status=0;
  if(U128_IS_LESS(a, product0)||carry){
    U128_SET_ONES(a);
    status=1;
  }
  *a_base=a;
  return status;
}
