#include "TSD.h"
#include <string.h>
#include <time.h>

// Nonlocal jumps.
jmp_buf TSD_GlobJumpRef;

static bool TestSt = true;
void        TSD_SetTestSt(const bool NewSt) { TestSt = NewSt; }
bool        TSD_GetTestSt(void) { return TestSt; }

static const TSD_TestSuite **Suites;
const TSD_TestSuite *const  *TSD_GetSuites(void) { return Suites; }

static size_t SuitesCount = 0;
static size_t SuitesIndex = 0;
size_t        TSD_GetSuitesCount(void) { return SuitesCount; }

void TSD_IncreaseNumberOfSuites(void) { SuitesCount++; }
void TSD_TestSuiteToRegistry(const TSD_TestSuite *Suite) {
  if (SuitesIndex < SuitesCount) {
    fprintf(stderr, "Detected suite: %s. Saving in slot #%zu/%zu.\n",
            Suite->Name, SuitesIndex + 1, SuitesCount);
    Suites[SuitesIndex] = Suite;
    SuitesIndex++;
  } else {
    fprintf(stderr, "Detected suite: %s. Cannot save.\n", Suite->Name);
  }
}

// Automatic suite allocation.
__attribute__((constructor(TSD_PRIORITY_REG + 1))) static void
AllocateSuitesArray(void) {
  Suites =
      (const TSD_TestSuite **)malloc(sizeof(TSD_TestSuite *) * SuitesCount);
  if (!Suites) {
    fprintf(
        stderr,
        "TSD: CRITICAL: could not allocate memory for test suite registry.\n");
    fprintf(stderr, "Requested %zu bytes.\n",
            sizeof(const TSD_TestSuite **) * SuitesCount);
    fprintf(stderr, "Stopping...\n");
    exit(EXIT_FAILURE);
  }
}
__attribute__((destructor(TSD_PRIORITY_REG + 1))) static void
FreeSuitesArray(void) {
  free(Suites);
}

