#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
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

      // Multi-threading process
      int inputHandler();

      // Logic responses
      void sendResponse(const std::string &response);
      void sendOk();
      void sendError(const std::string &errorMessage);
      void sendUnknown(const std::string &message);
      void sendMessage(const std::string &message);
      void sendDebug(const std::string &debugInfo);
      void sendCoordinate(int x, int y);

      // Algorithm functions
      void findBestMove();
      std::pair<int, std::size_t> minimax(State &state, int depth,
                                          bool maximizing, int alpha, int beta);
      State getPossibleMoves(const State &state);
      bool checkWinCondition(const State &state, int player);
      bool isBoardFull(const State &state);
      bool hasNeighbor(const State &state, int index, int range);
      int evaluate(const State &state, int player);
      int countPatterns(const State &state, int player);
      bool checkAlgorithmReturn(std::pair<std::size_t, std::size_t> index);

    private:
      Info info;
      bool boardIsActivated{false};
      std::pair<int, int> _boardSize{0, 0};
      State _goban;
      bool _running{false};
      std::unordered_map<std::string,
                         std::function<void(const std::string &payload)>>
          _commands;
      std::condition_variable _cv;
      std::chrono::steady_clock::time_point _startTime;
      bool _timeUp{false};
  };
}  // namespace Brain
