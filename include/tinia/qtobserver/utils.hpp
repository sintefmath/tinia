#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <tinia/policy/Policy.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {
/**
  Returns the human readable name of the key (by getting the annotation).
  If the element has no annotation, it defaults to the key.
  */
std::string prettyName(std::string key, std::shared_ptr<policy::Policy> policy);

}
}
#endif // UTILS_HPP
