#ifndef CMF_QUEUE_H
#define CMF_QUEUE_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

/**
 * @brief A variable size queue which is thread-safe, template,
 * and supports a custom allocator for all memory allocations.
 * @tparam T The type of data being held in the queue.
 * @tparam Allocator The type of allocator used for memory operations within the queue.
 */
template<typename T, typename Allocator = std::allocator<T>>
class Queue {
public:
	/**
	 * @brief The constructor with starting size option,
	 * @param size The size the queue starts its life with.
	 */
	inline explicit Queue(size_t size = DefaultSize) noexcept : bufferSize(size), waitSemaphore(xSemaphoreCreateBinary()) {
		reserve(bufferSize);
	}

	/**
	 * @brief The copy constructor from another queue of same data type.
	 * @param other The queue being copied.
	 */
	inline Queue(const Queue& other) noexcept : bufferSize(other.bufferSize), begin(other.begin), end(other.end), qSize(other.qSize), waitSemaphore(xSemaphoreCreateBinary()) {
		std::lock_guard lock(other.accessMutex);

		reserve(bufferSize);

		if(empty()){
			return;
		}

		if(begin > end){
			for(size_t i = 0; i <= end; ++i){
				new(&buffer[i]) T(other.buffer[i]);
			}

			for(size_t i = begin; i < bufferSize; ++i){
				new(&buffer[i]) T(other.buffer[i]);
			}
		}else{
			for(size_t i = begin; i <= end; ++i){
				new(&buffer[i]) T(other.buffer[i]);
			}
		}

		xSemaphoreGive(waitSemaphore);
	}

	/**
	 * @brief The move constructor from another queue with the same data type.
	 * @param other The queue being moved. The queue is empty after the constructor finishes execution.
	 */
	inline Queue(Queue&& other) noexcept : buffer(other.buffer), bufferSize(other.bufferSize), begin(other.begin), end(other.end), qSize(other.qSize), waitSemaphore(xSemaphoreCreateBinary()) {
		std::lock_guard lock(other.accessMutex);
		other.buffer = nullptr;
		other.bufferSize = 0;
		other.begin = other.end = 0;
		other.qSize = 0;

		if(!empty()){
			xSemaphoreGive(waitSemaphore);
		}
	}

	/**
	 * @brief Unblocks the thread-safe synchronization and frees memory, destroys the queue.
	 */
	inline ~Queue() noexcept {
		setKillPill();

		clear();

		std::lock_guard guard(accessMutex);
		allocator.deallocate(buffer, bufferSize);
		buffer = nullptr;
		begin = end = 0;
		bufferSize = 0;
		qSize = 0;

		vSemaphoreDelete(waitSemaphore);
	}

	/**
	 * @return The size of the queue.
	 */
	inline size_t size() const noexcept {
		return qSize;
	}

	/**
	 * @return The capacity of the queue (maximum size without resizing).
	 */
	inline size_t capacity() const noexcept {
		return bufferSize;
	}

	/**
	 * @brief Checker for an empty queue.
	 * @return True if size is 0. False otherwise.
	 */
	inline bool empty() const noexcept {
		return size() == 0;
	}

	/**
	 * @brief Checker for a full queue.
	 * @return True if size equals capacity. False otherwise.
	 */
	inline bool full() const noexcept {
		return size() >= bufferSize;
	}

	/**
	 * @brief Reserves a capacity of the queue to the given size.
	 * @param newSize The new capacity of the queue.
	 * @return True if successful, false otherwise.
	 */
	inline bool reserve(size_t newSize) noexcept {
		std::lock_guard guard(accessMutex);
		return reserveInternal(newSize);
	}

	/**
	 * @param value The variable that is set to the front value.
	 * @param wait The maximum time to halt thread execution and wait to retrieve a value.
	 * @return True if successful, false otherwise.
	 */
	inline bool front(T& value, TickType_t wait = portMAX_DELAY) noexcept {
		if(xSemaphoreTake(waitSemaphore, wait) != pdTRUE){
			return false;
		}

		std::lock_guard guard(accessMutex);

		if(kill){
			return false;
		}

		if(empty()){
			return false;
		}

		value = buffer[begin];

		xSemaphoreGive(waitSemaphore);

		return true;
	}

	/**
	 * @brief Pop function which retrieves a value from the internal buffer before removing it internally.
	 * @param value The value variable set to the value at the top of the stack before removing it.
	 * @param wait The maximum wait time for a successful value pop before abort.
	 * @return True if successful, false otherwise.
	 */
	inline bool pop(T& value, TickType_t wait = portMAX_DELAY) noexcept {
		if(xSemaphoreTake(waitSemaphore, wait) != pdTRUE){
			return false;
		}

		std::lock_guard guard(accessMutex);

		if(kill){
			return false;
		}

		if(empty()){
			return false;
		}

		value = std::move_if_noexcept(buffer[begin]);
		buffer[begin].~T();

		--qSize;
		if(!empty()){
			begin = (begin + 1) % bufferSize;
		}

		if(!empty()){
			xSemaphoreGive(waitSemaphore);
		}

		return true;
	}

