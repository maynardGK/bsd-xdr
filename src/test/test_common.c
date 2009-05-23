#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "test_common.h"

#if defined(_MSC_VER)
# define isnan _isnan
#endif

void
set_program_name (const char *name)
{
  program_name = base_name (name);
  if (!program_name || !*program_name)
    program_name = "<unknown program name>";
}

const char *
base_name (const char *name)
{
  const char *base;
#if defined (HAVE_DOS_BASED_FILE_SYSTEM)
  /* Skip over the disk name in MSDOS pathnames. */
  if (isalpha ((unsigned char) name[0]) && name[1] == ':')
    name += 2;
#endif

  for (base = name; *name; name++)
    if (IS_DIR_SEPARATOR (*name))
      base = name + 1;
  return base;
}

int
mkdir_p (const char *path, int mode)
{
  int len;
  char *new_path;
  int ret = 0;

  new_path = (char *) malloc (strlen(path) + 1);
  strcpy (new_path, path);
  
  while ((len = strlen (new_path)) && IS_DIR_SEPARATOR(new_path[len-1]))
    new_path[len-1] = 0;

#if defined(_MSC_VER) || defined(__MINGW32__)
  while (!_mkdir (new_path))
#else
  while (!mkdir (new_path, mode))
#endif
    {
      char *slash;
      int last_error = errno;
      if (last_error == EEXIST)
        break;
      if (last_error != ENOENT)
        {
          ret = -1;
          break;
        }
      slash = new_path + strlen (new_path);
      while (slash > new_path && !IS_DIR_SEPARATOR(*slash))
        slash--;
      if (slash == new_path)
        {
          ret = -1;
          break;
        }
      len = slash - new_path;
      new_path[len] = 0;
      if (!mkdir_p (new_path, mode))
        {
          ret = -1;
          break;
        }
      new_path[len] = '/';
    }
    free (new_path);
    return ret;
}

static void
puthex (long n, unsigned int digits, char *buf, unsigned int pos)
{
  if (digits > 1)
    puthex (n / 16, digits - 1, buf, pos);
  buf[pos + digits - 1] = "0123456789abcdef"[n % 16];
}

void
dumpmem (FILE * f, void *buf, unsigned int len, unsigned int offset)
{
  unsigned int i;
  unsigned int address;
  unsigned char *p;
  static const unsigned int MASK_LOWER = 0x0f;

  char line[80];

  if (offset >= len)
    return;

  address = offset;
  p = ((unsigned char *) buf) + offset;

  fputs("  Addr     0 1  2 3  4 5  6 7  8 9  A B  C D  E F 0 2 4 6 8 A C E \n",f);
  fputs("--------  ---- ---- ---- ---- ---- ---- ---- ---- ----------------\n",f);

  while (p < ((unsigned char *) buf) + len)
    {
      for (i = 0; i < 50; i++)
        line[i] = ' ';
      for (; i < 80; i++)
        line[i] = 0;
      if ((address & ~MASK_LOWER) != address)   /* address % 16 != 0 */
        {
          puthex ((address & ~MASK_LOWER), 8, line, 0);
          for (i = 0; i < (address & MASK_LOWER); i++)
            {
              line[10 + i * 2 + i / 2] = ' ';
              line[10 + i * 2 + i / 2 + 1] = ' ';
              line[50 + i] = ' ';
            }
          address = address & ~MASK_LOWER;
        }
      else
        {
          puthex (address, 8, line, 0);
          i = 0;
        }

      for (; i < 16; i++)
        {
          puthex (((long) *p) & 0x0ff, 2, line, 10 + i * 2 + i / 2);
          line[50 + i] = '.';
          if (isprint (*p))
            line[50 + i] = *p;
          if (++p >= (unsigned char *) buf + len)
            break;
        }
      fputs (line, f);
      fputs ("\n", f);
      address += 16;
    }
}

void
log_msg (log_opts * o, int level, const char *fmt, ...)
{
  va_list ap;
  if (o->level >= level)
    {
      va_start (ap, fmt);
      vfprintf (o->f, fmt, ap);
      va_end (ap);
    }
}

const int INT_DATA[TEST_DATA_SZ] =
  {
    0, INT_MAX, INT_MIN, INT32_C(-1),
    INT32_C( 54350646), INT32_C(-64809791), INT32_C(-11862367), INT32_C(-45059576),
    INT32_C( 23882188), INT32_C( 50534107), INT32_C(   907862), INT32_C(-18492451),
    INT32_C(-12814078), INT32_C(-19889041), INT32_C( 53335488), INT32_C( 11584024),
    INT32_C(-47482834), INT32_C( 63361713), INT32_C(-41899789), INT32_C(-59359264)
  };
const unsigned int UINT_DATA[TEST_DATA_SZ] =
  {
    0, UINT_MAX, (unsigned int)(INT_MAX), (unsigned int)(INT_MAX) + 1,
    UINT32_C(3798144277), UINT32_C(3595506505), UINT32_C( 619345635), UINT32_C( 318999729),
    UINT32_C(3584736518), UINT32_C(1187377429), UINT32_C(1883238210), UINT32_C(4216385634),
    UINT32_C(3854776543), UINT32_C( 115640192), UINT32_C( 469797816), UINT32_C(1435434147),
    UINT32_C( 434239253), UINT32_C(1964876812), UINT32_C(1643135139), UINT32_C(2754091278)
  };
const long LONG_DATA[TEST_DATA_SZ] =
  {
    0, LONG_MAX, LONG_MIN, INT32_C(-1),
    INT32_C( 54350646), INT32_C( -64809791), INT32_C(  -11862367), INT32_C(-45059576),
    INT32_C( 23882188), INT32_C(  50534107), INT32_C(     907862), INT32_C(-18492451),
    INT32_C(-12814078), INT32_C( -19889041), INT32_C(   53335488), INT32_C( 11584024),
    INT32_C(-47482834), INT32_C(  63361713), INT32_C(  -41899789), INT32_C(-59359264)
  };
const unsigned long ULONG_DATA[TEST_DATA_SZ] =
  {
    0, ULONG_MAX, (unsigned long)(LONG_MAX), (unsigned long)(LONG_MAX) + 1,
    UINT32_C(3798144277), UINT32_C(3595506505), UINT32_C( 619345635), UINT32_C( 318999729),
    UINT32_C(3584736518), UINT32_C(1187377429), UINT32_C(1883238210), UINT32_C(4216385634),
    UINT32_C(3854776543), UINT32_C( 115640192), UINT32_C( 469797816), UINT32_C(1435434147),
    UINT32_C( 434239253), UINT32_C(1964876812), UINT32_C(1643135139), UINT32_C(2754091278)
  };
const short SHORT_DATA[TEST_DATA_SZ] =
  {
    0, SHRT_MAX, SHRT_MIN, INT16_C(-1),
    INT16_C(-30352), INT16_C( 22597), INT16_C(-13665), INT16_C( 25695),
    INT16_C( 20236), INT16_C(-14780), INT16_C( -1088), INT16_C(-21478),
    INT16_C(-30488), INT16_C(  2801), INT16_C(-32307), INT16_C( -5883),
    INT16_C(  9908), INT16_C( 24299), INT16_C(-31958), INT16_C(-27067)
  };
