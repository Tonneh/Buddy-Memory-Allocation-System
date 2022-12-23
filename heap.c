/*
 * This is a C implementation of malloc( ) and free( ), based on the buddy
 * memory allocation algorithm. 
 */
#include <stdio.h> // printf

/*
 * The following global variables are used to simulate memory allocation
 * Cortex-M's SRAM space.
 */
// Heap
char array[0x8000];            // simulate SRAM: 0x2000.0000 - 0x2000.7FFF
int heap_top   = 0x20001000;   // the top of heap space
int heap_bot   = 0x20004FE0;   // the address of the last 32B in heap
int max_size   = 0x00004000;   // maximum allocation: 16KB = 2^14
int min_size   = 0x00000020;   // minimum allocation: 32B = 2^5

// Memory Control Block: 2^10B = 1KB space
int mcb_top    = 0x20006800;   // the top of MCB
int mcb_bot    = 0x20006BFE;   // the address of the last MCB entry
int mcb_ent_sz = 0x00000002;   // 2B per MCB entry
int mcb_total  = 512;          // # MCB entries: 2^9 = 512 entries

/*
 * Convert a Cortex SRAM address to the corresponding array index.
 * @param  sram_addr address of Cortex-M's SRAM space starting at 0x20000000.
 * @return array index.
 */
int m2a( int sram_addr ) {
	int index = sram_addr - 0x20000000;	// to get index we need to subtract sram address 
	return index;												// return the index 
}

/*
 * Reverse an array index back to the corresponding Cortex SRAM address.
 * @param  array index.
 * @return the corresponding Cortex-M's SRAM address in an integer.
 */ 
int a2m( int array_index ) {
	return array_index + 0x20000000;	// to get the address we need to add sram address to the index 
}

/*
 * In case if you want to print out, all array elements that correspond
 * to MCB: 0x2006800 - 0x20006C00.
 */
void printArray( ) {
	printf( "memroy ............................\n" );
	for ( int i = 0; i < 0x8000; i+=4 )																							// loop through until 0x8000, incrementing by 4 each iteration
		if ( a2m( i ) >= 0x20006800 ) 																								// checking if the sram address of index is greater or equal to 0x20006800 
			printf( "%x = %x(%d)\n", a2m( i ), *(int *)&array[i], *(int *)&array[i] ); 	// printing out the sram address+index, hex value of array[i], decimal value of array[i] 
}

/*
 * _ralloc is _kalloc's helper function that is recursively called to
 * allocate a requested space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  size  the size of a requested memory space
 * @param  left_mcb_addr  the address of the left boundary of MCB entries to examine
 * @param  right_mcb_addr the address of the right boundary of MCB entries to examine
 * @return the address of Cortex-M's SRAM space. While the computation is
 *         made in integers, cast it to (void *). The gcc compiler gives
 *         a warning sign:
                cast to 'void *' from smaller integer type 'int'
 *         Simply ignore it.
 */
