NAME := Matt_daemon
SRCS_DIR := srcs/
OBJS_DIR := objs/
INCS_DIR := includes/

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++20 -I$(INCS_DIR)

ifeq ($(DEBUG),true)
    CXXFLAGS += -DDEBUG
    LOCK_FILE := /tmp/matt_daemon.lock
    LOG_FILE := /tmp/matt_daemon.log
else
	LOCK_FILE := /var/lock/matt_daemon.lock
	LOG_FILE := /var/log/matt_daemon/matt_daemon.log
endif

SRCS := $(wildcard $(SRCS_DIR)*.cpp)
HEADERS := $(wildcard $(INCS_DIR)*.hpp)
FILENAMES := $(basename $(SRCS))
FOLDERS := $(sort $(dir $(SRCS)))
OBJS := $(FILENAMES:$(SRCS_DIR)%=$(OBJS_DIR)%.o)

RM := rm -rf
MKDIR := mkdir -p
VALGRIND := valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes -q

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
	@$(RM) $(OBJS_DIR)

kill:
	sudo pkill -9 $(NAME) || true
	sudo $(RM) $(LOCK_FILE) || true
	sudo $(RM) $(LOG_FILE) || true
	sudo $(RM) /tmp/matt_daemon* || true

term:
	sudo pkill -15 $(NAME) || true
	sudo $(RM) $(LOCK_FILE) || true
	sudo $(RM) $(LOG_FILE) || true
	sudo $(RM) /tmp/matt_daemon* || true

fclean: clean kill
	@echo "Removing $(NAME)"
	@$(RM) $(NAME) $(BONUS)

re: fclean
	@$(MAKE) all

run:
	$(MAKE) all --no-print-directory
	sudo ./$(NAME)
	$(MAKE) logs --no-print-directory

rerun: fclean run

debug:
	$(MAKE) all DEBUG=true --no-print-directory
	$(VALGRIND) ./$(NAME)

redebug: fclean debug

info:
	@ps aux | grep '[M]att_daemon' || true
	@ls -lah $(LOCK_FILE) || true
	@ls -lah $(LOG_FILE) || true

logs:
	@sudo tail -f $(LOG_FILE)

rmvagrant: fclean
	$(RM) *VBox*.log
	vagrant destroy -f

.PHONY: all clean kill fclean re run rerun debug redebug info logs rmvagrant