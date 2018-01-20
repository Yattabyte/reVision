/*
	Visibility_Token

	- An element that defines where and how a scene should be viewed
*/

#pragma once
#ifndef VISIBILITY_TOKEN
#define VISIBILITY_TOKEN
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Systems\World\ECSdefines.h"
#include <map>
#include <vector>

class Visibility_Token {
public:
	Visibility_Token() {}
	~Visibility_Token() {}
	
	void insert(char *c) {
		mList.insert(pair<char*, vector<Component*>>(c, vector<Component*>()));
	}
	template <typename T> 
	const std::vector<T*>& getTypeList(char *c) const {
		return *(vector<T*>*)(&mList.at(c));
	}
	std::vector<Component*>& operator[](char *c) {
		return mList.at(c);
	}
	size_t size() const {
		return mList.size();
	}
	bool find(char *c) const {
		return !(mList.find(c) == mList.end());
	}
	std::map<char*, std::vector<Component*>, cmp_str> mList;
};

#endif // VISIBILITY_TOKEN