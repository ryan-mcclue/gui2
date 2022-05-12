// SPDX-License-Identifier: zlib-acknowledgement

typedef struct BufferHeader {
	u64 len;
	u64 cap;
  MemoryArena arena;

	u8 buffer[0];
} BufferHeader;

#define __BUFFER_HEADER(buf)	\
	((BufferHeader *)((u8 *)(buf) - offsetof(BufferHeader, buffer)))

#define BUFFER_LEN(buf)	\
	(((buf) != NULL) ? __BUFFER_HEADER(buf)->len: 0)

#define BUFFER_CAP(buf)	\
	(((buf) != NULL) ? __BUFFER_HEADER(buf)->cap: 0)

#define __BUFFER_FITS(buf, amount) \
	(BUFFER_LEN(buf) + (amount) <= BUFFER_CAP(buf))

#define __BUFFER_FIT(buf, amount)	\
	(BUF__FITS(b, 1) ? 0 : ((b) = buf__grow((b), BUF_LENGTH(b) + (amount), sizeof(*(b)))))

#define BUF_PUSH(b, element)	\
	(BUF__FIT(b, 1), b[BUF_LENGTH(b)] = (element), BUF__HEADER(b)->length++) 

#define BUF_FREE(b)	\
	(((b) != NULL) ? free(BUF__HEADER(b)) : 0, (b) = NULL)

void* buf__grow(const void* buf, size_t new_length, size_t elem_size)
{
	size_t new_cap = MAX(1 + 2 * BUF_CAPACITY(buf), new_length);
	// length of header + length of payload
	size_t new_size = offsetof(BufferHeader, buffer) + new_cap * elem_size;

	BufferHeader* new_header = NULL;
	
	if (buf != NULL) {
		new_header = realloc(BUF__HEADER(buf), new_size); 
	} else {
		new_header = malloc(new_size);
		new_header->length = 0;
	}

	new_header->capacity = new_cap;

	return new_header->buffer; 
}
