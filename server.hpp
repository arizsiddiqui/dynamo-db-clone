#include "helpers.hpp"
#include <map>
#include <cstdlib>
using namespace std;

class Server
{
	bool active;
	string hostname;
	int port;
	vectorClock clock;
	map<string, string> objectStore;
	void update_object_store(Object &object);
	void save_object_in_file(Object &object, int server_id, int replicaNumber);
	void update_clock(vectorClock &vClock);
	Object get_object(string key);
	void recover();

public:
	Server(string hostname, int port);
	void start();
	string get_hostname();
	void toggle_active();
	vectorClock get_clock();
};

class Gremlin : private Server
{
public:
	Gremlin(string hostname, int port);
	void start();
	void toggle_servers();
};