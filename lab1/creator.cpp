#include <iostream>
#include <fstream>
using namespace std;

struct Employee {
	int num;
	char name[10];
	double hours;

	friend ostream& operator<<(ostream& out, Employee& employee) {
		out.write((char*)&employee.num, sizeof(int));
		out.write(employee.name, sizeof(char) * 10);
		out.write((char*)&employee.hours, sizeof(double));
		return out;
	}
};

int main(int argc, char* argv[]) {
	string filename = argv[1];
	int count = atoi(argv[2]);

	ofstream out(filename, ios::binary);

	cout << "\nEnter all employees' info (ID, name, hours):\n";

	for (int i = 0; i < count; i++) {
		Employee employee;
		cin >> employee.num >> employee.name >> employee.hours;
		out << employee;
	}

	return 0;
}