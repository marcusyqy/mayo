#include <iostream>
#include "entry.hpp"

namespace zoo {

application::exit_status main(application::settings args) {
    std::cout << "hello world from zoo main!" << std::endl;
    return application::exit_status::ok;
}

} // namespace zoo
