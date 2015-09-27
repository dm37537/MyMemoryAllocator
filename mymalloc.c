#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include "mymalloc.h"

struct Block
{
   int is_free;
   size_t size;
   struct Block *next;
};

static void *mem;
struct Block *block;

void my_malloc_init(size_t size)
{
   mem = malloc(size);
   block = mem;
   block->size = size;
   block->is_free = 1;
   block->next = NULL;
}

/*
 * slice current block into current using block and free block
 */
void slice(struct Block *cur, size_t size) {
   struct Block *free_b = (struct Block*)((char*)cur + size);
   free_b->size = (cur->size) - size;
   free_b->is_free = 1;  
   free_b->next = cur->next;
   cur->size = size;
   cur->is_free = 0;
   cur->next = free_b;
}

/*
 * FIFO: iteratively look for the first block that is big enough
 */
void *my_malloc(size_t size)
{
   struct Block *cur;
   void *result;

   cur = block;
   while ((cur->size < size || cur->is_free == 0) && (cur->next != NULL)) {
      cur = cur->next;
   }

   if (cur->size == size) {
      cur->is_free = 0;
      result = (void*) (cur);
      return result;
   } else if (cur->size > (size)) {
      slice(cur, size);
      result = (void*)(cur);
      return result;
   } else {
      printf("%s\n", "no memory");
      return NULL;
   }
}

/*
 * iterate all blocks and join those that are adjacently free
 */
void join() {
   struct Block *cur;
   cur = block;
   while(cur->next != NULL) {
      if (cur->is_free && cur->next->is_free) {
         cur->size += (cur->next->size);
         cur->next = cur->next->next;
      } else {
         cur = cur->next;
      }
      
   }
}

/*
 * free current and join
 */
void my_free(void *ptr)
{
   struct Block *cur = ptr;
   cur->is_free = 1;
   join();
}

static void draw_box(FILE *stream, int size, int empty, int last)
{
   int i;
   int pad = size / 2;

   fprintf(stream, "+---------------------+\n");

   if (!empty) fprintf(stream, "%c[%d;%dm", 0x1B, 7, 37);

   for (i=0; i<pad; i++)
   {
      fprintf(stream, "|                     |\n");
   }

   fprintf(stream, "|         %2d          |\n", size);

   for (i++; i<size; i++)
   {
      fprintf(stream, "|                     |\n");   
   }

   if (!empty) fprintf(stream, "%c[%dm", 0x1B, 0);

   if (last) fprintf(stream, "+---------------------+\n");
}

void my_dump_mem(FILE *stream)
{
   struct Block *cur;
   cur = block;
   while(cur->next != NULL) {
      draw_box(stream, (int)(cur->size/MiB), cur->is_free, 0);
      cur = cur->next;      
   }
   draw_box(stream, cur->size / MiB, cur->is_free, 1);

}

uint64_t my_address(void *ptr)
{
   return ((((uint64_t) ptr) - ((uint64_t) mem)) / MiB);
}
