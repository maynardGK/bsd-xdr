#ifndef _XDR_TEST_COMMON_H
#define _XDR_TEST_COMMON_H

#include <stdio.h>
#include <rpc/types.h>

#ifdef __cplusplus
# define XDR_TEST_DECLS_BEGIN extern "C" {
# define XDR_TEST_DECLS_END   }
#else
# define XDR_TEST_DECLS_BEGIN
# define XDR_TEST_DECLS_END
#endif

#ifndef DIR_SEPARATOR
# define DIR_SEPARATOR '/'
# define PATH_SEPARATOR ':'
#endif
#if defined (_WIN32) || defined (__MSDOS__) || defined (__DJGPP__) || defined (__OS2__)
# define HAVE_DOS_BASED_FILE_SYSTEM
# define FOPEN_WB "wb"
# ifndef DIR_SEPARATOR_2
#  define DIR_SEPARATOR_2 '\\'
# endif
# ifndef PATH_SEPARATOR_2
#  define PATH_SEPARATOR_2 ';'
# endif
#endif

#ifndef DIR_SEPARATOR_2
# define IS_DIR_SEPARATOR(ch) ((ch) == DIR_SEPARATOR)
#else
# define IS_DIR_SEPARATOR(ch) \
        (((ch) == DIR_SEPARATOR) || ((ch) == DIR_SEPARATOR_2))
#endif

#ifndef PATH_SEPARATOR_2
# define IS_PATH_SEPARATOR(ch) ((ch) == PATH_SEPARATOR)
#else
# define IS_PATH_SEPARATOR(ch) ((ch) == PATH_SEPARATOR_2)
#endif

#ifdef __CYGWIN__
# define FOPEN_WB "wb"
#endif

#ifndef FOPEN_WB
# define FOPEN_WB "w"
#endif
#ifndef _O_BINARY
# define _O_BINARY 0
#endif

#if defined(__MINGW32__) || defined(_MSC_VER)
# include <getopt.h>
/* getopt.h doesn't define this because stdlib.h is supposed to.
   But doesn't, on mingw or msvc */
XDR_TEST_DECLS_BEGIN
int getopt (int argc, char *const *argv, const char *shortopts);
XDR_TEST_DECLS_END
#endif

#include <stdint.h>
#include <inttypes.h>

#define XDR_LOG_SILENT     -1
#define XDR_LOG_NORMAL      0
#define XDR_LOG_INFO        1
#define XDR_LOG_DETAIL      2
#define XDR_LOG_DEBUG       3
#define XDR_LOG_DEBUG2      4

XDR_TEST_DECLS_BEGIN

extern const char *program_name;
void set_program_name (const char *argv0);
const char *base_name (const char *s);
void dumpmem (FILE * f, void *buf, unsigned int len, unsigned int offset);

typedef struct _log_opts
{
  int level;
  FILE *f;
} log_opts;
void log_msg (log_opts * o, int level, const char *fmt, ...);

/*********************************************************/
/* definitions for callbacks to manage/debug XDR streams */
/*********************************************************/

/* used to initialize an XDR * from the data given in
 * the void*. For instance, an xdrmem implementation 
 * would use a supplied buf and bufsz to call
 * xdrmem_create(). An xdrstdio implementation would
 * open a specified file, then call xdrstdio_create().
 * Should not be called twice on the same XDR * and/or
 * void*, unless xdr_finish_cb is called in between
 * the two occurances.
 */
typedef bool_t (*xdr_create_cb)(XDR *, enum xdr_op, void * /*userdata*/);

/* used to finalize an XDR * created from the data
 * given in the void*. For instance, an xdrstdio
 * implementation might close the specified file.
 * This operation should be callable multiple times
 * on the same object, without harm. Should also
 * call XDR_DESTROY() -- which is not callable in 
 * this way, so the xdr_finish_cb should use guard
 * variables in the void*.
 */
typedef bool_t (*xdr_finish_cb)(XDR *, enum xdr_op, void * /*userdata*/);

/* used after an encoding phase to produce debug
 * output. For instance, an xdrmem implementation
 * might produce a hexdump of the the buf in void*.
 */
