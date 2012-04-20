#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <tinia/policylib/PolicyLib.hpp>
#include <memory>
namespace qtobserver {
/**
  Returns the human readable name of the key (by getting the annotation).
  If the element has no annotation, it defaults to the key.
  */
std::string prettyName(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib);

}
#endif // UTILS_HPP
