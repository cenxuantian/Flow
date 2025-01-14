#include "args_parser.hpp"
#include <flow>

int main(int argc, char *argv[]) {
  flow::context ctx;
  auto args = flow_executor::parse_args(argc, argv);

  if (!args.size()) {
    while (1) {
      char line[30];
      memset(line, 0, sizeof(line));
      std::cin.getline(line, sizeof(line));
      flow::core::exec(ctx, line, args);
    }
  }
  return 0;
}
