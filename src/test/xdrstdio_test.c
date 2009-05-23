#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <float.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "test_common.h"

const char *program_name;

void
usage (FILE * f, const char *progname)
{
  fprintf (f, "%s [-h] [-v|-s] [-k]\n", progname);
  fputs ("  Tests the stdio functionality of the xdr library\n", f);
  fputs ("  on this specific platform. Does not test data interchange\n", f);
  fputs ("  between different platforms.\n", f);
  fputs ("   -h       : print this help\n", f);
  fputs ("   -v       : verbose mode (may be repeated)\n", f);
  fputs ("   -s       : silent mode\n", f);
  fputs ("   -k       : keep temporary files\n", f);
  fputs ("   -t <DIR> : use temporary dir\n", f);
}

typedef struct _opts
{
  log_opts *log;
  int       keep;
  char     *tmpdir;
} opts;

bool_t test_xdrstdio_int (opts *o);
bool_t test_xdrstdio_u_int (opts *o);
bool_t test_xdrstdio_long (opts *o);
bool_t test_xdrstdio_u_long (opts *o);
bool_t test_xdrstdio_short (opts *o);
bool_t test_xdrstdio_u_short (opts *o);
bool_t test_xdrstdio_char (opts *o);
bool_t test_xdrstdio_u_char (opts *o);
bool_t test_xdrstdio_int8_t (opts *o);
bool_t test_xdrstdio_u_int8_t (opts *o);
bool_t test_xdrstdio_uint8_t (opts *o);
bool_t test_xdrstdio_int16_t (opts *o);
bool_t test_xdrstdio_u_int16_t (opts *o);
bool_t test_xdrstdio_uint16_t (opts *o);
bool_t test_xdrstdio_int32_t (opts *o);
bool_t test_xdrstdio_u_int32_t (opts *o);
bool_t test_xdrstdio_uint32_t (opts *o);
bool_t test_xdrstdio_int64_t (opts *o);
bool_t test_xdrstdio_u_int64_t (opts *o);
bool_t test_xdrstdio_uint64_t (opts *o);
bool_t test_xdrstdio_hyper (opts *o);
bool_t test_xdrstdio_u_hyper (opts *o);
bool_t test_xdrstdio_longlong_t (opts *o);
bool_t test_xdrstdio_u_longlong_t (opts *o);
bool_t test_xdrstdio_float (opts *o);
bool_t test_xdrstdio_double (opts *o);
bool_t test_xdrstdio_bool (opts *o);
bool_t test_xdrstdio_enum (opts *o);
bool_t test_xdrstdio_union (opts *o);
bool_t test_xdrstdio_opaque (opts *o);
bool_t test_xdrstdio_bytes (opts *o);
bool_t test_xdrstdio_string (opts *o);
bool_t test_xdrstdio_wrapstring (opts *o);
bool_t test_xdrstdio_array (opts *o);
bool_t test_xdrstdio_vector (opts *o);
bool_t test_xdrstdio_reference (opts *o);
bool_t test_xdrstdio_pointer (opts *o);
bool_t test_xdrstdio_list (opts *o);
bool_t test_xdrstdio_primitive_struct (opts *o);

/* This is a xdr_create_cb callback function and data struct */
typedef struct _xdrstdio_creation_data {
  opts  *o;
  int    finish_guard;
  char  *name;
  char  *fullname;
  FILE  *f;
} xdrstdio_creation_data;

bool_t
xdrstdio_init_test_cb (enum xdr_op op, void * data)
{
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;

  if (xdrstdio_data->fullname)
    {
      free (xdrstdio_data->fullname);
      xdrstdio_data->fullname = NULL;
    }

  xdrstdio_data->fullname = (char *) malloc 
      (strlen (xdrstdio_data->name) +
       strlen (xdrstdio_data->o->tmpdir) + 2);
  strcpy (xdrstdio_data->fullname, xdrstdio_data->o->tmpdir);
  strcat (xdrstdio_data->fullname, "/");
  strcat (xdrstdio_data->fullname, xdrstdio_data->name);
  return TRUE;
}

bool_t
xdrstdio_create_cb (XDR *xdrs, enum xdr_op op, void * data)
{
  bool_t rVal = TRUE;
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;

  switch (op)
    {
      case XDR_DECODE:
      case XDR_FREE:
        xdrstdio_data->f = fopen (xdrstdio_data->fullname, FOPEN_RB);
        break;
      case XDR_ENCODE:
        xdrstdio_data->f = fopen (xdrstdio_data->fullname, FOPEN_WB);
        break;
    }

  if (!xdrstdio_data->f)
    {
      log_msg (xdrstdio_data->o->log, XDR_LOG_SILENT,
               "could not open data file: %s\n", xdrstdio_data->fullname);
      rVal = FALSE;
    }
  else
    {
      xdrstdio_create (xdrs, xdrstdio_data->f, op);
      xdrstdio_data->finish_guard = 1;
    }
  return rVal;
}

bool_t
xdrstdio_finish_cb (XDR * xdrs, enum xdr_op op, void * data)
{
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;
  if (xdrstdio_data->finish_guard)
    {
      xdrstdio_data->finish_guard = 0;
      XDR_DESTROY (xdrs);
      fclose (xdrstdio_data->f);
    }
  return TRUE;
}

