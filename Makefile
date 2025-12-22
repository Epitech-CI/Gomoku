##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Build the Gomoku AI project
##

CXXFLAGS = -std=c++20 -Wall -Wextra -I src/

SRC = src/Brain.cpp src/Info.cpp src/main.cpp
NAME = pbrain-gomoku-ai

all: $(NAME)

$(NAME): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(SRC)

clean:
	rm -f $(NAME)

fclean: clean

re: fclean all

.PHONY: all clean fclean re