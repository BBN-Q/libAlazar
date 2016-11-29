//#include <cstdlib>
//#include <string>
//#include <thread>
//#include <stdlib.h>
#include <iostream>
#include <map>
#include <unistd.h>
#include <cstring>

#include "AlazarError.h"
#include "libAlazar.h"

int main(int argc, char *argv[]) {

  if ((argc > 1) && !strncmp(argv[1], "-h", 2)) {
    printf("USAGE: %s <numerical error code between 512 and 592>\n", argv[0]);
    exit(-1);
  }

  AlazarATS9870 *board1 = new AlazarATS9870();

  std::cout << "ERROR TEST ..." << std::endl;

  if (argc > 1) {
    board1->printError((RETURN_CODE)std::stoi(argv[1]), __FILE__, __LINE__);
  } else {
    for (uint32_t i = 512; i <= 592; i++) {
      board1->printError((RETURN_CODE)i, __FILE__, __LINE__);
    }
  }

  delete (board1);
  return (0);
}
