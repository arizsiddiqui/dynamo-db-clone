#include "client.hpp"
using namespace std;

vector<int> Client::get_server_ids(string &key)
{
	const int modulo = 7, power = 100;
	int id = 0;
	vector<int> server_ids;

	for (int i = 0; i < key.size(); ++i)
	{
		int char_value = (int)key[i];
		id = (((id * power) % modulo) + (char_value % modulo)) % modulo;
	}

	id = id % modulo;

	for (int i = 0; i <= 4; i += 2)
	{
		server_ids.push_back((id + i) % modulo);
	}

	return server_ids;
}

void Client::process_get(string &line, int seek)
{
	vector<int> server_ids;
	string key;
	int result;

	// process key
	result = parse_data(line, seek, key, false);

	if (result < 0)
	{
		display_error(line);
		return;
	}

	server_ids = get_server_ids(key);
	random_shuffle(server_ids.begin(), server_ids.end());
	cout << endl;

	for (int server : server_ids)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		addrinfo hints, *res;
		try
		{
			if (sockfd < 0)
			{
				perror("Error opening socket");
				exit(1);
			}
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			if (getaddrinfo(SERVER_HOSTNAMES[server].c_str(), to_string(SERVER_PORTS[server]).c_str(), &hints, &res) != 0)
			{
				perror("Error getting address info");
				throw exception();
			}

			if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
			{
				perror("Error connecting to server");
				throw exception();
			}
		}
		catch (exception &e)
		{
			close(sockfd);
			continue;
		}

		freeaddrinfo(res);
		string message = to_string(server) + " " + "GET " + key;
		if (::send(sockfd, message.c_str(), message.length(), 0) < 0)
		{
			perror("Error sending data to server");
			return;
		}

		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));
		if (::recv(sockfd, buffer, sizeof(buffer), 0) <= 0)
		{
			close(sockfd);
			continue;
		}
		string response(buffer);
		if (response.compare("DISABLED") == 0)
		{
			close(sockfd);
			continue;
		}
		else if (response.compare("NOTFOUND") == 0)
		{
			cout << "Error 404: Key-Value pair not found." << endl;
			break;
		}
		else
		{
			Object object;
			object.deserialize(buffer);
			cout << object.getKey() << " " << object.getValue() << endl;
			close(sockfd);
			break;
		}
	}
}

void Client::process_put(string &line, int seek)
{
	vector<int> server_ids;
	string key, data;
	int result;

	result = parse_data(line, seek, key, false);

	if (result < 0)
	{
		display_error(line);
		return;
	}

	result = parse_data(line, seek, data, true);

	if (result < 0)
	{
		display_error(line);
		return;
	}

	server_ids = get_server_ids(key);

	int count_disabled_servers = 0;
	for (int server : server_ids)
	{
		if (count_disabled_servers == 2)
		{
			cout << "Error: Cannot store data at given moment. Please try again later." << endl;
			break;
		}

		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		addrinfo hints, *res;
		try
		{
			if (sockfd < 0)
			{
				perror("Error opening socket");
				exit(1);
			}
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			if (getaddrinfo(SERVER_HOSTNAMES[server].c_str(), to_string(SERVER_PORTS[server]).c_str(), &hints, &res) != 0)
			{
				perror("Error getting address info");
				throw exception();
			}

			if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
			{
				perror("Error connecting to server");
				throw exception();
			}
		}
		catch (exception &e)
		{
			count_disabled_servers++;
			close(sockfd);
			continue;
		}

		freeaddrinfo(res);
		Object object(key, data);
		string message = to_string(server_ids[0]) + " " + "PUT " + object.serialize();
		if (::send(sockfd, message.c_str(), message.length(), 0) < 0)
		{
			perror("Error sending data to server");
			exit(1);
		}

		char buffer[1024];
		try
		{
			if (::recv(sockfd, buffer, 1024, 0) <= 0)
			{
				count_disabled_servers++;
				close(sockfd);
				continue;
			}
		}
		catch (exception &e)
		{
			count_disabled_servers++;
			close(sockfd);
			continue;
		}
		close(sockfd);
		return;
	}
}

void Client::display_error(string &line)
{
	cout << "Error, unrecognized command: '" << line << "'" << endl;
}

void Client::loop()
{
	string line;
	int result;

	while (1)
	{
		cout << "Enter command: ";
		getline(cin, line);
		int seek, temp;
		result = recognize_command(line, seek, temp);

		switch (result)
		{
		case 0:
			process_get(line, seek);
			break;
		case 1:
			process_put(line, seek);
			break;
		case 2:
			return;
			break;
		default:
			display_error(line);
			break;
		}
	}
}
