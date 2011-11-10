#include "malloc.h"
#include "arrays.h"
#include "listex.h"

/*
Terminology: if the array is STUFF1_empty_STUFF2, then bighead refers to stuff2, bigtail refers to stuff1.
*/

struct ring_buffer{
	int *fields; // buffer contents
	int size;
	int first; // start counting with 0
	int len;
};

bool is_split_up(int size, int first, int len)
//@ requires size * 4 < INT_MAX &*& len <= size &*& len >= 0;
//@ ensures result == is_split_up_fp(size, first, len);
{
	return first > size - len;
}
/*@
fixpoint bool is_split_up_fp(int size, int first, int len){
	return first + len > size;
}

fixpoint int bighead_size(int size, int first, int len){
	return size - first;
}


fixpoint int bigtail_size(int size, int first, int len){
	return len - bighead_size(size, first, len);
}


predicate ring_buffer(struct ring_buffer *buffer, int size, int first, int len, list<int> items) =
	size >= 0 && size * 4 < INT_MAX
	&*& len <= size
	&*& first >= 0 && first < size
	
	&*& length(items) == len
	
	&*& buffer->fields |-> ?fields
	&*& buffer->size |-> size
	&*& buffer->first |-> first
	&*& buffer->len |-> len
	
	&*& malloc_block(fields, sizeof(int) * size)
	&*& malloc_block_ring_buffer(buffer)

	&*& fields + size <= (void*)UINTPTR_MAX
	
	&*& is_split_up_fp(size, first, len) ?
	
		// Bigtail
		array<int>(fields, bigtail_size(size, first, len), sizeof(int), integer, ?bigtail_elems)
	
		// emptyness
		&*& array<int>(fields + bigtail_size(size, first, len), size - len, sizeof(int), integer, _)
		
		// Bighead
		&*& array<int>(fields + first, bighead_size(size, first, len), sizeof(int), integer, ?bighead_elems)
		
		// make sure verification knows all arrays are next to each other.
		&*& first == bigtail_size(size, first, len) + (size - len)
		
		&*& append(bighead_elems, bigtail_elems) == items
		
	:
		// leading emptyness
		array<int>(fields, first, sizeof(int), integer, _)
		
		// Elems
		&*& array<int>(fields + first, len, sizeof(int), integer, items)
		
		// trailing emptyness
		&*& array<int>(fields + first + len, size - len - first, sizeof(int), integer, _)
	;
	
@*/


struct ring_buffer *ring_buffer_create(int size)
//@ requires size >= 1 &*& size * sizeof(int) < INT_MAX;
/*@ ensures
	result == 0 ? true
	: ring_buffer(result, size, 0, 0, nil)
;
@*/
{	
	struct ring_buffer *ring_buffer = malloc(sizeof (struct ring_buffer));
	if (ring_buffer == 0) return 0;
	int *fields = malloc(size * sizeof (int));
	if (fields == 0){
		free(ring_buffer);
		return 0;
	}
	//@ chars_limits((void*)fields);
	//@ chars_to_int_array(fields, size);
	ring_buffer->fields =  fields;
	ring_buffer->size = size;
	ring_buffer->first = 0;
	ring_buffer->len = 0;
	
	//@ close ring_buffer(ring_buffer, size, 0, 0, nil);
	return ring_buffer;
}

// i.e. add to end of queue.
void ring_buffer_push(struct ring_buffer *ring_buffer, int element)
/*@ requires
	ring_buffer(ring_buffer, ?size, ?first, ?len, ?items)
	&*& len < size // you can't push more elements if it's already full.
	;
@*/
//@ ensures ring_buffer(ring_buffer, size, first, len+1, ?items2) &*& append(items, cons(element, nil)) == items2;
{

	//@ open ring_buffer(ring_buffer, size, first, len, items);
	//@ assert ring_buffer->fields |-> ?fields;
	
	int put_at;
	if (is_split_up(ring_buffer->size, ring_buffer->first, ring_buffer->len+1)){
		put_at = ring_buffer->len - (ring_buffer->size - ring_buffer->first);
		
		//@ assert array<int>(fields, bigtail_size(size, first, len), sizeof(int), integer, ?bigtail_elems);
		//@ assert array<int>(fields + first, bighead_size(size, first, len), _, _, ?bighead_elems);

		//@ open array<int>(ring_buffer->fields + bigtail_size(size, first, len), size - len, sizeof(int), integer, _); // open <s>happyness</s>emptyness.
		*(ring_buffer->fields+put_at) = element;

		//@ close array<int>(ring_buffer->fields+put_at, 1, sizeof(int), integer, cons(element, nil)); // array of size one
		//@ append_assoc(bighead_elems, bigtail_elems, cons(element, nil));
		
		/*@
		// Only need to merge if there is something to merge width, i.e. this is not the first element of bigtail.
		if (is_split_up_fp(size,first,len)){
			array_merge<int>(ring_buffer->fields);
		}
		@*/
		
	}else{
		//@ assert ! is_split_up_fp(size, first, len) == true;
		put_at = ring_buffer->first + ring_buffer->len;
		//@ open array<int>(fields + first + len, _, _, _, _); // open trailing emptyness
		*(ring_buffer->fields+put_at) = element;
		//@ close array<int>(ring_buffer->fields+put_at, 1, sizeof(int), integer, cons(element, nil)); // array of size one
		//@ array_merge<int>(ring_buffer->fields+first);
	}
	ring_buffer->len++;
	//@ close ring_buffer(ring_buffer, size, first, len+1, append(items, cons(element, nil)));
}

