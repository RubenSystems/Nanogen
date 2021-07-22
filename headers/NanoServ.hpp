#pragma once


#include "BaseView.hpp"
#include "Helpers.hpp"

#include <string>
#include <unordered_map>
#include <stdio.h> 
#include <sstream>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace RubenSystems {
	namespace NanoGen {
		typedef std::function< std::shared_ptr<BaseView> (std::vector<std::string>)> Callback;



		class NanoServ {
			public:
				void start (int port) ;

				void route(const std::string & type, const std::string & path, const Callback & callback);

			private:
				std::unordered_map<std::string, Callback> callbacks;

				std::string getRouteID(const std::string type, const std::string & path);

				std::vector<std::string> split(const std::string & text, const char & breaker);

				std::string callback(const std::string type, const std::string & path);

				std::string createResponse(const std::string & message);

				void serveStaticFile(const std::string & staticFilepath, int sd);

				std::string staticFileHeader(const std::string & directory, int size);

				std::string getMIME(const std::string & ext);

				std::unordered_map<std::string, std::pair<std::shared_ptr<char>, int >> cdnCache;


		};

		std::string NanoServ::staticFileHeader(const std::string & directory, int size) {
			auto mime = getMIME(this->split(directory, '.')[1]);
			return "HTTP/1.1 200 OK\r\nContent-type: "+ mime +"\r\nContent-Length: " + std::to_string(size) + "\r\n\r\n";
		}

		void NanoServ::serveStaticFile(const std::string & staticFilepath, int sd) {

			if (this->cdnCache.find(staticFilepath) != this->cdnCache.end()){
				auto item = this->cdnCache[staticFilepath];
				// std::cout << "Serving " << staticFilepath << " from cache with " << item.second << std::endl;
				auto header = this->staticFileHeader(staticFilepath, item.second);
				send (sd, header.data(), header.size(), 0);
				send (sd, item.first.get(), item.second, 0);
				return;
			}

			std::ifstream f (std::string("static/") + staticFilepath , std::ios::binary|std::ios::ate);
			if (!f.is_open()) {
				auto response = this->createResponse("<h1>resource not found</h1>");
				send(sd, response.c_str(), response.size(), 0);
			} else {
				std::streampos size = f.tellg();
				std::shared_ptr<char> file (new char [size]);
				f.seekg (0, std::ios::beg);
				f.read (file.get(), size);
				f.close();
					
				auto header = this->staticFileHeader(staticFilepath, size);

				send (sd, header.data(), header.size(), 0);
				send (sd, file.get(), size, 0);
				this->cdnCache[staticFilepath] = std::make_pair(file, size);
				// std::cout << "Serving " << staticFilepath << std::endl;
			}
		}

		std::string NanoServ::getMIME(const std::string & ext) {
			if(mimeTypes.find(ext) != mimeTypes.end()){
				return mimeTypes[ext];
			} else {
				return "";
			}
		}


		std::string NanoServ::getRouteID(const std::string type, const std::string & path) {
			return type + std::string("_") + path;
		}

		std::string NanoServ::createResponse(const std::string & message) {
			return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "+ std::to_string( message.size() ) +"\r\nAccess-Control-Allow-Origin: *\r\n\r\n" + message + "\r\n";
		}

		std::string NanoServ::callback(const std::string type, const std::string & path) {
			std::vector<std::string> params;
			auto routeID = this->getRouteID(type, path);
			if (this->callbacks.find(routeID) != this->callbacks.end()) {
				return this->callbacks[routeID]({})->generate();
			} else {
				auto providedPath = this->split(path, '/');
				for (auto & i : this->callbacks) {
					bool works = true;
					auto existingPath = this->split(this->split(i.first, '_')[1], '/');
					if (providedPath.size() == existingPath.size()) {
						for(int i = 0; i < existingPath.size(); i++) {
							if (existingPath[i] != "$" && (existingPath[i] != providedPath[i])) {
								works = false;
								break;
							} else if (existingPath[i] == "$") {
								params.push_back(providedPath[i]);
							}
						}
					} else {continue;}
					if (works == true) {
						return i.second(params)->generate();
					}
				}
				return "<h1>404 error</h1>";
			}
		}

		std::vector<std::string> NanoServ::split(const std::string & text, const char & breaker) {
			std::string tmp;
			std::vector<std::string> stk;
			std::stringstream ss(text);
			while(getline(ss, tmp, breaker)) {
				stk.push_back(tmp);
			}
			return stk;
		}

		void NanoServ::route(const std::string & type, const std::string & path, const Callback & callback){
			auto routeID = getRouteID(type, path);
			this->callbacks[routeID] = callback;
		}

		void NanoServ::start (int port) {
			int opt = true;  
			int master_socket , addrlen , new_socket , client_socket[30] , 
				  max_clients = 30 , activity, i , valread , sd;  
			int max_sd;  
			struct sockaddr_in address;  
				 
			char buffer[1025];  //data buffer of 1K 
				 
			//set of socket descriptors 
			fd_set readfds;  
				 
			//a message 
			 
			//initialise all client_socket[] to 0 so not checked 
			for (i = 0; i < max_clients; i++)  {  
				client_socket[i] = 0;  
			}  
				 
			//create a master socket 
			if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {  
				perror("socket failed");  
				exit(EXIT_FAILURE);  
			}  
			 
			//set master socket to allow multiple connections , 
			//this is just a good habit, it will work without this 
			if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {  
				perror("setsockopt");  
				exit(EXIT_FAILURE);  
			}  
			 
			//type of socket created 
			address.sin_family = AF_INET;  
			address.sin_addr.s_addr = INADDR_ANY;  
			address.sin_port = htons( port );  
				 
			//bind the socket to localhost port 8888 
			if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){  
				perror("bind failed");  
				exit(EXIT_FAILURE);  
			}  
			printf("Listener on port %d \n", port);  
				 
			//try to specify maximum of 3 pending connections for the master socket 
			if (listen(master_socket, 3) < 0) {  
				perror("listen");  
				exit(EXIT_FAILURE);  
			}  
				 
			//accept the incoming connection 
			addrlen = sizeof(address);  
			puts("Waiting for connections ...");  
				 
			while(true) {  
				//clear the socket set 
				FD_ZERO(&readfds);  
			 
				//add master socket to set 
				FD_SET(master_socket, &readfds);  
				max_sd = master_socket;  
					 
				//add child sockets to set 
				for ( i = 0 ; i < max_clients ; i++) {  
					//socket descriptor 
					sd = client_socket[i];  
						 
					//if valid socket descriptor then add to read list 
					if(sd > 0)  
						FD_SET( sd , &readfds);  
						 
					//highest file descriptor number, need it for the select function 
					if(sd > max_sd)  
						max_sd = sd;  
				}  
			 
				//wait for an activity on one of the sockets , timeout is NULL , 
				//so wait indefinitely 
				activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
			   
				if ((activity < 0) && (errno!=EINTR)) {  
					printf("select error");  
				}  
					 
				//If something happened on the master socket , 
				//then its an incoming connection 
				if (FD_ISSET(master_socket, &readfds)){  
					if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {  
						perror("accept");  
						exit(EXIT_FAILURE);  
					}  
					 
					//inform user of socket number - used in send and receive commands 
					// printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
				   
						 
					//add new socket to array of sockets 
					for (i = 0; i < max_clients; i++) {  
						//if position is empty 
						if( client_socket[i] == 0 ) {  
							client_socket[i] = new_socket;  
							// printf("Adding to list of sockets as %d\n" , i);  
								 
							break;  
						}  
					}  
				}
				//else its some IO operation on some other socket
				for (i = 0; i < max_clients; i++) {  
					sd = client_socket[i];  
						 
					if (FD_ISSET( sd , &readfds)){  
						//Check if it was for closing , and also read the 
						//incoming message 
						if ((valread = read( sd , buffer, 1024)) == 0) {  
							
							//Somebody disconnected , get his details and print 
							// getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);  
							// printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
								 
							//Close the socket and mark as 0 in list for reuse 
							close( sd );  
							client_socket[i] = 0;  
						} else {  
							//Echo back the message that came in 
							//set the string terminating NULL byte on the end 
							//of the data read 
							buffer[valread] = '\0';  

							std::string request = this->split(std::string(buffer), '\n')[0];
							auto dirReq = this->split(request, ' ');
							auto directory = this->split(dirReq[1], '/');
							if (directory[1] == "static") {

								if (directory.size() >= 3) {
									this->serveStaticFile(directory[2], (int)sd);
								} else {
									auto response = this->createResponse("Not Found");
									send(sd , response.c_str() , response.size() , 0 );  
								}
							} else {
								auto response = this->createResponse(this->callback(dirReq[0], dirReq[1]));
								send(sd , response.c_str() , response.size() , 0 );  
							} 
						}  
					}  
				}
			}
		}
	}
}




