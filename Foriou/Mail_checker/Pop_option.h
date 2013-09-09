// deliberately not using include guard

#ifndef     HAS_PUSH_OPTION_
#   error   Pop_option can only be used exactly after push_option.h
#endif

#undef      HAS_PUSH_OPTION_

#ifdef      NEED_POP_OPTION_
#   define     UNICODE
#   define     _UNICODE
#   undef   NEED_POP_OPTION_
#endif

///////////////////////////////////////////////////////////////////////
// End of File
//      Dedicated for myself
///////////////////////////////////////////////////////////////////////
