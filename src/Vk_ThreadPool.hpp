#pragma once

#include <condition_variable>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <utility>
#include <functional>

#include "Defines.h"
#include "Vk_Function.hpp"

namespace VK4 {
	class Vk_ThreadPool {
	public:
		Vk_ThreadPool() : 
		_terminate(false),
		_stopped(false),
		_parent(nullptr)
		{
			Vk_Logger::Log(typeid(this), GlobalCasters::castConstructorTitle("Create Threadpool"));
		}

		~Vk_ThreadPool() {
			if(!_stopped) Vk_Logger::RuntimeError(typeid(this), GlobalCasters::castFatalTitle("Thread pool still running. Call pool.stop() first!"));
		}

		void start(int threadCount, void* parent) {
			Vk_Logger::Log(typeid(this), "[Threadpool] Start");

			_parent = parent;
			for (int i = 0; i < threadCount; ++i) {
				_pool.push_back(std::thread(&Vk_ThreadPool::infiniteLoop, this));
			}
			_stopped = false;
		}

		void stop() {
			{
				std::unique_lock<std::mutex> lock(_mutex);
				_terminate = true;
			}

			Vk_Logger::Log(typeid(this), "[Threadpool] Stop");

			_condition.notify_all();

			for (auto& i : _pool) {
				i.join();
			}

			_pool.clear();
			Vk_Logger::Log(typeid(this), "[Threadpool] all workers terminated");
			_stopped = true;
		}

		void enqueueJob(std::shared_ptr<VK4::Vk_Func> job, std::function<void()> followup) {
			{
				std::unique_lock<std::mutex> lock(_mutex);
				Vk_Logger::Log(typeid(this), "[Threadpool] Enqueue job");
				_jobs.push({job, followup});
			}

			_condition.notify_one();
		}

	private:
		std::queue<std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>> _jobs;
		std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>> _currentJob;

		std::vector<std::thread> _pool;
		std::condition_variable _condition;
		std::mutex _mutex;
		bool _terminate;
		bool _stopped;
		void* _parent;

		void infiniteLoop() {
			while (true) {
				{
					std::unique_lock<std::mutex> lock(_mutex);

					_condition.wait(lock, [this]() {
						return !_jobs.empty() || _terminate;
						});

					if (_terminate) {
						Vk_Logger::Log(typeid(this), "[Threadpool::Worker] got termination signal");
						std::queue<std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>> empty;
						std::swap(_jobs, empty);
						return;
					}

					Vk_Logger::Log(typeid(this), "[Threadpool::Worker] run job");
					_currentJob = _jobs.front();
					_jobs.pop();
				}

				(*_currentJob.first)(
					[this]() {
						enqueueJob(_currentJob.first, _currentJob.second);
					}
				);
				if (_currentJob.second != nullptr) {
					_currentJob.second();
				}
			}
		}
	};
}