const unsigned short USHORT_DATA[TEST_DATA_SZ] =
  {
    0, USHRT_MAX, (unsigned short)(SHRT_MAX), (unsigned short)(SHRT_MAX) + 1,
    UINT16_C( 3441), UINT16_C(11289), UINT16_C( 5632), UINT16_C(64499),
    UINT16_C(58162), UINT16_C(11112), UINT16_C(65273), UINT16_C(56465),
    UINT16_C(31873), UINT16_C(65442), UINT16_C(65114), UINT16_C(34024),
    UINT16_C(30053), UINT16_C(53460), UINT16_C(22331), UINT16_C( 5943)
  };
const signed char SCHAR_DATA[TEST_DATA_SZ] =
  {
    0, (signed char)SCHAR_MAX, (signed char)SCHAR_MIN, INT8_C(-1),
    INT8_C( 99), INT8_C( 49), INT8_C( 56), INT8_C(-41),
    INT8_C(-55), INT8_C(124), INT8_C( 16), INT8_C(-110),
    INT8_C( 71), INT8_C( 69), INT8_C( 25), INT8_C( -25),
    INT8_C( 87), INT8_C(102), INT8_C(-75), INT8_C(  23)
  };
const unsigned char UCHAR_DATA[TEST_DATA_SZ] =
  {
    0, UCHAR_MAX, (unsigned char)(SCHAR_MAX), (unsigned char)(SCHAR_MAX) + 1,
    UINT8_C(207), UINT8_C(214), UINT8_C(173), UINT8_C(153),
    UINT8_C(217), UINT8_C( 26), UINT8_C(147), UINT8_C( 54),
    UINT8_C(185), UINT8_C(  5), UINT8_C(111), UINT8_C( 68),
    UINT8_C(180), UINT8_C(109), UINT8_C( 74), UINT8_C( 83)
  };
const int8_t INT8_DATA[TEST_DATA_SZ] =
  {
    0, (int8_t)INT8_C(127), (int8_t)INT8_C(-128), (int8_t)INT8_C(-1),
    INT8_C( 99), INT8_C( 49), INT8_C( 56), INT8_C(-41),
    INT8_C(-55), INT8_C(124), INT8_C( 16), INT8_C(-110),
    INT8_C( 71), INT8_C( 69), INT8_C( 25), INT8_C( -25),
    INT8_C( 87), INT8_C(102), INT8_C(-75), INT8_C(  23)
  };
const u_int8_t UINT8_DATA[TEST_DATA_SZ] =
  {
    0, (u_int8_t)UINT8_C(255), (u_int8_t)UINT8_C(127), (u_int8_t)UINT8_C(128),
    UINT8_C(207), UINT8_C(214), UINT8_C(173), UINT8_C(153),
    UINT8_C(217), UINT8_C( 26), UINT8_C(147), UINT8_C( 54),
    UINT8_C(185), UINT8_C(  5), UINT8_C(111), UINT8_C( 68),
    UINT8_C(180), UINT8_C(109), UINT8_C( 74), UINT8_C( 83)
  };
const int16_t INT16_DATA[TEST_DATA_SZ] =
  {
    (int16_t)0, (int16_t)INT16_C(32767), (int16_t)INT16_C(-32768), (int16_t)INT16_C(-1),
    INT16_C(-30352), INT16_C( 22597), INT16_C(-13665), INT16_C( 25695),
    INT16_C( 20236), INT16_C(-14780), INT16_C( -1088), INT16_C(-21478),
    INT16_C(-30488), INT16_C(  2801), INT16_C(-32307), INT16_C( -5883),
    INT16_C(  9908), INT16_C( 24299), INT16_C(-31958), INT16_C(-27067)
  };
const u_int16_t UINT16_DATA[TEST_DATA_SZ] =
  {
    0, (u_int16_t)UINT16_C(65535), (u_int16_t)UINT16_C(32767), (u_int16_t)UINT16_C(32768),
    UINT16_C( 3441), UINT16_C(11289), UINT16_C( 5632), UINT16_C(64499),
    UINT16_C(58162), UINT16_C(11112), UINT16_C(65273), UINT16_C(56465),
    UINT16_C(31873), UINT16_C(65442), UINT16_C(65114), UINT16_C(34024),
    UINT16_C(30053), UINT16_C(53460), UINT16_C(22331), UINT16_C( 5943)
  };
const int32_t INT32_DATA[TEST_DATA_SZ] =
  {
    0, (int32_t)INT32_C(2147483647), (int32_t)(INT32_C(-2147483647) - 1), (int32_t)INT32_C(-1),
    INT32_C( 54350646), INT32_C(-64809791), INT32_C(-11862367), INT32_C(-45059576),
    INT32_C( 23882188), INT32_C( 50534107), INT32_C(   907862), INT32_C(-18492451),
    INT32_C(-12814078), INT32_C(-19889041), INT32_C( 53335488), INT32_C( 11584024),
    INT32_C(-47482834), INT32_C( 63361713), INT32_C(-41899789), INT32_C(-59359264)
  };
const u_int32_t UINT32_DATA[TEST_DATA_SZ] =
  {
    0, (u_int32_t)UINT32_C(4294967295), (u_int32_t)UINT32_C(2147483647), (u_int32_t)UINT32_C(2147483648),
    UINT32_C(3798144277), UINT32_C(3595506505), UINT32_C( 619345635), UINT32_C( 318999729),
    UINT32_C(3584736518), UINT32_C(1187377429), UINT32_C(1883238210), UINT32_C(4216385634),
    UINT32_C(3854776543), UINT32_C( 115640192), UINT32_C( 469797816), UINT32_C(1435434147),
    UINT32_C( 434239253), UINT32_C(1964876812), UINT32_C(1643135139), UINT32_C(2754091278)
  };
const int64_t INT64_DATA[TEST_DATA_SZ] =
  {
    0,
    (int64_t)INT64_C(9223372036854775807),
    (int64_t)(INT64_C(-9223372036854775807) - 1),
    (int64_t)INT64_C(-1),
    (int64_t)INT64_C(0xcd5ea305f5ab7210),
    (int64_t)INT64_C(0xf159826c77048e32),
    (int64_t)INT64_C(0x2df9fa9b5568856a),
    (int64_t)INT64_C(0xb48a54925bb00109),
    (int64_t)INT64_C(0x75abc1347acc9205),
    (int64_t)INT64_C(0x01bd8b12842b3da7),
    (int64_t)INT64_C(0x2fb1a2358c8962fb),
    (int64_t)INT64_C(0x12f9e34c78235659),
    (int64_t)INT64_C(0xcf032120fa647036),
    (int64_t)INT64_C(0xc60432d271fad6f4),
    (int64_t)INT64_C(0x2fbc922f5bc62ccd),
    (int64_t)INT64_C(0x13708c83035553b6),
    (int64_t)INT64_C(0x5e3ff2ac3200f85d),
    (int64_t)INT64_C(0x7c1924ac4c43a409),
    (int64_t)INT64_C(0x8bf153b4a989a02f),
    (int64_t)INT64_C(0x9ff792c92d4f6838)
  };
