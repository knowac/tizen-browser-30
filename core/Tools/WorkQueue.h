/*
 * WorkQueue.h
 *
 *  Created on: Jun 10, 2014
 *      Author: pchmielewski
 */

#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace tizen_browser
{
namespace tools
{

template<typename T, typename R>
class WorkData {
public:
    void setData(std::vector<T> data) {
        this->data = data;
    }
    void setWorker(R (*worker)(T)) {
        this->worker = worker;
    }
    T getNext() {
        lock.lock();
        if (data.size() == 0) {
            lock.unlock();
            throw std::exception();
        } else {
            T t = data.back();
            data.pop_back();
            lock.unlock();
            return t;
        }
    }

    void addResult(R r) {
        boost::lock_guard<boost::mutex> lock(results_lock);
        results.push_back(r);
    }

    std::vector<R> getResults() {
        return results;
    }

    R executeWorker(T t) {
        return worker(t);
    }

private:
    std::vector<T> data;
    std::vector<R> results;
    R (*worker)(T);
    boost::mutex lock;
    boost::mutex results_lock;
};

template<typename T, typename R>
class WorkQueue {
public:
    WorkQueue(int threads) {
        this->threadCount = threads;
    }

    void setWorker(R (*worker)(T)) {
        this->workData.setWorker(worker);
    }

    void setData(std::vector<T> data) {
        this->workData.setData(data);
    }

    std::vector<R> getResults() {
        return this->workData.getResults();
    }

    void run() {
        boost::thread taskThreads[threadCount];
        for (int i = 0; i < threadCount; i++) {
            taskThreads[i] = boost::thread(WorkQueue::threadWorker, &workData);
        }
        for (int i = 0; i < threadCount; i++) {
            taskThreads[i].join();
        }
    }

    static void threadWorker(WorkData<T, R>* workData) {
        bool done = false;
        while (done != true) {
            try {
                T t = workData->getNext();
                R r = workData->executeWorker(t);
                workData->addResult(r);
            } catch (std::exception& e) {
                done = true;
            }
        }
    }

private:
    WorkData<T, R> workData;
    int threadCount;
};

template<typename T, typename R>
std::vector<R> parallel_for_each(std::vector<T> inputs, R (*func)(T)) {
    tizen_browser::tools::WorkQueue<T, R> workQueue(4);
    workQueue.setData(inputs);
    workQueue.setWorker(func);
    workQueue.run();
    return workQueue.getResults();
}

}
}
#endif /* WORKQUEUE_H_ */
