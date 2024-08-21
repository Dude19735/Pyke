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

#ifdef PYVK
	namespace py = pybind11;
#endif

namespace VK4 {

	// typedef void(*followup)();
	// typedef void(*cppfunc)(std::function<void()>);

	// enum class Vk_ViewerJobType {
	// 	None,
	// 	CPP,
	// 	Python
	// };

	// struct Vk_ViewerJob {
	// 	Vk_ViewerJobType type;
// #ifdef PYVK
// 	py::function* pyFunc;
// #else
// 	typedef void(*cppfunc)(std::function<void()>);
// 	typedef void(*followup)();

// 	struct Vk_ViewerJob {
// 		cppfunc cppFunc;
// 		followup followUp;
// 	};
// #endif
	// 	cppfunc cppFunc;
	// 	followup secretFollowup;
	// };

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

#ifdef PYVK
		void enqueueJob(py::function* job, std::function<void()> followup) {
			{
				std::unique_lock<std::mutex> lock(_mutex);
				Vk_Logger::Log(typeid(this), "[Threadpool] Enqueue job");
				_jobs.push({job, followup});
			}

			_condition.notify_one();
		}
#else
		void enqueueJob(std::shared_ptr<VK4::Vk_Func> job, std::function<void()> followup) {
			{
				std::unique_lock<std::mutex> lock(_mutex);
				Vk_Logger::Log(typeid(this), "[Threadpool] Enqueue job");
				_jobs.push({job, followup});
			}

			_condition.notify_one();
		}
#endif

	private:
		// std::queue<Vk_ViewerJob> _jobs;
		// Vk_ViewerJob _currentJob;
#ifdef PYVK
		std::queue<std::pair<py::function*, std::function<void()>>> _jobs;
		std::pair<py::function*, std::function<void()>> _currentJob;
#else
		std::queue<std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>> _jobs;
		std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>> _currentJob;
#endif
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
#ifdef PYVK
						std::queue<std::pair<py::function*, std::function<void()>>> empty;
#else
						std::queue<std::pair<std::shared_ptr<VK4::Vk_Func>, std::function<void()>>> empty;
#endif
						std::swap(_jobs, empty);
						return;
					}

					Vk_Logger::Log(typeid(this), "[Threadpool::Worker] run job");
					_currentJob = _jobs.front();
					_jobs.pop();
				}
#ifdef PYVK
				py::gil_scoped_acquire acquire;
				(*_currentJob.first)(
					py::cpp_function([this]() {
						enqueueJob(_currentJob.first, _currentJob.second);
					})
				);
				py::gil_scoped_release nogil;
				if (_currentJob.second != nullptr) {
					_currentJob.second();
				}
#else
				(*_currentJob.first)(
					[this]() {
						enqueueJob(_currentJob.first, _currentJob.second);
					}
				);
				if (_currentJob.second != nullptr) {
					_currentJob.second();
				}
#endif
			}
		}
	};
}
