#include <iostream> //������� ó���ϱ� ���� �ش� ����. cout, end1�� ���.
#include <thread> // std::thread Ŭ������ ����.
#include <Windows.h>

using namespace std;

void worker();

///����1: join()
//void worker() {
//	for (int i = 0; i < 10; ++i) {
//		Sleep(1000);
//		cout << i << endl;
//	}
//}
//
//int main() {
//	thread t1(worker); //���ο� ������ t1�� ���� worker �Լ��� �����϶�� ��.
//	t1.join(); //t1 �����尡 ���� ������ ���� ���α׷��� ��ٸ��� ��.
//	printf("===end of main()===\n");
//	return 0;
//}


///����2: detach()
void worker() { 
	for (int i = 0; i < 10; ++i) { 
		Sleep(1000); 
		cout << i << endl; 
	} 
}
int main() { 
	thread t1(worker); 
	t1.detach(); //main�Լ��� ������� ����. main�� ��������, �и��� ������� �ý����� �����ϸ� ��� ����� �� ����. (��, ���α׷��� ������ ����Ǹ� �����嵵 �����).
	printf("===end of main()===\n"); 
	Sleep(5000); 
	return 0; 
}