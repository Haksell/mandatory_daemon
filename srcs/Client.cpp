#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

Client::Client(Server* server, int socketFd, sockaddr_in clientAddress)
	: _clientSocket(socketFd), _clientAddress(clientAddress), _server(server) {}

Client::~Client() {
	close(_clientSocket);
	std::cout << BLUE << "Client " << _clientSocket << " disconnected." << RESET
			  << std::endl;
}

int Client::getSocket() const { return _clientSocket; }

sockaddr_in Client::getClientAddress() const { return _clientAddress; }

Server* Client::getServer() const { return _server; }

void Client::reply(std::string replyMessage) const {
	replyMessage += "\n";
	send(_clientSocket, replyMessage.c_str(), replyMessage.length(),
		 MSG_NOSIGNAL);
}