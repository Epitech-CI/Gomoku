#include "Brain.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include "Constants.hpp"

int Brain::Brain::start() {
  initializeCommands();
  _running = true;
  _inputHandler = std::thread(&Brain::Brain::inputHandler, this);
  return Constants::SUCCESS;
}

int Brain::Brain::stop() {
  _running = false;
  if (_inputHandler.joinable())
    _inputHandler.join();
  return Constants::SUCCESS;
}

int Brain::Brain::inputHandler() {
  std::string data;
  _running = true;

  while (_running) {
    std::getline(std::cin, data);
    if (data.empty()) {
      continue;
    }
    for (const auto &command : _commands) {
      if (data.find(command.first) == 0) {
        std::string payload = data.substr(command.first.size());
        command.second(payload);
        break;
      }
    }
  }
  return Constants::SUCCESS;
}

void Brain::Brain::handleStart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "START command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
  std::stringstream ss(command);
  try {
    ss >> _boardSize;
    if (_boardSize < Constants::MIN_BOARD_SIZE) {
      std::cerr << "Invalid board size: " << _boardSize << std::endl;
      return;
    }
    _goban.resize(_boardSize * _boardSize, '0');
  } catch (...) {
    std::cerr << "Error parsing START command payload: " << command
              << std::endl;
    return;
  }
  std::cout << "Game started with board size: " << _boardSize << std::endl;
}

void Brain::Brain::handleTurn(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "TURN command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
  command[command.find(',')] = ' ';
  std::stringstream ss(command);
  int x, y;
  try {
    ss >> x >> y;
    if (x < 0 || x >= _boardSize || y < 0 || y >= _boardSize) {
      std::cerr << "Invalid move coordinates: (" << x << ", " << y << ")"
                << std::endl;
      return;
    }
    _goban[y * _boardSize + x] = '2';
  } catch (...) {
    std::cerr << "Error parsing TURN command payload: " << command << std::endl;
    return;
  }
  std::cout << "Opponent played at: (" << x << ", " << y << ")" << std::endl;
}

void Brain::Brain::handleBegin(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "BEGIN command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleBoard(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "BOARD command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleInfo(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "INFO command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleEnd(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "END command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
  _running = false;
}

void Brain::Brain::handleAbout(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "ABOUT command received with empty payload or missing terminators."
        << std::endl;
    return;
  }

  std::ifstream aboutFile("./src/ABOUT.txt");
  if (aboutFile.is_open()) {
    std::string line;
    while (std::getline(aboutFile, line)) {
      std::cout << line << std::endl;
    }
    aboutFile.close();
  } else {
    std::cerr << "Error: Unable to open help message file." << std::endl;
  }
}

void Brain::Brain::handleRecstart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr << "RECSTART command received with empty payload or missing "
                 "terminators."
              << std::endl;
    return;
  }
}

void Brain::Brain::handleRestart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "RESTART command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleTakeback(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr << "TAKEBACK command received with empty payload or missing "
                 "terminators."
              << std::endl;
    return;
  }
}

void Brain::Brain::handlePlay(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "PLAY command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleSwap2Board(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr << "SWAP2BOARD command received with empty payload or missing "
                 "terminators."
              << std::endl;
    return;
  }
}

void Brain::Brain::handleError(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "ERROR command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleUnknown(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "UNKNOWN command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleMessage(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "MESSAGE command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleDebug(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "DEBUG command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::handleSuggest(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "SUGGEST command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
}

void Brain::Brain::initializeCommands() {
  _commands["START"] = [this](const std::string &p) { this->handleStart(p); };
  _commands["TURN"] = [this](const std::string &p) { this->handleTurn(p); };
  _commands["BEGIN"] = [this](const std::string &p) { this->handleBegin(p); };
  _commands["BOARD"] = [this](const std::string &p) { this->handleBoard(p); };
  _commands["INFO"] = [this](const std::string &p) { this->handleInfo(p); };
  _commands["END"] = [this](const std::string &p) { this->handleEnd(p); };
  _commands["ABOUT"] = [this](const std::string &p) { this->handleAbout(p); };
  _commands["RECSTART"] = [this](const std::string &p) {
    this->handleRecstart(p);
  };
  _commands["RESTART"] = [this](const std::string &p) {
    this->handleRestart(p);
  };
  _commands["TAKEBACK"] = [this](const std::string &p) {
    this->handleTakeback(p);
  };
  _commands["PLAY"] = [this](const std::string &p) { this->handlePlay(p); };
  _commands["SWAP2BOARD"] = [this](const std::string &p) {
    this->handleSwap2Board(p);
  };
  _commands["UNKNOWN"] = [this](const std::string &p) {
    this->handleUnknown(p);
  };
  _commands["ERROR"] = [this](const std::string &p) { this->handleError(p); };
  _commands["MESSAGE"] = [this](const std::string &p) {
    this->handleMessage(p);
  };
  _commands["DEBUG"] = [this](const std::string &p) { this->handleDebug(p); };
  _commands["SUGGEST"] = [this](const std::string &p) {
    this->handleSuggest(p);
  };
}

bool Brain::Brain::checkTerminator(std::string &payload) {
  if (payload[payload.size() - 1] == 0x0d ||
      payload[payload.size() - 1] == 0x0a) {
    size_t end = payload.find_last_not_of("\r\n");
    if (end != std::string::npos) {
      payload = payload.substr(0, end + 1);
    } else {
      payload.clear();
    }
    return true;
  }
  return false;
}