#include <chrono>

#include "TaskScheduler.h";

std::mutex cout_mutex;

void complex_computation(int id, int seconds) {
    {
       
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Task " << id << "] Starting computation..." << std::endl;
    } 

    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Task " << id << "] Finished in " << seconds << " sec." << std::endl;
    }
}

int main() {
    TaskScheduler scheduler(4);

    scheduler.submit(complex_computation, 1, 2);
    scheduler.submit(complex_computation, 2, 1);

    scheduler.submit([]() {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[Lambda] Executing in thread ID: " << std::this_thread::get_id() << std::endl;
        });


    std::cout << "--- All tasks submitted. Main thread is free. ---" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(4));

    scheduler.submit(complex_computation, 3, 1);

    scheduler.stop();
    std::cout << "Scheduler stopped." << std::endl;

    return 0;
}