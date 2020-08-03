
#include <string>
#include <thread>
#include <iostream>
#include <random>
#include <thread>

#include "Dispatcher.h"
#include "Consumer.h"

int main() {
    Dispatcher<int, int> dispatcher;

    std::shared_ptr<Consumer> myConsumer = std::make_shared<Consumer>();

    while (true) {

        int counter = 1;
        while (counter < 10) {
            for (int i = 0; i < 100; ++i) {
                if (i % counter == 0) {
                    bool success = dispatcher.enqueue(counter, i);
                    if (!success)
                        std::cerr << "ERROR: the queue #" << counter << " is full." << std::endl;
                }
            }
            counter++;
        }

        dispatcher.subscribe(7, myConsumer);
        dispatcher.subscribe(2, myConsumer);
        dispatcher.subscribe(3, myConsumer);
        dispatcher.subscribe(4, myConsumer);
        dispatcher.subscribe(5, myConsumer);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        dispatcher.unsubscribe(2);

        //std::this_thread::sleep_for(std::chrono::seconds(1));

        counter = 1;
        while (counter < 10) {
            for (int i = 100; i < 1000; ++i) {
                if (i % counter == 0) {
                    bool success = dispatcher.enqueue(counter, i);
                    if (!success)
                        std::cerr << "ERROR: the queue #" << counter << " is full." << std::endl;
                }
            }
            counter++;
        }

        dispatcher.unsubscribeAll();
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        
    }

    system("pause");
    return 0;
}
