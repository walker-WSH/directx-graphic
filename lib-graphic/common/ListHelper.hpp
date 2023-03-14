#pragma once
#include <map>
#include <vector>
#include <algorithm>

template<typename T> bool FindItemInVector(const std::vector<T> &items, T obj)
{
	auto itr = find(items.begin(), items.end(), obj);
	return (itr != items.end());
}

template<typename T> bool RemoveItemInVector(std::vector<T> &items, T obj)
{
	auto itr = find(items.begin(), items.end(), obj);
	if (itr != items.end()) {
		items.erase(itr);
		return true;
	} else {
		return false;
	}
}

template<typename T, typename T2> bool FindItemInMap(const std::map<T, T2> &items, T obj)
{
	auto itr = items.find(obj);
	return (itr != items.end());
}

template<typename T, typename T2> bool RemoveItemInMap(std::map<T, T2> &items, T obj)
{
	auto itr = items.find(obj);
	if (itr != items.end()) {
		items.erase(itr);
		return true;
	} else {
		return false;
	}
}
