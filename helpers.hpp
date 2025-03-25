#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <mutex>
#include <algorithm>
using namespace std;

const vector<int> SERVER_PORTS({1111, 2222, 3333, 4444, 5555, 6666, 7777});

const vector<string> SERVER_HOSTNAMES({"dc01.utdallas.edu", "dc02.utdallas.edu", "dc03.utdallas.edu", "dc04.utdallas.edu", "dc05.utdallas.edu", "dc06.utdallas.edu", "dc07.utdallas.edu"});

const vector<string> COMMANDS({"GET", "PUT", "EXIT", "STATUS", "TOGGLE"});

class vectorClock
{
	vector<int> clock;

public:
	vectorClock();
	vectorClock(const vectorClock &vClock);
	void addClock(int value);
	void incrementClockValue(int index);
	int getClockValue(int index) const;
	void updateClockValue(int index, int value);
};

class Object
{
	string key;
	string value;

public:
	Object();
	Object(string key, string value);
	string getKey();
	string getValue();
	void setKey(string key);
	void setValue(string value);
	string serialize();
	void deserialize(string data);
};

class Message
{
	Object object;
	int replicaNumber;

public:
	Message(Object object, int replicaNumber);
	Object getObject();
	int getReplicaNumber();
};

int recognize_command(string &line, int &seek, int &server_id);
int parse_data(string &line, int &seek, string &data, bool parse_till_eol);