/*@
lemma void tail_of_singleton_is_nil<t>(list<t> l);
requires length(l) == 1;
ensures tail(l) == nil;
@*/

void ring_buffer_clear(struct ring_buffer *ring_buffer)
//@ requires ring_buffer(ring_buffer, ?size, ?first, ?len, ?elems);
//@ ensures ring_buffer(ring_buffer, size, first, 0, nil);
{
	//@ open ring_buffer(ring_buffer, size, first, len, elems);
	//@ array_merge(ring_buffer->fields);
	//@ array_merge(ring_buffer->fields);
	ring_buffer->len = 0;
	//@ array_split(ring_buffer->fields,first);
	//@ close ring_buffer(ring_buffer, size, first, 0, nil);
}


int ring_buffer_head(struct ring_buffer *ring_buffer)
//@ requires ring_buffer(ring_buffer, ?size, ?first, ?len, ?elems) &*& len > 0;
//@ ensures ring_buffer(ring_buffer, size, first, len, elems) &*& result == head(elems);
{
	//@ open ring_buffer(ring_buffer, size, first, len, elems);
	//@ open array(ring_buffer->fields + ring_buffer->first, _, _, _, _);
	return *(ring_buffer->fields + ring_buffer->first);
	//@ close ring_buffer(ring_buffer, size, first, len, elems);
	
}

// i.e. remove from beginning of queue
int ring_buffer_pop(struct ring_buffer *ring_buffer)
/*@ requires
	ring_buffer(ring_buffer, ?size, ?first, ?len, ?elems)
	&*& len > 0 // you can't pop nonexisting elements
	;
@*/
//@ ensures ring_buffer(ring_buffer, size, first == size - 1 ? 0 : first + 1, len-1, tail(elems)) &*& result == head(elems);
{
	//@ open ring_buffer(ring_buffer, _, _, _, _);
	int take_at = ring_buffer->first;
	int elem;
	//@ int  newfirst = first + 1 == size ? 0 : first + 1;
	//@ assert ring_buffer->fields |-> ?fields;
	
	//@ open array(ring_buffer->fields + first, _, _, _, ?elems_bighead);
	elem = *(ring_buffer->fields + take_at);
	ring_buffer->len = ring_buffer->len - 1;
	ring_buffer->first = ring_buffer->first + 1;
	if (ring_buffer->first == ring_buffer->size) ring_buffer->first = 0; // XXX hmm why can we also assign "first = ..." here if first is ghost??

	
	/*@
	if (bighead_size(size, first, len) == 1){
		assert newfirst == 0;
		tail_of_singleton_is_nil(elems_bighead);
		
		close array<int>(fields + take_at, 1, sizeof(int), integer, cons(elem, nil)); // array size one
		array_merge(fields + len-1);
	}else{
		close array<int>(ring_buffer->fields + take_at + 1, 0, sizeof(int), integer, nil);

		close array<int>(fields + take_at, 1, sizeof(int), integer, cons(elem, nil)); // array size one
		if (is_split_up_fp(size, first, len)){
			array_merge(ring_buffer->fields + bigtail_size(size, first, len));
			if ( ! is_split_up_fp(size, newfirst, len-1)){
				// convert to non-split up data structure
			
				// zero-size leading emptyness
				close array<int>(fields, 0, sizeof(int), integer, nil);
				assert bighead_size(size, first, len) == 1;
				//assert false;
			}
		}else{
			// Make trailing emptyness a bit larger
			assert array<int>(fields + first + len, size - first - len, sizeof(int), integer, ?trailing_emptyness_data);
		
			array_merge(ring_buffer->fields);
		}
	}
	@*/
	//@close ring_buffer(ring_buffer, size, newfirst, len-1,  tail(elems));
	
	return elem;
		
}

void ring_buffer_dispose(struct ring_buffer *ring_buffer)
//@ requires ring_buffer(ring_buffer, _, _, _, _);
//@ ensures true;
{
	//@ open ring_buffer(ring_buffer, _, _, _, _);
	//@ array_merge(ring_buffer->fields);
	//@ array_merge(ring_buffer->fields);
	//@ int_array_to_chars(ring_buffer->fields);
	free(ring_buffer->fields);
	free(ring_buffer);
}

void harness(int x, int y, int z)
//@ requires true;
//@ ensures true;
{
	struct ring_buffer *b = ring_buffer_create(2);
	if (b == 0) return;
	ring_buffer_push(b, x);
	ring_buffer_push(b, y);
	int h = ring_buffer_pop(b);
	//@ assert h == x;
	ring_buffer_push(b, z);
	h = ring_buffer_pop(b);
	//@ assert h == y;
	h = ring_buffer_pop(b);
	//@ assert h == z;
	ring_buffer_dispose(b);
	
}