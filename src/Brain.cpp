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

/**
 * @brief Main logic loop of the engine.
 *
 * Continuously retrieves and processes commands from the internal command queue
 * while the engine is running.
 *
 * @return int SUCCESS status code.
 */
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
    if (payload.find("DONE") == 0) {
      boardIsActivated = false;
    }
    if (boardIsActivated) {
      handleBoard(payload);
      continue;
    }
    for (const auto &command : _commands) {
      if (payload.find(command.first) == 0) {
        std::string commandPayload = payload.substr(command.first.size());
        if (command.first == "BOARD") {
          boardIsActivated = true;
          doesMessageExist = true;
          break;
        }
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

/**
 * @brief Sends a generic response to the standard output.
 *
 * @param response The string message to be printed.
 */
void Brain::Brain::sendResponse(const std::string &response) {
  std::lock_guard<std::mutex> lock(_responseMutex);
  std::cout << response << std::endl;
}

/**
 * @brief Sends an "OK" confirmation to the manager.
 */
void Brain::Brain::sendOk() {
  sendResponse("OK");
}

/**
 * @brief Sends an error message to the manager.
 *
 * @param errorMessage The description of the error.
 */
void Brain::Brain::sendError(const std::string &errorMessage) {
  sendResponse("ERROR " + errorMessage);
}

/**
 * @brief Sends an "UNKNOWN" response for unrecognized commands.
 *
 * @param message The content of the unrecognized command.
 */
void Brain::Brain::sendUnknown(const std::string &message) {
  sendResponse("UNKNOWN " + message);
}

/**
 * @brief Sends an informative message to the manager.
 *
 * @param message The text message to send.
 */
void Brain::Brain::sendMessage(const std::string &message) {
  sendResponse("MESSAGE " + message);
}

/**
 * @brief Sends debug information to the manager.
 *
 * @param debugInfo The debug string to send.
 */
void Brain::Brain::sendDebug(const std::string &debugInfo) {
  sendResponse("DEBUG " + debugInfo);
}

/**
 * @brief Sends coordinates to the manager as the engine's move.
 *
 * Validates that coordinates are non-negative before sending.
 *
 * @param x The X-coordinate.
 * @param y The Y-coordinate.
 */
void Brain::Brain::sendCoordinate(int x, int y) {
  if (x < 0 || y < 0) {
    sendError("No valid move found (minimax returned negative index)");
    return;
  }
  sendResponse(std::to_string(x) + "," + std::to_string(y));
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
    sendError(
        "START command received with empty payload or missing terminators.");
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
    _goban.resize(boardSize * boardSize, 0);
  } catch (...) {
    sendError("Error parsing START command payload: " + command);
    return;
  }
  sendOk();
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
    sendError(
        "TURN command received with empty payload or missing terminators.");
    return;
  }
  command[command.find(',')] = ' ';
  std::stringstream ss(command);
  int x, y;
  try {
    ss >> x >> y;
    if (x < 0 || x >= _boardSize.first || y < 0 || y >= _boardSize.second) {
      sendError("Invalid move coordinates: (" + std::to_string(x) + ", " +
                std::to_string(y) + ")");
      return;
    }
    _goban[y * _boardSize.first + x] = 2;
  } catch (...) {
    sendError("Error parsing TURN command payload");
    return;
  }
  auto result = minimax(_goban, 3, true, std::numeric_limits<int>::min(),
                        std::numeric_limits<int>::max());
  sendCoordinate(result.second % _boardSize.first,
                 result.second / _boardSize.first);
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
    sendError(
        "BEGIN command received with empty payload or missing terminators.");
    return;
  }
  auto result = minimax(_goban, 3, true, std::numeric_limits<int>::min(),
                        std::numeric_limits<int>::max());
  if (checkAlgorithmReturn(result) == false)
    return;
  _goban[result.second] = 1;
  sendCoordinate(result.second % _boardSize.first,
                 result.second / _boardSize.first);
}

