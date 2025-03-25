#include "server.hpp"
using namespace std;

Server::Server(string hostname, int port)
{
	this->hostname = hostname;
	this->port = port;
	this->active = true;
}

void Server::start()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Error opening socket");
		exit(1);
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (::bind(sockfd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Error on binding");
		exit(1);
	}

	if (listen(sockfd, SOMAXCONN) < 0)
	{
		perror("Error on listen");
		exit(1);
	}

	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	while (1)
	{
		if (active)
		{
			recover();
			int client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &client_len);
			if (client_sockfd < 0)
			{
				perror("Error on accept");
				exit(1);
			}

			char buffer[256];
			bzero(buffer, 256);
			int n = recv(client_sockfd, buffer, 256, 0);
			if (n < 0)
			{
				perror("Error reading from socket");
				exit(1);
			}

			string task = string(buffer);
			int seek, server_id;
			int res = recognize_command(task, seek, server_id);
			cout << res << endl;
			cout << task << " " << seek << " " << server_id << endl;
			string key;
			Object object;
			string data;
			switch (res)
			{
			case 0:
				parse_data(task, seek, key, true);
				object = get_object(key);
				if (object.getValue().empty())
				{
					if (write(client_sockfd, "NOTFOUND", 8) < 0)
					{
						perror("Error writing to socket");
						break;
					}
				}
				else
				{
					string response = object.serialize();
					if (write(client_sockfd, response.c_str(), response.length()) < 0)
					{
						perror("Error writing to socket");
						break;
					}
				}
				break;
			case 1:
				parse_data(task, seek, data, true);
				object.deserialize(data);
				if (server_id == find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin())
				{
					update_object_store(object);
					for (int i = 1; i <= 3; i++)
					{
						save_object_in_file(object, server_id, i);
					}
				}
				else
				{
					for (int replicaNumber = (find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin()) - server_id; replicaNumber <= 3; replicaNumber++)
						save_object_in_file(object, server_id, replicaNumber);
				}
				if (write(client_sockfd, "SUCCESS", 7) < 0)
				{
					perror("Error writing to socket");
					break;
				}
				close(client_sockfd);
				break;
			case 3:
				if (server_id == 7)
				{
					if (write(client_sockfd, (active ? "ACTIVE" : "DISABLED"), active ? 7 : 9) < 0)
					{
						perror("Error writing to socket");
						break;
					}
					break;
				}
			case 4:
				if (server_id == 7)
				{
					toggle_active();
					break;
				}
			default:
				perror("Invalid command");
			}
		}
		else
		{
			int client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &client_len);
			if (client_sockfd < 0)
			{
				perror("Error on accept");
				exit(1);
			}

			char buffer[256];
			bzero(buffer, 256);
			int n = read(client_sockfd, buffer, 255);
			if (n < 0)
			{
				perror("Error reading from socket");
				exit(1);
			}

			string task = string(buffer);
			int seek, server_id;
			int res = recognize_command(task, seek, server_id);
			string key;
			Object object;
			string data;
			switch (res)
			{
			case 0:
			case 1:
				close(client_sockfd);
				break;
			case 3:
				if (server_id == 7)
				{
					if (write(client_sockfd, (active ? "ACTIVE" : "DISABLED"), 7) < 0)
					{
						perror("Error writing to socket");
						break;
					}
				}
				else
				{
					if (write(client_sockfd, "DISABLED", 8) < 0)
					{
						perror("Error writing to socket");
						break;
					}
				}
				break;
			case 4:
				if (server_id == 7)
				{

					toggle_active();
					if (active)
					{
						recover();
					}
				}
				else
				{
					if (write(client_sockfd, "DISABLED", 8) < 0)
					{
						perror("Error writing to socket");
						break;
					}
				}
				break;
			default:
				perror("Invalid command");
			}
		}
	}
}

string Server::get_hostname()
{
	return hostname;
}

void Server::toggle_active()
{
	active = !active;
}

void Server::update_clock(vectorClock &vClock)
{
	cout << "Updating clock for " << hostname << ":" << port << endl;
}

vectorClock Server::get_clock()
{
	cout << "Getting clock for " << hostname << ":" << port << endl;
	return clock;
}

void Server::update_object_store(Object &object)
{
	objectStore[object.getKey()] = object.getValue();
}

