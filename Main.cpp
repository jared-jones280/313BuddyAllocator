#include <getopt.h>
#include "Ackerman.h"
#include "BuddyAllocator.h"

void easytest(BuddyAllocator* ba){
  // be creative here
  // know what to expect after every allocation/deallocation cycle
  cout<<"Block Header is "<<sizeof(BlockHeader)<<" Bytes.\n";
  // here are a few examples
  ba->printlist();
  // allocating a byte
  char * mem = ba->alloc (1);
  // now print again, how should the list look now
  ba->printlist ();

  ba->free (mem); // give back the memory you just allocated
  ba->printlist(); // shouldn't the list now look like as in the beginning

}

int main(int argc, char ** argv) {

  int basic_block_size = 128, memory_length = 512 * 1024;
  int opt;

  while((opt = getopt(argc, argv, "b:s:")) != -1){
      switch(opt){
          case 'b':
              basic_block_size = atoi(optarg);
              break;
          case 's':
              memory_length = atoi(optarg);
              break;
      }
  }
  cout<<"Basic Block Size is "<<basic_block_size<<" bytes."<<endl;
  cout<<"Memory Length is "<<memory_length<<" bytes."<<endl;
  //need to ensure that BBS is a power of 2!
  int i = 2;
  while(i<basic_block_size){
      i *= 2;
  }
  if(i!=basic_block_size){
      cerr<<"The Basic Block Size MUST be a Power of 2."<<endl;
      exit(-1);
  }
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);

  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  easytest (allocator);

  
  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman* am = new Ackerman ();
  am->test(allocator); // this is the full-fledged test. 
  
  // destroy memory manager
  delete allocator;
}