// Test runner dependencies.
static void RunnerParseInput(const size_t               argc,
                             const char *const          argv[],
                             const TSD_TestSuite *const Suites[],
                             const size_t               SuitesCount,
                             bool                      *Battery,
                             bool                      *Suite,
                             bool                      *Test,
                             size_t                    *SuiteId,
                             size_t                    *TestId) {
  size_t i;
  size_t Counter;

  if (argc > 3) {
    fprintf(stderr, "Too many arguments.\n");
    exit(EXIT_FAILURE);
  }

  if (argc == 1) {
    *Battery = true;
    *Suite   = false;
    *Test    = false;
    return;
  }

  *Battery = false;

  Counter = 0;
  for (i = 0; i < SuitesCount; i++) {
    if (strcmp(argv[1], Suites[i]->Name) == 0) {
      *SuiteId = i;
      break;
    } else {
      Counter++;
    }
  }
  if (Counter == SuitesCount) {
    fprintf(stderr, "Suite %s not found.\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  if (argc == 2) {
    *Suite = true;
    *Test  = false;
    return;
  }

  *Suite = false;

  Counter = 0;
  for (i = 0; i < Suites[*SuiteId]->Count; i++) {
    if (strcmp(argv[2], Suites[*SuiteId]->Tests[i].Name) == 0) {
      *TestId = i;
      break;
    } else {
      Counter++;
    }
  }
  if (Counter == Suites[*SuiteId]->Count) {
    fprintf(stderr, "Test %s not found in suite %s.\n", argv[2], argv[1]);
    exit(EXIT_FAILURE);
  }

  *Test = true;
  return;
}
void TSD_RunTest(const TSD_Test *TestInstance,
                 size_t         *NFailedTests) {
  void *ContextData = NULL;

  TSD_SetTestSt(true);
  if (TestInstance->Ctx) {
    if (TestInstance->Ctx->ContextSize > 0) {
      ContextData = (void *)malloc(TestInstance->Ctx->ContextSize);
      if (!ContextData) {
        fprintf(stderr,
                "TSD: CRITICAL: could not allocate memory for context.\n");
        fprintf(stderr, "Requested %zu bytes.\n",
                TestInstance->Ctx->ContextSize);
        fprintf(stderr, "Stopping...\n");
        exit(EXIT_FAILURE);
      }
    }
    if (TestInstance->Ctx->SetUp)
      TestInstance->Ctx->SetUp(ContextData);
    TestInstance->Procedure(ContextData);
    if (TestInstance->Ctx->TearDown)
      TestInstance->Ctx->TearDown(ContextData);
    free(ContextData);
  } else {
    TestInstance->Procedure(NULL);
  }
  if (TSD_GetTestSt()) {
    fprintf(stderr, "--> OK <--\n");
  } else {
    fprintf(stderr, "--> FAIL <--\n");
    *NFailedTests += (size_t)1;
  }
}
void TSD_RunSuite(const TSD_TestSuite *Suite,
                  size_t              *NFailedTests) {
  size_t i;

  fprintf(stderr, "Running suite: %s\n", Suite->Name);

  for (i = 0; i < Suite->Count; ++i) {
    fprintf(stderr, "--> Test %zu/%zu: %s: \n", i + 1, Suite->Count,
            Suite->Tests[i].Name);
    TSD_RunTest(&(Suite->Tests[i]), NFailedTests);
  }
}
void TSD_RunAll(const TSD_TestSuite *const Suites[],
                const size_t               SuitesCount,
                size_t                    *NFailedTests) {
  size_t i;

  for (i = 0; i < SuitesCount; ++i) {
    TSD_RunSuite(Suites[i], NFailedTests);
  }
}
size_t TSD_Driver(const size_t      argc,
                  const char *const argv[]) {
  time_t     RawTime;
  struct tm *TimeInfo;

  bool   Battery = false, Suite = false, Test = false;
  size_t SuiteId, TestId;
  size_t NFailedTests = (size_t)0;

  const TSD_TestSuite *const *Suites      = TSD_GetSuites();
  const size_t                SuitesCount = TSD_GetSuitesCount();

  RunnerParseInput(argc, argv, Suites, SuitesCount, &Battery, &Suite, &Test,
                   &SuiteId, &TestId);

  fprintf(stderr, "\n");
  fprintf(stderr, "<--------TSD-------->\n");

  if (Battery) {
    time(&RawTime);
    TimeInfo = localtime(&RawTime);
    fprintf(stderr, "Started running test battery at: %s\n", asctime(TimeInfo));
    TSD_RunAll(Suites, SuitesCount, &NFailedTests);
    fprintf(stderr, "Testing finished.\n");
    goto exit;
  }

  if (Suite) {
    time(&RawTime);
    TimeInfo = localtime(&RawTime);
    fprintf(stderr, "Started running suite %s at: %s\n", Suites[SuiteId]->Name,
            asctime(TimeInfo));
    TSD_RunSuite(Suites[SuiteId], &NFailedTests);
    fprintf(stderr, "Testing finished.\n");
    goto exit;
  }

  if (Test) {
    time(&RawTime);
    TimeInfo = localtime(&RawTime);
    fprintf(stderr, "Started running test %s:%s at: %s\n",
            Suites[SuiteId]->Name, Suites[SuiteId]->Tests[TestId].Name,
            asctime(TimeInfo));
    fprintf(stderr, "--> Test : %s: \n", Suites[SuiteId]->Tests[TestId].Name);
    TSD_RunTest(&(Suites[SuiteId]->Tests[TestId]), &NFailedTests);
    fprintf(stderr, "Testing finished.\n");
    goto exit;
  }

exit:
  fprintf(stderr, "%zu test(s) failed.\n", NFailedTests);
  return NFailedTests;
}
