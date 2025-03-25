#include "helpers.hpp"
using namespace std;
class Client
{
	vector<int> get_server_ids(string &key);

public:
	void loop();
	void process_get(string &line, int seek);
	void process_put(string &line, int seek);
	void display_error(string &line);
};
