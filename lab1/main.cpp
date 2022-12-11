#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

void printEmployees(string filename) {
	ifstream in(filename, ios::binary);
	
	int num;
	char name[10];
	double hours;
	while (!in.eof()) {
		in.read((char*)&num, sizeof(int));
		if (in.fail()) {
			break;
		}
		in.read(name, sizeof(char) * 10);
		in.read((char*)&hours, sizeof(double));

		cout << num << " " << name << " " << hours << '\n';
	}
}

void printFileContent(string filename) {
	ifstream in(filename);
	string buffer;

	while (getline(in, buffer)) {
		cout << buffer << '\n';
	}
}

int main(int argc, TCHAR *argv[]) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	string inputFilename;
	int n;
	cout << "Enter binary file name: ";
	cin >> inputFilename;
	cout << "Enter number of employees: ";
	cin >> n;

	stringstream ss;
	ss << "Creator.exe " << inputFilename << " " << n;

	char* args = (char*)malloc(ss.str().length() + 1);
	strcpy(args, ss.str().c_str());

	if (!CreateProcess(
		NULL,
		TEXT(args),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		printf("CreateProcess failed (%d).", GetLastError());
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	cout << "\nEmployees:\n";
	printEmployees(inputFilename);

	string reportFilename;
	double price;

	cout << "\nEnter report filename: ";
	cin >> reportFilename;
	cout << "Enter price: ";
	cin >> price;

	ss = stringstream();
	ss << "Reporter.exe " << inputFilename << " " << reportFilename << " " << price;

	args = (char*)malloc(ss.str().length() + 1);
	strcpy(args, ss.str().c_str());


	if (!CreateProcess(
		NULL,
		TEXT(args),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		return -1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	cout << "\nReport:\n";
	printFileContent(reportFilename);

	system("pause");

	return 0;
}