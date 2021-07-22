/*
 *   _|                                      _|_|_|_|            _|
 *   _|          _|_|    _|      _|      _|  _|        _|_|_|  _|_|_|_|
 *   _|        _|    _|  _|      _|      _|  _|_|_|  _|    _|    _|
 *   _|        _|    _|    _|  _|  _|  _|    _|      _|    _|    _|
 *   _|_|_|_|    _|_|        _|      _|      _|        _|_|_|      _|_|
 * 
 * Gregory J. Duck.
 *
 * Copyright (c) 2017 The National University of Singapore.
 * All rights reserved.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See the LICENSE file for details.
 */

#define LOWFAT_BIG_OBJECT           (1024 * LOWFAT_PAGE_SIZE)/4
#define LOWFAT_NUM_PAGES(size)                                          \
    ((((size) - 1) / LOWFAT_PAGE_SIZE) + 1)
#define LOWFAT_PAGES_BASE(ptr)                                          \
    ((void *)((uint8_t *)(ptr) - ((uintptr_t)(ptr) % LOWFAT_PAGE_SIZE)))
#define LOWFAT_PAGES_SIZE(ptr, size)                                    \
    (LOWFAT_NUM_PAGES(((uint8_t *)(ptr) -                               \
        (uint8_t *)LOWFAT_PAGES_BASE(ptr)) + (size)) * LOWFAT_PAGE_SIZE)

void lowfat_init(void);
extern size_t malloc_usable_size(void *ptr);
extern void *__libc_malloc(size_t size);
extern void *__libc_realloc(void *ptr, size_t size);
extern void __libc_free(void *ptr);



typedef size_t regionid_t;
typedef size_t sizeid_t;

struct sizeinfo_s{
    lowfat_mutex_t mutex;
    regionid_t freelist;
};
typedef struct sizeinfo_s * sizeinfo_t;

LOWFAT_DATA struct sizeinfo_s SIZEMETA[LOWFAT_NUM_REGIONS];

#define ALLOC_PER_REGION 16
lowfat_mutex_t regionmutex;
regionid_t freeregion;



/*
 * Allocator data-structures.
 */
struct lowfat_freelist_s
{
    uintptr_t _reserved;    // Reserved for meta-data.
    struct lowfat_freelist_s *next;
};
typedef struct lowfat_freelist_s *lowfat_freelist_t;

struct lowfat_regioninfo_s
{
    lowfat_mutex_t mutex;
    lowfat_freelist_t freelist;
    void *freeptr;
    void *endptr;
    void *accessptr;

    //assigned size
    lowfat_mutex_t linkmutex;
    sizeid_t allocsizeid;
    unsigned alloccount;
    regionid_t samesizenext;
};
typedef struct lowfat_regioninfo_s *lowfat_regioninfo_t;

LOWFAT_DATA struct lowfat_regioninfo_s LOWFAT_REGION_INFO[LOWFAT_NUM_REGIONS+1];

#define DEBUG(...) 

// added helpers

/*
 * Return the sizeid of the object pointed to by `_ptr`
 */
#ifdef ZOMTAG

#define TEST_NUM_REGIONS        25
#define TEST_MAX_REGIONS        32768                     // 2^15
#define TEST_PAGE_SIZE          4096
#define TEST_MAX_ADDRESS        0x1000000000000ull        // 48bit
#define TEST_REGION_SIZE        0x100000000ull            // 4G
#define TEST_HEAP_MEMORY_OFFSET 0
#define TEST_HEAP_MEMORY_SIZE   2147483648                // 2G

#define DEBUG                   1
#define TEST(s)                 printf("[TEST]\t%s\n", s);

//static bool test_malloc_inited = false;
unsigned long TEST_SIZES[TEST_MAX_REGIONS + 1];

struct test_freelist_s
{
  uintptr_t _reserved;
  struct test_freelist_s *next;
};
typedef struct test_freelist_s *test_freelist_t;

struct test_regioninfo_s
{
  //test_mutex_t mutex;       // disregard multithreading for now
  test_freelist_t freelist;
  void *freeptr;
  void *endptr;
  void *accessptr;

  // Additional elements
  bool    in_use;
  bool    is_mapped;
  int     obj_cnt;                      // number of objects in the region (<= 16)
  struct  test_regioninfo_s *next;      // pointer to next region with same object size
  struct  test_regioninfo_s *priv;      // pointer to prev region with same object size
};
typedef struct test_regioninfo_s *test_regioninfo_t;

struct test_regioninfo_s TEST_REGION_INFO[TEST_MAX_REGIONS + 1];

