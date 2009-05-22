top_srcdir=$(shell pwd)

uname:=$(shell uname -s)
ifeq ($(findstring CYGWIN,$(uname)),CYGWIN)
  PLATFORM:=cygwin
else ifeq ($(findstring MINGW,$(uname)),MINGW)
  PLATFORM:=mingw
else
  PLATFORM:=unknown
endif

ifeq ($(PLATFORM), cygwin)
CC=gcc
O=o
EXEEXT=.exe
DLLVER=0
SHRNAME=cygxdr-$(DLLVER).dll
DEFFILE=$(PLATFORM)/xdr.def
IMPNAME=libxdr.dll.a
LIBNAME=libxdr.a
LIB_LDEXTRA=-Wl,--out-implib=$(PLATFORM)/$(IMPNAME) -Wl,--enable-auto-image-base
LIBS =
else ifeq ($(PLATFORM), mingw)
CC=gcc
O=o
EXEEXT=.exe
DLLVER=0
SHRNAME=mgwxdr-$(DLLVER).dll
DEFFILE=$(PLATFORM)/xdr.def
IMPNAME=libxdr.dll.a
LIBNAME=libxdr.a
LIB_LDEXTRA=-Wl,--out-implib=$(PLATFORM)/$(IMPNAME) -Wl,--enable-auto-image-base
LIBS = -lws2_32
else
CC=gcc
O=o
EXEEXT=
SOMAJOR=0
SOMINOR=0
SOMICRO=0
SHRNAME=libxdr.so.$(SOMAJOR).$(SOMINOR).$(SOMICRO)
LIBNAME=libxdr.a
LIBS =
endif

FULLPATH_SHRLIB=$(PLATFORM)/$(SHRNAME)
FULLPATH_STATLIB=$(PLATFORM)/$(LIBNAME)

INCLUDES = -I$(top_srcdir)

LIB_HDRS = rpc/xdr.h rpc/types.h lib/xdr_private.h
LIB_SRCS = lib/xdr.c lib/xdr_array.c lib/xdr_float.c lib/xdr_mem.c \
	   lib/xdr_rec.c lib/xdr_reference.c lib/xdr_sizeof.c lib/xdr_stdio.c \
	   lib/xdr_private.c
LIB_OBJS = $(LIB_SRCS:%.c=$(PLATFORM)/%.$(O))

all:
	@if test "$(PLATFORM)" = "unknown" ; then \
	  echo "Can't build for $(uname) using this makefile" 1>&2 ;\
	  false ;\
	else \
	  echo "Building for $(PLATFORM)" ;\
	  make recursive-all ;\
	fi

recursive-all: $(FULLPATH_SHRLIB) $(FULLPATH_STATLIB)

builddir:
	@mkdir -p $(PLATFORM)
	@mkdir -p $(PLATFORM)/lib

$(DEFFILE): builddir lib/libxdr.def.in
	cat lib/libxdr.def.in | sed -e "s/@@LIBNAME@@/$(SHRNAME)/" > $@

$(FULLPATH_SHRLIB): builddir $(DEFFILE) $(LIB_OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $(LIB_LDEXTRA) $(DEFFILE) $(LIB_OBJS) $(LIBS)

$(FULLPATH_STATLIB): builddir $(LIB_OBJS)
	$(AR) cr $@ $(LIB_OBJS)

$(PLATFORM)/%.$(O) : %.c $(LIB_HDRS)
	$(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -f $(LIB_OBJS) $(DEFFILE)

.PHONY: realclean
realclean: clean
	-rm -f $(FULLPATH_STATLIB) $(FULLPATH_SHRLIB) $(PLATFORM)/$(IMPNAME)
	-rmdir $(PLATFORM)/lib
	-rmdir $(PLATFORM)


