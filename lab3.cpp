#include <iostream>
#include <vector>
#include <mutex>
#include <Windows.h>
using namespace std;

volatile int arrCount;
volatile int* arr;
volatile HANDLE* startEvents;
volatile HANDLE* workStoppedEvents;
volatile HANDLE* terminateEvents;
volatile HANDLE* continueEvents;
volatile bool* threadTerminated;
volatile HANDLE coutMutex;

DWORD WINAPI marker(LPVOID wrappedNum) {
	int n;
	memcpy(&n, &wrappedNum, sizeof n);

	WaitForSingleObject(startEvents[n], INFINITE);
	ResetEvent(startEvents[n]);

	srand(n);
	while (true) {
		int i = rand();
		i %= arrCount;
		if (arr[i] == 0) {
			Sleep(5);
			arr[i] = n + 1;
			Sleep(5);
		}
		else {
			vector<int> markedIndices;

			for (int i = 0; i < arrCount; i++) {
				if (arr[i] == n) {
					markedIndices.push_back(i);
				}
			}

			WaitForSingleObject(coutMutex, INFINITE);
			cout << "Thread " << n + 1 << ": marked " << markedIndices.size() << ", forbidden index " << i << '\n';
			ReleaseMutex(coutMutex);

			SetEvent(workStoppedEvents[n]);
			HANDLE* waitingHandles = new HANDLE[2];
			waitingHandles[0] = continueEvents[n];
			waitingHandles[1] = terminateEvents[n];
			DWORD response = WaitForMultipleObjects(2, waitingHandles, false, INFINITE);

			ResetEvent(continueEvents[n]);
			ResetEvent(terminateEvents[n]);

			if (response == 1) {
				for (int i = 0; i < markedIndices.size(); i++) {
					arr[markedIndices[i]] = 0;
				}

				return 0;
			}
		}
	}
}

int main() {
	coutMutex = CreateMutex(NULL, FALSE, NULL);

	int n;
	cout << "Enter number of elements: ";
	cin >> n;
	int* array = new int[n];
	cout << "Enter " << n << " elements: ";
	for (int i = 0; i < n; i++) {
		cin >> array[i];
	}

	arrCount = n;
	arr = array;

	int threadCount;
	cout << "Enter number of threads: ";
	cin >> threadCount;

	threadTerminated = new bool[threadCount];
	for (int i = 0; i < threadCount; i++) {
		threadTerminated[i] = false;
	}

	startEvents = new HANDLE[threadCount];
	workStoppedEvents = new HANDLE[threadCount];
	terminateEvents = new HANDLE[threadCount];
	continueEvents = new HANDLE[threadCount];

	for (int i = 0; i < threadCount; i++) {
		startEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		workStoppedEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		terminateEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		continueEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	HANDLE* threadHandles = new HANDLE[threadCount];

	for (int i = 0; i < threadCount; i++) {
		threadHandles[i] = CreateThread(NULL, 0, marker, (LPVOID)i, 0, NULL);
	}

	for (int i = 0; i < threadCount; i++) {
		SetEvent(startEvents[i]);
	}

	for (int z = 0; z < threadCount; z++) {
		for (int i = 0; i < threadCount; i++) {
			if (threadTerminated[i])
				continue;

			HANDLE* waitingEvents = new HANDLE[2];
			waitingEvents[0] = workStoppedEvents[i];
			waitingEvents[1] = terminateEvents[i];
			WaitForMultipleObjects(2, waitingEvents, false, INFINITE);
		}
		for (int i = 0; i < threadCount; i++) {
			ResetEvent(workStoppedEvents[i]);
		}

		cout << "Modified array:\n";
		for (int i = 0; i < n; i++) {
			cout << array[i] << " ";
		}
		cout << '\n';

		int terminateMarker = -1;
		while (true) {
			cout << "Enter ID of thread to terminate: ";
			cin >> terminateMarker;
			terminateMarker--;

			if (terminateMarker >= 0 && terminateMarker < threadCount && !threadTerminated[terminateMarker]) {
				break;
			}
			cout << "Unknown thread ID\n";
		}

		threadTerminated[terminateMarker] = true;
		SetEvent(terminateEvents[terminateMarker]);
		WaitForSingleObject(threadHandles[terminateMarker], INFINITE);

		cout << "Thread " << terminateMarker + 1 << " terminated\n";
		cout << "Modified array:\n";
		for (int i = 0; i < n; i++) {
			cout << array[i] << " ";
		}
		cout << '\n';

		for (int i = 0; i < threadCount; i++) {
			SetEvent(continueEvents[i]);
		}
	}

	return 0;
}