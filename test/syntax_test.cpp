#include <flow>

#define CODE(...) #__VA_ARGS__

std::string code = CODE(

    a = 1; c = a + 3; print(a, c, a + c, d = c + 3);

);

int main() {
  flow::context ctx;
  flow::core::exec(ctx, code);
  return 0;
}