void *_ralloc( int size, int left_mcb_addr, int right_mcb_addr ) {
	// initial parameter computation
	int entire_mcb_addr_space = right_mcb_addr - left_mcb_addr  + mcb_ent_sz; 						// initialize entire_mcb_addr_space by subtracting right by left then adding entry size 
	int half_mcb_addr_space = entire_mcb_addr_space / 2;																	// initialize half_mcb_addr_space by dividing entire_mcb_addr_space by 2 
	int midpoint_mcb_addr = left_mcb_addr + half_mcb_addr_space;													// initialize midpoint_mcb_addr by adding left_mcb_addr to half_mcb_addr_space
	int heap_addr = 0;																																		// initialize heap address to 0 
	int act_entire_heap_size = entire_mcb_addr_space * 16;																// initialize act_entire_heap_size by multiplying entire_mcb_addr_space by 16 
	int act_half_heap_size = half_mcb_addr_space * 16;																		// initialize act_half_heap_size by multiplying half_mcb_addr_space by 16

	// base case
	if ( size <= act_half_heap_size ) {																										// check if the size of the requested memory space is less than or equal to half of heap size
		void* heap_addr = _ralloc( size, left_mcb_addr, midpoint_mcb_addr - mcb_ent_sz );		// recursively call ralloc, but setting the right to midpoint subtracted by mcb entry size, then initializing _heap_addr to result of call
		if ( heap_addr == 0 ) {																															// check if the heap_addr is equal to 0
			return _ralloc( size, midpoint_mcb_addr, right_mcb_addr );												// recursively call ralloc, but setting the left to midpoint, then return the result 
		if ( ( array[ m2a( midpoint_mcb_addr ) ] & 0x01 ) == 0 )														// get the index of midpoint_mcb_addr and check if it's value's lsb is equal to 0
			*(short *)&array[ m2a( midpoint_mcb_addr ) ] = act_half_heap_size;								// get the index of midpoint_mcb_addr and set it's value to act_half_heap_size, 
		return heap_addr;																																		// returns heap address
	}
	else {
		if ( ( array[ m2a( left_mcb_addr ) ] & 0x01 ) != 0 ) {															// get the value of left_mcb_addr and check if value's lsb isn't 0 
			return 0;																																					// return 0  
		}
		else {
			if ( *(short *)&array[ m2a( left_mcb_addr ) ] < act_entire_heap_size )						// get the index of left_mcb_addr and check it it's value is less than act_entire_heap_size 
				return 0;																																				// return 0 
			*(short *)&array[ m2a( left_mcb_addr ) ] = act_entire_heap_size | 0x01;						// get array index of left_mcb_addr and set its value to result of act_entire_heap_size and set its lsb to 1
			return (void *)( heap_top + ( left_mcb_addr - mcb_top ) * 16 );										// return value calculated from subtracting left_mcb_addr by mcb_top and add heap_top, then multiply by 16
		}
	}
	return 0;
}

/*
 * _rfree is _kfree's helper function that is recursively called to
 * deallocate a space, using the buddy memory allocaiton algorithm.
 * Implement it by yourself in step 1.
 *
 * @param  mcb_addr that corresponds to a SRAM space to deallocate
 * @return the same as the mcb_addr argument in success, otherwise 0.
 */
int _rfree( int mcb_addr ) {
	short mcb_contents = *(short *)&array[ m2a( mcb_addr ) ];								// initialize mcb_contents with value from array
	int mcb_index = mcb_addr - mcb_top;																			// initialize mcb_index with result from mcb_adr subtracted by mcb_top 
	int mcb_disp = ( mcb_contents /= 16 );																	// initialize mcb_disp with result from mcb_content divided by 16, mcb_contents is also divided by 16 
	int my_size = ( mcb_contents *= 16 );																		// initialize my_size with mcb_contents multiplied by 16, mcb_content is also multiplied by 16

	// mcb_addr's used bit was cleared
	*(short *)&array[ m2a( mcb_addr ) ] = mcb_contents;											// get the mcb_addr and get it's array index then the value to 0 
	
	if ( ( mcb_index / mcb_disp ) % 2 == 0 ) {  														// check if result of mcb_index divided by mcb_disp, moduloed by 2 equals 0 
		if ( mcb_addr + mcb_disp >= mcb_bot )																	// check if mcb_addr plus mcb_disp is greater than or equal to mcb_bot
			return 0; // my buddy is beyond mcb_bot!														// return 0 
		else {																																				
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr + mcb_disp ) ];   // get index of mcb_addr + mcb_disp and set mcb_buddy to value of index in array
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {																// check if the mcb_buddy's lsb is equal to 0 
				mcb_buddy = ( mcb_buddy / 32 ) * 32;															// divide mcb_buddy by 32 then multiply by 32 for rounding 
				if ( mcb_buddy == my_size ) {																			// check if the mcb_buddy is equal to my_size 
					*(short *)&array[ m2a( mcb_addr + mcb_disp ) ] = 0;							// get the mcb_addr + mcb_disp and get it's array index then set the value to 0 
					my_size *= 2;																										// multiply the my_size by 2 
					*(short *)&array[ m2a( mcb_addr ) ] = my_size;									// get the index of mcb_addr and then set the value at array index to my_size 
					return _rfree( mcb_addr );																			// return result of recursive call, parameter value being mcb_addr
				}
			}
		}
	}
	else {  
		if ( mcb_addr - mcb_disp < mcb_top )																	// check if the result of mcb_addr subtracted by mcb_disp is less than mcb_top 
			return 0; // my buddy is below mcb_top!															// return 0 
		else {
			short mcb_buddy = *(short *)&array[ m2a( mcb_addr - mcb_disp ) ];   // get index from mcb_addr - mcb_disp and then get value at that index and set it to mcb_buddy 
			if ( ( mcb_buddy & 0x0001 ) == 0 ) {																// check if mcb_buddy lsb is equal to 1, then check if result of that is 0 
				mcb_buddy = ( mcb_buddy / 32 ) * 32;															// divide mcb_buddy by 32 then multiply by 32 for rounding 
				if ( mcb_buddy == my_size ) {																			// check if the mcb_buddy is equal to my_size 
					*(short *)&array[ m2a( mcb_addr ) ] = 0;												// get the mcb_addr and get it's array index then set the value to 0 
					my_size *= 2;																										// multiply the size by 2 
					*(short *)&array[ m2a( mcb_addr - mcb_disp ) ] = my_size;				// get the index of mcb_addr - mcb_disp and then set the value at array index to my_size 
					return _rfree( mcb_addr - mcb_disp );														// return result of recursive call, parameter value being result of mcb_addr subtracted by mcb_disp
				}
			}
		}
	}
  
  return mcb_addr;
}

