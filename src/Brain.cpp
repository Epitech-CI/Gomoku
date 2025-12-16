#include "Brain.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include "Constants.hpp"

/**
 * @brief Initializes the Brain instance and starts the input handling thread.
 *
 * This function sets up the command map, marks the engine as running,
 * and launches the main input loop in a separate thread.
 *
 * @return int SUCCESS status code.
 */
int Brain::Brain::start() {
  initializeCommands();
  _running = true;
  _inputHandler = std::thread(&Brain::Brain::inputHandler, this);
  return logicLoop();
}

/**
 * @brief Stops the Brain execution.
 *
 * Stops the running loop and joins the input thread if it is joinable,
 * ensuring a clean shutdown.
 *
 * @return int SUCCESS status code.
 */
int Brain::Brain::stop() {
  _running = false;
  if (_inputHandler.joinable())
    _inputHandler.join();
  return Constants::SUCCESS;
}

int Brain::Brain::logicLoop() {
  while (_running) {
    std::string payload;
    bool doesMessageExist = false;
    {
      std::unique_lock<std::mutex> lock(_queueMutex);
      _cv.wait(lock, [this] { return !_commandQueue.empty() || !_running; });
      if (!_running)
        break;
      payload = _commandQueue.front();
      _commandQueue.pop();
    }
    if (payload.empty())
      continue;
    for (const auto &command : _commands) {
      if (payload.find(command.first) == 0) {
        std::string commandPayload = payload.substr(command.first.size());
        command.second(commandPayload);
        doesMessageExist = true;
        break;
      }
    }
    if (!doesMessageExist) {
      sendUnknown(payload);
    }
  }
  return Constants::SUCCESS;
}

/**
 * @brief Main loop for handling standard input.
 *
 * Continuously reads lines from std::cin. It matches the beginning of the
 * line against registered commands and dispatches the payload to the
 * appropriate handler function.
 *
 * @return int SUCCESS status code.
 */
int Brain::Brain::inputHandler() {
  std::string data;
  _running = true;

  while (_running) {
    std::getline(std::cin, data);
    {
      std::lock_guard<std::mutex> lock(_queueMutex);
      _commandQueue.push(data);
    }
    _cv.notify_one();
  }
  return Constants::SUCCESS;
}

void Brain::Brain::sendResponse(const std::string &response) {
  std::lock_guard<std::mutex> lock(_responseMutex);
  std::cout << response << std::endl;
}

void Brain::Brain::sendOk() {
  sendResponse("OK");
}

void Brain::Brain::sendError(const std::string &errorMessage) {
  sendResponse("ERROR " + errorMessage);
}

void Brain::Brain::sendUnknown(const std::string &message) {
  sendResponse("UNKNOWN " + message);
}

void Brain::Brain::sendMessage(const std::string &message) {
  sendResponse("MESSAGE " + message);
}

void Brain::Brain::sendDebug(const std::string &debugInfo) {
  sendResponse("DEBUG " + debugInfo);
}

/**
 * @brief Handles the START command to initialize the board.
 *
 * Parses the board size requested by the manager. If valid, resizes the
 * internal goban representation.
 *
 * @param payload The string containing the board size argument.
 */
void Brain::Brain::handleStart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("START command received with empty payload or missing terminators.");
    return;
  }
  std::stringstream ss(command);
  int boardSize;
  try {
    ss >> boardSize;
    if (boardSize < Constants::MIN_BOARD_SIZE) {
      sendError("Invalid board size: " + std::to_string(boardSize));
      return;
    }
    _boardSize = std::make_pair(boardSize, boardSize);
    _goban.resize(boardSize * boardSize, '0');
  } catch (...) {
    sendError("Error parsing START command payload: " + command);
    return;
  }
  sendOk();
  sendDebug("Game started with board size: " +
                 std::to_string(_boardSize.first) + "x" +
                 std::to_string(_boardSize.second));
}

/**
 * @brief Handles the TURN command (opponent's move).
 *
 * Parses the X and Y coordinates played by the opponent and updates the
 * internal board state with '2' (indicating the opponent's piece).
 *
 * @param payload The string containing coordinates "X,Y".
 */
void Brain::Brain::handleTurn(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("TURN command received with empty payload or missing terminators.");
    return;
  }
  command[command.find(',')] = ' ';
  std::stringstream ss(command);
  int x, y;
  try {
    ss >> x >> y;
    if (x < 0 || x >= _boardSize.first || y < 0 || y >= _boardSize.second) {
      sendError("Invalid move coordinates: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
      return;
    }
    _goban[y * _boardSize.first + x] = '2';
  } catch (...) {
    sendError("Error parsing TURN command payload");
    return;
  }
  sendDebug("Opponent played at: (" + std::to_string(x) + ", " +
                 std::to_string(y) + ")");
  // TO DO: Implement AI move calculation and respond with the move
}

/**
 * @brief Handles the BEGIN command.
 *
 * Signals that the engine should make the first move.
 *
 * @param payload Command payload (usually empty).
 */
void Brain::Brain::handleBegin(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("BEGIN command received with empty payload or missing terminators.");
    return;
  }
  // TO DO: Choose a move and update _goban accordingly
}

