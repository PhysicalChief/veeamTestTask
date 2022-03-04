#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class safeQueue
{
private:
	mutable std::mutex mutex;
	std::queue<T> queue;
	std::condition_variable condition;

public:
	safeQueue() {};
	safeQueue(safeQueue const& anotherQueue) {
		std::lock_guard<std::mutex> lock_guard(anotherQueue.mutex);
		queue = anotherQueue;
	}

	void push(const T& element) {
		std::lock_guard<std::mutex> lock_guard(mutex);
		queue.push(element);
		condition.notify_one();
	}

	void pop(T& element) {
		std::unique_lock<std::mutex> unique_lock(mutex);
		condition.wait(unique_lock, [this] {return !queue.empty(); });
		element = queue.front();
		queue.pop();
	}
	bool empty() const {
		std::lock_guard<std::mutex> lock_guard(mutex);
		return queue.empty();
	}

	unsigned int size() const {
		std::lock_guard<std::mutex> lock_guard(mutex);
		return queue.size();
	}
};

