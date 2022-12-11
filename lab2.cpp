#include <iostream>
#include <mutex>
#include <Windows.h>
using namespace std;

volatile int arrLength, minValue, maxValue, averageValue;

mutex coutMutex;

DWORD WINAPI min_max_thread(LPVOID wrappedArr) {
	const int* arr = (int*)wrappedArr;
	int min = arr[0], max = arr[0];

	for (int i = 0; i < arrLength; i++) {
		if (min > arr[i]) {
			min = arr[i];
		}
		Sleep(7);
		if (max < arr[i]) {
			max = arr[i];
		}
		Sleep(12);
	}

	minValue = min, maxValue = max;

	coutMutex.lock();
	cout << "Min and max values:\n";
	cout << min << ' ' << max << '\n';
	coutMutex.unlock();

	return 0;
}

DWORD WINAPI average_thread(LPVOID wrappedArr) {
	const int* arr = (int*) wrappedArr;
	int sum = 0;

	for (int i = 0; i < arrLength; i++) {
		sum += arr[i];
		Sleep(12);
	}

	averageValue = sum / arrLength;

	coutMutex.lock();
	cout << "Average value:\n";
	cout << averageValue << '\n';
	coutMutex.unlock();

	return 0;
}

int main() {
	HANDLE hAverageThread, hMinMaxThread;
	DWORD IDAverageThread, IDMinMaxThread;

	int n;
	cin >> n;

	int* arr = new int[n];
	for (int i = 0; i < n; i++) {
		cin >> arr[i];
	}

	arrLength = n;
	hAverageThread = CreateThread(NULL, 0, average_thread, (void*) arr, 0, &IDAverageThread);
	hMinMaxThread = CreateThread(NULL, 0, min_max_thread, (void*) arr, 0, &IDMinMaxThread);

	HANDLE handles[2] = {hAverageThread, hMinMaxThread};
	WaitForMultipleObjects(2, handles, true, INFINITE);

	for (int i = 0; i < n; i++) {
		if (arr[i] == maxValue || arr[i] == minValue) {
			arr[i] = averageValue;
		}
	}

	cout << "Modified array:\n";
	for (int i = 0; i < n; i++) {
		cout << arr[i] << ' ';
	}
	cout << '\n';

	system("pause");

	return 0;
}