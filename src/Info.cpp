#include "Info.hpp"

bool Info::checkKeyExists(const std::string &key) const {
  if (key == "timeout_turn") {
    return true;
  } else if (key == "timeout_match") {
    return true;
  } else if (key == "max_memory") {
    return true;
  } else if (key == "time_left") {
    return true;
  } else if (key == "game_type") {
    return true;
  } else if (key == "rule") {
    return true;
  } else if (key == "evaluate") {
    return true;
  } else if (key == "folder") {
    return true;
  }
  return false;
}