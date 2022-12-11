#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <Windows.h>
#include <string>

using namespace std;

struct Employee
{
	int num; // идентификационный номер сотрудника
	char name[10]; // имя сотрудника
	double hours; // количество отработанных часов


	friend ostream& operator<< (ostream& out, Employee& employee) {
		out << employee.num << ' ' << employee.name << ' ' << employee.hours << '\n';
		return out;
	}
};

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
};


struct ClientInfo {
	HANDLE hNamedPipe;
	HANDLE process;
};

string filename;
HANDLE fileMutex;

map<int, HANDLE> readSemaphores;

volatile int processCount;

Employee readEmployee(int id) {
	WaitForSingleObject(fileMutex, INFINITE);

	ifstream in(filename);
	
	Employee employee;
	employee.num = id + 1;
	while (employee.num != id) {
		in.read((char*)&employee, sizeof(Employee));
	}

	in.close();

	ReleaseMutex(fileMutex);

	return employee;
}

void writeEmployee(Employee employee) {
	WaitForSingleObject(fileMutex, INFINITE);

	ifstream in(filename, ios::binary);

	vector<Employee> employees;

	Employee readEmployee;
	while (in.read((char*)&readEmployee, sizeof(Employee))) {
		if (employee.num == readEmployee.num) {
			employees.push_back(employee);
		}
		else {
			employees.push_back(readEmployee);
		}
	}

	in.close();

	ofstream out(filename, ios::binary);

	for (int i = 0; i < employees.size(); i++) {
		out.write((char*) &employees[i], sizeof(Employee));
	}

	out.close();

	ReleaseMutex(fileMutex);
}

DWORD WINAPI clientProcessing(LPVOID info) {
	ClientInfo clientInfo = *((ClientInfo*)info);

	Command command;
	Response response;
	DWORD cbRead;
	DWORD cbWritten;

	while (true) {	
		if (ReadFile(clientInfo.hNamedPipe, &command, sizeof(Command), &cbRead, NULL)) {
			if (command.type == read) {
				WaitForSingleObject(readSemaphores[command.num], INFINITE);

				Employee read = readEmployee(command.num);
			
				response.num = read.num;
				response.hours = read.hours;
				strcpy(response.name, read.name);
				WriteFile(clientInfo.hNamedPipe, &response, sizeof(Response), &cbWritten, NULL);

				ReadFile(clientInfo.hNamedPipe, &command, sizeof(Command), &cbRead, NULL);

				ReleaseSemaphore(readSemaphores[command.num], 1, NULL);
			}
			else if (command.type == modify) {
				for (int i = 0; i < processCount; i++) {
					WaitForSingleObject(readSemaphores[command.num], INFINITE);
				}

				Employee read = readEmployee(command.num);

				response.num = read.num;
				response.hours = read.hours;
				strcpy(response.name, read.name);
				WriteFile(clientInfo.hNamedPipe, &response, sizeof(Response), &cbWritten, NULL);

				ReadFile(clientInfo.hNamedPipe, &command, sizeof(Command), &cbRead, NULL);

				strcpy(read.name, command.name);
				read.hours = command.hours;

				writeEmployee(read);

				ReadFile(clientInfo.hNamedPipe, &command, sizeof(Command), &cbRead, NULL);

				for (int i = 0; i < processCount; i++) {
					ReleaseSemaphore(readSemaphores[command.num], 1, NULL);
				}
			}
		}
	}
}


int main() {
	cout << "Enter database filename: ";
	cin >> filename;

	ofstream out(filename, ios::binary);
	
	int employeesCount;
	cout << "Enter number of employees: ";
	cin >> employeesCount;
	
	Employee* employees = new Employee[employeesCount];
	for (int i = 0; i < employeesCount; i++) {
		cout << "Enter employee info (ID, name, hours): ";
		cin >> employees[i].num >> employees[i].name >> employees[i].hours;
	}



	out.write((char*) employees, sizeof(Employee) * employeesCount);

	out.close();

	int tempProcessCount;
	cout << "Enter desired number of clients: ";
	cin >> tempProcessCount;
	processCount = tempProcessCount;

	for (int i = 0; i < employeesCount; i++) {
		readSemaphores[employees[i].num] = CreateSemaphore(NULL, processCount, processCount, NULL);
	}

	STARTUPINFO* si = new STARTUPINFO[processCount];
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[processCount];
	ZeroMemory(si, sizeof(STARTUPINFO) * processCount);
	ZeroMemory(pi, sizeof(PROCESS_INFORMATION) * processCount);

	ClientInfo* clientsInfo = new ClientInfo[processCount];
	
	for (int i = 0; i < processCount; i++) {
		char str[50] = "\\\\.\\pipe\\operation_pipe";
		_itoa(i+1, &str[strlen(str)], 10);

		clientsInfo[i].hNamedPipe = CreateNamedPipe(
			str,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			PIPE_UNLIMITED_INSTANCES,
			0,
			0,
			INFINITE,
			NULL
		);

		string command = "Client.exe ";
		char* commandCstr = new char[100];
		strcpy(commandCstr, command.c_str());
		strcat(commandCstr, str);

		if (!CreateProcess(NULL, TEXT(commandCstr), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i])) {
			printf("Error (%d)", GetLastError());
		}

		clientsInfo[i].process = pi[i].hProcess;

		ConnectNamedPipe(clientsInfo[i].hNamedPipe, NULL);
	}

	HANDLE* processingThreads = new HANDLE[processCount];
	DWORD* processIDs = new DWORD[processCount];
	for (int i = 0; i < processCount; i++) {
		processingThreads[i] = CreateThread(NULL, 0, clientProcessing, &clientsInfo[i], 0, &processIDs[i]);
		
		if (processingThreads[i] == INVALID_HANDLE_VALUE) {
			printf("Error: %d", GetLastError());
		}
	}

	for (int i = 0; i < processCount; i++) {
		WaitForSingleObject(clientsInfo[i].process, INFINITE);
		TerminateThread(processingThreads[i], 0);
	}

	for (int i = 0; i < processCount; i++) {
		CloseHandle(clientsInfo[i].process);
		CloseHandle(clientsInfo[i].hNamedPipe);
		CloseHandle(processingThreads[i]);
	}

	for (auto semaphore : readSemaphores) {
		CloseHandle(semaphore.second);
	}

	CloseHandle(fileMutex);

	ifstream in(filename, ios::binary);

	Employee employee;
	while (in.read((char*)&employee, sizeof(Employee))) {
		cout << employee;
	}
	
	system("pause");
	return 0;
}