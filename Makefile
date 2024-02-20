NAME := Matt_daemon
SRCS_DIR := srcs/
OBJS_DIR := objs/
INCS_DIR := includes/

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++20 -I$(INCS_DIR)

SRCS		+= srcs/main.cpp
SRCS		+= srcs/crash.cpp
SRCS		+= srcs/Server.cpp
SRCS		+= srcs/Client.cpp

HEADERS		+= includes/matt_daemon.hpp
HEADERS		+= includes/Client.hpp
HEADERS		+= includes/Server.hpp

FILENAMES	:= $(basename $(SRCS))
FOLDERS 	:= $(sort $(dir $(SRCS)))
OBJS		:= $(FILENAMES:$(SRCS_DIR)%=$(OBJS_DIR)%.o)

RM			:= rm -rf
MKDIR		:= mkdir -p

END			:= \033[0m
RED			:= \033[31m
GREEN		:= \033[32m
BLUE		:= \033[34m

all: $(NAME)

$(OBJS_DIR):
	$(MKDIR) $(FOLDERS:$(SRCS_DIR)%=$(OBJS_DIR)%)

$(OBJS): $(OBJS_DIR)%.o: $(SRCS_DIR)%.cpp $(HEADERS) | $(OBJS_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "$(GREEN)✓ $@$(END)"

$(NAME) $(BONUS): $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(CXXLIBS)
	@echo "$(BLUE)$@ is compiled.$(END)"

clean:
	@echo "Removing $(OBJS_DIR)"
	@$(RM) $(OBJS_DIR) ${GARBAGE}

kill:
	sudo pkill -9 $(NAME) || true
	sudo rm -f /run/matt_daemon.lock
	sudo rm -f /run/matt_daemon.pid
	sudo rm -f /var/log/matt_daemon.log

fclean: clean kill
	@echo "Removing $(NAME)"
	@$(RM) $(NAME) $(BONUS)

re: fclean
	@$(MAKE) all

run:
	$(MAKE) all --no-print-directory
	sudo ./$(NAME)
	$(MAKE) clean --no-print-directory

rerun: fclean run

info:
	@ps aux | grep '[M]att_daemon' || true
	@ls -lah /run | grep matt_daemon || true

logs:
	@sudo tail -f /var/log/syslog | grep 'Matt_daemon'

.PHONY: all clean kill fclean re run info logs