void test_init_sizes()
{
  TEST_SIZES[1] = 16; TEST_SIZES[2] = 32; TEST_SIZES[3] = 64; TEST_SIZES[4] = 128; TEST_SIZES[5] = 256; 
  TEST_SIZES[6] = 512; TEST_SIZES[7] = 1024; TEST_SIZES[8] = 2048; TEST_SIZES[9] = 4096; TEST_SIZES[10] = 8192;
  TEST_SIZES[11] = 16384; TEST_SIZES[12] = 32768; TEST_SIZES[13] = 65536; TEST_SIZES[14] = 131072; TEST_SIZES[15] = 262144;
  TEST_SIZES[16] = 524288; TEST_SIZES[17] = 1048576; TEST_SIZES[18] = 2097152; TEST_SIZES[19] = 4194304; TEST_SIZES[20]= 8388608;
  TEST_SIZES[21] = 16777216; TEST_SIZES[22] = 33554432; TEST_SIZES[23] = 67108864;
  TEST_SIZES[24] = 1342217728; TEST_SIZES[25] = 268435456;
  //TEST_SIZES[24] = 1040; TEST_SIZES[25] = 1280;
  //TEST_SIZES[26] = 1536; TEST_SIZES[27] = 1792; TEST_SIZES[28] = 2048; TEST_SIZES[29] = 2064; TEST_SIZES[30] = 2560;
  //TEST_SIZES[31] = 3072; TEST_SIZES[32] = 3584; TEST_SIZES[33] = 4096; TEST_SIZES[34] = 4112; TEST_SIZES[35] = 5120;
  //TEST_SIZES[36] = 6144; TEST_SIZES[37] = 7168; TEST_SIZES[38] = 8192; TEST_SIZES[39] = 8208; TEST_SIZES[40] = 10240;
  //TEST_SIZES[41] = 12288; TEST_SIZES[42] = 16384; TEST_SIZES[43] = 32768; TEST_SIZES[44] = 65536; TEST_SIZES[45] = 131072;
  //TEST_SIZES[46] = 262144; TEST_SIZES[47] = 524288; TEST_SIZES[48] = 1048576; TEST_SIZES[49] = 2097152; TEST_SIZES[50] = 4194304;
  //TEST_SIZES[51] = 8388608; TEST_SIZES[52] = 16777216; TEST_SIZES[53] = 33554432; TEST_SIZES[54] = 67108864; // 4G/16
}

static void *test_region(size_t idx)
{
  return (void *)(idx * TEST_REGION_SIZE);
}

bool test_malloc_init(void)
{
  for (size_t i = 0; i < TEST_NUM_REGIONS; i++)
  {
    int prot = PROT_NONE;

    size_t idx = i + 1;
    uint8_t *heapptr = (uint8_t *)test_region(idx) + TEST_HEAP_MEMORY_OFFSET;
    uint8_t *startptr = heapptr;
    test_regioninfo_t info = TEST_REGION_INFO + idx;

    // if (!test_mutex_init(&info->mutex))
      // return false;

    info->freelist = NULL;
    info->freeptr = startptr;
    info->endptr = heapptr + TEST_HEAP_MEMORY_SIZE;
    //info->accessptr = TEST_PAGES_BASE(startptr);
    info->accessptr = ((void *)((uint8_t *)(startptr) - ((uintptr_t)(startptr) %
        TEST_PAGE_SIZE)));
    info->obj_cnt = 0;
    info->next = NULL;
    info->priv = NULL;
    info->in_use = true;
    info->is_mapped = true;

    // #ifdef TEST_NO_PROTECT
    prot |= PROT_READ | PROT_WRITE;
    if (mprotect(heapptr, TEST_HEAP_MEMORY_SIZE, prot) < 0)
        printf("[TEST]\tfailed to mprotect: %s", strerror(errno));
  
  }

  for (size_t i = TEST_NUM_REGIONS; i < TEST_MAX_REGIONS; i++)
  {
    size_t idx = i + 1;
    test_regioninfo_t info = TEST_REGION_INFO + idx;
    info->in_use = false;
    info->is_mapped = false;
  }

  return true;
}

static test_regioninfo_t test_map_region(int index, int size)
{
  int prot = PROT_NONE;
  int flags = MAP_NORESERVE | MAP_ANONYMOUS | MAP_PRIVATE;
  uint8_t *heapptr = (uint8_t *)test_region(index) + TEST_HEAP_MEMORY_OFFSET;
  uint8_t *startptr = heapptr;
  test_regioninfo_t info = TEST_REGION_INFO + index;

  // fill in test_regioninfo_t
  info->freelist = NULL;
  info->freeptr = startptr;
  info->endptr = heapptr + TEST_HEAP_MEMORY_SIZE;
  info->obj_cnt = 0;
  info->next = NULL;
  info->priv = NULL;

  // mmap new region
  if (info->is_mapped == 0)
  {
    void *ptr = mmap(heapptr, TEST_HEAP_MEMORY_SIZE, prot, flags, -1, 0);
    if (ptr != heapptr)
      printf("[TEST]\tfailed to mmap memory: %s", strerror(errno));

    prot |= PROT_READ | PROT_WRITE;
    if (mprotect(heapptr, TEST_HEAP_MEMORY_SIZE, prot) < 0)
      printf("[TEST]\tfailed to mprotect: %s", strerror(errno));
  
    info->is_mapped = true;
  }

  // update TEST_SIZES
  TEST_SIZES[index] = size;

  return info;
}