typedef void   (*xdr_debug_enc)(void * /*userdata*/);

typedef struct _xdr_stream_ops {
  xdr_create_cb create_cb;
  xdr_finish_cb finish_cb;
  xdr_debug_enc debug_cb;
} xdr_stream_ops;

/* placeholder - this should be moved to xdrstdio_test.c */
typedef struct _xdrstdio_creation_data {
  log_opts *log;
  int      finish_guard;
  char     *name;
  char     *fopen_mode;
  FILE     *f;
} xdrstdio_creation_data;

#define TEST_BASIC_TYPE_CORE_FUNCTION_DECL( FUNC, TYPE ) \
bool_t                                         \
test_basic_type_core_##FUNC (log_opts * log,   \
  const char     *testid,                      \
  TYPE           *input_data,                  \
  u_int           data_cnt,                    \
  xdr_stream_ops *stream_ops,                  \
  void           *xdr_data)

TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int, int);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int, u_int);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_long, long);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_long, unsigned long);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_short, short);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_short, unsigned short);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_char, char);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_char, u_char);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int8_t, int8_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int8_t, u_int8_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint8_t, u_int8_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int16_t, int16_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int16_t, u_int16_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint16_t, u_int16_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int32_t, int32_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int32_t, u_int32_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint32_t, u_int32_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int64_t, int64_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int64_t, u_int64_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint64_t, u_int64_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_hyper, quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_hyper, u_quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_longlong_t, quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_longlong_t, u_quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_float, float);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_double, double);
/* TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_quadruple, long double) */
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_bool, bool_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_enum, enum_t);


#define TEST_DATA_SZ 20
extern const int               INT_DATA[TEST_DATA_SZ];
extern const unsigned int     UINT_DATA[TEST_DATA_SZ];
extern const long             LONG_DATA[TEST_DATA_SZ];
extern const unsigned long   ULONG_DATA[TEST_DATA_SZ];
extern const short           SHORT_DATA[TEST_DATA_SZ];
extern const unsigned short USHORT_DATA[TEST_DATA_SZ];
extern const signed char     SCHAR_DATA[TEST_DATA_SZ];
extern const unsigned char   UCHAR_DATA[TEST_DATA_SZ];
extern const int8_t           INT8_DATA[TEST_DATA_SZ];
extern const u_int8_t        UINT8_DATA[TEST_DATA_SZ];
extern const int16_t         INT16_DATA[TEST_DATA_SZ];
extern const u_int16_t      UINT16_DATA[TEST_DATA_SZ];
extern const int32_t         INT32_DATA[TEST_DATA_SZ];
extern const u_int32_t      UINT32_DATA[TEST_DATA_SZ];
extern const int64_t         INT64_DATA[TEST_DATA_SZ];
extern const u_int64_t      UINT64_DATA[TEST_DATA_SZ];
extern const quad_t          HYPER_DATA[TEST_DATA_SZ];
extern const u_quad_t       UHYPER_DATA[TEST_DATA_SZ];
extern const quad_t       LONGLONG_DATA[TEST_DATA_SZ];
extern const u_quad_t    ULONGLONG_DATA[TEST_DATA_SZ];
extern const float           FLOAT_DATA[TEST_DATA_SZ];
extern const double         DOUBLE_DATA[TEST_DATA_SZ];
#define FLT_DATA_PINF_INDEX  5
#define FLT_DATA_NINF_INDEX  6
#define FLT_DATA_NAN_INDEX   7
#define FLT_DATA_NZERO_INDEX 8

extern void init_float_data (float *data);
extern void init_double_data (double *data);

extern const bool_t           BOOL_DATA[TEST_DATA_SZ];
typedef enum _test_enum {
  EN_A = 0,
  EN_B = 27,
  EN_C = -15,
  EN_D = 92
} test_enum_t;
extern const test_enum_t      ENUM_DATA[TEST_DATA_SZ];

