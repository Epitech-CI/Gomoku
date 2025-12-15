#pragma once

#include <fstream>
#include <iostream>

namespace HelpMessage {
  inline void printHelp() {
    std::ifstream helpFile(Constants::HELP_MESSAGE_FILE);
    if (helpFile.is_open()) {
      std::string line;
      while (std::getline(helpFile, line)) {
        std::cout << line << std::endl;
      }
      helpFile.close();
    } else {
      std::cerr << "Error: Unable to open help message file." << std::endl;
    }
  }
}  // namespace HelpMessage
