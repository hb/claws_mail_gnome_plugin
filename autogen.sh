#!/bin/sh

aclocal \
  && libtoolize --copy --force \
  && autoheader \
  && automake --add-missing --foreign --copy \
  && autoconf \
  && ./configure --enable-maintainer-mode $@