#endif

_LOWFAT_CONST /*_LOWFAT_INLINE*/ size_t lowfat_sizeid(const void * _ptr){
    regionid_t regionid = lowfat_index(_ptr);
    lowfat_regioninfo_t info = &LOWFAT_REGION_INFO[regionid];
    return info->allocsizeid;
}

// added helpers end

static void *lowfat_fallback_malloc(size_t size)
{
#ifdef LOWFAT_NO_STD_MALLOC_FALLBACK
    lowfat_error("memory allocation failed: %s", strerror(ENOMEM));
#else
    void *ptr = __libc_malloc(size);        // Std malloc().
    if (ptr == NULL)
        lowfat_error("memory allocation failed: %s", strerror(errno));
    //DEBUG(stderr, "lowfat_fallback_malloc %p with request size %lx\n", ptr, size);
    return ptr;
#endif      /* LOWFAT_NO_STD_MALLOC_FALLBACK */
}

#ifndef LOWFAT_WINDOWS
#define lowfat_fallback_free(x)         __libc_free(x)
#define lowfat_fallback_realloc(x, y)   __libc_realloc((x), (y))
#else
#define lowfat_fallback_free(x)         free(x)
#define lowfat_fallback_realloc(x, y)   realloc((x), (y))
#endif

/*
 * Initialize the lowfat_malloc() state.
 */
extern bool lowfat_malloc_init(void)
{
    // init sizemeta
    for(sizeid_t idx=0;idx<LOWFAT_NUM_REGIONS;idx++){
        sizeinfo_t sizeinfo = &SIZEMETA[idx];
        if(!lowfat_mutex_init(&sizeinfo->mutex))
            return false;
    	sizeinfo->freelist = 0; // NULL_REGION
    }
    if(!lowfat_mutex_init(&regionmutex))
        return false;
    freeregion = 1;
    // init sizemeta end
    return true;
}

static bool zomtag_malloc_init_region(regionid_t idx){
        uint8_t *heapptr = (uint8_t *)lowfat_region(idx) +
            LOWFAT_HEAP_MEMORY_OFFSET;

        /* uint32_t roffset;           // Offset for ASLR */
        /* lowfat_rand(&roffset, sizeof(roffset)); */
        /* roffset &= LOWFAT_HEAP_ASLR_MASK; */
        /* uint8_t *startptr = */
        /*     (uint8_t *)lowfat_base(heapptr + roffset + lowfat_size(heapptr) + */
        /*         LOWFAT_PAGE_SIZE); */

        uint8_t *startptr =
           (uint8_t *)lowfat_base(heapptr + lowfat_size(heapptr) + LOWFAT_PAGE_SIZE);
        lowfat_regioninfo_t info = LOWFAT_REGION_INFO + idx;
        if (!lowfat_mutex_init(&info->mutex))
            return false;
        info->freelist  = NULL;
        info->freeptr   = startptr;
        info->endptr    = heapptr + LOWFAT_HEAP_MEMORY_SIZE;
        info->accessptr = LOWFAT_PAGES_BASE(startptr);

        if(!lowfat_mutex_init(&info->linkmutex))
            return false;
        //info->allocsizeid = NUM_SIZES; // assume to be already assigned
	info->alloccount = ALLOC_PER_REGION; // can be smaller
	info->samesizenext = 0; // NULL_REGION

#ifdef LOWFAT_NO_PROTECT
        // In "no protect" mode, make entire heap region accessible
        lowfat_protect(heapptr, LOWFAT_HEAP_MEMORY_SIZE, true, true);
#endif      /* LOWFAT_NO_PROTECT */

	return true;
}

/*
 * LOWFAT malloc()
 */
