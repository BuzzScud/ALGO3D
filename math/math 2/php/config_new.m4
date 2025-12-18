PHP_ARG_ENABLE(cllm, whether to enable CLLM support,
[  --enable-cllm           Enable CLLM support])

if test "$PHP_CLLM" != "no"; then
  PHP_NEW_EXTENSION(cllm, cllm_extension.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  
  # Add include paths
  PHP_ADD_INCLUDE(../include)
  PHP_ADD_INCLUDE(../algorithms/include)
  PHP_ADD_INCLUDE(../math/include)
  
  # Add library paths
  PHP_ADD_LIBRARY_WITH_PATH(cllm, .., CLLM_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(algorithms, ../algorithms, CLLM_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(crystallinemath, ../math/lib, CLLM_SHARED_LIBADD)
  
  PHP_SUBST(CLLM_SHARED_LIBADD)
fi