/*
 * Initializes MCB entries. In step 2's assembly coding, this routine must
 * be called from Reset_Handler in startup_TM4C129.s before you invoke
 * driver.c's main( ).
 */
void _kinit( ) {
	for ( int i = 0x20001000; i < 0x20005000; i++ )					// loop from 0x20001000 to 0x2000500, incrementing by 4 bytes each iteration 
		array[ m2a( i ) ] = 0;																// set array[m2a(i)] to 0

		*(short *)&array[ m2a( mcb_top ) ] = max_size;				// set the value in array to max_size 

	for ( int i = 0x20006804; i < 0x20006C00; i += 2 ) {		// loop through 0x20006804 to 20006C00, incrementing by 8 bytes each iteration 
		array[ m2a( i ) ] = 0;																// set the array[m2a(i)]  to 0 
		array[ m2a( i + 1) ] = 0;															// set the array[m2a(i + 1)]  to 0 
	}
}

/*
 * Step 2 should call _kalloc from SVC_Handler.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_kalloc( int size ) {
  return _ralloc( size, mcb_top, mcb_bot );
}

/*
 * Step 2 should call _kfree from SVC_Handler.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_kfree( void *ptr ) {
	int addr = (int )ptr;																	// cast pointer to int, then set to addr

	if ( addr < heap_top || addr > heap_bot )							// check if the address is less than heap_top, or if the address is greater than heap_bot
		return NULL;																				// return null 
	int mcb_addr =  mcb_top + ( addr - heap_top ) / 16;		// initialize mcb_addr with mcb_top, added by a the result of addr subtracted by heap_top, divided by 16 

	if ( _rfree( mcb_addr ) == 0 )												// check if _rfree(mcb_addr) returns 0 
		return NULL;																				// return null
	else
		return ptr;																					// return pointer 
}

/*
 * _malloc should be implemented in stdlib.s in step 2.
 * _kalloc must be invoked through SVC in step 2.
 *
 * @param  the size of a requested memory space
 * @return a pointer to the allocated space
 */
void *_malloc( int size ) {
	static int init = 0;
	if ( init == 0 ) {
		init = 1;
		_kinit( );
	}
	return _kalloc( size );
}

/*
 * _free should be implemented in stdlib.s in step 2.
 * _kfree must be invoked through SVC in step 2.
 *
 * @param  a pointer to the memory space to be deallocated.
 * @return the address of this deallocated space.
 */
void *_free( void *ptr ) {
	return _kfree( ptr );
}
