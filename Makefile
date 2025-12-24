##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Build the Gomoku AI project
##

NAME = pbrain-gomoku-ai

BUILD_DIR = .build

DEBUG_BUILD_DIR = .build-debug

MAKEFLAGS += --no-print-directory

GREEN = \033[0;32m
RESET = \033[0m

all: cmake

debug:
	@cmake -B $(DEBUG_BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	@cmake --build $(DEBUG_BUILD_DIR) --parallel
	@echo "$(GREEN)[ OK ]$(RESET) Debug build compiled"

cmake:
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR) --parallel
	@echo "$(GREEN)[ OK ]$(RESET) Files compiled"

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DEBUG_BUILD_DIR)
	@rm -rf .cache

fclean: clean
	@rm -f $(NAME)
	@rm -f $(NAME)-debug
	@rm -f compile_commands.json

re: fclean
	@cmake -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --parallel

.PHONY: all debug cmake clean fclean re