const u_int64_t UINT64_DATA[TEST_DATA_SZ] =
  {
    0,
    (u_int64_t)UINT64_C(18446744073709551615),
    (u_int64_t)UINT64_C(9223372036854775807),
    (u_int64_t)UINT64_C(9223372036854775808),
    (u_int64_t)UINT64_C(0xfc1de89df6a68c8c),
    (u_int64_t)UINT64_C(0x37edbc5a5a427bf6),
    (u_int64_t)UINT64_C(0xd72cce3e10852d67),
    (u_int64_t)UINT64_C(0x8e36d22d7e51f190),
    (u_int64_t)UINT64_C(0x1203d9f5d0f7fbc5),
    (u_int64_t)UINT64_C(0xea2589f5e44797fe),
    (u_int64_t)UINT64_C(0x469edf0e08506d6e),
    (u_int64_t)UINT64_C(0xb8fe67fa804eb38d),
    (u_int64_t)UINT64_C(0x5675358411448cdd),
    (u_int64_t)UINT64_C(0xc884ddb18d4132ca),
    (u_int64_t)UINT64_C(0xe957255f3af03f9a),
    (u_int64_t)UINT64_C(0xd7b14a06739569c5),
    (u_int64_t)UINT64_C(0x2aef14a461a40e99),
    (u_int64_t)UINT64_C(0x349b6f6c3a718ad0),
    (u_int64_t)UINT64_C(0x2d88b55c6298f4c9),
    (u_int64_t)UINT64_C(0x32cd672ac64b8c7b)
  };
const quad_t HYPER_DATA[TEST_DATA_SZ] =
  {
    0,
    (quad_t)INT64_C(9223372036854775807),
    (quad_t)(INT64_C(-9223372036854775807) - 1),
    (quad_t)INT64_C(-1),
    (quad_t)INT64_C(0xcd5ea305f5ab7210),
    (quad_t)INT64_C(0xf159826c77048e32),
    (quad_t)INT64_C(0x2df9fa9b5568856a),
    (quad_t)INT64_C(0xb48a54925bb00109),
    (quad_t)INT64_C(0x75abc1347acc9205),
    (quad_t)INT64_C(0x01bd8b12842b3da7),
    (quad_t)INT64_C(0x2fb1a2358c8962fb),
    (quad_t)INT64_C(0x12f9e34c78235659),
    (quad_t)INT64_C(0xcf032120fa647036),
    (quad_t)INT64_C(0xc60432d271fad6f4),
    (quad_t)INT64_C(0x2fbc922f5bc62ccd),
    (quad_t)INT64_C(0x13708c83035553b6),
    (quad_t)INT64_C(0x5e3ff2ac3200f85d),
    (quad_t)INT64_C(0x7c1924ac4c43a409),
    (quad_t)INT64_C(0x8bf153b4a989a02f),
    (quad_t)INT64_C(0x9ff792c92d4f6838)
  };
const u_quad_t UHYPER_DATA[TEST_DATA_SZ] =
  {
    0,
    (u_quad_t)UINT64_C(18446744073709551615),
    (u_quad_t)UINT64_C(9223372036854775807),
    (u_quad_t)UINT64_C(9223372036854775808),
    (u_quad_t)UINT64_C(0xfc1de89df6a68c8c),
    (u_quad_t)UINT64_C(0x37edbc5a5a427bf6),
    (u_quad_t)UINT64_C(0xd72cce3e10852d67),
    (u_quad_t)UINT64_C(0x8e36d22d7e51f190),
    (u_quad_t)UINT64_C(0x1203d9f5d0f7fbc5),
    (u_quad_t)UINT64_C(0xea2589f5e44797fe),
    (u_quad_t)UINT64_C(0x469edf0e08506d6e),
    (u_quad_t)UINT64_C(0xb8fe67fa804eb38d),
    (u_quad_t)UINT64_C(0x5675358411448cdd),
    (u_quad_t)UINT64_C(0xc884ddb18d4132ca),
    (u_quad_t)UINT64_C(0xe957255f3af03f9a),
    (u_quad_t)UINT64_C(0xd7b14a06739569c5),
    (u_quad_t)UINT64_C(0x2aef14a461a40e99),
    (u_quad_t)UINT64_C(0x349b6f6c3a718ad0),
    (u_quad_t)UINT64_C(0x2d88b55c6298f4c9),
    (u_quad_t)UINT64_C(0x32cd672ac64b8c7b)
  };
const quad_t LONGLONG_DATA[TEST_DATA_SZ] =
  {
    0,
    (quad_t)INT64_C(9223372036854775807),
    (quad_t)(INT64_C(-9223372036854775807) - 1),
    (quad_t)INT64_C(-1),
    (quad_t)INT64_C(0xcd5ea305f5ab7210),
    (quad_t)INT64_C(0xf159826c77048e32),
    (quad_t)INT64_C(0x2df9fa9b5568856a),
    (quad_t)INT64_C(0xb48a54925bb00109),
    (quad_t)INT64_C(0x75abc1347acc9205),
    (quad_t)INT64_C(0x01bd8b12842b3da7),
    (quad_t)INT64_C(0x2fb1a2358c8962fb),
    (quad_t)INT64_C(0x12f9e34c78235659),
    (quad_t)INT64_C(0xcf032120fa647036),
    (quad_t)INT64_C(0xc60432d271fad6f4),
    (quad_t)INT64_C(0x2fbc922f5bc62ccd),
    (quad_t)INT64_C(0x13708c83035553b6),
    (quad_t)INT64_C(0x5e3ff2ac3200f85d),
    (quad_t)INT64_C(0x7c1924ac4c43a409),
    (quad_t)INT64_C(0x8bf153b4a989a02f),
    (quad_t)INT64_C(0x9ff792c92d4f6838)
  };
const u_quad_t ULONGLONG_DATA[TEST_DATA_SZ] =
  {
    0,
    (u_quad_t)UINT64_C(18446744073709551615),
    (u_quad_t)UINT64_C(9223372036854775807),
    (u_quad_t)UINT64_C(9223372036854775808),
    (u_quad_t)UINT64_C(0xfc1de89df6a68c8c),
    (u_quad_t)UINT64_C(0x37edbc5a5a427bf6),
    (u_quad_t)UINT64_C(0xd72cce3e10852d67),
    (u_quad_t)UINT64_C(0x8e36d22d7e51f190),
    (u_quad_t)UINT64_C(0x1203d9f5d0f7fbc5),
    (u_quad_t)UINT64_C(0xea2589f5e44797fe),
    (u_quad_t)UINT64_C(0x469edf0e08506d6e),
    (u_quad_t)UINT64_C(0xb8fe67fa804eb38d),
    (u_quad_t)UINT64_C(0x5675358411448cdd),
    (u_quad_t)UINT64_C(0xc884ddb18d4132ca),
    (u_quad_t)UINT64_C(0xe957255f3af03f9a),
    (u_quad_t)UINT64_C(0xd7b14a06739569c5),
    (u_quad_t)UINT64_C(0x2aef14a461a40e99),
    (u_quad_t)UINT64_C(0x349b6f6c3a718ad0),
    (u_quad_t)UINT64_C(0x2d88b55c6298f4c9),
    (u_quad_t)UINT64_C(0x32cd672ac64b8c7b)
  };

#define FLT_DATA_PINF_INDEX  5
#define FLT_DATA_NINF_INDEX  6
#define FLT_DATA_NAN_INDEX   7
#define FLT_DATA_NZERO_INDEX 8

const float FLOAT_DATA[TEST_DATA_SZ] =
  {
    0.0F,
    FLT_MAX,
    -FLT_MAX,
    FLT_EPSILON,
    -FLT_EPSILON,
    0.0, /* PLACEHOLDER FOR +inf */
    0.0, /* PLACEHOLDER FOR -inf */
    0.0, /* PLACEHOLDER FOR NaN */
    0.0, /* PLACEHOLDER FOR -0.0 */
    -1.721740E+09F,  1.546803E+03F,  1.266515E-09F,
     2.523149E+02F,  6.061230E+09F, -4.574789E+04F,
    -4.018012E+09F, -1.075038E-06F,  4.642894E-03F,
    -2.048046E+09F,  5.444124E+09F
  };