typedef union _test_union {
  float     flt;
  double    dbl;
  u_int32_t u32;
  char      c;
  int64_t   i64;
} test_union_t;
typedef enum _test_union_enum {
  TEST_UNION_FLOAT = 0,
  TEST_UNION_DOUBLE,
  TEST_UNION_UI32,
  TEST_UNION_CHAR,
  TEST_UNION_I64
} test_union_enum_t;
typedef struct _test_discrim_union {
  test_union_enum_t type;
  test_union_t      value;
} test_discrim_union_t;
extern const struct xdr_discrim test_union_dscrim[6];

extern test_discrim_union_t UNION_DATA[TEST_DATA_SZ];
extern void init_union_data (test_discrim_union_t *data);
extern bool_t encode_union_data (const char *testid, log_opts *o, XDR *xdrs,
                                 int cnt, test_discrim_union_t *v);
extern bool_t decode_union_data (const char *testid, log_opts *o, XDR *xdrs,
                                 int cnt, test_discrim_union_t *v);
extern bool_t compare_union_data (const char *testid, log_opts *o,
                                  test_discrim_union_t *s,
                                  test_discrim_union_t *d);

#define OPAQUE_DATA_SZ 61
extern const char OPAQUE_DATA[OPAQUE_DATA_SZ];

/* struct definitions and helper functions for xdr_reference */
typedef struct gnumbers_ {
  long g_assets;
  long g_liabilities;
} gnumbers_t;
extern bool_t xdr_gnumbers_t (XDR *, gnumbers_t *);
typedef struct pgn_ {
  char       *name;
  gnumbers_t *gnp;
} pgn_t;
extern bool_t xdr_pgn_t (XDR *, pgn_t *);
extern void init_pgn_contents (pgn_t *, int);
extern void free_pgn_content (pgn_t *);
extern void init_pgn (pgn_t **, int);
extern void free_pgn (pgn_t **);
extern void print_pgn (FILE *, pgn_t *);
extern int compare_pgn (pgn_t *, pgn_t *);

extern const char * NAMES_DATA[TEST_DATA_SZ];

/* list definitions and helper functions for xdr_pointer */
struct _pgn_node_t;
typedef struct _pgn_node_t  pgn_node_t;
struct _pgn_node_t {
       pgn_t       pgn;
       pgn_node_t *pgn_next;
};
typedef pgn_node_t* pgn_list_t;
extern bool_t xdr_pgn_node_t_RECURSIVE (XDR *, pgn_node_t *);
extern bool_t xdr_pgn_list_t_RECURSIVE (XDR *, pgn_list_t *);
extern bool_t xdr_pgn_list_t (XDR *, pgn_list_t *);
void init_pgn_node (pgn_node_t **pgn_node, int cnt);
void init_pgn_list (pgn_list_t *pgn_list);
void free_pgn_node (pgn_node_t *pgn_node);
void free_pgn_list (pgn_list_t *pgn_list);
void print_pgn_list (FILE *, pgn_list_t *);

typedef struct _test_struct_of_primitives
{
  int            a;
  unsigned int   b;
  long           c;
  unsigned long  d;
  short          e;
  unsigned short f;
  char           g;
  unsigned char  h;
  int8_t         i;
  u_int8_t       j;
  uint8_t        k;
  int16_t        l;
  u_int16_t      m;
  uint16_t       n;
  int32_t        o;
  u_int32_t      p;
  uint32_t       q;
  int64_t        r;
  u_int64_t      s;
  uint64_t       t;
  quad_t         X_hyper;
  u_quad_t       X_u_hyper;
  quad_t         X_ll;
  u_quad_t       X_ull;
  float          u;
  double         v;
  bool_t         w;
  test_enum_t    x;
} test_struct_of_primitives_t;
bool_t xdr_primitive_struct_t (XDR *, test_struct_of_primitives_t *);
void init_primitive_struct (test_struct_of_primitives_t *);
int  compare_primitive_structs (test_struct_of_primitives_t *,
                                test_struct_of_primitives_t *);
void print_primitive_struct (test_struct_of_primitives_t *);

XDR_TEST_DECLS_END
#endif /* _XDR_TEST_COMMON_H */
