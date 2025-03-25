CXX = g++
CXXFLAGS = -std=c++11 -pthread

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp helpers.cpp server_helper.cpp

gremlin: gremlin_server.cpp
	$(CXX) $(CXXFLAGS) -o gremlin gremlin_server.cpp helpers.cpp server_helper.cpp

.PHONY: clean

clean: 
	rm -f client server gremlin


clean-client:
	rm -f client

clean-server:
	rm -f server

clean-gremlin:
	rm -f gremlin

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp client_helper.cpp helpers.cpp
