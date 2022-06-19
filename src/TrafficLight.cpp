#include <iostream>
#include <random>
#include "TrafficLight.h"



/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock lck(_mqMutex);
    T msg;
    _msgQuConVar.wait(lck, [this, &msg ]() mutable {
            if(_queue.empty()) return false;
            msg = std::move(_queue.front());
            _queue.pop_front();
            return true;
           
            
        });

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard(this->_mqMutex);
    _queue.emplace_back(std::move(msg));

    _msgQuConVar.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
   

    while( true ) {
        if(TrafficLightPhase::green == _messageQueue.receive()){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void worker(){
    std::cout << "test\n";
}

void TrafficLight::simulate()
{

    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    
    // My note: https://stackoverflow.com/questions/50175002/invalid-use-of-non-static-member-function-c-thread-linux
    // member functions 
    // In C++ member functions have an implicit first parameter that binds to this. When creating a thread, the this pointer must be passed. 
    // and you must You must also qualify the member function with a class name. 
    this->threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // see https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    // and https://en.cppreference.com/w/cpp/numeric/random
    std::random_device rdev;
    std::default_random_engine randomTime(rdev());
    std::uniform_int_distribution<int> distrib(4, 6);
    int randomSeconds = distrib(randomTime);
    int breakFlag = 0;
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> stopTime = now + std::chrono::duration<int>(randomSeconds);

    // sleep at every iteration to reduce CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cout << "DEBUG: intital random seconds" << randomSeconds << '\n';
    while(true){
        if(now > stopTime){
            std::cout << "Now is greater than the stopTime\n";
            std::lock_guard<std::mutex> lock( _mutex );

            if(this->getCurrentPhase() == TrafficLightPhase::red ){
                std::cout << "Toggle TrafficLightPhase from red to green"<< '\n';
                _currentPhase = TrafficLightPhase::green;

            }
            else{            
                std::cout << "Now not greater than the stopTime\n";
                std::cout << "Toggle TrafficLightPhase from GREEN to RED"<< '\n';

                _currentPhase = TrafficLightPhase::red;
            }

            // send it back
            this->_messageQueue.send(std::move(_currentPhase));

            // reset the clock
            randomSeconds = distrib(randomTime);
            std::cout << "DEBUG: random seconds reset to" << randomSeconds << '\n';
            now = std::chrono::system_clock::now();
            stopTime = now + std::chrono::duration<int>(randomSeconds);

        }
        else{
            now = std::chrono::system_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

