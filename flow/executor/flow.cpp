#include "args_parser.hpp"
#include <flow>
#include <string.h>

int main(int argc, char *argv[]) {
  flow::context ctx;
  auto args = flow_executor::parse_args(argc, argv);

  if (!args.size()) {
    while (1) {
      char line[30];
      memset(line, 0, sizeof(line));
      std::cout << ">> ";
      std::cin.getline(line, sizeof(line));
      flow::core::exec(ctx, line, args);
    }
  }
  return 0;
}
