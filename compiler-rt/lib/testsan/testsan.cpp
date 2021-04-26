#include "testsan/testsan.h"
#include "unistd.h"

using namespace __sanitizer;

INTERCEPTOR(void *, malloc, uptr size)
{
	return testsan_Malloc(size);
}

void testsan_InitInterceptors()
{
	INTERCEPT_FUNCTION(malloc);
}
using namespace __testsan {
	static struct
	{
		int shadow_mem_size;
		char *shadow_mem_start;
		char *shadow_mem_end;
	} shadow_memory_storage;
} // end of __testsan

using namespace __testsan;

void testsan_AllocateShadowMemory()
{
	shadow_memory_storage.shadow_mem_size = 1000 * sizeof(*shadow_mem_storage.shadow_mem_start);
	shadow_memory_storage.shadow_mem_start = (char *)MmapNoReserveOrDie(shadow_memory_storage.shadow_mem_size, "Simple Shadow Memory");
	shadow_memory_storage.shadow_mem_end = shadow_memory_storage.shadow_mem_start + shadow_memory_storage.shadow_mem_size;

	VReport(1, "Shadow mem at %zx .. %zx\n", shadow_memory_storage.shadow_mem_start, shadow_memory_storage.shadow_mem_end);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void testsan_AfterMalloc(char *value)
{
	Printf("Malloc returned address %x\n", value);
	return;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void testsan_HelloFunction(char *func_name)
{
	return;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
void testsan_EndOfMain()
{
	write(1, "End of main function!\n", internal_strlen("End of main function!\n"));
	return;
}

void *testsan_Malloc(uptr size)
{
	void *ret = REAL(malloc)(size);
	write(1, "Hooked malloc!\n", internal_strlen("Hooked malloc!\n"));
	return ret;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
#if !SANITIZER_CAN_USE_PREINIT_ARRAY
__attribute__((constructor(0)))
#endif

void testsan_Init()
{
	SanitizerToolName = "testsan";
	SetCommonFlagsDefaults();
	testsan_InitInterceptors();
	testsan_AllocateShadowMemory();
	VReport(2, "Initialized testsan runtime!\n");
}

#if SANITIZER_CAN_USE_PREINIT_ARRAY
extern "C" {
	__attribute__((section(".preinit_array"), used)) void (*testsan_init_ptr)(void) = testsan_Init;
}
#endif
