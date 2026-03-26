#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib> 
#include <ctime>   

#include <omp.h>
#include <cmath>

long long partition(std::vector<long long>& arr, long long low, long long high) {
    long long pivot = arr[high];
    long long i = (low - 1);

    for (long long j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}


void quickSortSequential(std::vector<long long>& arr, long long low, long long high) {
    if (low < high) {
        long long pi = partition(arr, low, high);
        quickSortSequential(arr, low, pi - 1);
        quickSortSequential(arr, pi + 1, high);
    }
}


void quickSortParallel(std::vector<long long>& arr, long long low, long long high, long long cutoff) {
    if (low < high) {
        long long pi = partition(arr, low, high); 

        if (high - low < cutoff) { 
            
            quickSortSequential(arr, low, pi - 1);
            quickSortSequential(arr, pi + 1, high);
        }
        else {
            // Создаём задачу для левой части
#pragma omp task shared(arr) firstprivate(low, pi, cutoff)
            quickSortParallel(arr, low, pi - 1, cutoff);

            // Создаём задачу для правой части
#pragma omp task shared(arr) firstprivate(pi, high, cutoff)
            quickSortParallel(arr, pi + 1, high, cutoff);

#pragma omp taskwait
        }
    }
}



int g_max_depth = 0;

void quickSortAdaptive(std::vector<long long>& arr, long long low, long long high, int depth = 0) {
    if (low < high) {
        long long pi = partition(arr, low, high);


#pragma omp task shared(arr) firstprivate(low, pi, depth) if(depth < g_max_depth)
        quickSortAdaptive(arr, low, pi - 1, depth + 1);

#pragma omp task shared(arr) firstprivate(pi, high, depth) if(depth < g_max_depth)
        quickSortAdaptive(arr, pi + 1, high, depth + 1);

        if (depth < g_max_depth) {
#pragma omp taskwait
        }
    }
}

int main() {
    const long long N = 10000000;
    std::vector<long long> data_orig(N);
    std::srand(std::time(0));
    for (long long i = 0; i < N; ++i) data_orig[i] = std::rand();

    // ПОСЛЕДОВАТЕЛЬНЫЙ ЗАПУСК
    std::vector<long long> data_seq = data_orig;
    auto start = std::chrono::high_resolution_clock::now();
    quickSortSequential(data_seq, 0, N - 1);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_seq = end - start;
    std::cout << "Sequential Time: " << diff_seq.count() << " s" << std::endl;

    // ПАРАЛЛЕЛЬНЫЙ ЗАПУСК 100
    std::vector<long long> data_par_100 = data_orig;
    start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        // Только один поток начинает рекурсию, остальные ждут задач
#pragma omp single
        quickSortParallel(data_par_100, 0, N - 1, 100);
    }

    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_par = end - start;
    std::cout << "Parallel Time (cutoff = 100): " << diff_par.count() << " s" << std::endl;


    // ПАРАЛЛЕЛЬНЫЙ ЗАПУСК 1000
    std::vector<long long> data_par_1000 = data_orig;
    start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        
#pragma omp single
        quickSortParallel(data_par_1000, 0, N - 1, 1000);
    }

    end = std::chrono::high_resolution_clock::now();
    diff_par = end - start;
    std::cout << "Parallel Time (cutoff = 1000): " << diff_par.count() << " s" << std::endl;

    // ПАРАЛЛЕЛЬНЫЙ ЗАПУСК 10000
    std::vector<long long> data_par_10000 = data_orig;
    start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        
#pragma omp single
        quickSortParallel(data_par_10000, 0, N - 1, 10000);
    }

    end = std::chrono::high_resolution_clock::now();
    diff_par = end - start;
    std::cout << "Parallel Time (cutoff = 10000): " << diff_par.count() << " s" << std::endl;


    int num_threads;
#pragma omp parallel
    {
#pragma omp single
        num_threads = omp_get_num_threads(); 
    }

    g_max_depth = static_cast<int>(std::log2(num_threads))+2;

    std::cout << "Threads: " << num_threads << ", Adaptive Max Depth: " << g_max_depth << std::endl;

    std::vector<long long> data_par_ad = data_orig;

    start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
#pragma omp single
        quickSortAdaptive(data_par_ad, 0, N - 1, 0);
    }
    end = std::chrono::high_resolution_clock::now();
    diff_par = end - start;
    std::cout << "Parallel Time (adaptive): " << diff_par.count() << " s" << std::endl;


    if (std::is_sorted(data_seq.begin(), data_seq.end())) {
        std::cout << "[OK] Sequential Sort: Sorted correctly." << std::endl;
    }
    else {
        std::cout << "[ERROR] Sequential Sort: Sorting failed!" << std::endl;
    }

    if (std::is_sorted(data_par_100.begin(), data_par_100.end())) {
        std::cout << "[OK] Parallel Sort (100): Sorted correctly." << std::endl;
    }
    else {
        std::cout << "[ERROR] Parallel Sort (100): Sorting failed!" << std::endl;
    }

    if (std::is_sorted(data_par_1000.begin(), data_par_1000.end())) {
        std::cout << "[OK] Parallel Sort (1000): Sorted correctly." << std::endl;
    }
    else {
        std::cout << "[ERROR] Parallel Sort (1000): Sorting failed!" << std::endl;
    }

    if (std::is_sorted(data_par_10000.begin(), data_par_10000.end())) {
        std::cout << "[OK] Parallel Sort (10000): Sorted correctly." << std::endl;
    }
    else {
        std::cout << "[ERROR] Parallel Sort (10000): Sorting failed!" << std::endl;
    }
    if (std::is_sorted(data_par_ad.begin(), data_par_ad.end())) {
        std::cout << "[OK] Parallel Sort (adaptive): Sorted correctly." << std::endl;
    }
    else {
        std::cout << "[ERROR] Parallel Sort (adaptive): Sorting failed!" << std::endl;
    }

    return 0;
}