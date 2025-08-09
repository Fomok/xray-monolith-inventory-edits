#ifndef DESTRUCTOR_H
#define DESTRUCTOR_H

#include "xrMemory.h"

// destructor
template <class T>
class destructor
{
	T* ptr;
public:
	destructor(T* p) { ptr = p; }
	~destructor() { xr_delete(ptr); }
	IC T& operator()()
	{
		return *ptr;
	}
};

#endif