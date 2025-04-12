#include <iostream> //입출력을 처리하기 위한 해더 파일. cout, end1을 사용.
#include <thread> // std::thread 클래스를 제공.
#include <Windows.h>

using namespace std;

void worker();

void worker() {
	for (int i = 0; i < 10; ++i) {
		Sleep(1000);
		cout << i << endl;
	}
}

int main() {
	thread t1(worker);
	t1.join();
	printf("===end of main()===\n");
	return 0;
}