/*
namespace RubenSystems {
	namespace NanoGen {

		typedef std::function< std::shared_ptr<BaseView> (std::vector<std::string>)> Callback;

		class NanoServ {
			public: 
				NanoServ(int port, int maxClients, int maxPending);


				void start();

				void route(const std::string & type, const std::string & path, const Callback & callback);

			private:
				const int port;
				const int maxClients;
				const int maxPending;

				std::unordered_map<std::string, Callback> callbacks;

				std::string getRouteID(const std::string type, const std::string & path);

				std::vector<std::string> split(const std::string & text, const char & breaker);

				std::string callback(const std::string type, const std::string & path);

				std::string createResponse(const std::string & message);

				void configureSocket(int socketNo, bool master);
		};
	}
}


NanoServ::NanoServ(int port, int maxClients, int maxPending) : port(port), maxClients(maxClients), maxPending(maxPending) {}


void NanoServ::start() {
int opt = true;  
	int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;  
	int max_sd;  
	struct sockaddr_in address;  
		 
	char buffer[1025];  //data buffer of 1K 
		 

	//set of socket descriptors 
	fd_set readfds;  
 
	 
	//initialise all client_socket[] to 0 so not checked 
	for (i = 0; i < max_clients; i++)  
	{  
		client_socket[i] = 0;  
	}  
		 
	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {  
		perror("socket failed");  
		exit(EXIT_FAILURE);  
	}  
	 
	this->configureSocket(master_socket, true);

	//set master socket to allow multiple connections , 
	//this is just a good habit, it will work without this 
	// if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  {  
	//     perror("setsockopt");  
	//     exit(EXIT_FAILURE);  
	// }  
	 
	//type of socket created 
	address.sin_family = AF_INET;  
	address.sin_addr.s_addr = INADDR_ANY;  
	address.sin_port = htons( port );  
		 
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {  
		perror("bind failed");  
		exit(EXIT_FAILURE);  
	}  

	printf("Listener on port %d \n", PORT);  
		 
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(master_socket, 3) < 0) {  
		perror("listen");  
		exit(EXIT_FAILURE);  
	}  
		 
	//accept the incoming connection 
	addrlen = sizeof(address);  
	puts("Waiting for connections ...");  
		 
	while(true) {  
		//clear the socket set 
		FD_ZERO(&readfds);  
	 
		//add master socket to set 
		FD_SET(master_socket, &readfds);  
		max_sd = master_socket;  
			 
		//add child sockets to set 
		for ( i = 0 ; i < max_clients ; i++) {  
			//socket descriptor 
			sd = client_socket[i];  
				 
			//if valid socket descriptor then add to read list 
			if(sd > 0) {
				FD_SET( sd , &readfds);  
			}  
				
				 
			//highest file descriptor number, need it for the select function 
			if(sd > max_sd) {
				max_sd = sd;  
			}
		}  
	 
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
	   
		if ((activity < 0) && (errno!=EINTR))  {  
			printf("select error");  
		}  
			 
		//If something happened on the master socket , 
		//then its an incoming connection 
		if (FD_ISSET(master_socket, &readfds))  {  
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {  
				perror("accept");  
				exit(EXIT_FAILURE);  
			}  
			 
			this->configureSocket(new_socket, false);
			std::string handshake = this->recieve(new_socket).first;
			std::string message = this->generateWebSocketHeader(handshake);
			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
		   
			//send new connection greeting message 
			if(send(new_socket, message, strlen(message), 0) != strlen(message) ) {  
				perror("send");  
			}  
				 
			puts("Welcome message sent successfully");  
				 
			//add new socket to array of sockets 
			for (i = 0; i < max_clients; i++) {  
				//if position is empty 
				if( client_socket[i] == 0 ) {  
					client_socket[i] = new_socket;  
					printf("Adding to list of sockets as %d\n" , i);  
						 
					break;  
				}  
			}  
		}  
			 
		//else its some IO operation on some other socket
		for (i = 0; i < max_clients; i++)  {  
			sd = client_socket[i];  
				 
			if (FD_ISSET( sd , &readfds)) {  
				//Check if it was for closing , and also read the 
				//incoming message 
				if ((valread = read( sd , buffer, 1024)) == 0) {  
					//Somebody disconnected , get his details and print 
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);  
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
						 
					//Close the socket and mark as 0 in list for reuse 
					close( sd );  
					client_socket[i] = 0;  
				} else {  //Echo back the message that came in 
					//set the string terminating NULL byte on the end 
					//of the data read 
					buffer[valread] = '\0';  
					send(sd , buffer , strlen(buffer) , 0 );  
				}  
			}  
		}  
	}  
}

std::pair<std::string, int> NanoServer::recieve(int socketNo) {
	char buffer [1025];
	auto status = ::recv(socketNo, buffer, 1025, NULL);
	buffer[status + 1] = '\0';
	std::string bufString = std::string(buffer);

	return std::make_pair(bufString, status);
}

std::string NanoServ::getRouteID(const std::string type, const std::string & path) {
	return type + std::string("_") + path;
}

std::string NanoServ::createResponse(const std::string & message) {
	return "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: "+ std::to_string( message.size() ) +"\n\n" + message;
}

std::string NanoServ::callback(const std::string type, const std::string & path) {
	std::vector<std::string> params;
	auto routeID = this->getRouteID(type, path);
	if (this->callbacks.find(routeID) != this->callbacks.end()) {
		return this->callbacks[routeID]({})->generate();
	} else {
		auto providedPath = this->split(path, '/');
		for (auto & i : this->callbacks) {
			bool works = true;
			auto existingPath = this->split(this->split(i.first, '_')[1], '/');
			if (providedPath.size() == existingPath.size()) {
				for(int i = 0; i < existingPath.size(); i++) {
					if (existingPath[i] != "$" && (existingPath[i] != providedPath[i])) {
						works = false;
						break;
					} else if (existingPath[i] == "$") {
						params.push_back(providedPath[i]);
					}
				}
			} else {continue;}
			if (works == true) {
				return i.second(params)->generate();
			}
		}
		return "<h1>404 error</h1>";
	}
}

std::vector<std::string> NanoServ::split(const std::string & text, const char & breaker) {
	std::string tmp;
	std::vector<std::string> stk;
	std::stringstream ss(text);
	while(getline(ss, tmp, breaker)) {
		stk.push_back(tmp);
	}
	return stk;
}

void NanoServ::route(const std::string & type, const std::string & path, const Callback & callback){
	auto routeID = getRouteID(type, path);
	this->callbacks[routeID] = callback;
}


void NanoServer::configureSocket(int socketNo, bool master) {
	int opt = 1;
	if( master ) {
		if (setsockopt(socketNo, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
	} else {
		if ( setsockopt(socketNo, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, sizeof(opt)) < 0 ) {
			std::cout << "[WARN] - keeping client alive failed\n";
		} else if (setsockopt(socketNo, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
			std::cout << "[WARN] - tcp did not work\n";
		}
	}
}


void start(int port) {

					int server_fd, new_socket;
					int opt = 1;
					long valread;
					struct sockaddr_in address;
					int addrlen = sizeof(address);

					// Only this line has been changed. Everything is same.
					char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nH4l3o world!";

					// Creating socket file descriptor
					if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
						perror("In socket");
						exit(EXIT_FAILURE);
					}

					if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) { 
						perror("setsockopt"); 
						exit(EXIT_FAILURE); 
					} 


					if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { 
						perror("setsockopt"); 
						exit(EXIT_FAILURE); 
					} 

					address.sin_family = AF_INET;
					address.sin_addr.s_addr = INADDR_ANY;
					address.sin_port = htons( port );



					memset(address.sin_zero, '\0', sizeof address.sin_zero);


					if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
						perror("In bind");
						exit(EXIT_FAILURE);
					}
					if (listen(server_fd, 10) < 0)
					{
						perror("In listen");
						exit(EXIT_FAILURE);
					}
					while(1)
					{
						printf("\n+++++++ Waiting for new connection ++++++++\n\n");
						if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
						{
							perror("In accept");
							exit(EXIT_FAILURE);
						}
						printf("------------------Recieved connection-------------------");
						std::stringstream input;
						char buffer[30000] = {0};
						// valread = read( new_socket , buffer, 30000);

					 //    int rc;
						// while ((rc = recv(server_fd, buffer, sizeof(buffer) - 1, MSG_WAITALL)) != -1) {
						// 	buffer[rc] = '\0';
						// 	std::cout << "buffer" << std::endl;
						// }


						printf("%s\n",buffer );
						write(new_socket , hello , strlen(hello));
						printf("------------------Hello message sent-------------------");
						close(new_socket);
					}


			
					// int server_fd, new_socket, valread;
					// struct sockaddr_in address;
					// int opt = 1;
					// int addrlen = sizeof(address);
					
					// // Creating socket file descriptor
					// if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
					// 	perror("socket failed");
					// 	exit(EXIT_FAILURE);
					// }
					// // Forcefully attaching socket to the port 8080
					// if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
					// 	perror("setsockopt-1");
					// 	exit(EXIT_FAILURE);
					// } else if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
					// 	perror("setsockopt-2");
					// 	exit(EXIT_FAILURE);
					// }
					// address.sin_family = AF_INET;
					// address.sin_addr.s_addr = INADDR_ANY;
					// address.sin_port = htons( port );




					// // Forcefully attaching socket to the port 8080

					// if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
					// 	perror("bind failed");
					// 	exit(EXIT_FAILURE);
					// }


					// if (listen(server_fd, 10) < 0) {
					// 	perror("listen");
					// 	exit(EXIT_FAILURE);
					// }
					// while (true) {
					// 	std::cout << "here - 1" << std::endl;
					// 	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) == -1) {
					// 		perror("accept");
					// 		exit(EXIT_FAILURE);
					// 	}
					// 	std::cout << "here - 2" << std::endl;
					// 	char buffer[1025] = {0};
					// 	valread = recv( new_socket, buffer, 1024, 0 );
					// 	buffer[valread + 1] = '\0';
					// 	std::cout << buffer << std::endl;
					// 	std::cout << "here - 3" << std::endl;

					// 	std::string request = this->split(std::string(buffer), '\n')[0];
					// 	auto dirReq = this->split(request, ' ');
					// 	auto response = this->createResponse(this->callback(dirReq[0], dirReq[1]));
					// 	std::cout << "here - 4" << std::endl;
					// 	send(new_socket , response.c_str() , response.size() , 0 );
					// 	std::cout << "here - 5 \n ==============" << std::endl;
					// 	close(new_socket);
					// }
				}
*/






