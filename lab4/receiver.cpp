#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <Windows.h>
using namespace std;


int main() {
	string filename;
	int count;

	cout << "Enter filename:\n";
	cin >> filename;
	cout << "Enter buffer size:\n";
	cin >> count;

	int processCount;
	cout << "Enter count of senders:\n";
	cin >> processCount;

	STARTUPINFO* si = new STARTUPINFO[processCount];
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[processCount];
	ZeroMemory(si, sizeof(STARTUPINFO) * processCount);
	ZeroMemory(pi, sizeof(PROCESS_INFORMATION) * processCount);
	for (int i = 0; i < processCount; i++) {
		si[i].cb = sizeof(si);
	}

	HANDLE mutex = CreateMutex(NULL, FALSE, "GlobalFileCommunication");
	if (mutex == INVALID_HANDLE_VALUE) {
		printf("Error (%d)", GetLastError());
	}

	HANDLE semaphore = CreateSemaphore(NULL, count, count, "BufferSizeSemaphore");
	if (semaphore == INVALID_HANDLE_VALUE) {
		printf("Error (%d)", GetLastError());
	}

	HANDLE file = CreateFile(TEXT(filename.c_str()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	fstream in;

	HANDLE* processes = new HANDLE[processCount];
	string command = ("Sender.exe " + filename).c_str();
	char* commandCstr = new char[command.length() + 1];
	strcpy(commandCstr, command.c_str());
	for (int i = 0; i < processCount; i++) {
		if (!CreateProcess(NULL, TEXT(commandCstr), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i])) {
			printf("Error (%d)", GetLastError());
		}
	}
	

	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);

	cout << "Available commands:\nexit - quit the program\nread - read the message from the sender\n";

	char buffer[20];
	while (command != "exit") {
		cin >> command;
		if (command == "read") {

			bool failed;
			do {
				WaitForSingleObject(mutex, INFINITE);
				in.open(filename, ios::binary | ios::in);
				in.read(buffer, 20);

				failed = in.fail();
				string contents = "";
				while (!in.fail()) {
					contents += (char)in.get();
				}
				if (!contents.empty()) {
					contents.erase(contents.end() - 1);
				}
				in.close();
				in.open(filename, ios::binary | ios::out | ios::trunc);
				in.write(contents.c_str(), contents.length());
				in.flush();
				in.close();

				ReleaseMutex(mutex);
			} while (failed);
			ReleaseSemaphore(semaphore, 1, NULL);

			cout << buffer << endl;
		}
	}

	for (int i = 0; i < processCount; i++)
		CloseHandle(processes[i]);

	CloseHandle(file);
	CloseHandle(mutex);
	CloseHandle(semaphore);

	return 0;
}