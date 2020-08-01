#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  #define align_up(addr, align) (((addr) - ((align)-1)) & (~((align)-1))) */

#define UNUSED_ARG(arg) ((void)(arg))

static inline size_t align_up(const size_t num, const size_t alignment) {
  assert(alignment % 2 == 0);
  return (size_t)((num + alignment - 1) & ~(alignment - 1));
}

static const size_t kDefaultAlign = 8;

void *DefaultAllocFn(const size_t size, void *user) {
  UNUSED_ARG(user);
  return malloc(size);
}

void DefaultFreeFn(void *obj, const size_t size, void *user) {
  UNUSED_ARG(size);
  UNUSED_ARG(user);
  free(obj);
}

struct ForwardListAllocator {
  void *(*allocFn)(const size_t size, void *user);
  void (*freeFn)(void *obj, const size_t size, void *user);
};

#define DEFAULT_LIST_ALLOCATOR                                                 \
  { .allocFn = DefaultAllocFn, .freeFn = DefaultFreeFn }

struct ForwardListConfiguration {
  size_t elementSize;
  size_t elementAlign;
  void (*elementInitializeFn)(void *obj, void *user, void *arg);
  void (*elementDestroyFn)(void *obj, void *user);
  struct ForwardListAllocator allocator;
};

struct ForwardListNode {
  struct ForwardListNode *next;
  uint16_t offset;
  uint8_t storage[1];
};

struct ForwardList {
  struct ForwardListConfiguration cfg;
  struct ForwardListNode *root;
  size_t count;
  void *userData;
};

struct SomeType {
  int32_t x;
  int32_t y;
  uint8_t a;
  uint16_t b;
};

int main(int argc, char **argv) {
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);

  /*
  for (size_t addr = 0; addr < 32; ++addr) {
    printf("\n %zu aligned to %zu = %zu", addr, kDefaultAlign,
           align_up(addr, kDefaultAlign));
  }

  */
  struct ForwardListAllocator alloc = DEFAULT_LIST_ALLOCATOR;
  const struct ForwardListConfiguration listSetup = {
      .elementSize = sizeof(struct SomeType),
      .elementAlign = _Alignof(struct SomeType)};

  printf("\nE size = %zu, e align = %zu", listSetup.elementSize,
         listSetup.elementAlign);

  const size_t storageOffset = offsetof(struct ForwardListNode, storage);
  const size_t requiredAlignment = _Alignof(struct SomeType);
  const uint16_t off = (uint16_t)align_up(storageOffset, requiredAlignment);

  printf("\n Storage offset = %zu, required alignment = %zu, offset = %hu",
         storageOffset, requiredAlignment, off);

  const size_t allocSize = sizeof(struct ForwardListNode) +
                           sizeof(struct SomeType) + (off - storageOffset);
  printf("\nList node size = %zu, stored element size = %zu, total allocation "
         "size = %zu",
         sizeof(struct ForwardListNode), sizeof(struct SomeType), allocSize);
  return 0;
}
