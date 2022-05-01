#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <sstream>
#include <iomanip>

template<typename T>
class safe_queue : public std::queue<T>{
public:
    void safe_push(const T value){
        std::lock_guard<std::mutex> lock(changing);
        this->push(value);
    }

    template<class ... Args>
    void safe_emplace(Args ... args){
        std::lock_guard<std::mutex> lock(changing);
        this->emplace(args...);
    }

    void safe_pop() {
        std::lock_guard<std::mutex> lock(changing);
        this->pop();
    }

    void safe_swap(safe_queue& other){
        std::scoped_lock lock(changing, other.changing);
        this->swap(other);
    }

private:
    std::mutex changing;
};


bool is_interval(safe_queue<int>& check, int length, bool need_print = false){
    int start = check.front();

    //потому что все интервалы задаются как [i*length, (i+1)*length)
    if(check.size() != length ||  start%length != 0) return 0;

    std::vector<int> print(need_print ? length : 0);

    for(int t = start; t < start+length; t++){
        if(check.front() != t) return 0;

        if(need_print) print[t-start] = check.front();

        check.pop();
    }

    for(auto el : print) std::cout << el << " ";

    return 1;
}



void selfish_act(safe_queue<int>& obj, int start_value, int finish_value, int attempts){
    safe_queue<int> bully;
    for(int t = start_value; t < finish_value; t++){
        bully.push(t);
    }

    while(attempts--){
        obj.safe_swap(bully);
        bully.safe_swap(obj);
    }


    std::stringstream output;
    output << std::left << std::setw(15) << "[" + std::to_string(start_value) + " - " + std::to_string(finish_value) + ")";
    output << "after shuffle:  ";
    output << "[" << bully.front() << " - " << bully.back() << ")\n";
    std::cout << output.str();


    //проверка что в bully какой-то интервал
    if(!is_interval(bully, finish_value - start_value)) throw std::runtime_error("bad(");
}

int main(){
    int interval_lenght = 100;
    int count_of_threads = 10;
    int attempts_to_shuffle = 1000;

    safe_queue<int> object;
    for(int i = 0; i < interval_lenght; i++){
        object.push(i);
    }


    std::vector<std::thread> multi_thread;
    for(int i = 1; i <= count_of_threads; i++){
        multi_thread.emplace_back(selfish_act, std::ref(object),
                                  i*interval_lenght, (i+1)*interval_lenght, attempts_to_shuffle);
    }
    for(auto& th : multi_thread) th.join();


    //проверка что начальный object поменялся с каким-то другим интервалом, но не перемешался
    std::cout << "The original interval:\n";
    is_interval(object, interval_lenght, true);

    return 0;
}