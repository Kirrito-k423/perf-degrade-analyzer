#include <iostream>
#include <chrono>
#include <pthread.h>
#include <sched.h>

#define NUM_OPERATIONS 1000000

// 设置线程绑定到特定的 CPU 核心
void set_cpu_affinity(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    // 获取当前线程并设置 CPU 亲和性
    pthread_t current_thread = pthread_self();
    int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        std::cerr << "Error setting CPU affinity for thread on CPU " << cpu_id << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // 处理输入的绑定核心参数
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <cpu_id>" << std::endl;
        return -1;
    }

    int cpu_id = std::atoi(argv[1]);
    if (cpu_id < 0) {
        std::cerr << "Invalid CPU ID: " << argv[1] << std::endl;
        return -1;
    }

    // 绑定到指定的 CPU 核心
    set_cpu_affinity(cpu_id);

    // 执行加法运算
    volatile long result = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        result += i;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Elapsed time on CPU " << cpu_id << ": " 
              << elapsed.count() << " seconds." << std::endl;

    return 0;
}
