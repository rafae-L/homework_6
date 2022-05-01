#include <iostream>
#include <thread>
#include <numeric>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <atomic>

#include "Timer.h"

static std::mt19937_64 gen{static_cast<unsigned long long>(std::chrono::system_clock::now().time_since_epoch().count())};

template <typename Iterator, typename T>
void accumulate_block(Iterator first, Iterator last, T init, std::atomic<T>& result) {
    result += std::accumulate(first, last, init);
}

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init, size_t num_workers) {
    auto length = std::distance(first, last);
    if (length < 32) return std::accumulate(first, last, init);

    auto length_per_thread = length / num_workers;

    std::vector<std::thread> threads;
    threads.reserve(num_workers-1);

    std::atomic<T> result = init;

    auto left_border = first;
    auto right_border = std::next(first, length%num_workers + length_per_thread);
    //дополнительный сдвиг в right_border, что бы только первому потоку достался больший диапозон

    for(auto i = 0u; i < num_workers - 1; i++) {
        threads.emplace_back(
                accumulate_block<Iterator, T>,
                left_border, right_border, 0, std::ref(result));

        left_border = right_border;
        right_border = std::next(right_border, length_per_thread);
    }
    result += std::accumulate(left_border, last, 0);

    // std::mem_fun_ref -- для оборачивания join().
    std::for_each(std::begin(threads), std::end(threads), std::mem_fun_ref(&std::thread::join));

    return result;
}

//полностью скопипасченая с 5-го задания функция
std::vector<int> create_special(int n, int amplitude, int roof = 10'000'000){
    int sum_from_start = 0;
    roof = (abs(roof) < 2*amplitude) ? 2*amplitude : abs(roof);

    std::uniform_int_distribution<int> rnd(-roof/2/amplitude, roof/2/amplitude);

    std::vector<int> result(n);
    for(int i = 0; i < n; i++){
        int chose = amplitude * rnd(gen);

        if(2*(sum_from_start + chose) < -roof || roof < 2*(sum_from_start + chose)) chose *= -1;

        result[i] = chose;
        sum_from_start += chose;
    }
    return result;
}

int main() {
    size_t quantity_of_experements = 50; //по скольки суммированиям усредняем
    size_t quantity_of_thread = 9;      //перебераем кол-во потоков от 1 до quantity_thread
    size_t size = 100'000;               //количество элементов в векторе, сумму которого ищем

    std::vector<std::vector<int>> test_samples(quantity_of_experements);
    for(auto& sample : test_samples){
        sample = create_special(size, 1);
    }

    int pure_time;
    std::vector<int> thread_time(quantity_of_thread);

    //результаты, с которыми будем сравнивать
    std::vector<int> results(quantity_of_experements);

    //время работы базовой функции
    Timer<microseconds> t0;
    for(int j = 0; j < quantity_of_experements; j++) {
        results[j] = std::accumulate(test_samples[j].begin(), test_samples[j].end(), 0);
    }
    pure_time = t0.Get();


    for(int num_workers = 1; num_workers <= quantity_of_thread; num_workers++){
        std::vector<int> current_measuring(quantity_of_experements);

        Timer<microseconds> t;
        for(int j = 0; j < quantity_of_experements; j++){
            current_measuring[j] = parallel_accumulate(test_samples[j].begin(), test_samples[j].end(), 0, num_workers);
        }
        thread_time[num_workers-1] = t.Get();

        //проверка что суммирование рабочее
        for(int j = 0; j < quantity_of_experements; j++){
            if(current_measuring[j] != results[j]) throw std::runtime_error(" didnt work( ");
        }
    }


    std::cout << "Pure time: " << pure_time << std::endl;
    for(auto t : thread_time){
        std::cout << t << " ";
    }

    return 0;
}
