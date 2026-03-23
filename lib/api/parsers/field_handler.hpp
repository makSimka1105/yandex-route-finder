#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include "nlohmann/json_fwd.hpp"

namespace route_finder {

template <typename T>
class IFieldHandler {
 public:
  virtual ~IFieldHandler() = default;

  IFieldHandler<T>* SetNext(std::unique_ptr<IFieldHandler<T>> next) {
    next_ = std::move(next);
    return next_.get();
  }

  void Handle(const nlohmann::json& json, T& target) {
    Extract(json, target);
    if (next_) {
      next_->Handle(json, target);
    }
  }

 protected:
  virtual void Extract(const nlohmann::json& json, T& target) = 0;

 private:
  std::unique_ptr<IFieldHandler<T>> next_;
};

}
