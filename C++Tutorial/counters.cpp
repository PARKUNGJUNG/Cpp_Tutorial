#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <string>
#include <atomic>
using namespace std;

// ���� ����
mutex mtx; // ��� �� ������ ����ȭ
vector<int> counters; // �� ī������ ���� ��
vector<bool> is_counting; // �� ī������ ���� (true: counting, false: paused)
atomic<bool> terminate_flag{ false }; // ���α׷� ���� �÷���
int selected_counter = 0; // ���� ���õ� ī���� �ε���
int n, max_count; // ī���� ����, �ִ밪
double freq; // �ʴ� ī��Ʈ Ƚ�� (Hz)

/*
 * Visual Studio ���� ���:
 * 1. �ַ�� Ž���⿡�� ������Ʈ ��Ŭ�� -> �Ӽ�
 * 2. ���� �Ӽ� -> �Ϲ� -> C++ ��� ǥ��: ISO C++17 (/std:c++17)
 * 3. ���� �Ӽ� -> ����� -> ��� �μ�: ��: "3 2 9"
 * 4. ����� ���� ���� (F5)
 * ��� ������Ʈ ���� ��: C++Tutorial.exe 3 2 9
 * Ű ����:
 *   - 'n': ���� ī���� ���� (counter0 -> counter1 -> ...)
 *   - Space: ���õ� ī���� ī����/�Ͻ����� ��� (0~9 ��ȯ)
 *   - 'q': ���α׷� ����
 */

 // ȭ�� ����� �Լ�
void clear_screen() {
    system("cls");
}

// �ý��� �޽��� ���
void print_system_message(const string& msg) {
    lock_guard<mutex> lock(mtx);
    cout << msg << endl;
}

// ī���� ������ �Լ�
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

// UI ������ �Լ�
void ui_thread() {
    while (!terminate_flag) {
        // Ű �Է� ó��
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

        // ȭ�� ���
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
        Sleep(200); // ���� �ֱ�
    }
}

int main(int argc, char* argv[]) {
    // �⺻�� ����
    n = 3; // �⺻ ī���� ����
    freq = 2.0; // �⺻ �ʴ� Ƚ�� (Hz)
    max_count = 9; // �⺻ �ִ밪 (0~9)

    // ����� ���� ó��
    if (argc == 4) {
        try {
            n = stoi(argv[1]);
            freq = stod(argv[2]);
            // max_count�� 9�� ���� (�䱸����: 0~9)
        }
        catch (...) {
            lock_guard<mutex> lock(mtx);
            cout << "Invalid arguments. Using default values: n=3, freq=2, max=9" << endl;
        }

        // ���� ��ȿ�� �˻�
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

    // �ʱ�ȭ
    counters.resize(n, 0); // ��� ī���� 0���� �ʱ�ȭ
    is_counting.resize(n, false); // ��� ī���� paused ���·� �ʱ�ȭ
    double delay_ms = 1000.0 / freq; // ī��Ʈ �ֱ� (ms)

    // ������ ����
    vector<thread> counter_threads;
    counter_threads.reserve(n);
    for (int i = 0; i < n; ++i) {
        counter_threads.push_back(thread(counter_thread, i, delay_ms));
    }
    thread ui(ui_thread);

    // ������ ���� ���
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