#include "logger.h"
#include "array.h"

/* If 0, start buffer with 16 elements. Otherwise grow in increments of 2. */
#define GA_GROWBUFFER(capacity) ((capacity > 0) ? capacity * 2 : 16)

ga_array ga_init(size_t count, const ti_typeinfo *info)
{
    ga_array inst = {
        .rawbytes = (count != 0) ? malloc(info->size * count) : nullptr,
        .count = 0, // No elements written yet.
        .capacity = count, // Can store this many elements.
        .info = info
    };
    // Only indicate an error if we tried to `malloc` and it failed.
    if (count != 0 && inst.rawbytes == nullptr) {
        lg_perror("ga_init", "malloc failed");
    }
    return inst;
}

void ga_deinit(ga_array *self)
{
    if (!self) {
        return;
    }
    ti_deinitfn *deinit = self->info->fnlist->deinit;
    for (size_t i = 0; i < self->count; i++) {
        ga_byte *element = ga_rawretrieve(self, i);
        deinit(element);
    }
    free(self->rawbytes);
}

void *ga_assign(ga_array *self, void *dst, const void *src)
{
    return self->info->fnlist->copy(dst, src);
}

bool ga_resize(ga_array *self, size_t newcapacity)
{
    if (newcapacity == self->capacity) {
        return true;
    }
    // If shortening buffer, use `newcapacity` as number of written elements.
    if (newcapacity < self->capacity) {
        self->count = newcapacity;
    }
    self->capacity = newcapacity; // Update only after determining new counter.
    size_t rawsize = self->info->size * newcapacity;
    ga_byte *tmp = realloc(self->rawbytes, rawsize);
    if (tmp == nullptr) {
        lg_perror("ga_resize", "realloc buffer failed");
        return false;
    }
    self->rawbytes = tmp; // Successful realloc already frees original pointer.
    return true;
}

bool ga_push_back(ga_array *self, const void *src)
{
    // Reached current allocated limit so try to resize buffer.
    if (self->count + 1 > self->capacity) {
        size_t newcapacity = GA_GROWBUFFER(self->capacity);
        if (!ga_resize(self, newcapacity)) {
            lg_perror("ga_push_back", "push back failed");
            return false;
        }
    }
    void *dst = ga_rawretrieve(self, self->count);
    if (ga_assign(self, dst, src) != nullptr) {
        self->count++;
        return true;
    } 
    return false;
}

void *ga_retrieve(const ga_array *self, size_t index)
{
    if (index < self->capacity) {
        return ga_rawretrieve(self, index);
    }
    lg_logprintf("ga_retrieve", "Invalid index %zu (of %zu)", index, self->capacity);
    return nullptr;
}

void *ga_rawretrieve(const ga_array *self, size_t index)
{
    size_t rawoffset = self->info->size * index;
    return &(self->rawbytes[rawoffset]);
}
