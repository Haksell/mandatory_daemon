NAME := Matt_daemon
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror --std=c++20

all: $(NAME)

$(NAME): main.o
	$(CXX) -o $@ $^
	chmod 700 $(NAME)

main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) $<

clean:
	rm -f *.o

kill:
	sudo pkill -f $(NAME)
	sudo rm -f /run/matt_daemon.lock
	sudo rm -f /run/matt_daemon.pid

fclean: clean kill
	rm -f $(NAME)

re: fclean all

run:
	$(MAKE) all --no-print-directory
	sudo ./$(NAME)
	$(MAKE) fclean --no-print-directory
