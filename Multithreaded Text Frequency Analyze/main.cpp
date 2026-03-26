#include <fstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>
#include <filesystem>
#include <chrono>
#include <iomanip>


#include "FrequencyMap.h"
#include "D:\Учеба\4 курс\Основы распределенных вычислений\лабы\Thread-safety queue\Queue.h"

void processFile(const std::string& filePath, FrequencyMap& localMap) {
    std::ifstream file(filePath);
    if (!file.is_open()) return;

    std::string wordBuffer;
    char ch;

    while (file.get(ch)) {
      
        // приводим к нижнему регистру
        if (unsigned char u_ch = static_cast<unsigned char>(ch); std::isalnum(u_ch) || std::ispunct(u_ch)) {
            char lowerCh = static_cast<char>(std::tolower(u_ch));
            localMap.char_freq[lowerCh]++;
        }

        if (std::isalnum(static_cast<unsigned char>(ch))) {
            wordBuffer += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        else {
            if (!wordBuffer.empty()) {
                localMap.word_freq[wordBuffer]++;
                wordBuffer.clear();
            }
        }
    }
    if (!wordBuffer.empty()) {
        localMap.word_freq[wordBuffer]++;
    }
}

namespace fs = std::filesystem;

FrequencyMap runAnalysis(const std::string& directoryPath) {

    unsigned int numThreads = std::thread::hardware_concurrency();

    Queue<std::string> fileQueue;

    // локальные хранилища для каждого потока

    std::vector<FrequencyMap> localMaps(numThreads);
    std::vector<std::thread> thrs;

    for (unsigned int i = 0; i < numThreads; ++i) {
        thrs.emplace_back([&fileQueue, &localMaps, i]() { // без копирования
            std::string filePath;
            while (fileQueue.wait_and_pop(filePath)) {
               // std::cout << "file path: " << filePath << " ID: " << i<<std::endl;
                processFile(filePath, localMaps[i]);
            }
            });
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry)) {
                fileQueue.push(entry.path().string());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }

    fileQueue.set_finished();

    for (auto& t : thrs) {
        if (t.joinable()) t.join();
    }

    // cлияние результатов в один глобальный словарь
    FrequencyMap globalMap;
    for (const auto& lMap : localMaps) {
        globalMap.merge(lMap);
    }

    return globalMap;
}

FrequencyMap runAnalysisSequential(const std::string& directoryPath) {
    FrequencyMap resultMap;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry)) {
                processFile(entry.path().string(), resultMap);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }

    return resultMap;
}



void printTopResults(const FrequencyMap& globalMap, size_t topN = 10) {
  
    std::vector<std::pair<std::string, uint64_t>> sortedWords(
        globalMap.word_freq.begin(), globalMap.word_freq.end()
    );

    std::sort(sortedWords.begin(), sortedWords.end(),
        [](const auto& a, const auto& b) {
            return b.second < a.second; 
        });


    std::vector<std::pair<char, uint64_t>> sortedChars(
        globalMap.char_freq.begin(), globalMap.char_freq.end()
    );

    std::sort(sortedChars.begin(), sortedChars.end(),
        [](const auto& a, const auto& b) {
            return b.second < a.second;
        });

   
    std::cout  << "\n   --- TOP WORDS ---\n";
    for (size_t i = 0; i < std::min(topN, sortedWords.size()); ++i) {
        std::cout << std::setw(10) << sortedWords[i].first << " : " << sortedWords[i].second << "\n";
    }

    std::cout << "\n   --- TOP LETTERS ---\n";
    for (size_t i = 0; i < std::min(topN, sortedChars.size()); ++i) {
        std::cout << std::setw(8) << "'" << sortedChars[i].first << "' : " << sortedChars[i].second << "\n";
    }
}





int main(int argc, char* argv[]) {
  
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        std::cerr << "Error: Path does not exist or is not a directory." << std::endl;
        return 1;
    }

    std::cout << "Starting analysis in: " << path << std::endl;
    std::cout << "Using " << std::thread::hardware_concurrency() << " threads." << std::endl;



    auto start = std::chrono::high_resolution_clock::now();

    FrequencyMap globalMap;
    try {

        globalMap = runAnalysis(path);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Total time elapsed (threads): " << std::fixed << std::setprecision(3)
        << elapsed.count() << " seconds." << std::endl;
    /*std::cout << "-----------------------------------" << std::endl;
    std::cout << "Analysis finished successfully!" << std::endl;
    std::cout << "Unique words found:   " << globalMap.word_freq.size() << std::endl;
    std::cout << "Unique chars found:   " << globalMap.char_freq.size() << std::endl;*/
    /* printTopResults(globalMap, 20);*/
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Starting analysis sequential: " << std::endl;

    start = std::chrono::high_resolution_clock::now();

    FrequencyMap sequentialMap;
    try {

        sequentialMap = runAnalysisSequential(path);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;

    std::cout << "Total time elapsed (sequential): " << std::fixed << std::setprecision(3)
        << elapsed.count() << " seconds." << std::endl;



    return 0;
}