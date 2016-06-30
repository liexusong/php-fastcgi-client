dnl $Id$
dnl config.m4 for extension fastcgi_client

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(fastcgi_client, for fastcgi_client support,
dnl Make sure that the comment is aligned:
dnl [  --with-fastcgi_client             Include fastcgi_client support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(fastcgi_client, whether to enable fastcgi_client support,
dnl Make sure that the comment is aligned:
dnl [  --enable-fastcgi_client           Enable fastcgi_client support])

if test "$PHP_FASTCGI_CLIENT" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-fastcgi_client -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/fastcgi_client.h"  # you most likely want to change this
  dnl if test -r $PHP_FASTCGI_CLIENT/$SEARCH_FOR; then # path given as parameter
  dnl   FASTCGI_CLIENT_DIR=$PHP_FASTCGI_CLIENT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for fastcgi_client files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       FASTCGI_CLIENT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$FASTCGI_CLIENT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the fastcgi_client distribution])
  dnl fi

  dnl # --with-fastcgi_client -> add include path
  dnl PHP_ADD_INCLUDE($FASTCGI_CLIENT_DIR/include)

  dnl # --with-fastcgi_client -> check for lib and symbol presence
  dnl LIBNAME=fastcgi_client # you may want to change this
  dnl LIBSYMBOL=fastcgi_client # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FASTCGI_CLIENT_DIR/$PHP_LIBDIR, FASTCGI_CLIENT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_FASTCGI_CLIENTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong fastcgi_client lib version or lib not found])
  dnl ],[
  dnl   -L$FASTCGI_CLIENT_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(FASTCGI_CLIENT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(fastcgi_client, fastcgi_client.c, $ext_shared)
fi
