////////////////////////////////////////////////////////////////
//
// trace.h
//
// Copyright given to the public domain
//
////////////////////////////////////////////////////////////////

#ifndef __TRACE_H__
extern int TRACE_FLAG;
#define traceOn() (TRACE_FLAG = 1)
#define traceOff() (TRACE_FLAG = 0)
#ifdef __cplusplus
#include <iostream>
#define trace() (cout << (__FILE__) << "@" << (__LINE__) << endl << flush);

#define ctrace()                                                \
{                                                               \
if (TRACE_FLAG)                                                 \
 {                                                              \
    (cout << (__FILE__) << "@" << (__LINE__) << endl << flush); \
 }                                                              \
}

#else
#include <stdio.h>
#include <stdlib.h>
#define trace() printf("%s@%d\n", __FILE__, __LINE__), fflush(NULL);

#define ctrace()                                              \
{                                                             \
if (TRACE_FLAG)                                               \
 {                                                            \
   printf("%s@%d\n", __FILE__, __LINE__), fflush(NULL);       \
 }                                                            \
}

#endif

#endif



