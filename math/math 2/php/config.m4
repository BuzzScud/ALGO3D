PHP_ARG_ENABLE(crystalline_math,
  [whether to enable Crystalline Math support],
  [  --enable-crystalline-math   Enable Crystalline Math support], no)

if test "$PHP_CRYSTALLINE_MATH" != "no"; then
  dnl Check for Crystalline Math library
  AC_MSG_CHECKING([for Crystalline Math library])
  
  dnl Set library path
  CRYSTALLINE_MATH_DIR="../math"
  
  dnl Check if library exists
  if test -f "$CRYSTALLINE_MATH_DIR/lib/libcrystallinemath.so"; then
    AC_MSG_RESULT([found])
    
    dnl Add include path
    PHP_ADD_INCLUDE($CRYSTALLINE_MATH_DIR/include)
    
    dnl Add library path and link
    PHP_ADD_LIBRARY_WITH_PATH(crystallinemath, $CRYSTALLINE_MATH_DIR/lib, CRYSTALLINE_MATH_SHARED_LIBADD)
    
    dnl Define extension
    PHP_NEW_EXTENSION(crystalline_math, crystalline_math.c, $ext_shared)
    PHP_SUBST(CRYSTALLINE_MATH_SHARED_LIBADD)
  else
    AC_MSG_ERROR([Crystalline Math library not found. Please build it first with 'cd ../math && make'])
  fi
fi