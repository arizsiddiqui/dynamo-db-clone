#include "server.hpp"
using namespace std;

int main(int argc, char const *argv[])
{
	string gremlinServerName = "dc45.utdallas.edu";
	int gremlinServerPort = 8888;
	Gremlin gremlin(gremlinServerName, gremlinServerPort);
	gremlin.start();
	return 0;
}
