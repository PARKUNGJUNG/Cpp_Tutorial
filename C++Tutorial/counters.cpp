#include <iostream>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <conio.h>
#include <windows.h>

using namespace std;

const int MAX_COUNTERS = 16;  //최대 카운터 개수

//카운터 구조체
struct Counter {
    atomic<int> value{ 0 }; //카운트 값
    atomic<bool> running{ false }; //동작 상태
    int id = 0; //카운터 번호
};

//전역 변수
int selected = 0; //현재 선택된 카운터
atomic<bool> terminate_all{ false }; //프로그램 종료 플래그
mutex print_mutex; //출력 동기화용 뮤텍스

//콘솔 화면 너비 가져오기
int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

//커서를 화면 맨 위로 이동, 최적화를 위해 사용함
void moveCursorTop() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//한 줄 지우기
void clearLine(int y, int width) {
    COORD coord = { 0, static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    cout << string(width, ' ');
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//카운터 상태 출력
void print_status(Counter counters[MAX_COUNTERS], int n) {
    lock_guard<mutex> lock(print_mutex);
    const int width = getConsoleWidth();
    const int output_width = 25; //고정 출력 폭

    moveCursorTop(); //화면 최상단으로 이동, 최적화를 위해 사용함.

    //각 카운터 정보 출력
    for (int i = 0; i < n; ++i) {
        clearLine(i, width);
        cout << "counter" << counters[i].id << " : " <<
            counters[i].value << " (" <<
            (counters[i].running ? "counting" : "paused") << ")";
        if (cout.tellp() < output_width) cout << string(output_width - cout.tellp(), ' ');
    }

    clearLine(n, width);  //카운터 목록과 상태정보 구분용 공백

    //현재 선택/변경 상태 출력
    clearLine(n + 1, width);
    cout << "counter" << selected << " -> counter" << ((selected + 1) % n);
    if (cout.tellp() < output_width) cout << string(output_width - cout.tellp(), ' ');

    clearLine(n + 2, width);
    cout << "current: counter" << selected << " (" <<
        (counters[selected].running ? "counting" : "paused") << ")";
    if (cout.tellp() < output_width) cout << string(output_width - cout.tellp(), ' ');
}

//카운터 값을 주기적으로 갱신하는 스레드
void counter_manager(Counter counters[MAX_COUNTERS], int n, int max, int freq) {
    while (!terminate_all) {
        auto start = clock();

        for (int i = 0; i < n; ++i) {
            if (counters[i].running) {
                counters[i].value = (counters[i].value + 1) % (max + 1);
                print_status(counters, n); //값 변경 시 화면 갱신
            }
        }

        auto elapsed = clock() - start;
        auto sleep_ms = max(0, 1000 / freq - static_cast<int>(elapsed));
        this_thread::sleep_for(chrono::milliseconds(sleep_ms));
    }
}

//사용자 입력을 처리하는 스레드
void ui_thread(Counter counters[MAX_COUNTERS], int n) {
    print_status(counters, n); //초기 출력

    while (!terminate_all) {
        if (_kbhit()) { //키 입력 감지
            int key = _getch();
            if (key == 'q') { //q입력시 종료
                terminate_all = true;
                break;
            }
            else if (key == 'n') { //n입력시 다음 카운터 선택
                selected = (selected + 1) % n;
                print_status(counters, n);
            }
            else if (key == ' ') { //스페이스바로 선택된 카운터 실행/정지
                counters[selected].running = !counters[selected].running;
                print_status(counters, n);
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50)); //너무빠른 체크방지
    }
}

//문자열을 정수로 변환
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

    //명령줄 인자 파싱
    int n = string_to_int(argv[1]);
    int freq = string_to_int(argv[2]);
    int max = string_to_int(argv[3]);

    if (n < 1 || n > MAX_COUNTERS || freq <= 0 || max < 0) {
        cerr << "Invalid arguments.\n";
        return 1;
    }

    Counter counters[MAX_COUNTERS];
    for (int i = 0; i < n; ++i) counters[i].id = i;

    //출력 공간 확보
    for (int i = 0; i < n + 5; ++i) cout << endl;

    //스레드 실행
    thread counter(counter_manager, counters, n, max, freq);
    thread ui(ui_thread, counters, n);

    //스레드 종료 대기
    ui.join();
    counter.join();

    cout << "\n=== end of program ===\n";
    return 0;
}