	/**
	 * @brief Push function which adds a value at the end of the queue.
	 * @param value The value being added to the queue.
	 * @return True if successful, false otherwise.
	 */
	inline bool push(const T& value) noexcept {
		if(full() && !reserve(bufferSize * 2)){
			return false;
		}

		std::lock_guard guard(accessMutex);

		if(!empty()){
			end = (end + 1) % bufferSize;
		}

		new(&buffer[end]) T(value);

		++qSize;

		xSemaphoreGive(waitSemaphore);

		return true;
	}

	/**
	 * @brief A move push function. The give value is moved to the queue and emptied / invalidated.
	 * @param value The value being added to the queue.
	 * @return True if successful, false otherwise.
	 */
	inline bool push(T&& value) noexcept {
		if(full() && !reserve(bufferSize * 2)){
			return false;
		}

		std::lock_guard guard(accessMutex);

		if(!empty()){
			end = (end + 1) % bufferSize;
		}

		new(&buffer[end]) T(std::move(value));

		++qSize;

		xSemaphoreGive(waitSemaphore);

		return true;
	}

	/**
	 * @brief Checks if a value is present in the queue.
	 * @param value The value being checked.
	 * @return True if queue contains the given value.
	 */
	inline bool check(const T& value) const noexcept {
		std::lock_guard guard(accessMutex);

		if(empty()){
			return false;
		}

		if(begin > end){
			for(size_t i = 0; i <= end; ++i){
				if(buffer[i] == value){
					return true;
				}
			}

			for(size_t i = begin; i < bufferSize; ++i){
				if(buffer[i] == value){
					return true;
				}
			}
		}else{
			for(size_t i = begin; i <= end; ++i){
				if(buffer[i] == value){
					return true;
				}
			}
		}

		return false;
	}

	/**
	 * @brief Removes the given value from the queue no matter the location.
	 * @param value The value being removed.
	 */
	inline void remove(const T& value) noexcept {
		std::lock_guard guard(accessMutex);

		if(empty()){
			return;
		}

		// Single pass in logical order: keep non-matching elements by moving them to a write cursor,
		// destroy matching ones. Handles duplicates and wrapped (begin > end) layouts correctly.
		const size_t count = qSize;
		size_t write = begin;

		for(size_t n = 0; n < count; ++n){
			const size_t read = (begin + n) % bufferSize;

			if(buffer[read] == value){
				buffer[read].~T();
				--qSize;
				continue;
			}

			if(write != read){
				new(&buffer[write]) T(std::move_if_noexcept(buffer[read]));
				buffer[read].~T();
			}

			write = (write + 1) % bufferSize;
		}

		if(empty()){
			begin = end = 0;
			xSemaphoreTake(waitSemaphore, 0);
			return;
		}

		end = (write + bufferSize - 1) % bufferSize;
	}

	/**
	 * @brief Removes all values from the queue, leaving it empty.
	 */
	inline void clear() noexcept {
		std::lock_guard guard(accessMutex);

		if(empty()){
			return;
		}

		if(begin > end){
			for(size_t i = 0; i <= end; ++i){
				buffer[i].~T();
			}

			for(size_t i = begin; i < bufferSize; ++i){
				buffer[i].~T();
			}
		}else{
			for(size_t i = begin; i <= end; ++i){
				buffer[i].~T();
			}
		}

		end = begin = 0;
		qSize = 0;

		xSemaphoreTake(waitSemaphore, 0);
	}

	/**
	 * @brief Unblocks all thread-safe functionality with a kill pill, meaning all data retrieval attempts will fail, but will unblock threads waiting on it.
	 * @param value The value being set to the kill pill.
	 */
	inline void setKillPill(bool value = true) noexcept {
		std::lock_guard guard(accessMutex);
		kill = value;

		if(kill){
			xSemaphoreGive(waitSemaphore);
		}
	}

private:
	static constexpr size_t DefaultSize = CONFIG_CMF_EVENT_DEFAULT_QUEUE_SIZE;
	T* buffer = nullptr;
	Allocator allocator = Allocator();
	size_t bufferSize;
	size_t begin = 0;
	size_t qSize = 0;
	size_t end = 0;
	bool kill = false;
	std::mutex accessMutex;
	SemaphoreHandle_t waitSemaphore;

private:
	/**
	 * @brief An internal function used to reserve memory needed for resizing of the queue.
	 * Before resizing, all values that will be lost when downsizing are destroyed first. The rest are moved or copied to the new buffer.
	 * @param newSize The new size of the queue./
	 * @return True if successful, false otherwise.
	 */
	inline bool reserveInternal(size_t newSize) noexcept {
		T* newBuffer = allocator.allocate(newSize);
		if(newBuffer == nullptr){
			return false;
		}

		if(buffer == nullptr){
			buffer = newBuffer;
			bufferSize = newSize;
			return true;
		}

		const size_t count = qSize;
		for(size_t i = 0; i < count; ++i){
			const size_t src = (begin + i) % bufferSize;
			new(&newBuffer[i]) T(std::move_if_noexcept(buffer[src]));
			buffer[src].~T();
		}

		allocator.deallocate(buffer, bufferSize);
		buffer = newBuffer;
		bufferSize = newSize;
		begin = 0;
		end = count > 0 ? count - 1 : 0;

		return true;
	}
};

#endif //CMF_QUEUE_H
