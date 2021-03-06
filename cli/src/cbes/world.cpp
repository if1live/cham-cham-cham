﻿// Ŭnicode please 
#include "stdafx.h"
#include "world.h"
#include "object.h"
#include "component_list.h"
#include "sample_component.h"

using namespace std;
using namespace Loki;

typedef TYPELIST_3(
	CompHealthList,
	SimpleComponentList<CompHealth>,
	InheritanceComponentList<ICompVisual>
	) CompListTypeList;
static_assert(TL::Length<CompListTypeList>::value == cNumFamily, "");

template<typename TList> struct CompListFunctor;
template<>
struct CompListFunctor<NullType> {
	static void startUp(World *world) {}
};

template<typename T, typename U>
struct CompListFunctor< Typelist<T, U> > {
	static void startUp(World *world)
	{
		static_assert(std::is_base_of<BaseComponentList, T>::value == 1, "not valid component list class");

		auto familyCode = T::kFamily;
		T *compList = new T();
		compList->startUp();
		world->setCompList(familyCode, compList);
		CompListFunctor<U>::startUp(world);
	}
};

World::World()
	: nextId(1)
{
	std::fill(CompListMap.begin(), CompListMap.end(), nullptr);

	CompListFunctor<CompListTypeList>::startUp(this);

	for(BaseComponentList *compList : CompListMap) {
		SR_ASSERT(compList != nullptr && "component list not assigned");
	}
}

World::~World()
{
	auto it = ObjectMap.begin();
	auto endit = ObjectMap.end();
	for( ; it != endit ; ++it) {
		delete(it->second);
	}

	for(BaseComponentList *compList : CompListMap) {
		delete(compList);
	}
}

BaseComponentList *World::getCompList(comp_id_type familyType)
{
	BaseComponentList *compList = CompListMap[familyType];
	return compList;
}

void World::setCompList(comp_id_type familyType, BaseComponentList *compList)
{
	SR_ASSERT(compList != nullptr);
	SR_ASSERT(CompListMap[familyType] == nullptr);
	CompListMap[familyType] = compList;
}

obj_id_type World::NextId()
{
	int val = nextId;
	nextId++;
	return val;
}

Object *World::create()
{
	Object *obj = new Object(this);
	ObjectMap[obj->getId()] = obj;	
	return obj;
}
void World::remove(Object *obj)
{
	auto found = ObjectMap.find(obj->getId());
	if(found != ObjectMap.end()) {
		ObjectMap.erase(found);
		delete(obj);
	}
}

int World::size() const
{
	return ObjectMap.size();
}
