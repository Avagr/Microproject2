#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <utility>
#include <sstream>

struct Client {
    explicit Client(std::string name) : name(std::move(name)) {
        mutex.lock();
        thread = std::thread(&Client::ClientFunc, this);
    }

    std::string name;
    std::thread thread;
    std::mutex mutex;

    void ClientFunc() {
        mutex.lock();
        int barber_time = rand() % 3 + 1;
        std::cout << "The barber serves client " << name << ". This will take " << barber_time
                  << " seconds\n";
        std::this_thread::sleep_for(std::chrono::seconds(barber_time));
        std::cout << "Finished shaving client " << name << "\n";
    }
};

class BarberShop {
public:
    explicit BarberShop(int max_clients)
        : max_clients_(max_clients), barber_(std::thread(&BarberShop::Barber, this)) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        std::cout << "A new day begins. The barber is fast asleep, for now\n";
    }

    void Barber() {
        while (max_clients_ > 0) {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (client_waits_ && !queue_.empty()) {
                queue_.front().mutex.unlock();
                queue_.front().thread.join();
                queue_.pop();
                if (queue_.empty() && !client_in_queue_) {
                    std::cout << "The barber goes back to sleep\n";
                    client_waits_ = false;
                }
                max_clients_--;
            }
        }
    }

    void ClientWalksIn(const std::string& name) {
        std::stringstream stream;
        client_in_queue_ = true;
        stream << "Client " << name << " walks in\n";
        std::cout << stream.str();
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.emplace(name);
        client_in_queue_ = false;
        if (!client_waits_) {
            std::cout << "The barber wakes up!\n";
            client_waits_ = true;
        }
    }

    void WaitForEvening() {
        barber_.join();
        std::cout << "The barber has finished for the day\n";
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (!queue_.empty()) {
            std::cout << "The rest of the clients go home\n";
            while (!queue_.empty()) {
                queue_.pop();
            }
        }
    }

private:
    std::queue<Client> queue_;
    std::mutex queue_mutex_;
    std::atomic<int> max_clients_;
    std::thread barber_;
    std::atomic<bool> client_waits_ = false, client_in_queue_ = false;
};

int main() {
    std::cout << "Please enter the number of clients: ";
    int client_num = -1;
    std::cin >> client_num;
    srand(time(nullptr));
    std::vector<std::string> names{
        "Adam",  "Alex",  "Aaron",  "Ben",     "Carl", "Dan",    "David",  "Edward",
        "Fred",  "Frank", "George", "Hal",     "Hank", "Ike",    "John",   "Jack",
        "Joe",   "Larry", "Monte",  "Matthew", "Mark", "Nathan", "Otto",   "Paul",
        "Peter", "Roger", "Steve",  "Thomas",  "Tim",  "Ty",     "Victor", "Walter"};
    BarberShop shop(client_num);
    for (int i = 0; i < client_num; ++i) {
        shop.ClientWalksIn(names[rand() % names.size()]);
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 200));
    }
    shop.WaitForEvening();
    return 0;
}
