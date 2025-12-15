#pragma once

#include <utility>
#include <string>

enum GameType {
  HUMAN,
  BRAIN,
  TOURNAMENT,
  NETWORK
};

class Info {
public:
  Info() = default;
  ~Info() = default;

  void setTimeoutTurn(const int tt) {
    timeoutTurn = tt;
  }

  void setTimeoutMatch(const int tm) {
    timeoutMatch = tm;
  }

  void setMaxMemory(const int mm) {
    maxMemory = mm;
  }

  void setTimeLeft(const int tl) {
    timeLeft = tl;
  }

  void setGameType(const int &gt) {
    switch (gt) {
      case 0:
        gameType = HUMAN;
        break;
      case 1:
        gameType = BRAIN;
        break;
      case 2:
        gameType = TOURNAMENT;
        break;
      case 3:
        gameType = NETWORK;
        break;
    }
  }

  void setRule(const char r) {
    rule = r;
  }

  void setEvaluate(const std::pair<int, int> coord) {
    evaluate = coord;
  }

  void setFolder(const std::string &f) {
    folder = f;
  }

  const int &getTimeoutTurn() const {
    return timeoutTurn;
  }

  const int &getTimeoutMatch() const {
    return timeoutMatch;
  }

  const int &getMaxMemory() const {
    return maxMemory;
  }

  const int &getTimeLeft() const {
    return timeLeft;
  }

  const GameType &getGameType() const {
    return gameType;
  }

  const char getRule() const {
    return rule;
  }

  const std::pair<int, int> &getEvaluate() const {
    return evaluate;
  }

  const std::string &getFolder() const {
    return folder;
  }

  bool checkKeyExists(const std::string &key) const;

private:
  int timeoutTurn{0};
  int timeoutMatch{0};
  int maxMemory{0};
  int timeLeft{0};
  GameType gameType{HUMAN};
  char rule{0};
  std::pair<int, int> evaluate{0, 0};
  std::string folder;
};
