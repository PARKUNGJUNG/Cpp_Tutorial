#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <string>
#include <atomic>
using namespace std;

// 전역 변수
mutex mtx; // 출력 및 데이터 동기화
vector<int> counters; // 각 카운터의 현재 값
vector<bool> is_counting; // 각 카운터의 상태 (true: counting, false: paused)
atomic<bool> terminate_flag{ false }; // 프로그램 종료 플래그
int selected_counter = 0; // 현재 선택된 카운터 인덱스
int n, max_count; // 카운터 개수, 최대값
double freq; // 초당 카운트 횟수 (Hz)

/*
 * Visual Studio 설정 방법:
 * 1. 솔루션 탐색기에서 프로젝트 우클릭 -> 속성
 * 2. 구성 속성 -> 일반 -> C++ 언어 표준: ISO C++17 (/std:c++17)
 * 3. 구성 속성 -> 디버깅 -> 명령 인수: 예: "3 2 9"
 * 4. 디버그 모드로 실행 (F5)
 * 명령 프롬프트 실행 예: C++Tutorial.exe 3 2 9
 * 키 동작:
 *   - 'n': 다음 카운터 선택 (counter0 -> counter1 -> ...)
 *   - Space: 선택된 카운터 카운팅/일시정지 토글 (0~9 순환)
 *   - 'q': 프로그램 종료
 */

 // 화면 지우기 함수
void clear_screen() {
    system("cls");
}

// 시스템 메시지 출력
void print_system_message(const string& msg) {
    lock_guard<mutex> lock(mtx);
    cout << msg << endl;
}

// 카운터 스레드 함수
void counter_thread(int id, double delay_ms) {
    while (!terminate_flag) {
        if (is_counting[id]) {
            lock_guard<mutex> lock(mtx);
            counters[id]++;
            if (counters[id] > max_count) {
                counters[id] = 0;
            }
        }
        Sleep(static_cast<int>(delay_ms));
    }
}

// UI 스레드 함수
void ui_thread() {
    while (!terminate_flag) {
        // 키 입력 처리
        if (_kbhit()) {
            int key = _getch();
            lock_guard<mutex> lock(mtx);
            if (key == 'q') {
                terminate_flag = true;
                print_system_message("Program terminating...");
            }
            else if (key == 'n') {
                int prev = selected_counter;
                selected_counter = (selected_counter + 1) % n;
                print_system_message("counter" + to_string(prev) + " -> counter" + to_string(selected_counter));
            }
            else if (key == ' ') {
                is_counting[selected_counter] = !is_counting[selected_counter];
                string state = is_counting[selected_counter] ? "activated" : "paused";
                print_system_message("counter" + to_string(selected_counter) + " " + state);
            }
        }

        // 화면 출력
        {
            lock_guard<mutex> lock(mtx);
            clear_screen();
            for (int i = 0; i < n; ++i) {
                string state = is_counting[i] ? "counting" : "paused";
                cout << "counter" << i << " : " << counters[i] << " (" << state << ")" << endl;
            }
            string state = is_counting[selected_counter] ? "counting" : "paused";
            cout << "current: counter" << selected_counter << " (" << state << ")" << endl;
            cout.flush();
        }
        Sleep(200); // 갱신 주기
    }
}

int main(int argc, char* argv[]) {
    // 기본값 설정
    n = 3; // 기본 카운터 개수
    freq = 2.0; // 기본 초당 횟수 (Hz)
    max_count = 9; // 기본 최대값 (0~9)

    // 명령행 인자 처리
    if (argc == 4) {
        try {
            n = stoi(argv[1]);
            freq = stod(argv[2]);
            // max_count는 9로 고정 (요구사항: 0~9)
        }
        catch (...) {
            lock_guard<mutex> lock(mtx);
            cout << "Invalid arguments. Using default values: n=3, freq=2, max=9" << endl;
        }

        // 인자 유효성 검사
        if (n < 1 || n > 16 || freq <= 0) {
            lock_guard<mutex> lock(mtx);
            cout << "Invalid arguments. Constraints: 1 <= n <= 16, freq > 0" << endl;
            cout << "Using default values: n=3, freq=2, max=9" << endl;
            n = 3;
            freq = 2.0;
            max_count = 9;
        }
    }
    else if (argc != 1) {
        lock_guard<mutex> lock(mtx);
        cout << "Usage: " << argv[0] << " <n> <freq> <max>" << endl;
        cout << "Using default values: n=3, freq=2, max=9" << endl;
    }
    else {
        lock_guard<mutex> lock(mtx);
        cout << "No arguments provided. Using default values: n=3, freq=2, max=9" << endl;
    }

    // 초기화
    counters.resize(n, 0); // 모든 카운터 0으로 초기화
    is_counting.resize(n, false); // 모든 카운터 paused 상태로 초기화
    double delay_ms = 1000.0 / freq; // 카운트 주기 (ms)

    // 스레드 생성
    vector<thread> counter_threads;
    counter_threads.reserve(n);
    for (int i = 0; i < n; ++i) {
        counter_threads.push_back(thread(counter_thread, i, delay_ms));
    }
    thread ui(ui_thread);

    // 스레드 종료 대기
    if (ui.joinable()) {
        ui.join();
    }
    for (auto& t : counter_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    {
        lock_guard<mutex> lock(mtx);
        cout << "===end of main()===" << endl;
    }
    return 0;
}