void init_float_data (float *data)
{
  int i;
  static float ZERO = 0.0;
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = FLOAT_DATA[i];
  data[FLT_DATA_PINF_INDEX]  = 1.0 / ZERO;
  data[FLT_DATA_NINF_INDEX]  = logf (ZERO);
  data[FLT_DATA_NAN_INDEX]   = sqrtf (-1.0);
  data[FLT_DATA_NZERO_INDEX] = 1.0 / data[FLT_DATA_NINF_INDEX];
}

const double DOUBLE_DATA[TEST_DATA_SZ] =
  {
    0.0,
    DBL_MAX,
    -DBL_MAX,
    DBL_EPSILON,
    -DBL_EPSILON,
    0.0, /* PLACEHOLDER FOR +inf */
    0.0, /* PLACEHOLDER FOR -inf */
    0.0, /* PLACEHOLDER FOR NaN */
    0.0, /* PLACEHOLDER FOR -0.0 */
    -1.721739906316422E+09,  1.546802632887171E+03,  1.266514632558168E-09,
     2.523149000160256E+02,  6.061230007925079E+09, -4.574788745965622E+04,
    -4.018011578257721E+09, -1.075037635051873E-06,  4.642894658457186E-03,
    -2.048046158550385E+09,  5.444123732577301E+09
  };
void init_double_data (double *data)
{
  int i;
  static double ZERO = 0.0;
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = DOUBLE_DATA[i];
  data[FLT_DATA_PINF_INDEX]  = 1.0 / ZERO;
  data[FLT_DATA_NINF_INDEX]  = log (ZERO);
  data[FLT_DATA_NAN_INDEX]   = sqrt (-1.0);
  data[FLT_DATA_NZERO_INDEX] = 1.0 / data[FLT_DATA_NINF_INDEX];
}

const bool_t BOOL_DATA[TEST_DATA_SZ] =
  {
    FALSE, FALSE,  TRUE,  TRUE,
    FALSE,  TRUE,  TRUE, FALSE,
     TRUE,  TRUE, FALSE, FALSE,
     TRUE, FALSE, FALSE,  TRUE,
     TRUE,  TRUE, FALSE, FALSE
  };

const test_enum_t ENUM_DATA[TEST_DATA_SZ] =
  {
    EN_A, EN_B, EN_C, EN_D,
    EN_B, EN_C, EN_D, EN_A,
    EN_C, EN_D, EN_A, EN_B,
    EN_D, EN_A, EN_B, EN_C,
    EN_D, EN_C, EN_B, EN_A
  };

const struct xdr_discrim test_union_dscrim[6] =
  {
    {(int)TEST_UNION_FLOAT,  (xdrproc_t)&xdr_float},
    {(int)TEST_UNION_DOUBLE, (xdrproc_t)&xdr_double},
    {(int)TEST_UNION_UI32,   (xdrproc_t)&xdr_u_int32_t},
    {(int)TEST_UNION_CHAR,   (xdrproc_t)&xdr_char},
    {(int)TEST_UNION_I64,    (xdrproc_t)&xdr_int64_t},
    {__dontcare__,           (xdrproc_t)NULL}
  };

test_discrim_union_t UNION_DATA[TEST_DATA_SZ];
void init_union_data (test_discrim_union_t *data)
{
  data[ 0].value.flt = 27.3;       data[ 0].type = TEST_UNION_FLOAT;
  data[ 1].value.dbl = -15.3;      data[ 1].type = TEST_UNION_DOUBLE;
  data[ 2].value.u32 = 8712384;    data[ 2].type = TEST_UNION_UI32;
  data[ 3].value.c   = 'a';        data[ 3].type = TEST_UNION_CHAR;
  data[ 4].value.i64 = -781628734; data[ 4].type = TEST_UNION_I64;
  data[ 5].value.i64 = 6182364;    data[ 5].type = TEST_UNION_I64;
  data[ 6].value.c   = 'b';        data[ 6].type = TEST_UNION_CHAR;
  data[ 7].value.u32 = 12348;      data[ 7].type = TEST_UNION_UI32;
  data[ 8].value.dbl = 100.0;      data[ 8].type = TEST_UNION_DOUBLE;
  data[ 9].value.flt = -50.0;      data[ 9].type = TEST_UNION_FLOAT;
  data[10].value.c   = 'c';        data[10].type = TEST_UNION_CHAR;
  data[11].value.c   = 'd';        data[11].type = TEST_UNION_CHAR;
  data[12].value.c   = 'e';        data[12].type = TEST_UNION_CHAR;
  data[13].value.flt = 23.0;       data[13].type = TEST_UNION_FLOAT;
  data[14].value.flt = 22.0;       data[14].type = TEST_UNION_FLOAT;
  data[15].value.dbl = 10.2;       data[15].type = TEST_UNION_DOUBLE;
  data[16].value.dbl = 11.3;       data[16].type = TEST_UNION_DOUBLE;
  data[17].value.dbl = 12.4;       data[17].type = TEST_UNION_DOUBLE;
  data[18].value.u32 = 78163;      data[18].type = TEST_UNION_UI32;
  data[19].value.u32 = 98234;      data[19].type = TEST_UNION_UI32;
}

bool_t encode_union_data (const char *testid, log_opts *o, XDR *xdrs,
                          int cnt, test_discrim_union_t *data)
{
  /* note that the actual xdr_union call is the same in every case.
     the only reason for this function and the switch() is so that
     the debug and error messages are properly formatted */
  switch (data->type)
    {
      case TEST_UNION_FLOAT:
        log_msg (o, XDR_LOG_DEBUG2,
                 "%s: about to encode union(float) (cnt=%d, val=%.8e)\n",
                 testid, cnt, data->value.flt);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (float) XDR_ENCODE (cnt=%d, val=%.8e)\n",
                     testid, cnt, data->value.flt);
            return FALSE;
          }
        break;
      case TEST_UNION_DOUBLE:
        log_msg (o, XDR_LOG_DEBUG2,
                 "%s: about to encode union(double) (cnt=%d, val=%.15e)\n",
                 testid, cnt, data->value.dbl);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (double) XDR_ENCODE (cnt=%d, val=%.15e)\n",
                     testid, cnt, data->value.dbl);
            return FALSE;
          }
        break;
      case TEST_UNION_UI32:
        log_msg (o, XDR_LOG_DEBUG2,
                 "%s: about to encode union(uint32) (cnt=%d, val=%" PRIu32 ")\n",
                 testid, cnt, data->value.u32);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (uint32) XDR_ENCODE (cnt=%d, val=%" PRIu32 ")\n",
                     testid, cnt, data->value.u32);
            return FALSE;
          }
        break;
      case TEST_UNION_CHAR:
        log_msg (o, XDR_LOG_DEBUG2,
                 "%s: about to encode union(char) (cnt=%d, val=0x%02x)\n",
                 testid, cnt, data->value.c);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (char) XDR_ENCODE (cnt=%d, val=0x%02x)\n",
                     testid, cnt, data->value.c);
            return FALSE;
          }
        break;
      case TEST_UNION_I64:
        log_msg (o, XDR_LOG_DEBUG2,
                 "%s: about to encode union(int64) (cnt=%d, val=%" PRId64 ")\n",
                 testid, cnt, data->value.i64);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (int64) XDR_ENCODE (cnt=%d, val=%" PRId64 ")\n",
                     testid, cnt, data->value.i64);
            return FALSE;
          }
        break;
      default:
        /* NOT REACHED */
        return FALSE;
    } /* switch */
  return TRUE;
}

