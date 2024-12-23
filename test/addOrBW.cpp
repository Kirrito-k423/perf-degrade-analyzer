#include <iostream>
#include <chrono>
#include <pthread.h>
#include <sched.h>
#include <cstdlib>

#define BASE_NUM_OPERATIONS 10000
#define BASE_CACHE_SIZE (1024 * 1024 * 100)  // 默认 100 MB 缓存大小

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
void perform_addition(int num_operations) {
    volatile long result = 0;
    for (int i = 0; i < num_operations; ++i) {
        result += i;
    }
}

// 内存带宽测试
void perform_memory_bandwidth_test(int cache_size) {
    // 创建一个大的数组用于缓存访问测试
    long* data = new long[cache_size / sizeof(long)];

    // 初始化数组
    for (int i = 0; i < cache_size / sizeof(long); ++i) {
        data[i] = i;
    }

    // 测量内存带宽
    auto start_time = std::chrono::high_resolution_clock::now();

    volatile long sum = 0;
    for (int i = 0; i < cache_size / sizeof(long); ++i) {
        sum += data[i];  // 访问内存
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    delete[] data;

    std::chrono::duration<double> elapsed = end_time - start_time;
    double bandwidth = cache_size / elapsed.count() / (1024 * 1024 * 1024);  // GB/s

    std::cout << "Memory bandwidth: " << bandwidth << " GB/s" << std::endl;
}

int main(int argc, char* argv[]) {
    // 处理输入的绑定核心参数和操作类型
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <cpu_id> <operation_type> <scale_factor>" << std::endl;
        return -1;
    }

    int cpu_id = std::atoi(argv[1]);
    if (cpu_id < 0) {
        std::cerr << "Invalid CPU ID: " << argv[1] << std::endl;
        return -1;
    }

    std::string operation_type = argv[2];
    int scale_factor = std::atoi(argv[3]);
    if (scale_factor <= 0) {
        std::cerr << "Invalid scale factor: " << argv[3] << std::endl;
        return -1;
    }

    // 计算实际的 NUM_OPERATIONS 和 CACHE_SIZE
    int num_operations = BASE_NUM_OPERATIONS * scale_factor;
    int cache_size = BASE_CACHE_SIZE * scale_factor;

    // 绑定到指定的 CPU 核心
    set_cpu_affinity(cpu_id);

    // 根据第二个参数选择操作类型
    if (operation_type == "add") {
        auto start_time = std::chrono::high_resolution_clock::now();
        perform_addition(num_operations);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        std::cout << "Addition operation elapsed time on CPU " << cpu_id << " with " << num_operations << " operations: " << elapsed.count() << " seconds." << std::endl;
    } 
    else if (operation_type == "memory_bandwidth") {
        auto start_time = std::chrono::high_resolution_clock::now();
        perform_memory_bandwidth_test(cache_size);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        std::cout << "Memory bandwidth test elapsed time on CPU " << cpu_id << " with " << cache_size / (1024 * 1024) << " MB of data: " << elapsed.count() << " seconds." << std::endl;
    }
    else {
        std::cerr << "Invalid operation type. Use 'add' or 'memory_bandwidth'." << std::endl;
        return -1;
    }

    return 0;
}
