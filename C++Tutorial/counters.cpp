#include <iostream> //입출력을 처리하기 위한 해더 파일. cout, end1을 사용.
#include <thread> // std::thread 클래스를 제공.
#include <Windows.h>

using namespace std;

void worker();

///예제1: join()
//void worker() {
//	for (int i = 0; i < 10; ++i) {
//		Sleep(1000);
//		cout << i << endl;
//	}
//}
//
//int main() {
//	thread t1(worker); //새로운 스레드 t1을 만들어서 worker 함수를 실행하라는 뜻.
//	t1.join(); //t1 스레드가 끝날 때까지 메인 프로그램을 기다리게 함.
//	printf("===end of main()===\n");
//	return 0;
//}


///예제2: detach()
void worker() { 
	for (int i = 0; i < 10; ++i) { 
		Sleep(1000); 
		cout << i << endl; 
	} 
}
int main() { 
	thread t1(worker); 
	t1.detach(); //main함수와 연결되지 않음. main이 끝나더라도, 분리된 스레드는 시스템이 관리하며 계속 실행될 수 있음. (단, 프로그램이 완전히 종료되면 스레드도 종료됨).
	printf("===end of main()===\n"); 
	Sleep(5000); 
	return 0; 
}