extern void *lowfat_malloc_index(size_t idx, size_t size);
extern void *lowfat_malloc(size_t size)
{
#ifdef ZOMTAG

  void *ptr; int idx; size_t alloc_size;
  
  // Search for proper allocation size
  // Can be optimized
  for (int i = 1; i <= TEST_NUM_REGIONS; i++)
    {
      if (TEST_SIZES[i] >= size)
        {
          idx = i;
          alloc_size = TEST_SIZES[i];
          break;
        }
    }
  test_regioninfo_t info = TEST_REGION_INFO + idx;
  //printf("[TEST]\tmalloc region index: %d\n", idx);

  // (1) First, attempt to allocate from freelist.
  for(;;)
    {
      if (info->freelist != NULL)
        {
          if (DEBUG) TEST("allocated from freelist");
          test_freelist_t freelist = info->freelist;
          info->freelist = freelist->next;
          ptr = (void *)freelist;
          info->obj_cnt += 1;
          return ptr;
        }

      if (info->next == NULL)
        break;
    
      info = info->next;
    }
  idx = (uintptr_t)info->endptr / TEST_REGION_SIZE;

  // (2) Next, attempt to allocated from fresh space.
  ptr = info->freeptr;
  void *freeptr = (uint8_t *)ptr + size;
  if (info->obj_cnt == 16)
    {
      // region doesn't have enough space or
      // is full with 16 objects.
      // allocate object to the next unused region

      int index;
      // find unused (even if it's mapped already) region
      for (size_t i = TEST_NUM_REGIONS + 1; i <= TEST_MAX_REGIONS; i++)
        {
          if ((TEST_REGION_INFO+i)->in_use == 0)
            {
              index = i;
              break;
            }
        }

      if (DEBUG)
        printf("[TEST]\tmapping new region: index = %d\n", index);

      test_regioninfo_t new_region = test_map_region(index, alloc_size);
      info->next = new_region;
      new_region->priv = info;
      info = new_region;

      ptr = info->freeptr;
      freeptr = (uint8_t *)ptr + size;
      info->obj_cnt += 1;
      info->freeptr = freeptr;
      info->in_use = true;
      return ptr;
    }

  info->freeptr = freeptr;
  info->obj_cnt += 1;
  return ptr;

#else

    //lowfat_heap_select -> sizeid_select
    sizeid_t sizeid = lowfat_heap_select(size);
    if(sizeid == 0){
        return lowfat_fallback_malloc(size);
    }
    else sizeid--;
    sizeinfo_t sizeinfo = &SIZEMETA[sizeid];

    regionid_t regionid;
    lowfat_regioninfo_t info;

    lowfat_mutex_lock(&sizeinfo->mutex);

    //select a heap
    if(sizeinfo->freelist == 0)// NULL_REGION
    {
        if(freeregion > LOWFAT_NUM_REGIONS){
            // all regions are allocated
            // fallback
          return lowfat_fallback_malloc(size);
        }

        //allocate a region to sizeid
        sizeinfo_t sizeinfo = &SIZEMETA[sizeid]; //redundant

        lowfat_mutex_lock(&regionmutex);
        regionid = freeregion++;
        lowfat_mutex_unlock(&regionmutex);

        info = &LOWFAT_REGION_INFO[regionid];

        info->samesizenext = 0; // NULL_REGION
        info->allocsizeid = sizeid;
        zomtag_malloc_init_region(regionid);
        lowfat_mutex_lock(&info->linkmutex);
        sizeinfo->freelist = regionid;
    }
    else{
        regionid = sizeinfo->freelist;
        info = &LOWFAT_REGION_INFO[regionid];
        lowfat_mutex_lock(&info->linkmutex);
    }

    info->alloccount--;
    if(info->alloccount == 0){
        sizeinfo->freelist = info->samesizenext;
    }
    lowfat_mutex_unlock(&info->linkmutex);
    lowfat_mutex_unlock(&sizeinfo->mutex);

    void * ptr = lowfat_malloc_index(regionid, size);
    /*
    putchar('m');
    displayhex((unsigned long long)ptr);
    putchar(' ');
    displayhex((unsigned long long)size);
    putchar(' ');
    displayhex((unsigned long long)LOWFAT_SIZES[sizeid]);
    putchar('\n');
    */

    if(lowfat_is_ptr(ptr)) DEBUG(stderr, "lowfat_malloc %p with request size %lx alloc size %lx\n", ptr, size, lowfat_sizes[sizeid]);
    
		// tag allocated memory
		int64_t tag = 1;
		void *tag_addr;
		void *tag_ptr = ptr;
		for (int i = 0; i < size / 16; i++)
		{
			tag_addr = (int *)((uintptr_t)tag_ptr / 32 + 0x100000000 * 32702);
			tag_ptr += 16;
			/*
			__asm ("STRB %w[t], %[addr]"
						: [t] "=r" (tag)
						: [addr] "r" (tag_addr)
						);
			*/
			//fprintf(stderr, "tag_addr: %p\n", tag_addr);
			//fprintf(stderr, "ptr: %p\n", ptr);
			//fprintf(stderr, "tag_ptr: %p\n", tag_ptr);
			__asm__ __volatile__ ("STR %0, %1":"+r"(tag):"m"(tag_addr));
		}

		return ptr;
#endif
}
extern void *lowfat_malloc_index(size_t /* regionid_t */ idx, size_t size)
{
#ifdef LOWFAT_STANDALONE
    // In "standalone" mode, malloc() may be called before the constructors,
	// so must initialize here.
    if (!lowfat_malloc_inited)
        lowfat_init();
#endif

    if (idx == 0)
    {
        // We cannot handle the allocation size.
        // Fallback to stdlib malloc().
        return lowfat_fallback_malloc(size);
    }
    
    lowfat_regioninfo_t info = LOWFAT_REGION_INFO + idx;
    size_t alloc_size = lowfat_sizes[info->allocsizeid];     // Real allocation size.

    void *ptr;

    lowfat_mutex_lock(&info->mutex);

    // (1) First, attempt to allocate from the freelist.
    lowfat_freelist_t freelist = info->freelist;
    if (freelist != NULL)
    {
        info->freelist = freelist->next;
        lowfat_mutex_unlock(&info->mutex);

        ptr = (void *)freelist;

#ifndef LOWFAT_NO_PROTECT
        // For a free-list object, only the first page of the object is
        // guaranteed to be accessible.  Make the rest accessible here:
        if (alloc_size >= LOWFAT_BIG_OBJECT)
        {
            uint8_t *prot_ptr = (uint8_t *)LOWFAT_PAGES_BASE(ptr);
            size_t prot_size = LOWFAT_PAGES_SIZE(ptr, size);
            lowfat_protect(prot_ptr + LOWFAT_PAGE_SIZE,
                prot_size - LOWFAT_PAGE_SIZE, true, true);

            // Any remaining pages should be PROT_NONE as enforced by
            // lowfat_free().  These serve as guard pages.
        }
#endif      /* LOWFAT_NO_PROTECT */

        return ptr;
    }

    // (2) Next, attempt to allocate from fresh space.
    ptr = info->freeptr;
    void *freeptr = (uint8_t *)ptr + alloc_size;
    if (freeptr > info->endptr)
    {
        // The region is now full.
        // Fallback to stdlib malloc().
        lowfat_mutex_unlock(&info->mutex);
        return lowfat_fallback_malloc(size);
    }
    info->freeptr = freeptr;

#ifndef LOWFAT_NO_PROTECT
    void *accessptr = info->accessptr;
    if (freeptr > accessptr)
    {
        // Ensure that the new space is accessible.
        uint8_t *prot_ptr = (uint8_t *)LOWFAT_PAGES_BASE(ptr);
        size_t prot_size = LOWFAT_PAGES_SIZE(ptr, size);
        if (prot_size < LOWFAT_BIG_OBJECT)
            prot_size = LOWFAT_BIG_OBJECT;
        // Syscall while holding the mutex... :(
        lowfat_protect(prot_ptr, prot_size, true, true);
        info->accessptr = prot_ptr + prot_size;
    }
#endif      /* LOWFAT_NO_PROTECT */
    
    lowfat_mutex_unlock(&info->mutex);
    return ptr;
}

