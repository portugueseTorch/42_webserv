# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: touteiro <touteiro@student.42lisboa.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/28 15:46:49 by gda_cruz          #+#    #+#              #
#    Updated: 2023/09/07 20:55:44 by touteiro         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

####### COLORS #######
RED		=	\033[0;31m
GREEN_B	=	\033[1;32m
BLUE	=	\033[0;34m
RESET	=	\033[0m

####### COMPILER #######
CC		=	c++
FLAGS	=	-Wall -Wextra -Werror -std=c++98
FLAGS	+=	-fsanitize=address -g

####### DIRECTORIES #######
OBJ_DIR	=	./obj
INC_DIR	=	inc

####### FILES #######
SRC		=	$(shell find src/ -name '*.cpp')
# SRC		=	src/main.cpp \
# 			src/HTTPRequest/HTTPRequest.cpp \
# 			src/HTTPRequest/Lexer.cpp \
# 			src/HTTPRequest/Parser.cpp \
# 			src/utils/utils.cpp
OBJ		=	$(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
NAME 	=	webserv

all: $(NAME)

debug: $(NAME)
	@./webserv > out.log
	@printf "INFO:\tOutput log redirected to file out.log\n"

$(NAME): $(OBJ)
	@printf "$(BLUE)[Compiling]     "$(NAME)"$(RESET)\n"
	@$(CC) $(OBJ) $(FLAGS) -o $(NAME)
	@printf "$(GREEN_B)[$(NAME) ready to use]\n$(RESET)"

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(eval TMP := $(shell echo $@ | sed 's|\(.*\)/.*|\1|'))
	@mkdir -p $(TMP)
	@$(CC) $(FLAGS) -I$(INC_DIR) -c $< -o $@
	@printf "$(BLUE)[Compiling]     "$@"$(RESET)\n"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -rf $(OBJ_DIR) $(NAME)
	@echo "$(RED)[All $(NAME) files removed]$(RESET)"

re: fclean all

.PHONY: all clean fclean re