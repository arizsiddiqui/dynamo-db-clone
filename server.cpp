#include "server.hpp"

int main(int argc, char const *argv[])
{
	vectorClock vClock;
	mutex vClockMutex;
	int currenthost = stoi(argv[1]);

	Server server(SERVER_HOSTNAMES[currenthost], SERVER_PORTS[currenthost]);
	server.start();
	return 0;
}
