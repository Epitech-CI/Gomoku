#pragma once

#include <thread>
#include <unordered_map>
#include "Info.hpp"

namespace Brain {
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

      bool checkTerminator(std::string &payload);

      // Multi-threading process
      int inputHandler();

    private:
      Info info;
      int _boardSize{0};
      std::vector<char> _goban;
      std::thread _inputHandler;
      std::atomic<bool> _running{false};
      std::unordered_map<std::string,
                         std::function<void(const std::string &payload)>>
          _commands;
  };
}  // namespace Brain
