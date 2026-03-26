#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <omp.h>


void fill_vector(std::vector<double>& v, int n) {
 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-100.0, 100.0);

    for (int i = 0; i < n; ++i) {
        v[i] = dis(gen);
    }
}
double scalar_product(const std::vector<double>& a, const std::vector<double>& b, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

double parallel_scalar_product(const std::vector<double>& a, const std::vector<double>& b, int n) {
    double sum = 0.0;
    int i;

    // parallel — создает группу потоков
    // shared(a, b) — вектора общие для всех
    // private(i) — у каждого потока свой счетчик цикла
    // reduction(+:sum) — каждый поток копирует sum себе, а в конце всё суммируется
#pragma omp parallel shared(a, b) private(i) reduction(+:sum)
    {
#pragma omp for
        for (i = 0; i < n; ++i) {
            sum += a[i] * b[i];
        }
    }
    return sum;
}

void fill_matrix(std::vector<std::vector<double>>& mat, int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-10.0, 10.0); 

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mat[i][j] = dis(gen);
        }
    }
}

void matrix_multiplication(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B,
                                 std::vector<std::vector<double>>& C, int M, int K, int N) {
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < K; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void parallel_matrix_multiplication(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B,
                                          std::vector<std::vector<double>>& C, int M, int K, int N) {

    int i, j, k;
   #pragma omp parallel for shared(A, B, C) private(i, j, k)
    for (i = 0; i < M; ++i) {
        for (j = 0; j < N; ++j) {
            double sum = 0.0; 
            for (k = 0; k < K; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}



bool compare_matrices(const std::vector<std::vector<double>>& A,
                      const std::vector<std::vector<double>>& B,
                                                   int M, int N) {
    const double EPS = 1e-9; 

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            if (std::abs(A[i][j] - B[i][j]) > EPS) {
                std::cout << "Mismatch at [" << i << "][" << j << "]: "
                    << A[i][j] << " != " << B[i][j] << std::endl;
                return false;
            }
        }
    }
    return true;
}




int main() {
    int n = 10000000; // 10 миллионов элементов
    std::vector<double> v1(n), v2(n);



    std::cout << "Filling vectors with random data..." << std::endl;
    fill_vector(v1, n);
    fill_vector(v2, n);
   
    
    auto start = std::chrono::high_resolution_clock::now();
    double res = scalar_product(v1, v2, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Sequential vector scalar product result:" << res << std::endl;
    std::cout << "Time: " << duration << " ms" << std::endl;

  
    start = std::chrono::high_resolution_clock::now();
    res = parallel_scalar_product(v1, v2, n);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Parallel vector scalar product result: " << res << std::endl;
    std::cout << "Time: " << duration << " ms" << std::endl;



    int M = 500; // Строки A
    int K = 500; // Столбцы A / Строки B
    int N = 500; // Столбцы B

    std::vector<std::vector<double>> A(M, std::vector<double>(K));
    std::vector<std::vector<double>> B(K, std::vector<double>(N));
    std::vector<std::vector<double>> C(M, std::vector<double>(N));

    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Filling matrices with random data..." << std::endl;
    fill_matrix(A, M, K);
    fill_matrix(B, K, N);


    start = std::chrono::high_resolution_clock::now();
    matrix_multiplication(A, B, C, M, K, N);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Sequential matrix multiplication result:" << C[0][0] << std::endl;
    std::cout << "Time: " << duration << " ms" << std::endl;

    std::vector<std::vector<double>> D(M, std::vector<double>(N));

    start = std::chrono::high_resolution_clock::now();
    parallel_matrix_multiplication(A, B, D, M, K, N);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Parallel matrix multiplication result:" << D[0][0] << std::endl;
    std::cout << "Time: " << duration << " ms" << std::endl;
    
    if (compare_matrices(C, D, M, N)) {
        std::cout << "Matrices are equal!" << std::endl;
    }
    else {
        std::cout << "Matrices differ!" << std::endl;
    }

    return 0;
}