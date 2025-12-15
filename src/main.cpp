#include <string>
#include "Brain.hpp"
#include "HelpMessage.hpp"
#include "Constants.hpp"

int main(int argc, char* argv[]) {
  Brain::Brain brain;
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      std::string arg = argv[i];
      if (arg == "--help" || arg == "-h") {
        try {
          HelpMessage::printHelp();
        } catch (const std::exception& e) {
          std::cerr << "An error occurred while displaying help: " << e.what() << std::endl;
        }
        return Constants::SUCCESS;
      }
    }
    return Constants::ERROR;
  }
  return brain.start();
}
