#ifndef ALLOC_H
#define ALLOC_H

#ifdef USE_ARENA_ALLOCATOR
//	extern doug_lea_allocator	g_collision_allocator;

#	define CNEW(type)			new ( g_collision_allocator.alloc_impl<type>(1) ) type 
#	define CDELETE(ptr)			cdelete(ptr)
#	define CFREE(ptr)			g_collision_allocator.free_impl(ptr)
#	define CMALLOC(size)		g_collision_allocator.malloc_impl(size)
#	define CALLOC(type,count)	g_collision_allocator.alloc_impl<type>(count)
#else // #ifdef USE_ARENA_ALLOCATOR
#	define CNEW(type)			new ( xr_alloc<type>(1) ) type
#	define CDELETE(ptr)			xr_delete(ptr)
#	define CFREE(ptr)			xr_free(ptr)
#	define CMALLOC(size)		xr_malloc(size)
#	define CALLOC(type,count)	xr_alloc<type>(count)
#endif // #ifdef USE_ARENA_ALLOCATOR

#include <fast_dynamic_cast.h>

template <bool _is_pm, typename T>
struct cspecial_free
{
	IC void operator()(T* & ptr)
	{
		void* _real_ptr = fast_dynamic_cast<void*>(ptr);
		ptr->~T();
		CFREE(_real_ptr);
	}
};

template <typename T>
struct cspecial_free<false, T>
{
	IC void operator()(T* & ptr)
	{
		ptr->~T();
		CFREE(ptr);
	}
};

template <class T>
IC void cdelete(T* & ptr)
{
	if (ptr)
	{
		cspecial_free<is_polymorphic<T>::result, T>()(ptr);
		ptr = NULL;
	}
}

#endif