/*
 * LOWFAT free()
 */
extern void lowfat_free(void *ptr)
{

#ifdef ZOMTAG
    if (ptr == NULL)
    return;
  //if (!test_is_ptr(ptr))
  //{
    // If 'ptr' is not low-fat, then it is assumed to from a legacy
    // malloc() allocation
    // test_fallback_free(ptr);
    //return;
  //}

  // It is possible that 'ptr' does not point to the object's base
  // (for memalign() type allocations)
  int index = (uintptr_t)ptr / TEST_REGION_SIZE;
  printf("[TEST]\tregion index of freeing object: %d\n", index);

  size_t alloc_size = TEST_SIZES[index];
  void *ptr_aligned = (void *)((uintptr_t)ptr - (uintptr_t)ptr % alloc_size);

  if (DEBUG)
    printf("[TEST]\tfreeing object: ptr = %p, ptr_aligned = %p\n", ptr, ptr_aligned);

  //if (alloc_size > LOWFAT_BIG_OBJECT { ... }

  test_regioninfo_t info = TEST_REGION_INFO + index;

  info->obj_cnt -= 1;
  if (info->obj_cnt == 0)
  {
    // If freeing an object results an empty region,
    // re-initialize the region
    info->in_use = false;
    info->freelist = NULL;
    info->freeptr = (uint8_t *)test_region(index) + TEST_HEAP_MEMORY_OFFSET; 
    info->priv->next = NULL;
    
    TEST_SIZES[index] = 0;
  }
  else
  {
    //if (DEBUG) TEST("adding to freelist");
    test_freelist_t newfreelist = (test_freelist_t)ptr_aligned;
    test_freelist_t oldfreelist = info->freelist;
    newfreelist->next = oldfreelist;
    info->freelist = newfreelist;
  }

#else
    if (ptr == NULL)    // free(NULL) is a NOP.
        return;
    DEBUG(stderr, "lowfat_free receive free request ptr %p\n", ptr);
    if (!lowfat_is_ptr(ptr))
    {
        // If `ptr' is not low-fat, then it is assumed to from a legacy
        // malloc() allocation.
      
        lowfat_fallback_free(ptr);
        return;
    }
    if (!lowfat_is_heap_ptr(ptr))
    {
        // Attempt to free a stack or global pointer.
        const char *kind = (lowfat_is_stack_ptr(ptr)? "stack": "global");
        lowfat_error(
            "attempt to free a %s pointer detected!\n"
            "\tpointer = %p (%s)\n"
            "\tbase    = %p\n"
            "\tsize    = %zd\n",
            kind, ptr, kind, lowfat_base(ptr), lowfat_size(ptr));
    }

    // It is possible that `ptr' does not point to the object's base (for
    // memalign() type allocations).
    ptr = lowfat_base(ptr);

    regionid_t idx = lowfat_index(ptr);
    lowfat_regioninfo_t info = &LOWFAT_REGION_INFO[idx];

    sizeid_t sizeid = info->allocsizeid;
    sizeinfo_t sizeinfo = &SIZEMETA[sizeid];

    size_t alloc_size = lowfat_sizes[sizeid];
    if (alloc_size >= LOWFAT_BIG_OBJECT)
    {
        // This is a big object, so return memory to the OS.
        // The first page is not returned size it is used as the freelist node.
        uint8_t *prot_ptr = (uint8_t *)LOWFAT_PAGES_BASE(ptr);
        uint8_t *prot_end_ptr = (uint8_t *)ptr + alloc_size;
        prot_end_ptr = prot_end_ptr -
            ((uintptr_t)prot_end_ptr % LOWFAT_PAGE_SIZE);
        size_t prot_size = prot_end_ptr - prot_ptr;

        lowfat_dont_need(prot_ptr + LOWFAT_PAGE_SIZE,
            prot_size - LOWFAT_PAGE_SIZE);
#ifndef LOWFAT_NO_PROTECT
        lowfat_protect(prot_ptr + LOWFAT_PAGE_SIZE,
            prot_size - LOWFAT_PAGE_SIZE, false, false);
#endif      /* LOWFAT_NO_PROTECT */
    }

    lowfat_mutex_lock(&info->mutex);
    lowfat_freelist_t newfreelist = (lowfat_freelist_t)ptr;
    lowfat_freelist_t oldfreelist = info->freelist;
    DEBUG(stderr, "insert %p to freelist in region %lu\n", ptr, idx);
    newfreelist->next = oldfreelist;
    info->freelist = newfreelist;
    lowfat_mutex_unlock(&info->mutex);

    lowfat_mutex_lock(&sizeinfo->mutex);
    lowfat_mutex_lock(&info->linkmutex);

    if(info->alloccount == 0){
        info->samesizenext = sizeinfo->freelist;
        sizeinfo->freelist = idx;
    }
    info->alloccount++;
    lowfat_mutex_unlock(&info->linkmutex);
    lowfat_mutex_unlock(&sizeinfo->mutex);
#endif
}