/**
 * @brief Handles the BOARD command payload for batch move updates.
 *
 * Updates the board state based on the provided coordinates and player ID.
 *
 * @param payload The coordinate and player information string "X,Y,P".
 */
void Brain::Brain::handleBoard(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "BOARD command received with empty payload or missing terminators.");
    return;
  }
  for (char &ch : command) {
    if (ch == ',') {
      ch = ' ';
    }
  }
  std::stringstream ss(command);
  int x, y, player;
  ss >> x >> y >> player;
  if (x < 0 || x >= _boardSize.first || y < 0 || y >= _boardSize.second) {
    sendError("Invalid BOARD coordinates: (" + std::to_string(x) + ", " +
              std::to_string(y) + ")");
    return;
  }
  if (player == 3) {
    sendError("Invalid player number in BOARD command: " +
              std::to_string(player));
    return;
  }
  _goban[y * _boardSize.first + x] = player;
  for (int i = 0; i < _boardSize.first; i++) {
    for (int j = 0; j < _boardSize.second; j++) {
      std::cerr << _goban[i * _boardSize.first + j] << " ";
    }
    std::cerr << std::endl;
  }
  std::cerr << "----" << std::endl;
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
    sendError(
        "END command received with empty payload or missing terminators.");
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
    sendError(
        "ABOUT command received with empty payload or missing terminators.");
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
    sendError(
        "RECSTART command received with empty payload or missing terminators.");
    return;
  }
  command[command.find(',')] = ' ';
  std::stringstream ss(command);
  int width, height;
  try {
    ss >> width >> height;
    if (width < Constants::MIN_BOARD_SIZE ||
        height < Constants::MIN_BOARD_SIZE) {
      sendError("Invalid move coordinates: (" + std::to_string(width) + ", " +
                std::to_string(height) + ")");
      return;
    }
    _boardSize = std::make_pair(width, height);
    _goban.resize(_boardSize.first * _boardSize.second, 0);
  } catch (...) {
    sendError("Error parsing RECSTART command payload");
    return;
  }
  sendOk();
  sendDebug(
      "Game started with board size: " + std::to_string(_boardSize.first) +
      "x" + std::to_string(_boardSize.second));
}

/**
 * @brief Handles the RESTART command.
 *
 * Clears the board by filling it with zeros and sends an "OK" response.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleRestart(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "RESTART command received with empty payload or missing terminators.");
    return;
  }
  try {
    std::fill(_goban.begin(), _goban.end(), 0);
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
    sendError(
        "TAKEBACK command received with empty payload or missing terminators.");
    return;
  }

  std::stringstream ss(command);
  int x, y;
  try {
    ss >> x >> y;
    if (x < 0 || x >= _boardSize.first || y < 0 || y >= _boardSize.second) {
      sendError("Invalid takeback coordinates: (" + std::to_string(x) + ", " +
                std::to_string(y) + ")");
      return;
    }
    _goban[y * _boardSize.first + x] = 0;
  } catch (...) {
    sendError("Error parsing TAKEBACK command payload: " + command);
    return;
  }
}

/**
 * @brief Processes the PLAY command from the manager.
 *
 * @param payload Command payload containing move data.
 */
void Brain::Brain::handlePlay(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "PLAY command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Processes the SWAP2BOARD command from the manager.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleSwap2Board(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "SWAP2BOARD command received with empty payload or missing "
        "terminators.");
    return;
  }
}

/**
 * @brief Processes an ERROR message received from the manager.
 *
 * @param payload Command payload containing error info.
 */
void Brain::Brain::handleError(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "ERROR command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Processes an UNKNOWN command notification from the manager.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleUnknown(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "UNKNOWN command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Processes a MESSAGE command from the manager.
 *
 * @param payload Command payload content.
 */
void Brain::Brain::handleMessage(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "MESSAGE command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Processes a DEBUG message from the manager.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleDebug(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "DEBUG command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Processes a SUGGEST command from the manager.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleSuggest(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "SUGGEST command received with empty payload or missing terminators.");
    return;
  }
}