void
xdrstdio_debug_cb (void * data)
{
}

bool_t
xdrstdio_fini_test_cb (void * data)
{
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;
  if (!xdrstdio_data->o->keep)
    {
      if (unlink (xdrstdio_data->fullname) != 0)
        {
          log_msg (xdrstdio_data->o->log, XDR_LOG_SILENT,
                   "Could not delete %s\n", xdrstdio_data->fullname);
        }
    }
  free (xdrstdio_data->fullname);
  return TRUE;
}


static xdr_stream_ops xdrstdio_stream_ops =
{
  xdrstdio_init_test_cb,
  xdrstdio_create_cb,
  xdrstdio_finish_cb,
  xdrstdio_debug_cb,
  xdrstdio_fini_test_cb
};

void init_tmpdir (opts *o)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
  static const char * DEFAULT_TMPDIR = "C:/Temp/xdrstdio_test_XXXXXX";
  static const char * FALLBACK_TMPDIR = "C:/Temp";
#else
  static const char * DEFAULT_TMPDIR = "/tmp/xdrstdio_test_XXXXXX";
  static const char * FALLBACK_TMPDIR = "/tmp";
#endif
  char * p;
  
  if (o->tmpdir)
    {
      char * p = (char *) malloc (strlen(o->tmpdir));
      strcpy (p, o->tmpdir);
      if (!mkdir_p (p, 0700))
        {
          log_msg(o->log, XDR_LOG_SILENT,
                  "Couldn't create tmpdir %s; trying %s\n",
                  p, DEFAULT_TMPDIR);
          free (p);
          o->tmpdir = NULL;
        }
      else
        o->tmpdir = p;
        return;
    }

  o->tmpdir = (char *) malloc (strlen(DEFAULT_TMPDIR));
  strcpy (o->tmpdir, DEFAULT_TMPDIR);
  p = mkdtemp(o->tmpdir);
  if (!p)
    {
      log_msg(o->log, XDR_LOG_SILENT,
              "Couldn't create tmpdir; using %s\n", FALLBACK_TMPDIR);
      strcpy (o->tmpdir, FALLBACK_TMPDIR);
      if (!mkdir_p (o->tmpdir, 0700))
        {
          log_msg(o->log, XDR_LOG_SILENT,
                  "Couldn't even access %s; using '.'\n", FALLBACK_TMPDIR);
          strcpy (o->tmpdir, ".");
        }
    }
}

void cleanup_tmpdir (opts * o)
{
  if (!o->keep)
    {
      if (rmdir (o->tmpdir) != 0)
        {
          log_msg (o->log, XDR_LOG_SILENT,
                   "Could not remove directory: %s\n", o->tmpdir);
        }
    }
  else
    {
      log_msg (o->log, XDR_LOG_INFO,
               "Temporary files saved in %s\n", o->tmpdir);
    }
  free (o->tmpdir);
}

int
main (int argc, char *argv[])
{
  int c, i;
  log_opts log;
  opts o;
  bool_t rc = TRUE;

  set_program_name (argv[0]);
  log.level = 0;
  log.f = stderr;
  o.log = &log;
  o.keep = 0;
  o.tmpdir = NULL;

  while ((c = getopt (argc, argv, "hvskt:")) != -1)
    switch (c)
      {
      case 'h':
        usage (stdout, program_name);
        return 0;
        break;
      case 'v':
        o.log->level++;
        break;
      case 's':
        o.log->level = XDR_LOG_SILENT;
        break;
      case 'k':
        o.keep = 1;
        break;
      case 't':
        o.tmpdir = optarg;
        break;
      }

  init_tmpdir (&o);
  log_msg (o.log, XDR_LOG_INFO,
           "Using temp directory '%s'\n", o.tmpdir);

  rc &= test_xdrstdio_int (&o);
//  rc &= test_xdrstdio_u_int (&o);
//  rc &= test_xdrstdio_long (&o);
//  rc &= test_xdrstdio_u_long (&o);
//  rc &= test_xdrstdio_short (&o);
//  rc &= test_xdrstdio_u_short (&o);
//  rc &= test_xdrstdio_char (&o);
//  rc &= test_xdrstdio_u_char (&o);
//  rc &= test_xdrstdio_int8_t (&o);
//  rc &= test_xdrstdio_u_int8_t (&o);
//  rc &= test_xdrstdio_uint8_t (&o);
//  rc &= test_xdrstdio_int16_t (&o);
//  rc &= test_xdrstdio_u_int16_t (&o);
//  rc &= test_xdrstdio_uint16_t (&o);
//  rc &= test_xdrstdio_int32_t (&o);
//  rc &= test_xdrstdio_u_int32_t (&o);
//  rc &= test_xdrstdio_uint32_t (&o);
//  rc &= test_xdrstdio_int64_t (&o);
//  rc &= test_xdrstdio_u_int64_t (&o);
//  rc &= test_xdrstdio_uint64_t (&o);
//  rc &= test_xdrstdio_hyper (&o);
//  rc &= test_xdrstdio_u_hyper (&o);
//  rc &= test_xdrstdio_longlong_t (&o);
//  rc &= test_xdrstdio_u_longlong_t (&o);
//  rc &= test_xdrstdio_float (&o);
//  rc &= test_xdrstdio_double (&o);
//  rc &= test_xdrstdio_bool (&o);
//  rc &= test_xdrstdio_enum (&o);
//  rc &= test_xdrstdio_union (&o);
//  rc &= test_xdrstdio_opaque (&o);
//  rc &= test_xdrstdio_bytes (&o);
//  rc &= test_xdrstdio_string (&o);
//  rc &= test_xdrstdio_wrapstring (&o);
//  rc &= test_xdrstdio_array (&o);
//  rc &= test_xdrstdio_vector (&o);
//  rc &= test_xdrstdio_reference (&o);
//  rc &= test_xdrstdio_pointer (&o);
//  rc &= test_xdrstdio_list (&o);
//  rc &= test_xdrstdio_primitive_struct (&o);

  cleanup_tmpdir (&o);
  return (rc == TRUE ? EXIT_SUCCESS : EXIT_FAILURE);
}

