// deliberately not using include guard

#ifdef HAS_PUSH_OPTION_
#   error   SYMBOL COLLIDE ERROR
#else
#   define HAS_PUSH_OPTION_
#endif

#ifdef UNICODE
#   undef  NEED_POP_OPTION_
#   define NEED_POP_OPTION_
#   undef  UNICODE
#   undef  _UNICODE
#endif

///////////////////////////////////////////////////////////////////////
// End of File
//      Dedicated for myself
///////////////////////////////////////////////////////////////////////
