#include <stdlib.h>

#include "dslink/mem/mem.h"
#include "dslink/col/vector.h"

#include <string.h>

static inline int vector_resize(Vector* vec)
{
    if(!vec) {
        return -1;
    }
    if(vec->size >= vec->capacity) {
        uint32_t cap = vec->capacity * 2;
        if(!cap) {
            cap = 64;
        }
        void* data = dslink_realloc(vec->data, cap*vec->element_size);
        if(!data) {
            return -1;
        }
        vec->data = data;
        vec->capacity = cap;
    }

    return 0;
}

static inline void* element_ptr(const Vector* vec, uint32_t index)
{
  return (char*)vec->data + (index*vec->element_size);
}

int vector_init(Vector* vec, uint32_t initial_size, size_t element_size)
{
    if(!vec) {
        return -1;
    }
    vec->data = dslink_malloc(initial_size*element_size);
    if(!vec->data) {
        return -1;
    }

    vec->element_size = element_size;
    vec->capacity = initial_size;
    vec->size = 0;

    return 0;
}

uint32_t vector_count(const Vector* vec)
{
    if(!vec) {
        return 0;
    }

    return vec->size;
}

long vector_append(Vector* vec, void* data)
{
    if(!vec) {
        return -1;
    }
    if(vec->size >= vec->capacity) {
        // not enough room left, resize
        if(vector_resize(vec) != 0) {
            return -1;
        }
    }
    memcpy( element_ptr( vec, vec->size), data, vec->element_size);
    ++vec->size;

    return vec->size-1;
}

int vector_swap(Vector* vec, Vector* vec2)
{
  if(!vec || !vec2) {
    return -1;
  }
  if (vec->element_size != vec2->element_size) {
    return -1;
  }
  Vector dummy;
  memcpy( &dummy, vec, sizeof(Vector) );
  memcpy( vec, vec2, sizeof(Vector) );
  memcpy( vec2, &dummy, sizeof(Vector) );
  return 0;
}

int vector_set(Vector* vec, uint32_t index, void* data)
{
    if(!vec || index >= vec->size) {
        return -1;
    }
    memcpy( element_ptr( vec, index), data, vec->element_size);

    return 0;
}

void* vector_get(const Vector* vec, uint32_t index)
{
    if(!vec || index >= vec->size) {
        return NULL;
    }

    return element_ptr( vec, index);
}

int vector_erase(Vector* vec, uint32_t index)
{
    if(!vec || index >= vec->size-1) {
        return -1;
    }
    if(index != vec->size-1) {
      memmove( element_ptr( vec, index), element_ptr( vec, index+1), (vec->size-(index+1))*vec->element_size);
    }
    --(vec->size);

    return 0;
}

int vector_erase_range(Vector* vec, uint32_t lower, uint32_t upper)
{
    if(!vec || lower > vec->size-1 || lower >= upper) {
        return -1;
    }

    if(upper >= vec->size) {
        upper = vec->size-1;
    } else {
        --upper;
    }

    if(lower == 0 && upper == vec->size) {
        vec->size = 0;
        return 0;
    }

    if(upper < vec->size-1) {
      memmove( element_ptr( vec, lower), element_ptr( vec, upper+1), (vec->size-(upper-lower))*vec->element_size);
    }
    vec->size -= (upper - lower)+1;

    return 0;
}

long vector_find(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn)
{
    if(!vec || vec->size == 0) {
        return -1;
    }

    for(uint32_t n = 0; n < vec->size; ++n) {
        if(cmp_fn(data, (char*)vec->data + (n*vec->element_size)) == 0) {
            return n;
        }
    }

    return -1;
}

uint32_t vector_remove_if(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn)
{
  uint32_t first;

  long found = vector_find( vec, data, cmp_fn );
  if ( found >= 0 ) {
    first = (uint32_t)found;
    for (uint32_t i = first; ++i != vec->size; ) {
      if (cmp_fn( data, element_ptr( vec, i)) != 0) {
	memmove( element_ptr( vec, first), element_ptr( vec, i), vec->element_size);
	++first;
      }
    }
  } else {
    first = vec->size;
  }
  
  return first;
}

long vector_binary_search(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn)
{
    if(!vec || vec->size == 0) {
        return -1;
    }

    return vector_binary_search_range( vec, data, cmp_fn, 0, vec->size );
}

long vector_binary_search_range(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn, uint32_t lower, uint32_t r)
{
    if ( !vec || vec->size == 0 || !r || lower >= r) {
        return -1;
    }

    int res = cmp_fn( data, element_ptr( vec, lower));
    if ( res <= 0 ) {
      return res == 0 ? (long)lower : -1;
    }
    
    if ( r >= vec->size ) {
      r = vec->size-1;
    } else {
      --r;
    }
    
    res = cmp_fn( data, element_ptr( vec, r));
    if ( res >= 0 ) {
      return res == 0 ? (long)r : -1;
    }
    
    uint32_t l = lower;
    uint32_t m = 0;
    while(l <= r) {
        m = (l + r) / 2;
        int res = cmp_fn( data, element_ptr( vec, m));
        if(res == 0) {
            return m;
        } else if(res > 0) {
            l = m+1;
        } else {
            r = m-1;
        }
    }

    return -1;
}

uint32_t vector_upper_bound(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn)
{
    if(!vec || vec->size == 0) {
        return 0;
    }

    return vector_upper_bound_range( vec, data, cmp_fn, 0, vec->size );
}

uint32_t vector_upper_bound_range(const Vector* vec, void* data, vector_comparison_fn_type cmp_fn, uint32_t first, uint32_t last)
{
    if ( !vec || vec->size == 0 || !last) {
        return first;
    }

    if ( last > vec->size ) {
      last = vec->size;
    } 
    if (last <= first) {
      return last;
    }

    uint32_t count = last- first;

    while (count > 0) {
      uint32_t it = first;
      uint32_t step = count / 2;
      it += step;
      
      int res = cmp_fn( data, element_ptr( vec, it));
      if(res >= 0) {
	first = ++it;
	count -= step + 1;
      } else {
	count = step;
      }
    }

    return first;
}


int vector_free(Vector* vec)
{
    if(!vec) {
        return -1;
    }

    dslink_free(vec->data);

    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;

    return 0;
}
