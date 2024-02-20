NAME := Matt_daemon
SRCS_DIR := srcs/
OBJS_DIR := objs/
INCS_DIR := includes/

LOCK_FILE := /run/matt_daemon.lock
LOG_FILE := /var/log/matt_daemon.log
PID_FILE := /run/matt_daemon.pid

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++20 -I$(INCS_DIR)

SRCS := $(wildcard $(SRCS_DIR)*.cpp)
HEADERS := $(wildcard $(INCS_DIR)*.cpp)
FILENAMES := $(basename $(SRCS))
FOLDERS := $(sort $(dir $(SRCS)))
OBJS := $(FILENAMES:$(SRCS_DIR)%=$(OBJS_DIR)%.o)

RM := rm -rf
MKDIR := mkdir -p

END := \033[0m
RED := \033[31m
GREEN := \033[32m
BLUE := \033[34m

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
	sudo rm -f $(LOCK_FILE)
	sudo rm -f $(LOG_FILE)
	sudo rm -f $(PID_FILE)

fclean: clean kill
	@echo "Removing $(NAME)"
	@$(RM) $(NAME) $(BONUS)

re: fclean
	@$(MAKE) all

run:
	$(MAKE) all --no-print-directory
	sudo ./$(NAME)
	$(MAKE) clean --no-print-directory
	$(MAKE) client_logs --no-print-directory

rerun: fclean run

info:
	@ps aux | grep '[M]att_daemon' || true
	@ls -lah /run | grep matt_daemon || true

sys_logs:
	@sudo tail -f /var/log/syslog | grep 'Matt_daemon'

client_logs:
	@sudo tail -f $(LOG_FILE)

.PHONY: all clean kill fclean re run info sys_logs client_logs