/*
 * Stdlib malloc() and free() replacements.
 */

#ifndef LOWFAT_NO_REPLACE_STD_FREE
// free()/realloc() should always be intercepted.  This handles the case where
// memory is allocated by the main program, but free'ed by an uninstrumented
// library.
extern void free(void *ptr) LOWFAT_ALIAS("lowfat_free");
extern void *realloc(void *ptr, size_t size) LOWFAT_ALIAS("lowfat_realloc");
extern void _ZdlPv(void *ptr) LOWFAT_ALIAS("lowfat_free");
extern void _ZdaPv(void *ptr) LOWFAT_ALIAS("lowfat_free");
#endif      /* LOWFAT_NO_REPLACE_STD_FREE */

#ifndef LOWFAT_NO_REPLACE_STD_MALLOC
extern void *malloc(size_t size) LOWFAT_ALIAS("lowfat_malloc");
extern void *calloc(size_t nmemb, size_t size) LOWFAT_ALIAS("lowfat_calloc");
extern int posix_memalign(void **memptr, size_t align, size_t size)
    LOWFAT_ALIAS("lowfat_posix_memalign");
extern void *memalign(size_t align, size_t size)
    LOWFAT_ALIAS("lowfat_memalign");
extern void *valloc(size_t size) LOWFAT_ALIAS("lowfat_valloc");
extern void *pvalloc(size_t size) LOWFAT_ALIAS("lowfat_pvalloc");
extern void *_Znwm(size_t size) LOWFAT_ALIAS("lowfat_malloc");
extern void *_Znam(size_t size) LOWFAT_ALIAS("lowfat_malloc");
extern void *_ZnwmRKSt9nothrow_t(size_t size) LOWFAT_ALIAS("lowfat_malloc");
extern void *_ZnamRKSt9nothrow_t(size_t size) LOWFAT_ALIAS("lowfat_malloc");
#ifdef __strdup
#undef __strdup
#endif
extern char *__strdup(const char *str) LOWFAT_ALIAS("lowfat_strdup");
#ifdef __strndup
#undef __strndup
#endif
extern char *__strndup(const char *str, size_t n)
	LOWFAT_ALIAS("lowfat_strndup");
