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


void slice(struct Block *cur, size_t size) {
   struct Block *free_b = (struct Block*)((char*)cur + size + sizeof(struct Block));
   free_b->size = (cur->size) - size - sizeof(struct Block);
   free_b->is_free = 1;  
   free_b->next = cur->next;
   cur->size = size;
   cur->is_free = 0;
   cur->next = free_b;
}

void *my_malloc(size_t size)
{
   struct Block *cur;
   void *result;

   cur = block;
   while ((cur->size < size || cur->is_free == 0) && (cur->next != NULL)) {
      cur = cur->next;
      printf("Address of cur is %" PRIu64 "\n",my_address(cur));
   }

   if (cur->size == size) {
      cur->is_free = 0;
      result = (void*) (++cur);
      printf("%s\n", "fit space");
      return result;
   } else if (cur->size > (size + sizeof(struct Block))) {
      slice(cur, size);
      result = (void*)(++cur);
      printf("%s\n", "sliced");
      return result;
   } else {
      printf("%s\n", "no memory");
      return NULL;
   }
}


void join() {
   struct Block *cur;
   cur = block;
   while(cur->next != NULL) {
      if (cur->is_free && cur->next->is_free) {
         printf("Address of cur is %" PRIu64 "\n",my_address(cur));
         cur->size += (cur->next->size) + sizeof(struct Block);
         cur->next = cur->next->next;
      }
      cur = cur->next;
   }
}

void my_free(void *ptr)
{
   struct Block *cur = ptr;
   --cur;
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
   /* TODO: implement */
   struct Block *cur;
   cur = block;
   while(cur->next != NULL) {
      //printf("Address is %" PRIu64 "\n", my_address(cur));
      draw_box(stream, (int)(cur->size/MiB), cur->is_free, 0);
      cur = cur->next;      
   }
   draw_box(stream, cur->size / MiB + 1, cur->is_free, 1);

   /* the following is an example */
   /* of a heap with four */
   /* blocks of sizes  */
   /* 1, 4, 1, and 5  */
   /* and the third block unallocated */
   /* (empty) */

}

uint64_t my_address(void *ptr)
{
   return ((((uint64_t) ptr) - ((uint64_t) mem)) / MiB);
}