void Brain::Brain::handleBoard(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("BOARD command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Handles the INFO command to update game settings.
 *
 * Parses key-value pairs regarding time limits, memory, rules, etc.,
 * and updates the internal info configuration.
 *
 * @param payload The string containing the key and value.
 */
void Brain::Brain::handleInfo(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    std::cerr
        << "INFO command received with empty payload or missing terminators."
        << std::endl;
    return;
  }
  std::stringstream ss(command);
  std::string key;
  int value;

  try {
    ss >> key;
    if (info.checkKeyExists(key) == false) {
      sendError("Unknown INFO key: " + key);
      return;
    }
    if (key == "evaluate") {
      int x, y;
      ss >> x >> y;
      info.setEvaluate(std::make_pair(x, y));
    } else if (key == "folder") {
      std::string folder;
      ss >> folder;
      info.setFolder(folder);
    } else {
      ss >> value;
      if (key == "timeout_turn")
        info.setTimeoutTurn(value);
      else if (key == "timeout_match")
        info.setTimeoutMatch(value);
      else if (key == "max_memory")
        info.setMaxMemory(value);
      else if (key == "time_left")
        info.setTimeLeft(value);
      else if (key == "game_type")
        info.setGameType(value);
      else if (key == "rule")
        info.setRule(static_cast<char>(value));
    }
  } catch (...) {
    sendError("Error parsing INFO command payload");
    return;
  }
}

/**
 * @brief Handles the END command.
 *
 * Sets the running flag to false to exit the main loop and terminate the
 * engine.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleEnd(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("END command received with empty payload or missing terminators.");
    return;
  }
  _running = false;
}

/**
 * @brief Handles the ABOUT command.
 *
 * Reads from the ABOUT.txt file and prints the engine information to stdout.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleAbout(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("ABOUT command received with empty payload or missing terminators.");
    return;
  }

  std::ifstream aboutFile(Constants::ABOUT_FILE);
  if (aboutFile.is_open()) {
    std::string line;
    while (std::getline(aboutFile, line)) {
      std::cout << line << std::endl;
    }
    aboutFile.close();
  } else {
    sendDebug("Unable to open ABOUT file");
  }
}

/**
 * @brief Handles the RECSTART command for rectangular boards.
 *
 * Parses the width and height from the payload (handling comma separation).
 * Validates the dimensions against the minimum board size. If valid, updates
 * the internal board dimensions and resizes the goban vector to fit
 * the new rectangular configuration.
 *
 * @param payload The string containing the dimensions (e.g., "width,height").
 */
void Brain::Brain::handleRecstart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("RECSTART command received with empty payload or missing terminators.");
    return;
  }
  command[command.find(',')] = ' ';
  std::stringstream ss(command);
  int width, height;
  try {
    ss >> width >> height;
    if (width < Constants::MIN_BOARD_SIZE ||
        height < Constants::MIN_BOARD_SIZE) {
      sendError("Invalid move coordinates: (" + std::to_string(width) + ", " + std::to_string(height) + ")");
      return;
    }
    _boardSize = std::make_pair(width, height);
    _goban.resize(_boardSize.first * _boardSize.second, '0');
  } catch (...) {
    sendError("Error parsing RECSTART command payload");
    return;
  }
  sendDebug("Game started with board size: " +
                 std::to_string(_boardSize.first) + "x" +
                 std::to_string(_boardSize.second));
}

void Brain::Brain::handleRestart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("RESTART command received with empty payload or missing terminators.");
    return;
  }
  try {
    std::fill(_goban.begin(), _goban.end(), '0');
  } catch (...) {
    sendError("Error resetting the board on RESTART command");
    return;
  }
  sendOk();
}

/**
 * @brief Handles the TAKEBACK command.
 *
 * Reverts a move at the specified coordinates by setting the board cell back
 * to '0'. Note: The logic for this was recovered from the mangled handlePlay
 * function.
 *
 * @param payload Coordinates of the move to take back "X,Y".
 */
void Brain::Brain::handleTakeback(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("TAKEBACK command received with empty payload or missing terminators.");
    return;
  }

  std::stringstream ss(command);
  int x, y;
  try {
    ss >> x >> y;
    if (x < 0 || x >= _boardSize.first || y < 0 || y >= _boardSize.second) {
      sendError("Invalid takeback coordinates: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
      return;
    }
    _goban[y * _boardSize.first + x] = '0';
  } catch (...) {
    sendError("Error parsing TAKEBACK command payload: " + command);
    return;
  }
}

void Brain::Brain::handlePlay(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("PLAY command received with empty payload or missing terminators.");
    return;
  }
}

void Brain::Brain::handleSwap2Board(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("SWAP2BOARD command received with empty payload or missing terminators.");
    return;
  }
}

void Brain::Brain::handleError(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("ERROR command received with empty payload or missing terminators.");
    return;
  }
}

void Brain::Brain::handleUnknown(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("UNKNOWN command received with empty payload or missing terminators."); 
    return;
  }
}

void Brain::Brain::handleMessage(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("MESSAGE command received with empty payload or missing terminators.");
    return;
  }
}

void Brain::Brain::handleDebug(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("DEBUG command received with empty payload or missing terminators.");
    return;
  }
}

void Brain::Brain::handleSuggest(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError("SUGGEST command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Registers all supported commands to their respective handler
 * functions.
 *
 * Uses lambdas to bind member functions to the string command keys in the
 * _commands map.
 */
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

/**
 * @brief Checks and strips the command terminator (CR/LF).
 *
 * Verifies if the payload ends with a valid terminator (\r or \n).
 * If valid, it strips the terminator from the string.
 *
 * @param payload The command string to check and modify.
 * @return true If a valid terminator was found and removed.
 * @return false If no terminator was found.
 */
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