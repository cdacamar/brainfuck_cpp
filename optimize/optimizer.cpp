#include <optimize/optimizer.h>

#include <optimize/folder.h>

namespace bf {

namespace optimize {

void optimize(prog::program& p) {
  fold(p);
}

} // namespace optimize

} // namespace bf