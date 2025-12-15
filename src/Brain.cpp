#include "Brain.hpp"
#include <iostream>
#include <string>
#include "Constants.hpp"

int Brain::Brain::start() {
  _inputHandler = std::thread(&Brain::Brain::inputHandler, this);
  return Constants::SUCCESS;
}

int Brain::Brain::stop() {
  if (_inputHandler.joinable()) {
    _inputHandler.join();
  }
  return Constants::SUCCESS;
}

int Brain::Brain::inputHandler() {
  std::string data;
  while (std::getline(std::cin, data)) {
  }
  return Constants::SUCCESS;
}

void Brain::Brain::handleStart(const std::string &payload) {
}

void Brain::Brain::handleTurn(const std::string &payload) {
}

void Brain::Brain::handleBegin(const std::string &payload) {
}

void Brain::Brain::handleBoard(const std::string &payload) {
}

void Brain::Brain::handleInfo(const std::string &payload) {
}

void Brain::Brain::handleEnd(const std::string &payload) {
}

void Brain::Brain::handleAbout(const std::string &payload) {
}

void Brain::Brain::handleRecstart(const std::string &payload) {
}

void Brain::Brain::handleRestart(const std::string &payload) {
}

void Brain::Brain::handleTakeback(const std::string &payload) {
}

void Brain::Brain::handlePlay(const std::string &payload) {
}

void Brain::Brain::handleSwap2Board(const std::string &payload) {
}

void Brain::Brain::handleError(const std::string &payload) {
}

void Brain::Brain::handleUnknown(const std::string &payload) {
}

void Brain::Brain::handleMessage(const std::string &payload) {
}

void Brain::Brain::handleDebug(const std::string &payload) {
}

void Brain::Brain::handleSuggest(const std::string &payload) {
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
