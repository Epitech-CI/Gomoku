#pragma once

#include <iostream>
#include <fstream>

namespace HelpMessage {
  inline void printHelp() {
    std::ifstream helpFile("./src/HELP_MESSAGE.txt");
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
}