bool_t decode_union_data (const char *testid, log_opts *o, XDR *xdrs,
                          int cnt, test_discrim_union_t *data)
{
  /* note that the actual xdr_union call is the same in every case.
     the only reason for this function and the switch() is so that
     the debug and error messages are properly formatted */
  switch (data->type)
    {
      case TEST_UNION_FLOAT:
        log_msg (o, XDR_LOG_DEBUG2,
               "%s: about to decode union(float) (cnt=%d)\n",
               testid, cnt);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (float) XDR_DECODE (cnt=%d, val=%.8e)\n",
                     testid, cnt, data->value.flt);
            return FALSE;
          }
        break;
      case TEST_UNION_DOUBLE:
        log_msg (o, XDR_LOG_DEBUG2,
               "%s: about to decode union(double) (cnt=%d)\n",
               testid, cnt);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (double) XDR_DECODE (cnt=%d, val=%.15e)\n",
                     testid, cnt, data->value.dbl);
            return FALSE;
          }
        break;
      case TEST_UNION_UI32:
        log_msg (o, XDR_LOG_DEBUG2,
               "%s: about to decode union(uint32) (cnt=%d)\n",
               testid, cnt);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (uint32) XDR_DECODE (cnt=%d, val=%" PRId32 ")\n",
                     testid, cnt, data->value.u32);
            return FALSE;
          }
        break;
      case TEST_UNION_CHAR:
        log_msg (o, XDR_LOG_DEBUG2,
               "%s: about to decode union(char) (cnt=%d)\n",
               testid, cnt);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (char) XDR_DECODE (cnt=%d, val=0x%02x)\n",
                     testid, cnt, data->value.c);
            return FALSE;
          }
        break;
      case TEST_UNION_I64:
        log_msg (o, XDR_LOG_DEBUG2,
               "%s: about to decode union(int64) (cnt=%d)\n",
               testid, cnt);
        if (!xdr_union (xdrs, (enum_t *)&(data->type), (char *)&(data->value), test_union_dscrim, NULL_xdrproc_t))
          {
            log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union (int64) XDR_DECODE (cnt=%d, val=%" PRId64 ")\n",
                     testid, cnt, data->value.u32);
            return FALSE;
          }
        break;
      default:
        /* NOT REACHED */
        return FALSE;
    } /* switch */
  return TRUE;
}

bool_t compare_union_data (const char *testid, log_opts *o,
                           test_discrim_union_t *s,
                           test_discrim_union_t *d)
{
  int cnt;
  for (cnt=0; cnt < TEST_DATA_SZ; cnt++)
    {
      switch (s[cnt].type)
        {
          case TEST_UNION_FLOAT:
            if ((s[cnt].value.flt != d[cnt].value.flt)
               && !(isnan(s[cnt].value.flt) && isnan(d[cnt].value.flt)))
              {
                log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union XDR_DECODE (float) (cnt=%d, exp=%.8e val=%.8e)\n",
                     testid, cnt, s[cnt].value.flt, d[cnt].value.flt);
                return FALSE;
              }
            break;
          case TEST_UNION_DOUBLE:
            if ((s[cnt].value.dbl != d[cnt].value.dbl)
               && !(isnan(s[cnt].value.dbl) && isnan(d[cnt].value.dbl)))
              {
                log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union XDR_DECODE (double) (cnt=%d, exp=%.15e val=%.15e)\n",
                     testid, cnt, s[cnt].value.dbl, d[cnt].value.dbl);
                return FALSE;
              }
            break;
          case TEST_UNION_UI32:
            if (s[cnt].value.u32 != d[cnt].value.u32)
              {
                log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union XDR_DECODE (uint32) (cnt=%d, exp=%"
                     PRIu32 " val=%" PRIu32 ")\n",
                     testid, cnt, s[cnt].value.u32, d[cnt].value.u32);
                return FALSE;
              }
            break;
          case TEST_UNION_CHAR:
            if (s[cnt].value.c != d[cnt].value.c)
              {
                log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union XDR_DECODE (char) (cnt=%d, exp=0x%02x val=0x%02x)\n",
                     testid, cnt, s[cnt].value.c, d[cnt].value.c);
                return FALSE;
              }
            break;
          case TEST_UNION_I64:
            if (s[cnt].value.i64 != d[cnt].value.i64)
              {
                log_msg (o, XDR_LOG_INFO,
                     "%s: failed xdr_union XDR_DECODE (int64) (cnt=%d, exp=%"
                     PRId64 " val=%" PRId64 ")\n",
                     testid, cnt, s[cnt].value.i64, d[cnt].value.i64);
                return FALSE;
              }
            break;
          default:
            /* NOT REACHED */
            return FALSE;
        } /* switch */
    } /* for */
  return TRUE;
}

const char OPAQUE_DATA[OPAQUE_DATA_SZ] =
{
  0xbd, 0x36, 0x56, 0x52, 0xff, 0x4a, 0x02, 0x81, 
  0x96, 0x47, 0x6a, 0x5c, 0x60, 0x53, 0x30, 0xb6, 
  0x87, 0x52, 0xf6, 0xff, 0x96, 0xad, 0xd9, 0xbb, 
  0xd7, 0x6f, 0xb2, 0xa0, 0x21, 0xbd, 0x36, 0x9a, 
  0x45, 0x85, 0x8f, 0x29, 0x8a, 0x7f, 0x2f, 0x3a, 
  0xbb, 0x01, 0x3e, 0x98, 0xd2, 0x6b, 0xb3, 0xe0, 
  0x25, 0x9b, 0x5f, 0xa9, 0x03, 0x29, 0x2f, 0x64, 
  0x70, 0x38, 0x3b, 0xa8, 0x2e
};

bool_t
xdr_gnumbers_t (XDR *xdrs, gnumbers_t *gp)
{
  return (xdr_long (xdrs, &gp->g_assets) &&
          xdr_long (xdrs, &gp->g_liabilities));
}

#define MAX_NAME_LEN 128
bool_t
xdr_pgn_t (XDR *xdrs, pgn_t *pp)
{
  return (xdr_string (xdrs, (char **)&pp->name, MAX_NAME_LEN) &&
          xdr_reference (xdrs, (char **)&pp->gnp, 
                         sizeof(gnumbers_t), (xdrproc_t)xdr_gnumbers_t));
}

const char * NAMES_DATA[TEST_DATA_SZ] = 
{
  "Alex Andrews",
  "Betty Boop",
  "Charlie Chan",
  "David Dinkins",
  "Elliot Edwards",
  "Felicia Franks",
  "Gordon Gecko",
  "Howard Hughes",
  "Ian Ingles",
  "Janis Joplin",
  "Karen Kurtz",
  "Lana Lang",
  "Mark Michaels",
  "Nora Norman",
  "Oliver Oldman",
  "Patrick Pullman",
  "Quinten Quatrell",
  "Robin Rivers",
  "Steve Smith",
  "Terrence Tate"
};

