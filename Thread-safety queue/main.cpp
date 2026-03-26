#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include "Queue.h"
std::mutex cout_mtx;
int main()
{
    Queue<std::string> q;
    

    /*std::cout << "Queue size: " << q.size() << std::endl;
    std::string val;
    if (!q.pop(val))
        std::cout << "Queue is empty";*/

    Queue<int> q_1;
    q_1.push(1);
    q_1.push(2);
    int a;
    int b;
    q_1.pop(a);
    q_1.pop(b);
    std::cout << a << " " << b;
    

  /*  std::cout << "\n--- Test: Waiting in the stream ---" << std::endl;

    std::thread consumer([&]() {
        std::string data;
        std::cout << "[Consumer] Waiting for the data..." << std::endl;
        q.wait_and_pop(data);
        std::cout << "[Consumer] Received: " << data << std::endl;
        });

    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "[Producer] Put the data 'TSU'" << std::endl;
        q.push("TSU");
        });

    consumer.join();
    producer.join();*/


    std::cout << "\n--- Test: many threads ---" << std::endl;
    Queue<int> q2;
    const int numEl = 100;
    const int numThr = 4;
    
    std::vector<std::thread> consumers_2;
    for (int i = 0; i < numThr; ++i) {
        consumers_2.emplace_back([&q2, i, numEl, numThr]() {
            for (int j = 0; j < numEl / numThr; ++j) {
                int v;
                {
                    std::lock_guard<std::mutex> lock(cout_mtx);
                    std::cout << "[Consumer_" << i << "] Waiting for the data..." << std::endl;
                }
                q2.wait_and_pop(v);
                {
                    std::lock_guard<std::mutex> lock(cout_mtx);
                    std::cout << "[Consumer_" << i << "] Received: " << v << std::endl;
                }
            }
            });
    }

    std::thread producer_2([&]() {
        for (int i = 0; i < numEl; ++i) {
            q2.push(i);
        }
        });

    producer_2.join();
    for (auto& t : consumers_2) t.join();




	return 0;
}