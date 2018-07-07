#include "server.h"


int main(int argc, char** argv)
{
  if (argc > 1 && argc < 2)
  {
    Server s("16555", argv[1]);
    s.begin_listen();
  }
  else
  {
    Server s("16555");
    s.begin_listen();
  }
  return 0;
}
