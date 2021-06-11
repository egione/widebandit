/*
Widebandit
Copyright 2018 Russell Leidich

This collection of files constitutes the Widebandit Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Widebandit Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Widebandit Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Widebandit Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
32-Bit Float Management via Unsigned Integers
*/
#define FLOAT_U32_EXP_GET(_float) (u8)((_float)>>23)

#define FLOAT_U32_GET(_exp, _mantissa, _sign) (((u32)(_exp)<<23)|(_mantissa)|((u32)(_sign)<<31))

#define FLOAT_U32_IS_GREATER(_exp, _mantissa, _sign, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    u8 __sign; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    __sign=(u8)((_target)>>31); \
    _status=0; \
    if(__sign==(_sign)){ \
      _status=1; \
      if((_exp)<=__exp){ \
        _status=0; \
        if(__exp==(_exp)){ \
          _status=(__mantissa<(_mantissa)); \
        } \
      } \
      _status=(u8)(__sign^_status); \
    }else{ \
      _status=0; \
      if(__exp||(_exp)||__mantissa||(_mantissa)){ \
        _status=!_sign; \
      } \
    } \
  }while(0)

#define FLOAT_U32_IS_GREATER_EQUAL(_exp, _mantissa, _sign, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    u8 __sign; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    __sign=(u8)((_target)>>31); \
    _status=0; \
    if(__sign==(_sign)){ \
      _status=1; \
      if((_exp)<=__exp){ \
        _status=0; \
        if(__exp==(_exp)){ \
          _status=(__mantissa<=(_mantissa)); \
          if(__mantissa==(_mantissa)){ \
            __sign=0; \
          } \
        } \
      } \
      _status=(u8)(__sign^_status); \
    }else{ \
      _status=1; \
      if(__exp||(_exp)||__mantissa||(_mantissa)){ \
        _status=!_sign; \
      } \
    } \
  }while(0)

#define FLOAT_U32_IS_INFINITY(_float) (u8)(((u8)((_float)>>23)==U8_MAX)&&!((_float)&0x7FFFFF))

#define FLOAT_U32_IS_LESS(_exp, _mantissa, _sign, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    u8 __sign; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    __sign=(u8)((_target)>>31); \
    _status=0; \
    if(__sign==(_sign)){ \
      _status=1; \
      if(__exp<=(_exp)){ \
        _status=0; \
        if(__exp==(_exp)){ \
          _status=((_mantissa)<__mantissa); \
        } \
      } \
      _status=(u8)(__sign^_status); \
    }else{ \
      _status=0; \
      if(__exp||(_exp)||__mantissa||(_mantissa)){ \
        _status=_sign; \
      } \
    } \
  }while(0)

#define FLOAT_U32_IS_LESS_EQUAL(_exp, _mantissa, _sign, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    u8 __sign; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    __sign=(u8)((_target)>>31); \
    _status=0; \
    if(__sign==(_sign)){ \
      _status=1; \
      if(__exp<=(_exp)){ \
        _status=0; \
        if(__exp==(_exp)){ \
          _status=((_mantissa)<=__mantissa); \
          if(__mantissa==(_mantissa)){ \
            __sign=0; \
          } \
        } \
      } \
      _status=(u8)(__sign^_status); \
    }else{ \
      _status=1; \
      if(__exp||(_exp)||__mantissa||(_mantissa)){ \
        _status=_sign; \
      } \
    } \
  }while(0)

#define FLOAT_U32_IS_NAN(_float) (u8)(((u8)((_float)>>23)==U8_MAX)&&((_float)&0x7FFFFF))

#define FLOAT_U32_IS_SIGNED(_float) (u8)((_float)>>31)

#define FLOAT_U32_IS_SUBNORMAL(_float) (u8)((!(u8)((_float)>>23))&&((_float)&0x7FFFFF))

#define FLOAT_U32_MANTISSA_GET(_float) ((_float)&0x7FFFFF)

#define FLOAT_U32_UNSIGNED_GET(_exp, _mantissa) (((u32)(_exp)<<23)|(_mantissa))

#define FLOAT_U32_UNSIGNED_IS_GREATER(_exp, _mantissa, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    _status=1; \
    if((_exp)<=__exp){ \
      _status=0; \
      if(__exp==(_exp)){ \
        _status=(__mantissa<(_mantissa)); \
      } \
    } \
  }while(0)

#define FLOAT_U32_UNSIGNED_IS_GREATER_EQUAL(_exp, _mantissa, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    _status=1; \
    if((_exp)<=__exp){ \
      _status=0; \
      if(__exp==(_exp)){ \
        _status=(__mantissa<=(_mantissa)); \
      } \
    } \
  }while(0)

#define FLOAT_U32_UNSIGNED_IS_LESS(_exp, _mantissa, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    _status=1; \
    if(__exp<=(_exp)){ \
      _status=0; \
      if(__exp==(_exp)){ \
        _status=((_mantissa)<__mantissa); \
      } \
    } \
  }while(0)

#define FLOAT_U32_UNSIGNED_IS_LESS_EQUAL(_exp, _mantissa, _target, _status) \
  do{ \
    u8 __exp; \
    u32 __mantissa; \
    \
    __exp=(u8)((_target)>>23); \
    __mantissa=(u32)((_target)&0x7FFFFF); \
    _status=1; \
    if(__exp<=(_exp)){ \
      _status=0; \
      if(__exp==(_exp)){ \
        _status=((_mantissa)<=__mantissa); \
      } \
    } \
  }while(0)

#define FLOAT_U32_IS_ZERO(_float) (u8)(!((_float)<<1))
