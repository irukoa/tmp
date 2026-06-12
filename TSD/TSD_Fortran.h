#ifndef _TSD_FORTRAN
#define _TSD_FORTRAN

#ifdef TSD_FORTRAN_IMPL

#define xstr(s)        str(s)
#define str(s)         #s
#define TESTNAME(NAME) TEST_##NAME

#define TSD_FORTRAN_INTERFACES                                                 \
  interface;                                                                   \
  subroutine TSD_SetTestSt(NewSt) bind(C, name = "TSD_SetTestSt");             \
  use, intrinsic ::ISO_C_BINDING;                                              \
  logical(c_bool), intent(in), value ::NewSt;                                  \
  end subroutine TSD_SetTestSt;                                                \
  end            interface

#define TEST_E(NAME)                                                           \
  subroutine TESTNAME(NAME)() bind(C, name = xstr(TESTNAME(NAME)));            \
  use, intrinsic ::ISO_C_BINDING;                                              \
  use, intrinsic ::ISO_FORTRAN_ENV
#define END_TEST_E end subroutine

#define ASSERT_NOEXIT_WRAP(EXPR, FILE, LINE)                                   \
  if (.not.(EXPR))                                                             \
    then;                                                                      \
  call TSD_SetTestSt(logical(.false., kind = c_bool));                         \
  write(ERROR_UNIT, "(A, A, I0, A, A, A)") FILE, ":", LINE, ": Assertion ",    \
      #EXPR, " failed.";                                                       \
  end if
#define ASSERT_EXIT_WRAP(EXPR, FILE, LINE)                                     \
  if (.not.(EXPR))                                                             \
    then;                                                                      \
  call TSD_SetTestSt(logical(.false., kind = c_bool));                         \
  write(ERROR_UNIT, "(A, A, I0, A, A, A)") FILE, ":", LINE, ": Assertion ",    \
      #EXPR, " failed.";                                                       \
  error stop;                                                                  \
  end if

#define ASSERT(EXPR)   ASSERT_NOEXIT_WRAP(EXPR, __FILE__, __LINE__)
#define ASSERT_X(EXPR) ASSERT_EXIT_WRAP(EXPR, __FILE__, __LINE__)

#elif TSD_KERNEL

#include "test/common/TSD.h"
#define END_TEST_E

#endif

#endif
