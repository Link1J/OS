#include <stddef.h>

extern "C" 
{
	void*	memcpy	(void* dsc, void* src, size_t n				);
	void*	memset	(void* dsc, int val, size_t n				);
	int		memcmp	(const void* s1, const void* s2, size_t n	);
	void*	malloc	(size_t size								);
	void	free	(void* block								);
}