/**
 * @brief Handles the final DONE command of the BOARD protocol sequence.
 *
 * Resets the board activation state and triggers the minimax algorithm to
 * find and send the best move.
 *
 * @param payload Command payload.
 */
void Brain::Brain::handleDone(const std::string &payload) {
  std::string command = payload;
  if (checkTerminator(command) == false) {
    sendError(
        "DONE command received with empty payload or missing terminators.");
    return;
  }
  boardIsActivated = false;
  auto result = minimax(_goban, 5, true, std::numeric_limits<int>::min(),
                        std::numeric_limits<int>::max());
  sendCoordinate(result.second % _boardSize.first,
                 result.second / _boardSize.first);
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
  _commands["DONE"] = [this](const std::string &p) { this->handleDone(p); };
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

/**
 * @brief Core AI algorithm for determining the best move.
 *
 * Implements a recursive minimax search with alpha-beta pruning to find the
 * optimal cell index based on the current board state and desired search depth.
 *
 * @param state Current representation of the board.
 * @param depth Remaining recursion depth.
 * @param maximizingPlayer Boolean indicating if it is the engine's turn to
 * maximize score.
 * @param alpha The alpha value for pruning.
 * @param beta The beta value for pruning.
 * @return std::pair<int, int> A pair containing the evaluation score and the
 * best move index.
 */
std::pair<int, int> Brain::Brain::minimax(State state, int depth,
                                          bool maximizingPlayer, int alpha,
                                          int beta) {
  if (checkWinCondition(state, 1)) {
    return {PLAYER_ONE_WIN, 0};
  } else if (checkWinCondition(state, 2)) {
    return {PLAYER_TWO_WIN, 0};
  } else if (depth == 0 || isBoardFull(state)) {
    return {DRAW, DRAW};
  }

  int bestMoveFound = -1;

  if (maximizingPlayer) {
    int maxEval = std::numeric_limits<int>::min();
    State possibleMoves = getPossibleMoves(state);
    if (possibleMoves.empty()) {
      return {DRAW, DRAW};
    }
    for (int move : possibleMoves) {
      State newState = applyMove(state, move, 1);
      int eval = minimax(newState, depth - 1, false, alpha, beta).first;
      if (eval > maxEval) {
        maxEval = eval;
        bestMoveFound = move;
      }
      alpha = std::max(alpha, eval);
      if (beta <= alpha) {
        break;
      }
    }
    if (bestMoveFound < 0 && !possibleMoves.empty())
      bestMoveFound = possibleMoves[0];
    return {maxEval, bestMoveFound};
  } else {
    int minEval = std::numeric_limits<int>::max();
    State possibleMoves = getPossibleMoves(state);
    if (possibleMoves.empty()) {
      return {DRAW, DRAW};
    }
    for (int move : possibleMoves) {
      State newState = applyMove(state, move, 2);
      int eval = minimax(newState, depth - 1, true, alpha, beta).first;
      if (eval < minEval) {
        minEval = eval;
        bestMoveFound = move;
      }
      beta = std::min(beta, eval);
      if (beta <= alpha) {
        break;
      }
    }
    if (bestMoveFound < 0 && !possibleMoves.empty())
      bestMoveFound = possibleMoves[0];
    return {minEval, bestMoveFound};
  }
}

/**
 * @brief Checks if every cell on the board is occupied.
 *
 * @param state The current board state.
 * @return true If no '0' (empty) cells are found.
 * @return false If at least one cell is empty.
 */
bool Brain::Brain::isBoardFull(const State &state) {
  for (int cell : state) {
    if (cell == 0) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Evaluates if a specific player has achieved a winning line.
 *
 * Scans the board for five consecutive pieces of the same player horizontally,
 * vertically, or diagonally.
 *
 * @param state The current board state.
 * @param player The ID of the player to check (1 or 2).
 * @return true If the player has five in a row.
 * @return false Otherwise.
 */
bool Brain::Brain::checkWinCondition(const State &state, int player) {
  for (int i = 0; i < _boardSize.second; ++i) {
    for (int j = 0; j < _boardSize.first; ++j) {
      if (state[i * _boardSize.first + j] == player) {
        if (j <= _boardSize.first - 5) {
          if (state[i * _boardSize.first + j + 1] == player &&
              state[i * _boardSize.first + j + 2] == player &&
              state[i * _boardSize.first + j + 3] == player &&
              state[i * _boardSize.first + j + 4] == player) {
            return true;
          }
        }
        if (i <= _boardSize.second - 5) {
          if (state[(i + 1) * _boardSize.first + j] == player &&
              state[(i + 2) * _boardSize.first + j] == player &&
              state[(i + 3) * _boardSize.first + j] == player &&
              state[(i + 4) * _boardSize.first + j] == player) {
            return true;
          }
        }
        if (i <= _boardSize.second - 5 && j <= _boardSize.first - 5) {
          if (state[(i + 1) * _boardSize.first + j + 1] == player &&
              state[(i + 2) * _boardSize.first + j + 2] == player &&
              state[(i + 3) * _boardSize.first + j + 3] == player &&
              state[(i + 4) * _boardSize.first + j + 4] == player) {
            return true;
          }
        }
        if (i >= 4 && j <= _boardSize.first - 5) {
          if (state[(i - 1) * _boardSize.first + j + 1] == player &&
              state[(i - 2) * _boardSize.first + j + 2] == player &&
              state[(i - 3) * _boardSize.first + j + 3] == player &&
              state[(i - 4) * _boardSize.first + j + 4] == player) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

/**
 * @brief Creates a copy of the board state and applies a move.
 *
 * @param state The original board state.
 * @param move The index where the piece should be placed.
 * @param player The player ID placing the piece.
 * @return State The modified board state.
 */
State Brain::Brain::applyMove(const State &state, int move, int player) {
  State newState = state;
  newState[move] = player;
  return newState;
}

/**
 * @brief Identifies valid cell indices for the next move.
 *
 * Focuses on empty cells that are adjacent to already occupied cells to
 * optimize search time.
 *
 * @param state The current board state.
 * @return State A list of indices representing potential moves.
 */
State Brain::Brain::getPossibleMoves(const State &state) {
  State moves;
  int proximity_range = 1;

  for (int i = 0; i < state.size(); ++i) {
    if (state[i] == 0) {
      if (hasNeighbor(state, i, proximity_range)) {
        moves.push_back(i);
      }
    }
  }
  if (moves.empty()) {
    int center =
        (_boardSize.second / 2) * _boardSize.first + (_boardSize.first / 2);
    moves.push_back(center);
  }
  return moves;
}

/**
 * @brief Helper to check if a specific cell has occupied neighbors.
 *
 * Scans the immediate vicinity of a cell within a specified range.
 *
 * @param state The board state.
 * @param index The index of the cell to check.
 * @param range The search radius.
 * @return true If at least one neighbor is not '0'.
 * @return false Otherwise.
 */
bool Brain::Brain::hasNeighbor(const State &state, int index, int range) {
  int x = index / _boardSize.first;
  int y = index % _boardSize.first;

  for (int dx = -range; dx <= range; ++dx) {
    for (int dy = -range; dy <= range; ++dy) {
      if (dx == 0 && dy == 0)
        continue;
      int nx = x + dx;
      int ny = y + dy;
      if (nx >= 0 && nx < _boardSize.second && ny >= 0 &&
          ny < _boardSize.first) {
        if (state[nx * _boardSize.first + ny] != 0) {
          return true;
        }
      }
    }
  }
  return false;
}

bool Brain::Brain::checkAlgorithmReturn(std::pair<int, int> index) {
  if (index.first == DRAW) {
    sendError("No valid move found (minimax returned DRAW)");
    return false;
  }
  if (index.second == PLAYER_ONE_WIN || index.second == PLAYER_TWO_WIN) {
    sendError("No valid move found (minimax returned PLAYER_WIN)");
    return false;
  }
  if (index.second >= _goban.size() || index.second < 0) {
    sendError("No valid move found (minimax returned invalid index)");
    return false;
  }
  return true;
}