void
init_pgn_contents (pgn_t *data, int cnt)
{
  if (data == NULL) return;

  memset (data, 0, sizeof(pgn_t));
  data->name = (char *) malloc (strlen (NAMES_DATA[cnt]) + 1);
  strcpy (data->name, NAMES_DATA[cnt]);
  data->gnp = (gnumbers_t *) malloc (sizeof(gnumbers_t));
  data->gnp->g_assets = LONG_DATA[cnt];
  data->gnp->g_liabilities = LONG_DATA[TEST_DATA_SZ - cnt - 1];
}
void
init_pgn (pgn_t **data, int cnt)
{
  *data = (pgn_t *) malloc (sizeof(pgn_t));
  init_pgn_contents (*data, cnt);
}
void
free_pgn_contents (pgn_t *data)
{
  if (!data) return;
  if (data->name) { free (data->name); data->name = NULL; }
  if (data->gnp)  { free (data->gnp); data->gnp = NULL; }
}
void
free_pgn (pgn_t **data)
{
  if (!data) return;
  free_pgn_contents (*data);
  free (*data);
  *data = NULL;
}

int
compare_pgn (pgn_t *a, pgn_t *b)
{
  int name_compare = 0;
  if (!a && !b) return 0; /* a == b */
  if (!a && b) return -1; /* a < b */
  if (a && !b) return 1;  /* a > b */
  if (!a->name && b->name) return -1; /* a < b */
  if (a->name && !b->name) return 1; /* a > b */
  if (a->name && b->name) name_compare = strcmp(a->name, b->name);

  if (name_compare != 0)
    return name_compare;

  if (!a->gnp && !b->gnp) return 0; /* a == b */
  if (!a->gnp && b->gnp) return -1; /* a < b */
  if (a->gnp && !b->gnp) return 1; /* a > b */
  
  if (a->gnp->g_assets < b->gnp->g_assets) return -1; /* a < b */
  if (a->gnp->g_assets > b->gnp->g_assets) return 1; /* a > b */

  /* negate sense of liabilities */
  if (a->gnp->g_liabilities < b->gnp->g_liabilities) return 1;
  if (a->gnp->g_liabilities > b->gnp->g_liabilities) return -1;
  return 0;
}

void
print_pgn (FILE * f, pgn_t *a)
{
  if (!a) { fputs ("<NULL>",f); return; }
  fprintf (f, "name='%s'\t", (a->name ? (*a->name ? a->name : "''") : "<NULL>"));
  if (!a->gnp)
    fputs ("gnp: <NULL>",f);
  else
    {
      fprintf (f, "gnp->g_assets=%d\tgnp->g_liabilities=%d",
               a->gnp->g_assets, a->gnp->g_liabilities);
    }
}

/* List serialization implementation adapted from
 * http://publib.boulder.ibm.com/infocenter/systems/index.jsp?\
 *      topic=/com.ibm.aix.progcomm/doc/progcomc/xdr_passlink_ex.htm
 * The recursive implementations are easier to understand, and
 * allow to exercise the xdr_pointer function. However, for lists
 * of more than a dozen or so elements, the non-recursive implementation
 * is preferred, to avoid exhausting the stack.
 */

/* xdr_pgn_node_t_RECURSIVE
 * DECODE: populate a node, where the storage already
 *         exists and the data is known to be present
 *         in the xdr stream.
 * ENCODE: serialize a node that actually exists to
 *         the xdr stream.
 * FREE:   handled by the xdr_pointer (and, internally,
 *         the xdr_reference and xdr_string routines
 *         employed by xdr_pgn_t).
 */
bool_t
xdr_pgn_node_t_RECURSIVE (XDR * xdrs, pgn_node_t *pgn_node)
{
  return (xdr_pgn_t (xdrs, &pgn_node->pgn) &&
          xdr_pgn_list_t_RECURSIVE (xdrs, &pgn_node->pgn_next));
}
bool_t
xdr_pgn_list_t_RECURSIVE (XDR *xdrs, pgn_list_t *pgn_list)
{
  return (xdr_pointer (xdrs, (char **)pgn_list, sizeof(pgn_node_t),
                       (xdrproc_t)xdr_pgn_node_t_RECURSIVE));
}

/* DECODE: implemented as an implicit union. Check for
 *         a bool value on the xdr stream. If TRUE,
 *         then decode a pgn_node_t object (allocating
 *         memory via xdr_pointer). If FALSE, then
 *         set pointer value to NULL. Loops until 
 *         curr->gn_next is NULL. Non-recursive impl.
 * ENCODE: implemented as an implicit union, where
 *         if *pgn_list is NULL, then serialize a
 *         bool false value. If it is non-null, then
 *         serialize a bool true value AND the pgn_node_t
 *         that pgn_list points to. Non-recursive
 *         implementation.
 */
bool_t
xdr_pgn_list_t (XDR *xdrs, pgn_list_t *pgn_list)
{
  bool_t more_data;
  pgn_list_t *nextp;
  for (;;)
    {
      more_data = (*pgn_list != NULL);
      if (!xdr_bool (xdrs, &more_data))
        return FALSE;

      if (!more_data)
        break;

      if (xdrs->x_op == XDR_FREE)
        nextp = &(*pgn_list)->pgn_next;

      if (!xdr_reference (xdrs, 
                          (char **)pgn_list,     /* pgn_list is ptr to current node */
                          sizeof (pgn_node_t),   /* allocate for data + nextptr */
                          (xdrproc_t)xdr_pgn_t)) /* but only xdr the data here */
        return FALSE;

      pgn_list = (xdrs->x_op == XDR_FREE) ? nextp : &(*pgn_list)->pgn_next;
    }
  *pgn_list = NULL;
  return TRUE;
}

void
init_pgn_node (pgn_node_t **pgn_node, int cnt)
{
  *pgn_node = (pgn_node_t *) malloc (sizeof (pgn_node_t));
  memset (*pgn_node, 0, sizeof (pgn_node_t));
  init_pgn_contents (&(*pgn_node)->pgn, cnt);
  (*pgn_node)->pgn_next = NULL;
}

void
init_pgn_list (pgn_list_t *pgn_list)
{
  int cnt;
  pgn_node_t *newp;
  
  for (cnt = 0; cnt < TEST_DATA_SZ; cnt++)
    {
      init_pgn_node (&newp, cnt);
      *pgn_list = newp;
      pgn_list = &(newp->pgn_next);
    }
}

void
free_pgn_node (pgn_node_t *pgn_node)
{
  if (!pgn_node) return;
  free_pgn_node (pgn_node->pgn_next);
  pgn_node->pgn_next = NULL;
  free_pgn_contents (&(pgn_node->pgn));
}

void
free_pgn_list (pgn_list_t *pgn_list)
{
  if (!pgn_list) return;
  free_pgn_node (*pgn_list);
  *pgn_list = NULL;  
}

void
print_pgn_list (FILE * f, pgn_list_t *pgn_list)
{
  pgn_node_t *curr;
  int cnt = 0;

  if (!pgn_list)
  {
    fprintf (f, "<NULL>");
    return;
  }
  curr = *pgn_list;
  while (curr)
    {
      fprintf (f, "Entry #%d: ", cnt);
      print_pgn (f, &(curr->pgn));
      fprintf (f, "\n", cnt++);
      curr = curr->pgn_next;
    }
}

