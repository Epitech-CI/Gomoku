#pragma once

#include <thread>
#include <unordered_map>
#include "Info.hpp"
#include "Types.hpp"

namespace Brain {
  enum WIN_CONDITION {
    NO_WIN,
    PLAYER_ONE_WIN,
    PLAYER_TWO_WIN,
    DRAW
  };
  class Brain {
    public:
      Brain() = default;
      ~Brain() {
        stop();
      }

      int start();
      int stop();

      // Initialization
      void initializeCommands();

      // Commands handler
      void handleStart(const std::string &payload);
      void handleTurn(const std::string &payload);
      void handleBegin(const std::string &payload);
      void handleBoard(const std::string &payload);
      void handleEnd(const std::string &payload);
      void handleInfo(const std::string &payload);
      void handleAbout(const std::string &payload);
      void handleRecstart(const std::string &payload);
      void handleRestart(const std::string &payload);
      void handleTakeback(const std::string &payload);
      void handlePlay(const std::string &payload);
      void handleSwap2Board(const std::string &payload);
      void handleError(const std::string &payload);
      void handleUnknown(const std::string &payload);
      void handleMessage(const std::string &payload);
      void handleDebug(const std::string &payload);
      void handleSuggest(const std::string &payload);
      void handleDone(const std::string &payload);

      bool checkTerminator(std::string &payload);

      // Multi-threading process
      int inputHandler();
      int logicLoop();

      // Logic responses
      void sendResponse(const std::string &response);
      void sendOk();
      void sendError(const std::string &errorMessage);
      void sendUnknown(const std::string &message);
      void sendMessage(const std::string &message);
      void sendDebug(const std::string &debugInfo);
      void sendCoordinate(int x, int y);

      // Algorithm functions
      std::pair<int, int> minimax(State state, int depth, bool maximizing, int alpha, int beta);
      State getPossibleMoves(const State &state);
      State applyMove(const State &state, int move, int player);
      bool checkWinCondition(const State &state, int player);
      bool isBoardFull(const State &state);
      bool hasNeighbor(const State &state, int index, int range);

    private:
      Info info;
      bool boardIsActivated{false};
      std::pair<int, int> _boardSize{0, 0};
      State _goban;
      std::thread _inputHandler;
      std::atomic<bool> _running{false};
      std::unordered_map<std::string,
                         std::function<void(const std::string &payload)>>
          _commands;
      std::queue<std::string> _commandQueue;
      std::mutex _queueMutex;
      std::mutex _responseMutex;
      std::condition_variable _cv;
  };
}  // namespace Brain