#endif      /* LOWFAT_NO_REPLACE_STD_MALLOC */

/*
 * LOWFAT realloc()
 */
extern void *lowfat_realloc(void *ptr, size_t size)
{
#ifdef ZOMTAG

  if (ptr == NULL || size == 0)
    return NULL;

  // (2) 'ptr' and 'size' map to the same region
  int index = (uintptr_t)ptr / TEST_REGION_SIZE;
  size_t index_size = TEST_SIZES[index];
  size_t alloc_size;
  for (int i = 1; i <= TEST_NUM_REGIONS; i++)
  {
    if (TEST_SIZES[i] >= size)
    {
      alloc_size = TEST_SIZES[i];
      break;
    }
  }
  if (index_size == alloc_size)
    return ptr;

  // (3) Do the reallocation + copy
  // if (!lowfat_is_ptr(ptr)) fallback(ptr)

  void *newptr = lowfat_malloc(size);
  if (newptr == NULL) return NULL;
  size_t cpy_size;
  if (alloc_size > index_size) cpy_size = index_size;
  else cpy_size = alloc_size;

  memcpy(newptr, ptr, cpy_size);
  lowfat_free(ptr);

  return newptr;

#else
    // (1) Check for cheap exits:
    if (ptr == NULL || size == 0)
        return lowfat_malloc(size);
    if (lowfat_is_ptr(ptr) &&
        LOWFAT_REGION_INFO[lowfat_index(ptr)].allocsizeid == lowfat_heap_select(size)-1)
    {
#ifndef LOWFAT_NO_PROTECT
        // `ptr' and `size' map to the same region; allocation can be avoided.
        size_t alloc_size = lowfat_sizes[LOWFAT_REGION_INFO[lowfat_index(ptr)].allocsizeid];
        if (alloc_size >= LOWFAT_BIG_OBJECT)
        {
            void *prot_ptr = LOWFAT_PAGES_BASE(ptr);
            size_t prot_size = LOWFAT_PAGES_SIZE(ptr, alloc_size);
            lowfat_protect(prot_ptr, prot_size, true, true);
        }
#endif      /* LOWFAT_NO_PROTECT */
        return ptr;
    }
    if (!lowfat_is_ptr(ptr)){
        void * ptr2 = lowfat_fallback_realloc(ptr, size);
	DEBUG(stderr, "lowfat_fallback_realloc %p request size %lu\n", ptr2, size);
	return ptr2;
    }

    // (2) Do the reallocation + copy:
    void *newptr = lowfat_malloc(size);
    if (newptr == NULL)
        return NULL;
    size_t cpy_size;
    size_t idx = lowfat_index(ptr);
    sizeid_t sizeid = LOWFAT_REGION_INFO[idx].allocsizeid;
    size_t ptr_size = lowfat_sizes[sizeid];
    cpy_size = (size < ptr_size? size: ptr_size);
#ifndef LOWFAT_NO_PROTECT
    if (ptr_size >= LOWFAT_BIG_OBJECT)
    {
        // Note: the allocator does not track the object size; only the
        //       allocation size.  Some pages may be inaccessible.
        //       The inaccessible pages must be made accessible before
        //       copying.
        void *prot_ptr = LOWFAT_PAGES_BASE(ptr);
        size_t prot_size = LOWFAT_PAGES_SIZE(ptr, ptr_size);
        lowfat_protect(prot_ptr, prot_size, true, true);
    }
#endif      /* LOWFAT_NO_PROTECT */
    memcpy(newptr, ptr, cpy_size);
    lowfat_free(ptr);

    return newptr;
#endif
}

/*
 * LOWFAT calloc()
 */
