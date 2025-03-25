#include "helpers.hpp"
using namespace std;

bool is_whitespace_or_eol(char c)
{
	const string whitespace = " \t";
	bool matched = false;

	for (int i = 0; i < whitespace.length(); ++i)
	{
		if (whitespace[i] == c)
		{
			matched = true;
			break;
		}
	}

	return matched;
}

int recognize_command(string &line, int &seek, int &server_id)
{
	int init_seek = 0;
	if (line[0] >= '0' && line[0] <= '7')
	{
		init_seek = 2;
		server_id = line[0] - '0';
	}

	for (int i = 0; i < COMMANDS.size(); ++i)
	{
		bool matched = true;
		seek = init_seek;
		for (int j = 0; j < COMMANDS[i].length() && seek < line.size(); ++seek, ++j)
		{
			if (COMMANDS[i][j] != line[seek])
			{
				matched = false;
				break;
			}
		}

		if (matched)
		{
			if (is_whitespace_or_eol(line[seek]))
			{
				return i;
			}
			else
				break;
		}
	}

	seek = -1;
	return -1;
}

int parse_data(string &line, int &seek, string &data, bool parse_till_eol)
{
	data.clear(); // Clear previous data

	// iterate over spaces
	do
	{
		++seek;
	} while (is_whitespace_or_eol(line[seek]));

	if (seek == line.length())
		return -1;

	// Push the rest of the line from seek -> line.length() into data
	for (; seek < line.length(); ++seek)
	{
		if (!parse_till_eol && is_whitespace_or_eol(line[seek]))
		{
			break;
		}

		data.push_back(line[seek]);
	}

	return 0;
}

vectorClock::vectorClock()
{
	clock = vector<int>(SERVER_PORTS.size(), 0);
}

vectorClock::vectorClock(const vectorClock &vClock)
{
	clock = vClock.clock;
}

void vectorClock::addClock(int value)
{
	clock.push_back(value);
}

void vectorClock::incrementClockValue(int index)
{
	clock[index]++;
}

int vectorClock::getClockValue(int index) const
{
	return clock[index];
}

void vectorClock::updateClockValue(int index, int value)
{
	clock[index] = value;
}

Object::Object()
{
	key = "";
	value = "";
}

Object::Object(string key, string value)
{
	this->key = key;
	this->value = value;
}

void Object::setKey(string key)
{
	this->key = key;
}

void Object::setValue(string value)
{
	this->value = value;
}

string Object::getKey()
{
	return key;
}

string Object::getValue()
{
	return value;
}

string Object::serialize()
{
	string delimiter = ",";
	return key + delimiter + value;
}

void Object::deserialize(string data)
{
	int commaIndex = data.find(",");
	key = data.substr(0, commaIndex);
	value = data.substr(commaIndex + 1);
}

Message::Message(Object object, int replicaNumber)
{
	this->object.setKey(object.getKey());
	this->object.setValue(object.getValue());
	this->replicaNumber = replicaNumber;
}