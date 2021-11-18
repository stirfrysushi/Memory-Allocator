#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h> 

bool validator(block *block) {
	block *ptr; 
	if(block -> head == NULL) {
		puts("NOOOOO"); 
		return -1; 
	}
	ptr = block -> head; 
	if(ptr != NULL) {
		while(ptr -> next != NULL) {
			if(ptr -> block_header == 0) {
				puts("NOOO");
				return -1; 
			}
			ptr = ptr -> next; 
		}
	}
	puts("passed"); 
	return 0; 
}