bool_t
test_xdrstdio_int (opts * o)
{
  static const char *testid= "test_xdrstdio_int";
  int i;
  bool_t rv;
  char buf[80]; /* 20*min size */
  int data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_int";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT_DATA[i];
  rv = test_basic_type_core_xdr_int (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

// bool_t
// test_xdrstdio_u_int (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_int";
//   int i;
//   char buf[80]; /* 20*min size */
//   unsigned int data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT_DATA[i];
//   return test_basic_type_core_xdr_u_int (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_long (opts * o)
// {
//   static const char *testid= "test_xdrstdio_long";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   long data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = LONG_DATA[i];
//   return test_basic_type_core_xdr_long (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_long (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_long";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   unsigned long data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = ULONG_DATA[i];
//   return test_basic_type_core_xdr_u_long (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_short (opts * o)
// {
//   static const char *testid= "test_xdrstdio_short";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   short data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = SHORT_DATA[i];
//   return test_basic_type_core_xdr_short (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_short (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_short";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   unsigned short data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = USHORT_DATA[i];
//   return test_basic_type_core_xdr_u_short (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_char (opts * o)
// {
//   static const char *testid= "test_xdrstdio_char";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   char data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
// #if CHAR_MIN < 0
//   /* char is signed */
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = (char)SCHAR_DATA[i];
// #else
//     /* char is unsigned */
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = (char)UCHAR_DATA[i];
// #endif
//   return test_basic_type_core_xdr_char (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_char (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_char";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   unsigned char data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UCHAR_DATA[i];
//   return test_basic_type_core_xdr_u_char (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_int8_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_int8_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   int8_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT8_DATA[i];
//   return test_basic_type_core_xdr_char (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_int8_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_int8_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   u_int8_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT8_DATA[i];
//   return test_basic_type_core_xdr_u_int8_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_uint8_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_uint8_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   u_int8_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT8_DATA[i];
//   return test_basic_type_core_xdr_uint8_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_int16_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_int16_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   int16_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT16_DATA[i];
//   return test_basic_type_core_xdr_int16_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_int16_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_int16_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   u_int16_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT16_DATA[i];
//   return test_basic_type_core_xdr_u_int16_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_uint16_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_uint16_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   u_int16_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT16_DATA[i];
//   return test_basic_type_core_xdr_uint16_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_int32_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_int32_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   int32_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT32_DATA[i];
//   return test_basic_type_core_xdr_int32_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_int32_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_int32_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   u_int32_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT32_DATA[i];
//   return test_basic_type_core_xdr_u_int32_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_uint32_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_uint32_t";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   u_int32_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT32_DATA[i];
//   return test_basic_type_core_xdr_uint32_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_int64_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_int64_t";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   int64_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT64_DATA[i];
//   return test_basic_type_core_xdr_int64_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_int64_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_int64_t";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   u_int64_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT64_DATA[i];
//   return test_basic_type_core_xdr_u_int64_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_uint64_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_uint64_t";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   u_int64_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT64_DATA[i];
//   return test_basic_type_core_xdr_uint64_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_hyper (opts * o)
// {
//   static const char *testid= "test_xdrstdio_hyper";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   quad_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = HYPER_DATA[i];
//   return test_basic_type_core_xdr_hyper (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_hyper (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_hyper";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   u_quad_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = UHYPER_DATA[i];
//   return test_basic_type_core_xdr_u_hyper (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_longlong_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_longlong_t";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   quad_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = LONGLONG_DATA[i];
//   return test_basic_type_core_xdr_longlong_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_u_longlong_t (opts * o)
// {
//   static const char *testid= "test_xdrstdio_u_longlong_t";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   u_quad_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = ULONGLONG_DATA[i];
//   return test_basic_type_core_xdr_u_longlong_t (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_float (opts * o)
// {
//   static const char *testid= "test_xdrstdio_float";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*4 */
//   float data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   init_float_data (data);
//   return test_basic_type_core_xdr_float (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_double (opts * o)
// {
//   static const char *testid= "test_xdrstdio_double";
//   int i;
//   char buf[160]; /* TEST_DATA_SZ*8 */
//   double data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 160;
// 
//   init_double_data (data);
//   return test_basic_type_core_xdr_double (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_bool (opts * o)
// {
//   static const char *testid= "test_xdrstdio_bool";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   bool_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = BOOL_DATA[i];
//   return test_basic_type_core_xdr_bool (o->log, testid, data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_enum (opts * o)
// {
//   static const char *testid= "test_xdrstdio_enum";
//   int i;
//   char buf[80]; /* TEST_DATA_SZ*min size */
//   test_enum_t data[TEST_DATA_SZ];
//   xdrstdio_creation_data xdr_data;
//   xdr_data.o = o;
//   xdr_data.finish_guard = 0;
//   xdr_data.buf = &(buf[0]);
//   xdr_data.buf_sz = 80;
// 
//   for (i=0;i<TEST_DATA_SZ;i++) data[i] = ENUM_DATA[i];
//   return test_basic_type_core_xdr_enum (o->log, testid, (enum_t*) data,
//       TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
// }
// 
// bool_t
// test_xdrstdio_union (opts * o)
// {
//   static const char *testid= "test_xdrstdio_union";
//   /* size of a xdr'ed union depends on what branch 
//      is active. Therefore, the size of an array of
//      unions...is difficult to determine a priori.
//      Perhaps using a separate xdr_sizeof phase?
//      However, for this test data set we have exactly:
//   */
//   char buf[188];
//   int buf_sz = 188;
//   test_discrim_union_t data[TEST_DATA_SZ];
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   init_union_data (UNION_DATA);
//   init_union_data (data);
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
// 
//   for (cnt = 0; cnt < TEST_DATA_SZ && pass == TRUE; cnt++)
//     pass &= encode_union_data (testid, o->log, &xdr_enc, cnt, &data[cnt]);
//   if (pass != TRUE) goto test_xdrstdio_union_end;
// 
//   if (xdr_union (&xdr_enc, (enum_t *)&(data[0].type), 
//                  (char *)&(data[0].value), test_union_dscrim, NULL_xdrproc_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: unexpected pass xdr_union XDR_ENCODE\n", testid);
//       pass = FALSE;
//       goto test_xdrstdio_union_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
//  
//   for (cnt = 0; cnt < TEST_DATA_SZ; cnt++)
//     {
//       data[cnt].value.u32 = 0;
//       data[cnt].type = TEST_UNION_UI32;
//     }
// 
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);                \
//   for (cnt = 0; cnt < TEST_DATA_SZ && pass == TRUE; cnt++)
//       pass &= decode_union_data (testid, o->log, &xdr_dec, cnt, &data[cnt]);
//   if (pass != TRUE) goto test_xdrstdio_union_end;
// 
//   if (xdr_union (&xdr_dec, (enum_t *)&(data[0].type),
//                  (char *)&(data[0].value), test_union_dscrim, NULL_xdrproc_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: unexpected pass xdr_union XDR_DECODE\n", testid);
//       pass = FALSE;
//       goto test_xdrstdio_union_end2;
//     }
// 
//   pass = compare_union_data (testid, o->log, UNION_DATA, data);
// test_xdrstdio_union_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_union_end:
//   XDR_DESTROY (&xdr_enc);
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_opaque (opts * o)
// {
//   static const char *testid= "test_xdrstdio_opaque";
//   char buf[OPAQUE_DATA_SZ + (BYTES_PER_XDR_UNIT - (OPAQUE_DATA_SZ % BYTES_PER_XDR_UNIT))];
//   int buf_sz = OPAQUE_DATA_SZ + (BYTES_PER_XDR_UNIT - (OPAQUE_DATA_SZ % BYTES_PER_XDR_UNIT));
//   char data[OPAQUE_DATA_SZ];
//   int data_sz = OPAQUE_DATA_SZ;
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
//     data[cnt] = OPAQUE_DATA[cnt];
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
// 
//   if (!xdr_opaque (&xdr_enc, data, data_sz))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_opaque XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_opaque_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
//     data[cnt] = 0;
// 
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_opaque (&xdr_dec, data, data_sz))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_opaque XDR_DECODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_opaque_end2;
//     }
// 
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ && pass == TRUE; cnt++)
//     {
//       pass &= (data[cnt] == OPAQUE_DATA[cnt]);
//       if (pass != TRUE)
//         {
//           log_msg (o->log, XDR_LOG_INFO,
//                    "%s: failed xdr_opaque compare: (cnt=%d, exp=0x%02x, val=0x%02x)\n",
//                    testid, cnt, OPAQUE_DATA[cnt], data[cnt]);
//           pass = FALSE;
//           goto test_xdrstdio_opaque_end2;
//         }
//     }
// 
// test_xdrstdio_opaque_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_opaque_end:
//   XDR_DESTROY (&xdr_enc);
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_bytes (opts * o)
// {
//   static const char *testid= "test_xdrstdio_bytes";
// #define MAX_BYTES_SZ 128
//   char buf[MAX_BYTES_SZ + BYTES_PER_XDR_UNIT];
//   int buf_sz = MAX_BYTES_SZ + BYTES_PER_XDR_UNIT;
//   char *data = NULL;
//   int data_sz = OPAQUE_DATA_SZ;
//   char *p = NULL;
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   data = (char *)malloc(data_sz * sizeof(char));
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
//     data[cnt] = OPAQUE_DATA[cnt];
//   memset (buf, 0, buf_sz);
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   if (!xdr_bytes (&xdr_enc, (char **)&data, &data_sz, MAX_BYTES_SZ))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_bytes XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_bytes_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
//     data[cnt] = 0;
//   data_sz = 0;
// 
// 
//   /* decode into static buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_bytes (&xdr_dec, (char **)&data, &data_sz, MAX_BYTES_SZ))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_bytes XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_bytes_end2;
//     }
//   
//   /* check decoded size */
//   if (data_sz != OPAQUE_DATA_SZ)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_bytes XDR_DECODE (size: exp=%d, val=%d)\n",
//                testid, OPAQUE_DATA_SZ, data_sz);
//       pass = FALSE;
//       goto test_xdrstdio_bytes_end2;
//     }
// 
//   /* check decoded bytes */
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ && pass == TRUE; cnt++)
//     {
//       pass &= (data[cnt] == OPAQUE_DATA[cnt]);
//       if (pass != TRUE)
//         {
//           log_msg (o->log, XDR_LOG_INFO,
//                    "%s: failed xdr_bytes compare: (cnt=%d, exp=0x%02x, val=0x%02x)\n",
//                    testid, cnt, OPAQUE_DATA[cnt], data[cnt]);
//           pass = FALSE;
//           goto test_xdrstdio_bytes_end2;
//         }
//     }
//   XDR_DESTROY (&xdr_dec);
// 
//   /* decode into allocated buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_bytes (&xdr_dec, &p, &data_sz, MAX_BYTES_SZ))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_bytes (allocated) XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_bytes_end2;
//     }
//   
//   /* check decoded size */
//   if (data_sz != OPAQUE_DATA_SZ)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_bytes (allocated) XDR_DECODE (size: exp=%d, val=%d)\n",
//                testid, OPAQUE_DATA_SZ, data_sz);
//       pass = FALSE;
//       goto test_xdrstdio_bytes_end2;
//     }
// 
//   /* check decoded bytes */
//   for (cnt = 0; cnt < OPAQUE_DATA_SZ && pass == TRUE; cnt++)
//     {
//       pass &= (p[cnt] == OPAQUE_DATA[cnt]);
//       if (pass != TRUE)
//         {
//           log_msg (o->log, XDR_LOG_INFO,
//                    "%s: failed xdr_bytes (allocated) compare: (cnt=%d, exp=0x%02x, val=0x%02x)\n",
//                    testid, cnt, OPAQUE_DATA[cnt], p[cnt]);
//           pass = FALSE;
//           goto test_xdrstdio_bytes_end2;
//         }
//     }
// 
// test_xdrstdio_bytes_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_bytes_end:
//   XDR_DESTROY (&xdr_enc);
//   if (p)    { free (p); p = NULL; }
//   if (data) { free (data); data = NULL; }
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_string (opts * o)
// {
//   static const char *testid= "test_xdrstdio_string";
// #define MAX_STRING_SZ 128
//   char buf[MAX_STRING_SZ + BYTES_PER_XDR_UNIT];
//   int buf_sz = MAX_STRING_SZ + BYTES_PER_XDR_UNIT;
//   const char * STRING_DATA = "This is a test string. It is fairly short.";
//   const int STRING_DATA_SZ = strlen (STRING_DATA);
//   char *data = NULL;
//   char *p = NULL;
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   data = (char *)malloc((MAX_STRING_SZ + 1)* sizeof(char));
//   strncpy (data, STRING_DATA, MAX_STRING_SZ + 1);
//   memset (buf, 0, buf_sz);
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   if (!xdr_string (&xdr_enc, &data, MAX_STRING_SZ))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_string XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_string_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   for (cnt = 0; cnt < MAX_STRING_SZ + 1; cnt++)
//     data[cnt] = 0;
// 
// 
//   /* decode into static buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_string (&xdr_dec, &data, MAX_STRING_SZ))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_string XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_string_end2;
//     }
//   
//   /* check decoded string */
//   if (strncmp (data, STRING_DATA, MAX_STRING_SZ + 1) != 0)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_string compare: (exp='%s', val='%s')\n",
//                testid, STRING_DATA, data);
//       pass = FALSE;
//       goto test_xdrstdio_string_end2;
//     }
//   XDR_DESTROY (&xdr_dec);
// 
//   /* decode into allocated buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_string (&xdr_dec, &p, MAX_STRING_SZ))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_string (allocated) XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_string_end2;
//     }
//   
//   /* check decoded string */
//   if (strncmp (p, STRING_DATA, MAX_STRING_SZ + 1) != 0)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_string (allocated) compare: (exp='%s', val='%s')\n",
//                testid, STRING_DATA, p);
//       pass = FALSE;
//       goto test_xdrstdio_string_end2;
//     }
// 
// test_xdrstdio_string_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_string_end:
//   XDR_DESTROY (&xdr_enc);
//   if (p)    { free (p); p = NULL; }
//   if (data) { free (data); data = NULL; }
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_wrapstring (opts * o)
// {
//   static const char *testid= "test_xdrstdio_wrapstring";
// #define MAX_STRING_SZ 128
//   char buf[MAX_STRING_SZ + BYTES_PER_XDR_UNIT];
//   int buf_sz = MAX_STRING_SZ + BYTES_PER_XDR_UNIT;
//   const char * STRING_DATA = "This is a test string. It is fairly short.";
//   const int STRING_DATA_SZ = strlen (STRING_DATA);
//   char *data = NULL;
//   char *p = NULL;
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   data = (char *)malloc((MAX_STRING_SZ + 1)* sizeof(char));
//   strncpy (data, STRING_DATA, MAX_STRING_SZ + 1);
//   memset (buf, 0, buf_sz);
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   if (!xdr_wrapstring (&xdr_enc, &data))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_wrapstring XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_wrapstring_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   /* decode into allocated buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_wrapstring (&xdr_dec, &p))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_wrapstring (allocated) XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_wrapstring_end2;
//     }
//   
//   /* check decoded string */
//   if (strncmp (p, STRING_DATA, MAX_STRING_SZ + 1) != 0)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_wrapstring (allocated) compare: (exp='%s', val='%s')\n",
//                testid, STRING_DATA, p);
//       pass = FALSE;
//       goto test_xdrstdio_wrapstring_end2;
//     }
// 
// test_xdrstdio_wrapstring_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_wrapstring_end:
//   XDR_DESTROY (&xdr_enc);
//   if (p)    { free (p); p = NULL; }
//   if (data) { free (data); data = NULL; }
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_array (opts * o)
// {
//   static const char *testid= "test_xdrstdio_array";
//   /* use INT16_DATA as the array data source */
//   /* also test XDR_FREE filter */
//   char buf[BYTES_PER_XDR_UNIT + TEST_DATA_SZ * BYTES_PER_XDR_UNIT];
//   int buf_sz = BYTES_PER_XDR_UNIT + TEST_DATA_SZ * BYTES_PER_XDR_UNIT;
//   int16_t * data;
//   int data_sz = TEST_DATA_SZ;
// 
//   int16_t *p = NULL;
//   int p_sz = 0;
// 
//   XDR xdr_enc;
//   XDR xdr_dec;
//   XDR xdr_destroy;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   data = (int16_t *) malloc (data_sz * sizeof(int16_t));
//   for (cnt = 0; cnt < data_sz; cnt++)
//     data[cnt] = INT16_DATA[cnt];
//   memset (buf, 0, buf_sz);
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   if (!xdr_array (&xdr_enc, (char **)&data, &data_sz, TEST_DATA_SZ + 7, sizeof(int16_t), (xdrproc_t)xdr_int16_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_array XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_array_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   /* decode into allocated buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_array (&xdr_dec, (char **)&p, &p_sz, TEST_DATA_SZ + 7, sizeof(int16_t), (xdrproc_t)xdr_int16_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_array (allocated) XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_array_end2;
//     }
//   
//   /* check decoded array */
//   if (p_sz != data_sz)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_array size compare: (exp=%d, val=%d)\n",
//                testid, data_sz, p_sz);
//       pass = FALSE;
//       goto test_xdrstdio_array_end2;
//     } 
//   for (cnt = 0; cnt < data_sz; cnt++)
//     {
//       if (p[cnt] != data[cnt])
//         {
//           log_msg (o->log, XDR_LOG_INFO,
//                    "%s: failed xdr_array compare: (cnt=%d, exp=%"
//                    PRId16 ", val=%" PRId16 ")\n",
//                    testid, cnt, data[cnt], p[cnt]);
//            pass = FALSE;
//            goto test_xdrstdio_array_end2;
//         }
//     }
// 
//   /* Free allocated buffer; not very efficient in this
//    * case, because we deserialize each element from the 
//    * buffer AGAIN, before freeing the entire array.
//    * This makes more sense to do when you have structures
//    * that contain pointers. But, here we just verify that
//    * it works.
//    */
//   xdrstdio_create (&xdr_destroy, buf, buf_sz, XDR_FREE);
//   if (!xdr_array (&xdr_destroy, (char **)&p, &p_sz, TEST_DATA_SZ + 7, sizeof(int16_t), (xdrproc_t)xdr_int16_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_array XDR_FREE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_array_end3;
//     }
//   if (p != NULL)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: xdr_array XDR_FREE returned success, but did not NULL out ptr\n",
//                testid);
//       /* assume that data was freed, so null out manually */
//       p = NULL;
//       pass = FALSE;
//       goto test_xdrstdio_array_end3;
//     }
// 
// test_xdrstdio_array_end3:
//   XDR_DESTROY (&xdr_destroy);
// test_xdrstdio_array_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_array_end:
//   XDR_DESTROY (&xdr_enc);
//   if (data) { free (data); data = NULL; }
//   if (p)    { free (p); data = NULL; }
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_vector (opts * o)
// {
//   static const char *testid= "test_xdrstdio_vector";
//   /* use INT16_DATA as the vector data source */
//   /* these have static unfreeable storage, and fixed size */
//   char buf[TEST_DATA_SZ * BYTES_PER_XDR_UNIT];
//   int buf_sz = TEST_DATA_SZ * BYTES_PER_XDR_UNIT;
//   int16_t data[TEST_DATA_SZ];
//   int data_sz = TEST_DATA_SZ;
// 
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   for (cnt = 0; cnt < data_sz; cnt++)
//     data[cnt] = INT16_DATA[cnt];
//   memset (buf, 0, buf_sz);
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   if (!xdr_vector (&xdr_enc, (char *)data, data_sz, sizeof(int16_t), (xdrproc_t)xdr_int16_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_vector XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_vector_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   for (cnt = 0; cnt < data_sz; cnt++)
//     data[cnt] = 0;
// 
//   /* decode buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_vector (&xdr_dec, (char *)data, data_sz, sizeof(int16_t), (xdrproc_t)xdr_int16_t))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_vector XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_vector_end2;
//     }
//   
//   for (cnt = 0; cnt < data_sz; cnt++)
//     {
//       if (data[cnt] != INT16_DATA[cnt])
//         {
//           log_msg (o->log, XDR_LOG_INFO,
//                    "%s: failed xdr_vector compare: (cnt=%d, exp=%"
//                    PRId16 ", val=%" PRId16 ")\n",
//                    testid, cnt, INT16_DATA[cnt], data[cnt]);
//            pass = FALSE;
//            goto test_xdrstdio_vector_end2;
//         }
//     }
// 
// test_xdrstdio_vector_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_vector_end:
//   XDR_DESTROY (&xdr_enc);
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_reference (opts * o)
// {
//   static const char *testid= "test_xdrstdio_reference";
//   pgn_t *data;
//   char buf[100];
//   int buf_sz = 100;
//   pgn_t *p;
// 
//   XDR xdr_enc;
//   XDR xdr_dec;
//   int cnt;
//   bool_t pass = TRUE;
// 
//   /* initiaize input data */ 
//   init_pgn (&data, 0);
//   memset (buf, 0, buf_sz);
//   p = (pgn_t *) malloc (sizeof(pgn_t));
//   memset (p, 0, sizeof(pgn_t));
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   /* this struct contains a reference to another struct */
//   if (!xdr_pgn_t (&xdr_enc, data))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_pgn_t XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_reference_end;
//     }
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   /* decode into output variable */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_pgn_t (&xdr_dec, p))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_pgn_t XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_reference_end2;
//     }
//   
//   /* check decoded values */
//   if (compare_pgn(p, data) != 0)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s: failed xdr_reference data compare.\n",
//                testid);
//       if (o->log->level >= XDR_LOG_INFO)
//         {
//           fprintf (o->log->f, "Expected: ");
//           print_pgn (o->log->f, data);
//           fprintf (o->log->f, "\nReceived: ");
//           print_pgn (o->log->f, p);
//           fprintf (o->log->f, "\n");
//         }
//       pass = FALSE;
//       goto test_xdrstdio_reference_end2;
//     }
// 
// test_xdrstdio_reference_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_reference_end:
//   XDR_DESTROY (&xdr_enc);
//   if (data) { free_pgn (&data); }
//   if (p)    { free_pgn (&p); }
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_list_impl (opts * o,
//                        const char *testid,
//                        const char *proc_name,
//                        bool_t (*list_proc)(XDR *, pgn_list_t *))
// {
//   /*
//    * Same test, called with xdr_pgn_list_t_RECURSIVE() and
//    * xdr_pgn_list_t. The former is relatively inefficient, 
//    * but explicitly exercises the xdr_pointer primitive.
//    * The latter is the "correct" way to serialize a list.
//    */
//   pgn_list_t data = NULL;
//   char buf[1024];
//   int buf_sz = 1024;
//   pgn_list_t p = NULL;
// 
//   XDR xdr_enc;
//   XDR xdr_dec;
//   XDR xdr_destroy;
//   int cnt;
//   u_int pos;
//   bool_t pass = TRUE;
//   pgn_node_t *currA;
//   pgn_node_t *currB;
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   /* initiaize input data */
//   init_pgn_list (&data);
//   memset (buf, 0, buf_sz);
//   if (o->log->level >= XDR_LOG_DEBUG)
//     {
//       fprintf (o->log->f, "%s: Linked List Contents (input):\n", testid);
//       print_pgn_list (o->log->f, &data);
//     }
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   /* this struct contains a pointer to another struct */
//   if (!(*list_proc)(&xdr_enc, &data))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(%s): failed XDR_ENCODE\n",
//                testid, proc_name);
//       pass = FALSE; 
//       goto test_xdrstdio_list_end;
//     }
//   pos = XDR_GETPOS (&xdr_enc);
//   log_msg (o->log, XDR_LOG_INFO, "%s(%s): used %d bytes\n", 
//            testid, proc_name, pos);
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   /* decode into output variable */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!(*list_proc)(&xdr_dec, &p))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(%s): failed XDR_DECODE\n",
//                testid, proc_name);
//       pass = FALSE;
//       goto test_xdrstdio_list_end2;
//     }
//   pos = XDR_GETPOS (&xdr_dec);
//   log_msg (o->log, XDR_LOG_INFO, "%s(%s): used %d bytes\n",
//            testid, proc_name, pos);
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     {
//       fprintf (o->log->f, "%s(%s): Linked List Contents (output):\n",
//                testid, proc_name);
//       print_pgn_list (o->log->f, &p);
//     }
// 
//   /* compare decoded list */
//   currA = data;
//   currB = p;
//   cnt = 0;
//   while (currA && currB)
//     {
//       if (compare_pgn(&currA->pgn, &currB->pgn) != 0)
//         {
//           log_msg (o->log, XDR_LOG_INFO,
//                    "%s(%s): failed data compare (element %d).\n",
//                    testid, proc_name, cnt);
//           if (o->log->level >= XDR_LOG_INFO)
//             {
//               fprintf (o->log->f, "Expected: ");
//               print_pgn (o->log->f, &currA->pgn);
//               fprintf (o->log->f, "\nReceived: ");
//               print_pgn (o->log->f, &currB->pgn);
//               fprintf (o->log->f, "\n");
//             }
//           pass = FALSE;
//           goto test_xdrstdio_list_end2;
//         }
//       currA = currA->pgn_next;
//       currB = currB->pgn_next;
//       cnt++;
//     }
//   if ((currA && !currB) || (currB && !currA))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(%s): failed data compare: "
//                "# output elements != # input elements).\n",
//                testid, proc_name);
//       pass = FALSE;
//       goto test_xdrstdio_list_end2;
//     }
// 
//   /* Free allocated list. */
//   xdrstdio_create (&xdr_destroy, buf, buf_sz, XDR_FREE);
//   if (!(*list_proc)(&xdr_destroy, &p))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(%s): failed XDR_FREE\n",
//                testid, proc_name);
//       pass = FALSE;
//       goto test_xdrstdio_list_end3;
//     }
//   pos = XDR_GETPOS (&xdr_destroy);
//   log_msg (o->log, XDR_LOG_INFO, "%s(%s): used %d bytes\n",
//            testid, proc_name,pos);
// 
//   if (p != NULL)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(%s): XDR_FREE returned "
//                "success, but did not NULL out ptr\n",
//                testid, proc_name);
//       /* assume that data was freed, so null out manually */
//       p = NULL;
//       pass = FALSE;
//       goto test_xdrstdio_list_end3;
//     }
// 
// test_xdrstdio_list_end3:
//   XDR_DESTROY (&xdr_destroy);
// test_xdrstdio_list_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_list_end:
//   XDR_DESTROY (&xdr_enc);
//   if (data) { free_pgn_list (&data); }
//   if (p)    { free_pgn_list (&p); }
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
// bool_t
// test_xdrstdio_pointer (opts * o)
// {
//   return test_xdrstdio_list_impl (o, "test_xdrstdio_pointer",
//                                 "xdr_pgn_list_t_RECURSIVE",
//                                 xdr_pgn_list_t_RECURSIVE);
// }
// 
// bool_t
// test_xdrstdio_list (opts * o)
// {
//   return test_xdrstdio_list_impl (o, "test_xdrstdio_list",
//                                 "xdr_pgn_list_t",
//                                 xdr_pgn_list_t);
// }
// 
// bool_t
// test_xdrstdio_primitive_struct (opts * o)
// {
//   static const char *testid= "test_xdrstdio_primitive_struct";
//   char buf[144];
//   int buf_sz = 144;
//   test_struct_of_primitives_t data;
//   test_struct_of_primitives_t p;
//   u_int pos;
// 
//   XDR xdr_enc;
//   XDR xdr_dec;
//   bool_t pass = TRUE;
// 
//   init_primitive_struct (&data);
//   memset (buf, 0, buf_sz);
// 
//   log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     {
//       fprintf (o->log->f, "%s: structure contents (input):\n", testid);
//       print_primitive_struct (o->log->f, &data);
//     }
// 
//   xdrstdio_create (&xdr_enc, buf, buf_sz, XDR_ENCODE);
//   if (!xdr_primitive_struct_t (&xdr_enc, &data))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(xdr_primitive_struct_t): failed XDR_ENCODE\n",
//                testid);
//       pass = FALSE; 
//       goto test_xdrstdio_array_end;
//     }
//   pos = XDR_GETPOS (&xdr_enc);
//   log_msg (o->log, XDR_LOG_INFO,
//            "%s(xdr_primitive_struct_t): XDR_ENCODE used %d bytes\n",
//            testid, pos);
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     dumpmem (o->log->f, buf, buf_sz, 0);
// 
//   /* decode into allocated buffer */
//   xdrstdio_create (&xdr_dec, buf, buf_sz, XDR_DECODE);
//   if (!xdr_primitive_struct_t (&xdr_dec, &p))
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(xdr_primitive_struct_t): failed XDR_DECODE\n",
//                testid);
//       pass = FALSE;
//       goto test_xdrstdio_array_end2;
//     }
//   pos = XDR_GETPOS (&xdr_dec);
//   log_msg (o->log, XDR_LOG_INFO,
//            "%s(xdr_primitive_struct_t): XDR_DECODE used %d bytes\n",
//            testid, pos);
// 
//   if (o->log->level >= XDR_LOG_DEBUG)
//     {
//       fprintf (o->log->f, "%s: structure contents (output):\n", testid);
//       print_primitive_struct (o->log->f, &p);
//     }
//   
//   if (compare_primitive_structs (&data, &p) != 0)
//     {
//       log_msg (o->log, XDR_LOG_INFO,
//                "%s(xdr_primitive_struct_t): failed compare\n",
//                testid);
//        if (o->log->level >= XDR_LOG_INFO)
//          {
//            fprintf (o->log->f, "Expected:\n");
//            print_primitive_struct (o->log->f, &p);
//            fprintf (o->log->f, "Received:\n");
//            print_primitive_struct (o->log->f, &p);
//          }
//        pass = FALSE;
//        goto test_xdrstdio_array_end2;
//     }
// 
// test_xdrstdio_array_end2:
//   XDR_DESTROY (&xdr_dec);
// test_xdrstdio_array_end:
//   XDR_DESTROY (&xdr_enc);
//   if (pass == TRUE)
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
//   else
//     log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);
//   return pass;
// }
// 
