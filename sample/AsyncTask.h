#pragma once
#include <Windows.h>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

class AsyncTask {
public:
	AsyncTask() = default;
	virtual ~AsyncTask() { ClearAllTask(); }

	void PushVideoTask(std::function<void()> func, uint64_t key = 0);
	bool RunAllTask();

	void ClearAllTask();
	void RemoveTask(uint64_t key);

private:
	struct ST_TaskInfo {
		uint64_t key = 0;
		std::function<void()> func;
	};

	std::recursive_mutex m_lockTask;
	std::vector<ST_TaskInfo> m_vTaskList;
};
