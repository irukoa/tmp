#include "TSD.h"

int main(const int         argc,
         const char *const argv[]) {
  size_t NFailedTests;

  NFailedTests = TSD_Driver((size_t)argc, argv);
  if (NFailedTests > (size_t)0) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
