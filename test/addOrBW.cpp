#include <iostream>
#include <chrono>
#include <pthread.h>
#include <sched.h>
#include <cstdlib>

#define NUM_OPERATIONS 10000
#define CACHE_SIZE (1024 * 1024 * 100)  // 100 MB 缓存大小，用于内存带宽测试

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

// 加法运算测试
void perform_addition() {
    volatile long result = 0;
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        result += i;
    }
}

// 内存带宽测试
void perform_memory_bandwidth_test() {
    // 创建一个大的数组用于缓存访问测试
    long* data = new long[CACHE_SIZE / sizeof(long)];

    // 初始化数组
    for (int i = 0; i < CACHE_SIZE / sizeof(long); ++i) {
        data[i] = i;
    }

    // 测量内存带宽
    auto start_time = std::chrono::high_resolution_clock::now();

    volatile long sum = 0;
    for (int i = 0; i < CACHE_SIZE / sizeof(long); ++i) {
        sum += data[i];  // 访问内存
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    delete[] data;

    std::chrono::duration<double> elapsed = end_time - start_time;
    double bandwidth = CACHE_SIZE / elapsed.count() / (1024 * 1024 * 1024);  // GB/s

    std::cout << "Memory bandwidth: " << bandwidth << " GB/s" << std::endl;
}

int main(int argc, char* argv[]) {
    // 处理输入的绑定核心参数
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <cpu_id> <operation_type>" << std::endl;
        return -1;
    }

    int cpu_id = std::atoi(argv[1]);
    if (cpu_id < 0) {
        std::cerr << "Invalid CPU ID: " << argv[1] << std::endl;
        return -1;
    }

    std::string operation_type = argv[2];

    // 绑定到指定的 CPU 核心
    set_cpu_affinity(cpu_id);

    // 根据第二个参数选择操作类型
    if (operation_type == "add") {
        auto start_time = std::chrono::high_resolution_clock::now();
        perform_addition();
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        std::cout << "Addition operation elapsed time on CPU " << cpu_id << ": " << elapsed.count() << " seconds." << std::endl;
    } 
    else if (operation_type == "memory_bandwidth") {
        auto start_time = std::chrono::high_resolution_clock::now();
        perform_memory_bandwidth_test();
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        std::cout << "Memory bandwidth test elapsed time on CPU " << cpu_id << ": " << elapsed.count() << " seconds." << std::endl;
    }
    else {
        std::cerr << "Invalid operation type. Use 'add' or 'memory_bandwidth'." << std::endl;
        return -1;
    }

    return 0;
}
