#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <conio.h>
#include <windows.h>

using namespace std;

const int MAX_COUNTERS = 16;  //�ִ� ī���� ����

//ī���� ����ü
struct Counter {
    atomic<int> value{ 0 }; //ī��Ʈ ��
    atomic<bool> running{ false }; //���� ����
    int id = 0; //ī���� ��ȣ
};

//���� ����
int selected = 0; //���� ���õ� ī����
atomic<bool> terminate_all{ false }; //���α׷� ���� �÷���
mutex print_mutex; //��� ����ȭ�� ���ؽ�

//�ܼ� ȭ�� �ʺ� ��������
int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

//Ŀ���� ȭ�� �� ���� �̵�, ����ȭ�� ���� �����
void moveCursorTop() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//�� �� �����
void clearLine(int y, int width) {
    COORD coord = { 0, static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    cout << string(width, ' ');
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//ī���� ���� ���
void print_status(Counter counters[MAX_COUNTERS], int n) {
    lock_guard<mutex> lock(print_mutex);
    const int width = getConsoleWidth();
    const int output_width = 25; //���� ��� ��

    moveCursorTop(); //ȭ�� �ֻ������ �̵�, ����ȭ�� ���� �����.

    //�� ī���� ���� ���
    for (int i = 0; i < n; ++i) {
        clearLine(i, width);
        cout << "counter" << counters[i].id << " : " <<
            counters[i].value << " (" <<
            (counters[i].running ? "counting" : "paused") << ")";
        if (cout.tellp() < output_width) cout << string(output_width - cout.tellp(), ' ');
    }

    clearLine(n, width);  //ī���� ��ϰ� �������� ���п� ����

    //���� ����/���� ���� ���
    clearLine(n + 1, width);
    cout << "counter" << selected << " -> counter" << ((selected + 1) % n);
    if (cout.tellp() < output_width) cout << string(output_width - cout.tellp(), ' ');

    clearLine(n + 2, width);
    cout << "current: counter" << selected << " (" <<
        (counters[selected].running ? "counting" : "paused") << ")";
    if (cout.tellp() < output_width) cout << string(output_width - cout.tellp(), ' ');
}

//ī���� ���� �ֱ������� �����ϴ� ������
void counter_manager(Counter counters[MAX_COUNTERS], int n, int max, int freq) {
    while (!terminate_all) {
        auto start = clock();

        for (int i = 0; i < n; ++i) {
            if (counters[i].running) {
                counters[i].value = (counters[i].value + 1) % (max + 1);
                print_status(counters, n); //�� ���� �� ȭ�� ����
            }
        }

        auto elapsed = clock() - start;
        auto sleep_ms = max(0, 1000 / freq - static_cast<int>(elapsed));
        this_thread::sleep_for(chrono::milliseconds(sleep_ms));
    }
}

//����� �Է��� ó���ϴ� ������
void ui_thread(Counter counters[MAX_COUNTERS], int n) {
    print_status(counters, n); //�ʱ� ���

    while (!terminate_all) {
        if (_kbhit()) { //Ű �Է� ����
            int key = _getch();
            if (key == 'q') { //q�Է½� ����
                terminate_all = true;
                break;
            }
            else if (key == 'n') { //n�Է½� ���� ī���� ����
                selected = (selected + 1) % n;
                print_status(counters, n);
            }
            else if (key == ' ') { //�����̽��ٷ� ���õ� ī���� ����/����
                counters[selected].running = !counters[selected].running;
                print_status(counters, n);
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50)); //�ʹ����� üũ����
    }
}

//���ڿ��� ������ ��ȯ
int string_to_int(const char* str) {
    int result = 0;
    while (*str != '\0') {
        result = result * 10 + (*str - '0');
        ++str;
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <n: 1~16> <freq: Hz> <max>\n";
        return 1;
    }

    //����� ���� �Ľ�
    int n = string_to_int(argv[1]);
    int freq = string_to_int(argv[2]);
    int max = string_to_int(argv[3]);

    if (n < 1 || n > MAX_COUNTERS || freq <= 0 || max < 0) {
        cerr << "Invalid arguments.\n";
        return 1;
    }

    Counter counters[MAX_COUNTERS];
    for (int i = 0; i < n; ++i) counters[i].id = i;

    //��� ���� Ȯ��
    for (int i = 0; i < n + 5; ++i) cout << endl;

    //������ ����
    thread counter(counter_manager, counters, n, max, freq);
    thread ui(ui_thread, counters, n);

    //������ ���� ���
    ui.join();
    counter.join();

    cout << "\n=== end of program ===\n";
    return 0;
}
