#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>
#include <conio.h>   // _kbhit, _getch
#include <cstdlib>   // atoi
#include <windows.h> // Sleep, SetConsoleCursorPosition

using namespace std;

struct Counter {
    atomic<int> value{ 0 };
    atomic<bool> running{ false };
    int id;
};

int selected = 0;
bool terminate_all = false;
mutex print_mutex;

// 화면 커서를 맨 위로 이동
void moveCursorTop() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 상태 출력
void print_status(const vector<Counter>& counters) {
    lock_guard<mutex> lock(print_mutex);
    moveCursorTop();

    for (const auto& counter : counters) {
        cout << "counter" << counter.id << " : " << counter.value
            << " (" << (counter.running ? "counting" : "paused") << ")       " << endl;
    }
    cout << "\ncurrent: counter" << selected
        << " (" << (counters[selected].running ? "counting" : "paused") << ")       " << endl;
}

// 카운터 스레드 함수
void counter_thread(Counter& counter, int max, int freq) {
    while (!terminate_all) {
        if (counter.running) {
            this_thread::sleep_for(chrono::milliseconds(1000 / freq));
            counter.value = (counter.value + 1) % (max + 1);
        }
        else {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
}

// UI 스레드 함수
void ui_thread(vector<Counter>& counters) {
    print_status(counters);
    while (!terminate_all) {
        if (_kbhit()) {
            int key = _getch();
            if (key == 'q') {
                terminate_all = true;
                break;
            }
            else if (key == 'n') {
                int prev = selected;
                selected = (selected + 1) % counters.size();
                {
                    lock_guard<mutex> lock(print_mutex);
                    cout << "counter" << prev << " -> counter" << selected << endl;
                }
            }
            else if (key == ' ') {
                counters[selected].running = !counters[selected].running;
                {
                    lock_guard<mutex> lock(print_mutex);
                    cout << "counter" << selected << " "
                        << (counters[selected].running ? "activated" : "paused") << endl;
                }
            }
            print_status(counters);
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <n: 1~16> <freq: Hz> <max>" << endl;
        return 1;
    }

    int n = atoi(argv[1]);
    int freq = atoi(argv[2]);
    int max = atoi(argv[3]);

    if (n < 1 || n > 16 || freq <= 0 || max < 0) {
        cerr << "Invalid arguments." << endl;
        return 1;
    }

    vector<Counter> counters(n);
    for (int i = 0; i < n; ++i)
        counters[i].id = i;

    // 출력 공간 확보 (버퍼에 최소한의 줄 확보)
    for (int i = 0; i < n + 5; ++i) cout << endl;

    // 카운터 스레드 시작
    vector<thread> threads;
    for (int i = 0; i < n; ++i)
        threads.emplace_back(counter_thread, ref(counters[i]), max, freq);

    // UI 스레드 실행
    thread ui(ui_thread, ref(counters));
    ui.join();

    // 종료 후 카운터 스레드 정리
    for (auto& t : threads)
        t.join();

    cout << "\n=== end of program ===" << endl;
    return 0;
}