extern void *lowfat_calloc(size_t nmemb, size_t size)
{
    void *ptr = lowfat_malloc(nmemb * size);
    memset(ptr, 0, nmemb * size);
    return ptr;
}

/*
 * LOWFAT posix_memalign()
 */
extern int lowfat_posix_memalign(void **memptr, size_t align, size_t size)
{
    if (align < sizeof(void *) || (align & (align - 1)) != 0)
        lowfat_error("invalid posix_memalign parameter: %s",
            strerror(EINVAL));
    if (align == LOWFAT_MIN_ALLOC_SIZE)
        *memptr = lowfat_malloc(size);
    else if (size < align)
        *memptr = lowfat_malloc(align-1);
    else
    {
        size_t nsize = size + align - 1;
        uint8_t *ptr = (uint8_t *)lowfat_malloc(nsize);
        size_t offset = (uintptr_t)ptr % align;
        offset = (offset != 0? align - offset: offset);
        ptr += offset;
        *memptr = (void *)ptr;
    }
    return 0;
}

/*
 * LOWFAT memalign()
 */
extern void *lowfat_memalign(size_t align, size_t size)
{
    void *ptr = NULL;
    lowfat_posix_memalign(&ptr, align, size);
    return ptr;
}

/*
 * LOWFAT aligned_alloc()
 */
extern void *lowfat_aligned_alloc(size_t align, size_t size)
    LOWFAT_ALIAS("lowfat_memalign");

/*
 * LOWFAT valloc()
 */
extern void *lowfat_valloc(size_t size)
{
    return lowfat_memalign(LOWFAT_PAGE_SIZE, size);
}

/*
 * LOWFAT pvalloc()
 */
extern void *lowfat_pvalloc(size_t size)
{
    return lowfat_memalign(LOWFAT_PAGE_SIZE,
        LOWFAT_NUM_PAGES(size) * LOWFAT_PAGE_SIZE);
}

/*
 * LOWFAT C++ new
 */
extern void *lowfat__Znwm(size_t size) LOWFAT_ALIAS("lowfat_malloc");

/*
 * LOWFAT C++ new[]
 */
extern void *lowfat__Znam(size_t size) LOWFAT_ALIAS("lowfat_malloc");

/*
 * LOWFAT C++ new nothrow
 */
extern void *lowfat__ZnwmRKSt9nothrow_t(size_t size)
    LOWFAT_ALIAS("lowfat_malloc");

/*
 * LOWFAT C++ new[] nothrow
 */
extern void *lowfat__ZnamRKSt9nothrow_t(size_t size)
    LOWFAT_ALIAS("lowfat_malloc");

/*
 * LOWFAT C++ delete
 */
extern void lowfat__ZdlPv(void *ptr) LOWFAT_ALIAS("lowfat_free");

/*
 * LOWFAT C++ delete[]
 */
extern void lowfat__ZdaPv(void *ptr) LOWFAT_ALIAS("lowfat_free");

/*
 * LOWFAT strdup()
 */
extern char *lowfat_strdup(const char *str)
{
    size_t str_size = lowfat_buffer_size(str);
    size_t len = strnlen(str, str_size);
    if (len == str_size)
        lowfat_oob_error(LOWFAT_OOB_ERROR_STRDUP, str + str_size,
            lowfat_base(str));
    char *str2 = (char *)lowfat_malloc(len+1);
    memcpy(str2, str, len+1);
    return str2;
}

/*
 * LOWFAT strndup()
 */
extern char *lowfat_strndup(const char *str, size_t n)
{
    size_t str_size = lowfat_buffer_size(str);
    size_t len = strnlen(str, (n > str_size? str_size: n));
    if (len == str_size)
        lowfat_oob_error(LOWFAT_OOB_ERROR_STRDUP, str + str_size,
            lowfat_base(str));
    char *str2 = (char *)lowfat_malloc(len+1);
    memcpy(str2, str, len);
    str2[len] = '\0';
    return str2;
}

#if !defined(LOWFAT_WINDOWS)
/*
 * LOWFAT malloc_usable_size()
 */
typedef size_t (*malloc_usable_size_t)(void *);
extern size_t malloc_usable_size(void *ptr)
{
    if (lowfat_is_ptr(ptr))
        return lowfat_size(ptr);
    static malloc_usable_size_t libc_malloc_usable_size = NULL;
    if (libc_malloc_usable_size == NULL)
    {
        libc_malloc_usable_size =
            (malloc_usable_size_t)dlsym(RTLD_NEXT, "malloc_usable_size");
        if (libc_malloc_usable_size == NULL)
            lowfat_error("failed to find libc malloc_usable_size()");
    }
    return libc_malloc_usable_size(ptr);
}
#endif

