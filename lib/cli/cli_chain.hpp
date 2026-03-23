#pragma once

#include <iosfwd>
#include <memory>

#include "cli_handler.hpp"
#include "core/application.hpp"

namespace route_finder {

class CliChain {
 public:
  CliChain(Application& app, std::ostream& out, std::ostream& err);

  int Run();
  int Run(std::istream& input);

 private:
  std::ostream& out_;
  std::ostream& err_;
  std::unique_ptr<ICliHandler> head_;
};

}
