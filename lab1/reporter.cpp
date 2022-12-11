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

	friend istream& operator>>(istream& in, Employee& employee) {
		in.read((char*)&employee.num, sizeof(int));
		in.read(employee.name, sizeof(char) * 10);
		in.read((char*)&employee.hours, sizeof(double));
		return in;
	}
};

int main(int argc, char* argv[]) {
	string inputFilename = argv[1];
	string reportFilename = argv[2];
	int price = atoi(argv[3]);

	ifstream in(inputFilename);
	ofstream out(reportFilename);

	out << "Report for file " << inputFilename << '\n';
	
	Employee employee;
	while(!in.eof()) {
		in >> employee;
		if (in.fail()) {
			break;
		}
		out << employee.num << " " << employee.name << " " << employee.hours << " " << price * employee.hours << '\n';
	}

	return 0;
}