bool_t
xdr_primitive_struct_t (XDR *xdrs, test_struct_of_primitives_t *ps)
{
  return (
      xdr_int            (xdrs, &(ps->a)) &&
      xdr_u_int          (xdrs, &(ps->b)) &&
      xdr_long           (xdrs, &(ps->c)) &&
      xdr_u_long         (xdrs, &(ps->d)) &&
      xdr_short          (xdrs, &(ps->e)) &&
      xdr_u_short        (xdrs, &(ps->f)) &&
      xdr_char           (xdrs, &(ps->g)) &&
      xdr_u_char         (xdrs, &(ps->h)) &&
      xdr_int8_t         (xdrs, &(ps->i)) &&
      xdr_u_int8_t       (xdrs, &(ps->j)) &&
      xdr_uint8_t        (xdrs, &(ps->k)) &&
      xdr_int16_t        (xdrs, &(ps->l)) &&
      xdr_u_int16_t      (xdrs, &(ps->m)) &&
      xdr_uint16_t       (xdrs, &(ps->n)) &&
      xdr_int32_t        (xdrs, &(ps->o)) &&
      xdr_u_int32_t      (xdrs, &(ps->p)) &&
      xdr_uint32_t       (xdrs, &(ps->q)) &&
      xdr_int64_t        (xdrs, &(ps->r)) &&
      xdr_u_int64_t      (xdrs, &(ps->s)) &&
      xdr_uint64_t       (xdrs, &(ps->t)) &&
      xdr_hyper          (xdrs, &(ps->X_hyper))   &&
      xdr_u_hyper        (xdrs, &(ps->X_u_hyper)) &&
      xdr_longlong_t     (xdrs, &(ps->X_ll))      &&
      xdr_u_longlong_t   (xdrs, &(ps->X_ull))     &&
      xdr_float          (xdrs, &(ps->u)) &&
      xdr_double         (xdrs, &(ps->v)) &&
      xdr_bool           (xdrs, &(ps->w)) &&
      xdr_enum           (xdrs, &(ps->x)));
}

void
init_primitive_struct (test_struct_of_primitives_t *ps)
{
  ps->a =    INT_DATA [ 4]; 
  ps->b =   UINT_DATA [ 5]; 
  ps->c =   LONG_DATA [ 6]; 
  ps->d =  ULONG_DATA [ 7]; 
  ps->e =  SHORT_DATA [ 8]; 
  ps->f = USHORT_DATA [ 9];
#if CHAR_MIN < 0
  ps->g =  SCHAR_DATA [10]; 
#else
  ps->g =  UCHAR_DATA [10]; 
#endif
  ps->h =  UCHAR_DATA [11]; 
  ps->i =   INT8_DATA [12]; 
  ps->j =  UINT8_DATA [13]; 
  ps->k =  UINT8_DATA [14]; 
  ps->l =  INT16_DATA [15]; 
  ps->m = UINT16_DATA [16]; 
  ps->n = UINT16_DATA [17]; 
  ps->o =  INT32_DATA [18]; 
  ps->p = UINT32_DATA [19]; 
  ps->q = UINT32_DATA [ 4]; 
  ps->r =  INT64_DATA [ 5]; 
  ps->s = UINT64_DATA [ 6]; 
  ps->t = UINT64_DATA [ 7]; 
  ps->X_hyper   =      HYPER_DATA [ 8];
  ps->X_u_hyper =     UHYPER_DATA [ 9];
  ps->X_ll      =   LONGLONG_DATA [10];
  ps->X_ull     =  ULONGLONG_DATA [11];
  ps->u = FLOAT_DATA  [12]; 
  ps->v = DOUBLE_DATA [13]; 
  ps->w = BOOL_DATA   [14]; 
  ps->x = ENUM_DATA   [15]; 
}

int
compare_primitive_structs (test_struct_of_primitives_t *lhs,
                           test_struct_of_primitives_t *rhs)
{
  static int v[28];
  int rVal = 0;
  int i;
  v[ 0] = ((lhs->a == rhs->a) ? 0 : ((lhs->a < rhs->a) ? -1 : 1));
  v[ 1] = ((lhs->b == rhs->b) ? 0 : ((lhs->b < rhs->b) ? -1 : 1));
  v[ 2] = ((lhs->c == rhs->c) ? 0 : ((lhs->c < rhs->c) ? -1 : 1));
  v[ 3] = ((lhs->d == rhs->d) ? 0 : ((lhs->d < rhs->d) ? -1 : 1));
  v[ 4] = ((lhs->e == rhs->e) ? 0 : ((lhs->e < rhs->e) ? -1 : 1));
  v[ 5] = ((lhs->f == rhs->f) ? 0 : ((lhs->f < rhs->f) ? -1 : 1));
  v[ 6] = ((lhs->g == rhs->g) ? 0 : ((lhs->g < rhs->g) ? -1 : 1));
  v[ 7] = ((lhs->h == rhs->h) ? 0 : ((lhs->h < rhs->h) ? -1 : 1));
  v[ 8] = ((lhs->i == rhs->i) ? 0 : ((lhs->i < rhs->i) ? -1 : 1));
  v[ 9] = ((lhs->j == rhs->j) ? 0 : ((lhs->j < rhs->j) ? -1 : 1));
  v[10] = ((lhs->k == rhs->k) ? 0 : ((lhs->k < rhs->k) ? -1 : 1));
  v[11] = ((lhs->l == rhs->l) ? 0 : ((lhs->l < rhs->l) ? -1 : 1));
  v[12] = ((lhs->m == rhs->m) ? 0 : ((lhs->m < rhs->m) ? -1 : 1));
  v[13] = ((lhs->n == rhs->n) ? 0 : ((lhs->n < rhs->n) ? -1 : 1));
  v[14] = ((lhs->o == rhs->o) ? 0 : ((lhs->o < rhs->o) ? -1 : 1));
  v[15] = ((lhs->p == rhs->p) ? 0 : ((lhs->p < rhs->p) ? -1 : 1));
  v[16] = ((lhs->q == rhs->q) ? 0 : ((lhs->q < rhs->q) ? -1 : 1));
  v[17] = ((lhs->r == rhs->r) ? 0 : ((lhs->r < rhs->r) ? -1 : 1));
  v[18] = ((lhs->s == rhs->s) ? 0 : ((lhs->s < rhs->s) ? -1 : 1));
  v[19] = ((lhs->t == rhs->t) ? 0 : ((lhs->t < rhs->t) ? -1 : 1));
  v[20] = ((lhs->X_hyper   == rhs->X_hyper)   ? 0 : ((lhs->X_hyper   < rhs->X_hyper)   ? -1 : 1));
  v[21] = ((lhs->X_u_hyper == rhs->X_u_hyper) ? 0 : ((lhs->X_u_hyper < rhs->X_u_hyper) ? -1 : 1));
  v[22] = ((lhs->X_ll      == rhs->X_ll)      ? 0 : ((lhs->X_ll      < rhs->X_ll)      ? -1 : 1));
  v[23] = ((lhs->X_ull     == rhs->X_ull)     ? 0 : ((lhs->X_ull     < rhs->X_ull)     ? -1 : 1));
  v[24] = ((lhs->u == rhs->u) ? 0 : ((lhs->u < rhs->u) ? -1 : 1));
  v[25] = ((lhs->v == rhs->v) ? 0 : ((lhs->v < rhs->v) ? -1 : 1));
  v[26] = ((lhs->w == rhs->w) ? 0 : ((lhs->w < rhs->w) ? -1 : 1));
  v[27] = ((lhs->x == rhs->x) ? 0 : ((lhs->x < rhs->x) ? -1 : 1));

  for (i = 0; i < 28; i++)
    if (v[i] != 0)
      {
        rVal = v[i];
        break;
      }
  return rVal;
}

