PHP_ARG_ENABLE(recovery, whether to enable recovery support,
[  --enable-recovery           Enable recovery support])

if test "$PHP_RECOVERY" != "no"; then
  dnl Check for recovery libraries
  AC_MSG_CHECKING([for recovery libraries])
  
  dnl Search for libraries in common locations
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/recovery/recovery_core.h"
  
  if test -r $PHP_RECOVERY/$SEARCH_FOR; then
    RECOVERY_DIR=$PHP_RECOVERY
  else
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        RECOVERY_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$RECOVERY_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please install recovery libraries first])
  fi
  
  dnl Add include path
  PHP_ADD_INCLUDE($RECOVERY_DIR/include)
  
  dnl Add library paths and libraries
  PHP_ADD_LIBRARY_WITH_PATH(recovery_core, $RECOVERY_DIR/lib, RECOVERY_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(recovery_crypto, $RECOVERY_DIR/lib, RECOVERY_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(recovery_network, $RECOVERY_DIR/lib, RECOVERY_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(recovery_signal, $RECOVERY_DIR/lib, RECOVERY_SHARED_LIBADD)
  
  PHP_SUBST(RECOVERY_SHARED_LIBADD)
  
  dnl Define extension
  PHP_NEW_EXTENSION(recovery, recovery.c, $ext_shared)
fi