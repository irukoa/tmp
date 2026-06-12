#ifndef _TSD_H
#define _TSD_H

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* TSD test suite registration startup priority. Let P = TSD_PRIORITY_REG.
Steps are:
1- Startup priority P: count number of test suites.
2- Startup priority P+1: allocate global suites array.
3- Startup priority P+2: register test suites in the global array.
4- Main program execution.
5- Shutdown priority P+1: free global suites array.*/
#ifndef TSD_PRIORITY_REG
#define TSD_PRIORITY_REG 2210
#endif

/* TSD Definitions.*/
// Nonlocal jumps.
extern jmp_buf TSD_GlobJumpRef;

// Function signature.
typedef void (*TSD_FncProto)(void *Data);

// Test context (environment).
/*Note: in tests created with macros, the allocated data can be accessed through
  the implicit pointer: `void *ContextData`.*/
typedef struct _TSD_Context {
  const TSD_FncProto SetUp;    // Runs before the test is executed.
  const TSD_FncProto TearDown; // Runs after the test is executed.
  const size_t ContextSize;    // An allocation of this size is performed before
                               // the test is executed.
} TSD_Context;

// Test object.
typedef struct _TSD_Test {
  const char *const  Name;      // Test name.
  const TSD_FncProto Procedure; // Test function pointer.
  const TSD_Context *Ctx;       // Context (optional).
} TSD_Test;

// Test suite object.
typedef struct _TSD_TestSuite {
  const char *const Name;  // Suite name.
  const TSD_Test   *Tests; // Array of tests.
  const size_t      Count; // Number of tests.
} TSD_TestSuite;

// Test status (true = success, false = failure).
void TSD_SetTestSt(const bool NewSt);
bool TSD_GetTestSt(void);

// - Test suite object helpers:
// -- get a pointer to the array of registered suites.
const TSD_TestSuite *const *TSD_GetSuites(void);
// -- get the number of registered suites.
size_t TSD_GetSuitesCount(void);

// Suite registration on runner startup.
/*Note: these functions get called by macro-specific functions with priority
 * TSD_PRIORITY_REG and TSD_PRIORITY_REG+2, respectively.*/
void TSD_IncreaseNumberOfSuites(void);
void TSD_TestSuiteToRegistry(const TSD_TestSuite *Suite);

// Runner dependencies.
void TSD_RunTest(const TSD_Test *TestInstance,
                 size_t         *NFailedTests);
void TSD_RunSuite(const TSD_TestSuite *Suite,
                  size_t              *NFailedTests);
void TSD_RunAll(const TSD_TestSuite *const Suites[],
                const size_t               SuitesCount,
                size_t                    *NFailedTests);
// Returns the number of failed tests.
size_t TSD_Driver(const size_t      argc,
                  const char *const argv[]);

/* TSD Macros.*/
// Assert with program termination.
#define ASSERT_X(EXPR) assert(EXPR)

// Assert without program termination.
#define ASSERT_NOEXIT_WRAP(EXPR, FILE, LINE, FUNCTION)                         \
  if (!(EXPR)) {                                                               \
    TSD_SetTestSt(false);                                                      \
    fprintf(stderr, "%s:%d: %s: Assertion ", FILE, LINE, FUNCTION);            \
    fprintf(stderr, "%s ", #EXPR);                                             \
    fprintf(stderr, "failed.\n");                                              \
  }                                                                            \
  (void)0
#define ASSERT(EXPR) ASSERT_NOEXIT_WRAP(EXPR, __FILE__, __LINE__, __func__)

// - Test context creator.
#define CONTEXT(NAME, SETUP, TEARDOWN, ALLOCSIZE)                              \
  static const TSD_Context NAME = {.SetUp       = SETUP,                       \
                                   .TearDown    = TEARDOWN,                    \
                                   .ContextSize = ((size_t)ALLOCSIZE)}

// - Test context function creator (for setup/teardown).
#define CONTEXT_FN(NAME) static void NAME(void *ContextData)

// - Test creator: regular test.
#define TEST(NAME)                                                             \
  static void TEST_##NAME(void);                                               \
  static void TEST_##NAME##_Wrapper(void *ContextData) {                       \
    (void)ContextData;                                                         \
    TEST_##NAME();                                                             \
  }                                                                            \
  static const TSD_Test NAME = {.Name      = #NAME,                            \
                                .Procedure = TEST_##NAME##_Wrapper,            \
                                .Ctx       = NULL};                            \
  static void           TEST_##NAME(void)

// - Test creator: context-dependent test.
#define TEST_F(CONTEXT, NAME)                                                  \
  static void           TEST_##NAME(void *ContextData);                        \
  static const TSD_Test NAME = {.Name      = #NAME,                            \
                                .Procedure = TEST_##NAME,                      \
                                .Ctx       = &CONTEXT};                        \
  static void           TEST_##NAME(void *ContextData)

// - Test creator: externally-implemented test.
#define TEST_E(NAME)                                                           \
  extern void TEST_##NAME(void);                                               \
  static void TEST_##NAME##_Wrapper(void *ContextData) {                       \
    (void)ContextData;                                                         \
    TEST_##NAME();                                                             \
  }                                                                            \
  static const TSD_Test NAME = {.Name      = #NAME,                            \
                                .Procedure = TEST_##NAME##_Wrapper,            \
                                .Ctx       = NULL};

// - Test suite creator.
#define TEST_SUITE(NAME, ...)                                                  \
  static const TSD_Test      NAME[]      = {__VA_ARGS__};                      \
  static const TSD_TestSuite Suite##NAME = {.Name  = #NAME,                    \
                                            .Tests = (NAME),                   \
                                            .Count = (sizeof((NAME))) /        \
                                                     (sizeof((NAME)[0]))};     \
  __attribute__((constructor(TSD_PRIORITY_REG))) static void                   \
  _TSD_IncreaseNumberOfSuites##NAME(void) {                                    \
    TSD_IncreaseNumberOfSuites();                                              \
  }                                                                            \
  __attribute__((constructor(TSD_PRIORITY_REG + 2))) static void               \
  _TSD_DriverRegisterSuite##NAME(void) {                                       \
    TSD_TestSuiteToRegistry(&Suite##NAME);                                     \
  }                                                                            \
  enum { RegisterTestSuiteDummy_##NAME = 0 }

#endif