void
print_primitive_struct (FILE *f, test_struct_of_primitives_t *ps)
{
  if (!ps) { fputs ("<NULL>",f); return; }
  fprintf (f, "int:            %d\n",          ps->a);
  fprintf (f, "u_int:          %u\n",          ps->b);
  fprintf (f, "long:           %ld\n",         ps->c);
  fprintf (f, "u_long:         %lu\n",         ps->d);
  fprintf (f, "short:          %hd\n",         ps->e);
  fprintf (f, "u_short:        %hu\n",         ps->f);
#if CHAR_MIN < 0
  fprintf (f, "char:           %hhd\n",        ps->g);
#else
  fprintf (f, "char:           %hhu\n",        ps->g);
#endif
  fprintf (f, "u_char:         %hhu\n",        ps->h);
  fprintf (f, "int8_t:         %" PRId8 "\n",  ps->i);
  fprintf (f, "u_int8_t:       %" PRIu8 "\n",  ps->j);
  fprintf (f, "uint8_t:        %" PRIu8 "\n",  ps->k);
  fprintf (f, "int16_t:        %" PRId16 "\n", ps->l);
  fprintf (f, "u_int16_t:      %" PRIu16 "\n", ps->m);
  fprintf (f, "uint16_t:       %" PRIu16 "\n", ps->n);
  fprintf (f, "int32_t:        %" PRId32 "\n", ps->o);
  fprintf (f, "u_int32_t:      %" PRIu32 "\n", ps->p);
  fprintf (f, "uint32_t:       %" PRIu32 "\n", ps->q);
  fprintf (f, "int64_t:        %" PRId64 "\n", ps->r);
  fprintf (f, "u_int64_t:      %" PRIu64 "\n", ps->s);
  fprintf (f, "uint64_t:       %" PRIu64 "\n", ps->t);
  fprintf (f, "hyper:          %" PRId64 "\n", ps->X_hyper);
  fprintf (f, "u_hyper:        %" PRIu64 "\n", ps->X_u_hyper);
  fprintf (f, "longlong_t:     %" PRId64 "\n", ps->X_ll);
  fprintf (f, "u_longlong_t:   %" PRIu64 "\n", ps->X_ull);
  fprintf (f, "float:          %.8e\n",        ps->u);
  fprintf (f, "double:         %.15e\n",       ps->v);
  fprintf (f, "bool:           %d\n",          ps->w);
  fprintf (f, "enum:           %d\n",          ps->x);
}


#define TEST_BASIC_TYPE_CORE_FUNCTION_DEF( FUNC, TYPE, TYPEFMT ) \
bool_t                                         \
test_basic_type_core_##FUNC (log_opts * log,   \
  const char     *testid,                      \
  TYPE           *input_data,                  \
  u_int           data_cnt,                    \
  xdr_stream_ops *stream_ops,                  \
  bool_t          check_too_much,              \
  void           *xdr_data)                    \
{                                              \
  XDR xdr_enc;                                 \
  XDR xdr_dec;                                 \
  int cnt;                                     \
  TYPE val;                                    \
  bool_t pass = TRUE;                          \
                                               \
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);    \
  (*(stream_ops->init_test_cb))(XDR_ENCODE, xdr_data);              \
                                                                    \
  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);       \
  for (cnt = 0; cnt < data_cnt; cnt++)                              \
    {                                                               \
      log_msg (log, XDR_LOG_DEBUG2,                                 \
               "%s: about to encode (cnt=%d, val=%"                 \
               TYPEFMT ")\n", testid, cnt, input_data[cnt]);        \
      if (!FUNC (&xdr_enc, (TYPE*)&input_data[cnt]))                \
        {                                                           \
          log_msg (log, XDR_LOG_INFO,                               \
                   "%s: failed " #FUNC " XDR_ENCODE (cnt=%d, val=%" \
                   TYPEFMT ")\n", testid, cnt, input_data[cnt]);    \
          pass = FALSE;                                             \
          goto test_##FUNC##_end;                                   \
        }                                                           \
    }                                                               \
\
  if (check_too_much)                                               \
    {                                                               \
      if (FUNC (&xdr_enc, (TYPE*)&input_data[0]))                   \
        {                                                           \
          log_msg (log, XDR_LOG_INFO,                               \
                   "%s: unexpected pass " #FUNC " XDR_ENCODE\n",    \
                    testid);                                        \
          pass = FALSE;                                             \
          goto test_##FUNC##_end;                                   \
        }                                                           \
    }                                                               \
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);       \
\
  (*(stream_ops->debug_cb))(xdr_data);                              \
\
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);       \
  for (cnt = 0; cnt < data_cnt; cnt++)                              \
    {                                                               \
      log_msg (log, XDR_LOG_DEBUG2,                                 \
               "%s: about to decode (cnt=%d, exp=%"                 \
               TYPEFMT ")\n", testid, cnt, input_data[cnt]);        \
      if (!FUNC (&xdr_dec, &val) ||                                 \
          (val != input_data[cnt] && !isnan ((double)val)))         \
        {                                                           \
          log_msg (log, XDR_LOG_INFO,                               \
                   "%s: failed " #FUNC " XDR_DECODE (cnt=%d, exp=%" \
                   TYPEFMT ", val=%" TYPEFMT ")\n",                 \
                   testid, cnt, input_data[cnt], val);              \
          pass = FALSE;                                             \
          goto test_##FUNC##_end2;                                  \
        }                                                           \
    }                                                               \
\
  if (FUNC (&xdr_dec, &val))                                        \
    {                                                               \
      log_msg (log, XDR_LOG_INFO,                                   \
               "%s: unexpected pass " #FUNC " XDR_DECODE\n",        \
               testid);                                             \
      pass = FALSE;                                                 \
      goto test_##FUNC##_end2;                                      \
    }                                                               \
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);       \
\
test_##FUNC##_end2:                                                 \
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);       \
test_##FUNC##_end:                                                  \
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);       \
  (*(stream_ops->fini_test_cb))(xdr_data);                          \
  if (pass == TRUE)                                                 \
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);            \
  else                                                              \
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);            \
\
  return pass;                                                      \
}

TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int, int, "d")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int, u_int, "u")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_long, long, "ld")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_long, unsigned long, "lu")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_short, short, "hd")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_short, unsigned short, "hu")
#if CHAR_MIN < 0
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_char, char, "hhd")
#else
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_char, char, "hhu")
#endif
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_char, u_char, "hhu")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int8_t, int8_t, PRId8)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int8_t, u_int8_t, PRIu8)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint8_t, u_int8_t, PRIu8)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int16_t, int16_t, PRId16)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int16_t, u_int16_t, PRIu16)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint16_t, u_int16_t, PRIu16)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int32_t, int32_t, PRId32)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int32_t, u_int32_t, PRIu32)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint32_t, u_int32_t, PRIu32)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int64_t, int64_t, PRId64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int64_t, u_int64_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint64_t, u_int64_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_hyper, quad_t, PRId64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_hyper, u_quad_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_longlong_t, quad_t, PRId64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_longlong_t, u_quad_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_float, float, ".8e")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_double, double, ".15e")
/* TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_quadruple, long double, "%Lf") */
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_bool, bool_t, "d")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_enum, enum_t, "d")

