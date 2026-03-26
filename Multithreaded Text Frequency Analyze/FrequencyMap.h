#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

struct FrequencyMap {
    std::unordered_map<std::string, uint64_t> word_freq;
    std::unordered_map<char, uint64_t> char_freq;
    void merge(const FrequencyMap& other) {

        //gервый элемент (ключ) кладет в word, второй (значение) в count
        for (const auto& [word, count] : other.word_freq) {
       /* Программа ищет, есть ли уже ключ word в текущей карте.
          Если слова еще нет, std::unordered_map автоматически создает его и инициализирует значением по умолчанию
          Затем он прибавляет count к тому, что там было*/
            word_freq[word] += count;
        }
        for (const auto& [ch, count] : other.char_freq) {
            char_freq[ch] += count;
        }
    }

  
    void clear() {
        word_freq.clear();
        char_freq.clear();
    }
};
