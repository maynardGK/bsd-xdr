bsd-xdr is an port of Sun's [XDR](http://en.wikipedia.org/wiki/External_Data_Representation) ([RFC 4506](http://tools.ietf.org/html/rfc4506)) library, mostly for Windows, although it also works on 32- and 64-bit linux.  bsd-xdr was derived from the XDR implementation in the libtirpc package (version 0.1.10-7) from Fedora 11. That version was relicensed with explicit permission from the copyright holder (Sun Microsystems) to a BSD license.

## Goals of this port ##
  * Maintain the BSD license rather than copylefting it, as in portableXDR and various other versions. (Actually, some other non-copyleft licenses such as MIT/X apply to a few files).
  * Avoid autotools.
  * Avoid "config.h" pollution (or similar) in public headers.
  * Compile successfully at highest reasonable warning level, with warnings treated as errors.
  * Support windows hosts:
    1. Cygwin
    1. MinGW
    1. MSVC
  * Compile library as both static and shared.
  * Provide thorough tests of all libxdr facilities.
  * Where feasible, link test programs against both static and shared libraries.
  * Verify identical output on a variety of platforms, especially 32- and 64-bit hosts.

## Origins ##
The initial checkin of bsd-xdr was extracted directly from the libtirpc package (version 0.1.10-7) from Fedora 11, as patched by its rpm.spec.

As documented in the libtirpc rpm.spec file from Fedora 11:
  * Tue May 19 2009 Tom "spot" Callaway <redacted@redhat.com> 0.1.10-7
    * Replace the Sun RPC license with the BSD license, with the explicit permission of Sun Microsystems