Object Server::get_object(string key)
{
	string value = objectStore[key];
	if (!value.empty())
	{
		Object object(key, value);
		return object;
	}
	else
	{
		ifstream reader;
		reader.open("./replicas/server_" + to_string(find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin() - 2 >= 0 ? find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin() - 1 : find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin() - 2 + 7) + "_replica2.store");
		string line;
		map<string, string> tempObjectStore;
		while (getline(reader, line))
		{
			int commaIndex = line.find(",");
			string key = line.substr(0, commaIndex);
			string value = line.substr(commaIndex + 1);
			tempObjectStore[key] = value;
		}
		reader.close();
		value = tempObjectStore[key];
		if (!value.empty())
		{
			Object object(key, value);
			return object;
		}
		else
		{
			reader.open("./replicas/server_" + to_string(find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin() - 4 >= 0 ? find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin() - 2 : find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin() - 4 + 7) + "_replica3.store");
			while (getline(reader, line))
			{
				int commaIndex = line.find(",");
				string key = line.substr(0, commaIndex);
				string value = line.substr(commaIndex + 1);
				tempObjectStore[key] = value;
			}
			reader.close();
			value = tempObjectStore[key];
			if (!value.empty())
			{
				Object object(key, value);
				return object;
			}
			else
			{
				Object object(key, "");
				return object;
			}
		}
	}
}

void Server::save_object_in_file(Object &object, int server_id, int replicaNumber)
{
	ofstream writer;
	writer.open("./replicas/server_" + to_string(server_id) + "_replica" + to_string(replicaNumber) + ".store", ios::app);
	writer << object.serialize() << endl;
	writer.close();
}

void Server::recover()
{
	vector<int> counts{0, 0, 0};
	for (int i = 1; i <= 3; i++)
	{
		ifstream reader;
		reader.open("./replicas/server_" + to_string(find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin()) + "_replica" + to_string(i) + ".store");
		string line;
		while (getline(reader, line))
		{
			counts[i - 1]++;
		}
		reader.close();
	}
	int maxCount = *max_element(counts.begin(), counts.end());
	int maxIndex = distance(counts.begin(), max_element(counts.begin(), counts.end()));

	ifstream reader;
	reader.open("./replicas/server_" + to_string(find(SERVER_HOSTNAMES.begin(), SERVER_HOSTNAMES.end(), get_hostname()) - SERVER_HOSTNAMES.begin()) + "_replica" + to_string(maxIndex + 1) + ".store");
	string line;
	while (getline(reader, line))
	{
		int commaIndex = line.find(",");
		string key = line.substr(0, commaIndex);
		string value = line.substr(commaIndex + 1);
		Object object(key, value);
		update_object_store(object);
	}
}

Gremlin::Gremlin(string hostname, int port) : Server(hostname, port) {}

void Gremlin::start()
{
	while (1)
	{
		int option;
		system("clear");
		cout << "1. Toggle servers" << endl;
		cout << "2. Exit" << endl;
		cout << "Enter option: ";
		cin >> option;
		switch (option)
		{
		case 1:
			toggle_servers();
			break;
		case 2:
			exit(0);
			break;
		default:
			cout << "Invalid option" << endl;
		}
	}
}

void Gremlin::toggle_servers()
{
	vector<bool> serverStatus;
	for (int i = 0; i < SERVER_HOSTNAMES.size(); i++)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			perror("Error opening socket");
			return;
		}

		addrinfo hints, *serverInfo;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		if (getaddrinfo(SERVER_HOSTNAMES[i].c_str(), to_string(SERVER_PORTS[i]).c_str(), &hints, &serverInfo) != 0)
		{
			perror("Error getting address info");
			return;
		}

		if (connect(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
		{
			perror("Error connecting to server");
			return;
		}

		freeaddrinfo(serverInfo);

		if (::send(sockfd, "7 STATUS ", 9, 0) < 0)
		{
			perror("Error sending data to server");
			return;
		}

		char buffer[256];
		bzero(buffer, 256);
		if (recv(sockfd, buffer, 255, 0) < 0)
		{
			perror("Error receiving data from server");
			return;
		}
		string status = string(buffer);
		if (status == "ACTIVE")
		{
			serverStatus.push_back(true);
		}
		else
		{
			serverStatus.push_back(false);
		}
	}
	int serverNumber = -1;
	while (serverNumber < 0 || serverNumber >= serverStatus.size())
	{
		cout << "Server Status:-" << endl;
		for (int i = 0; i < serverStatus.size(); i++)
		{
			cout << i + 1 << ". " << SERVER_HOSTNAMES[i] << ":" << SERVER_PORTS[i] << " - " << (serverStatus[i] ? "ACTIVE" : "DISABLED") << endl;
		}
		cout << "Enter server number to toggle: ";
		cin >> serverNumber;
		serverNumber--;
	}
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Error opening socket");
		return;
	}

	addrinfo hints, *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(SERVER_HOSTNAMES[serverNumber].c_str(), to_string(SERVER_PORTS[serverNumber]).c_str(), &hints, &serverInfo) != 0)
	{
		perror("Error getting address info");
		return;
	}

	if (connect(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
	{
		perror("Error connecting to server");
		return;
	}

	freeaddrinfo(serverInfo);

	if (::send(sockfd, "7 TOGGLE ", 10, 0) < 0)
	{
		perror("Error sending data to server");
		return;
	}

	return;
}