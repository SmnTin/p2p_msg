#ifndef P2P_MSG_SHEDULER_H
#define P2P_MSG_SHEDULER_H

#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <tuple>
#include <queue>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace p2p {

    class Task {
    public:
        typedef uint16_t Id;

        Task(std::chrono::steady_clock::time_point executionTime, const std::function<void()> & f = {}, Id id = generateId()) {
            _executionTime = executionTime;
            _func = f;
            _id = id;
        }

        bool operator< (const Task & b) const {
            return std::tie(_executionTime, _id) < std::tie(b._executionTime, b._id);
        }

        void setFunc(const std::function<void()> & f) {
            _func = f;
        }

        void cancel() {
            _func = {};
        }

        void operator() () {
            if(_func)
                _func();
        }

        std::chrono::steady_clock::time_point getExecutionTime() {
            return _executionTime;
        }

        Id getId() {
            return _id;
        }

        static Id generateId() {
            return _tasksCounter++;
        }

    private:
        std::chrono::steady_clock::time_point _executionTime;
        std::function<void()> _func;
        Id _id;

        static Id _tasksCounter;
    };

    class Sheduler {
    public:
        //O(n*log(n))
        void update() {
            std::lock_guard<std::recursive_mutex> guard(_tasksMutex);

            auto now = std::chrono::steady_clock::now();

            while(!_tasks.empty() && _tasks.top()->getExecutionTime() <= now) {
                std::shared_ptr<Task> task = _tasks.top();
                _tasks.pop();
                (*task)();
            }
        }

        //O(log(n))
        void executeTask(const std::shared_ptr<Task> & task) {
            std::lock_guard<std::recursive_mutex> guard(_tasksMutex);

            _tasks.push(task);
        }

        std::shared_ptr<Task> executeTaskAt(std::chrono::steady_clock::time_point executeAt,
                const std::function<void ()> & cb, Task::Id id = Task::generateId()) {
            std::shared_ptr<Task> task(new Task(executeAt, cb, id));

            executeTask(task);

            return task;
        }

        std::shared_ptr<Task> executeTaskIn(std::chrono::milliseconds executeIn,
                const std::function<void ()> & cb, Task::Id id = Task::generateId()) {
            return executeTaskAt(std::chrono::steady_clock::now() + executeIn, cb, id);
        }

        void run() {
            _running = true;
            _runningThread = std::thread(_runable(this));
        }

        void stop() {
            _running = false;
            if(_runningThread.joinable())
                _runningThread.join();
        }

        bool isRunning() {
            return _running;
        }

        ~Sheduler() {
            stop();
        }

    private:
        std::priority_queue<std::shared_ptr<Task>> _tasks;
        std::recursive_mutex _tasksMutex;
        bool _running = true;
        std::thread _runningThread;

        struct _runable {
            explicit _runable(Sheduler * instance) : _instance(instance) {}
            void operator() (){
                while(_instance->_running) {
                    _instance->update();
                    sleepThread(1);
                }
            };

            void sleepThread(uint32_t sleepTime_ms) {
                std::chrono::duration<long long, std::milli> sleepDur {sleepTime_ms};
                std::this_thread::sleep_for(sleepDur);
            }

            Sheduler * _instance;
        };
    };
} //p2p

#endif //P2P_MSG_SHEDULER_H
