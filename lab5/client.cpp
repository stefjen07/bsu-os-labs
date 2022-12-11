#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <Windows.h>
using namespace std;

enum CommandType {
	read,
	modify
};

struct Command {
	CommandType type;
	int num;
	char name[10];
	double hours;
};

struct Response {
	int num;
	double hours;
	char name[10];

	friend ostream& operator<< (ostream & out, Response& response) {
		out << response.num << ' ' << response.name << ' ' << response.hours << '\n';
		return out;
	}
};

int main(int argc, char** argv) {
	WaitNamedPipe(argv[1], INFINITE);
	HANDLE hNamedPipe = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hNamedPipe == INVALID_HANDLE_VALUE) {
		printf("Error: %d", GetLastError());
	}

	cout << "Available commands:\nexit - quit the program\nread - read the record\nmodify - modify the record\n";

	string command;
	DWORD cbRead;
	DWORD cbWritten;
	Command commandToSend;

	while (command != "exit") {
		cin >> command;
		if (command == "read") {
			cout << "Enter record ID: ";
			cin >> commandToSend.num;
			
			commandToSend.type = read;
			WriteFile(hNamedPipe, (char*) &commandToSend, sizeof(Command), &cbWritten, NULL);

			Response response;
			ReadFile(hNamedPipe, &response, sizeof(Response), &cbRead, NULL);

			cout << response;

			system("pause");
			WriteFile(hNamedPipe, (char*) &commandToSend, sizeof(Command), &cbWritten, NULL);
		} else if (command == "modify") {
			cout << "Enter record ID: ";
			cin >> commandToSend.num;

			commandToSend.type = modify;
			WriteFile(hNamedPipe, (char*) &commandToSend, sizeof(Command), &cbWritten, NULL);

			Response response;
			ReadFile(hNamedPipe, &response, sizeof(Response), &cbRead, NULL);

			cout << response;

			char name[10];
			double hours;
			cout << "Enter new name: ";
			cin >> name;
			cout << "Enter new hours: ";
			cin >> hours;

			strcpy(commandToSend.name, name);
			commandToSend.hours = hours;
			
			system("pause");
			WriteFile(hNamedPipe, (char*) &commandToSend, sizeof(Command), &cbWritten, NULL);

			system("pause");
			WriteFile(hNamedPipe, (char*) &commandToSend, sizeof(Command), &cbWritten, NULL);
		}
	}

	CloseHandle(hNamedPipe);

	return 0;
}