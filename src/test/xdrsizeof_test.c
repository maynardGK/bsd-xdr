#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "test_common.h"
#include "test_data.h"
#include "test_xdrs.h"

const char *program_name;

void
usage (FILE * f, const char *progname)
{
  fprintf (f, "%s [-h] [-v]\n", progname);
  fputs ("  Tests the 'sizeof' functionality of the xdr library\n", f);
  fputs ("  on this specific platform. Does not test data interchange\n", f);
  fputs ("  between different platforms.\n", f);
  fputs ("   -h : print this help\n", f);
  fputs ("   -v : verbose mode (may be repeated)\n", f);
  fputs ("   -s : silent mode\n", f);
}

typedef struct _opts
{
  log_opts *log;
} opts;

bool_t test_xdrsizeof_list (opts *o);
bool_t test_xdrsizeof_primitive_struct (opts *o);

int
main (int argc, char *argv[])
{
  int c;
  log_opts log;
  opts o;
  bool_t rc = TRUE;

  set_program_name (argv[0]);
  log.level = 0;
  log.f = stderr;
  o.log = &log;

  while ((c = getopt (argc, argv, "hvs")) != -1)
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
      }

  rc &= test_xdrsizeof_list (&o);
  rc &= test_xdrsizeof_primitive_struct (&o);

  if (rc == TRUE)
    log_msg (o.log, XDR_LOG_NORMAL, "All tests passed!\n");
  else
    log_msg (o.log, XDR_LOG_NORMAL, "Some tests failed!\n");

  return (rc == TRUE ? EXIT_SUCCESS : EXIT_FAILURE);
}

bool_t
test_xdrsizeof_list (opts * o)
{
  static const char * testid = "test_xdrsizeof_list";
  pgn_list_t data = NULL;
  bool_t pass = TRUE;
  unsigned long sz_result;
  unsigned long sz_expected = 600;

  /* initialize data */
  init_pgn_list (&data);

  log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
  sz_result = xdr_sizeof ((xdrproc_t)xdr_pgn_list_t, &data);
  if (sz_result != sz_expected)
    {
      log_msg (o->log, XDR_LOG_INFO,
               "%s(xdr_pgn_list_t): failed xdr_sizeof (exp=%lu, val=%lu)\n",
               testid, sz_expected, sz_result);
      pass = FALSE; 
      goto test_xdrsizeof_list_end;
    }

test_xdrsizeof_list_end:
  if (data) { free_pgn_list (&data); }
  if (pass == TRUE)
    log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;                                                 
}

bool_t
test_xdrsizeof_primitive_struct (opts * o)
{
  static const char * testid = "test_xdrsizeof_primitive_struct";
  test_struct_of_primitives_t data;
  bool_t pass = TRUE;
  unsigned long sz_result;
  unsigned long sz_expected = 144;

  /* initialize input data */
  init_primitive_struct (&data);

  log_msg (o->log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
  sz_result = xdr_sizeof ((xdrproc_t)xdr_primitive_struct_t, &data);
  if (sz_result != sz_expected)
    {
      log_msg (o->log, XDR_LOG_INFO,
               "%s(xdr_primitive_struct_t): failed xdr_sizeof "
               "(exp=%lu, val=%lu)\n", testid, sz_expected, sz_result);
      pass = FALSE; 
      goto test_xdrsizeof_primitive_struct_end;
    }

test_xdrsizeof_primitive_struct_end:
  if (pass == TRUE)
    log_msg (o->log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (o->log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;                                                 
}

