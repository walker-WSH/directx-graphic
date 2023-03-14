#include "pch.h"
#include "AsyncTask.h"

void AsyncTask::PushVideoTask(std::function<void()> func, uint64_t key)
{
	ST_TaskInfo info;
	info.key = key;
	info.func = func;

	std::lock_guard<std::recursive_mutex> autoLock(m_lockTask);
	m_vTaskList.push_back(info);
}

bool AsyncTask::RunAllTask()
{
	std::vector<AsyncTask::ST_TaskInfo> temTasks;

	{
		std::lock_guard<std::recursive_mutex> autoLock(m_lockTask);
		temTasks.swap(m_vTaskList);
	}

	for (auto &item : temTasks)
		item.func();

	return !temTasks.empty();
}

void AsyncTask::ClearAllTask()
{
	std::lock_guard<std::recursive_mutex> autoLock(m_lockTask);
	m_vTaskList.clear();
}

void AsyncTask::RemoveTask(uint64_t key)
{
	std::lock_guard<std::recursive_mutex> autoLock(m_lockTask);

	auto itr = m_vTaskList.begin();
	for (; itr != m_vTaskList.end(); ++itr) {
		if (itr->key == key) {
			itr = m_vTaskList.erase(itr);
			continue;
		}
	}
}
