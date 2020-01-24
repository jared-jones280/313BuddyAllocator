#include "BuddyAllocator.h"
#include <iostream>
#include <cmath>

using namespace std;

BuddyAllocator::BuddyAllocator(int _basic_block_size, int _total_memory_length) {
    basic_block_size = _basic_block_size;
    total_memory_size = _total_memory_length;
    int freemem = total_memory_size;
    char *memory = new char[total_memory_size];
    memory_block = memory;
    int fbs = basic_block_size; //first block size
    while(fbs<=total_memory_size){
        fbs*=2;
    }
    fbs/=2;//fbs is max size it can be and fit in mem
    FreeList = vector<LinkedList>(log2(fbs)-log2(basic_block_size)+1,LinkedList());
    auto *header = (BlockHeader *) (memory);
    while (fbs>=basic_block_size) {
        header->next = nullptr;
        header->block_size=fbs;
       FreeList[log2(fbs)-log2(basic_block_size)].insert(header);
       header = (BlockHeader*)(long(header) + fbs);
       freemem -= fbs;
       while(fbs>freemem){
           fbs/=2;
       }
    }

}

BlockHeader *BuddyAllocator::split(BlockHeader *block) {//make recursive? add final size in pass in variable and untill it get that size keep splitting?
    FreeList[log2(block->block_size)-log2(basic_block_size)].remove(block);
    int block_size = block->block_size;
    int half = block_size/2;
    block->block_size=half;
    BlockHeader* second_block = getbuddy(block);
    second_block->block_size = half;
    FreeList[log2(half)-log2(basic_block_size)].insert(block);
    return second_block;
}

BuddyAllocator::~BuddyAllocator() {
    delete[] memory_block;
}

char *BuddyAllocator::alloc(int _length) {
    /* This preliminary implementation simply hands the call over the
       the C standard library!
       Of course this needs to be replaced by your implementation.
    */
    //take length and add block header size
    _length += sizeof(BlockHeader);
    //find next largest power of 2  (smallest power of 2 that is greater than len + head)
    bool notFound = true;
    int bs = basic_block_size;
    while(notFound){
        if(bs > _length){
            notFound = false;
        }
        else{
            bs *= 2;
        }
    }
    //check if block is in free list
    int index = log2(bs)-log2(basic_block_size);
    int lastIndex = FreeList.size()-1;
    if(index > lastIndex){
        return nullptr;
    }
    if(FreeList[index].head != nullptr){
        BlockHeader* mem = FreeList[index].head;
        FreeList[index].remove(FreeList[index].head);
        mem += 1;
        return (char*)(mem);
    }
    //if yes, return block + blockheader (see above)
    //if no, find next largest block in free list
    notFound = true;
    int i  = index;
    while(notFound){
        if((i == lastIndex) && (FreeList[i].head == nullptr)){
            //unable to allocate because nothing large enough in free list
            return nullptr;
        }
        if(FreeList[i].head == nullptr){
            i++;
        }
        else{
            notFound = false;
        }
    }
    //split that block (block is index i on freelist)
    bool isCorrectSize = false;
    BlockHeader *bh = split(FreeList[i].head);
    while(!isCorrectSize) {
        //check if block is correct size
        if (bh->block_size == bs) {
            isCorrectSize = true;
        }
        else{
            bh = split(bh);
        }
    }
    //if not keep splitting
    bh += 1;
    return (char*)(bh);
    //when block is finally right size return block - blockheader
}

int BuddyAllocator::free(char *_a) { //need to account for if user passes bad memory later
    /* Same here! */
    //delete _a;
    //return 0;
    //subtract the header size so pointer points to header
    _a -= sizeof(BlockHeader);
    //cast char* to blockheader*
    BlockHeader* mem = (BlockHeader*)(_a);
    //get size of returned block
    int size = mem->block_size;
    // use getbuddy to find address of buddy
    BlockHeader* potentailBuddy = getbuddy(mem);
    //check through freelist in the correct size to see if buddy address is there
    int i = log2(size)-log2(basic_block_size);
    BlockHeader* curr = FreeList[i].head;
    if(curr == nullptr){
        FreeList[log2(mem->block_size)-log2(basic_block_size)].insert(mem);
        return 0;
    }
    else {
        while (curr != nullptr) {
            if (curr == potentailBuddy) {
                FreeList[log2(curr->block_size)-log2(basic_block_size)].remove(curr);
                mem = merge(mem, curr);
                //offset
                mem += 1;
                //recursive
                int a = free((char*)(mem));
                //return
                return a;
            } else {
                curr = curr->next;
            }
        }
        FreeList[log2(mem->block_size)-log2(basic_block_size)].insert(mem);
        return 0;
    }
    //if buddy address is there merge them , and remove buddy from free list
    //see if its buddy is there, if so repeat
    //if buddy is not there then just add to free list
}

BlockHeader *BuddyAllocator::merge(BlockHeader *block1, BlockHeader *block2) {
    if(long(block1)>long(block2)){//if block 2 comes first in memory
        block2->block_size *=2;
        return block2;
    }
    else{
        block1->block_size *=2;
        return block1;
    }
}

BlockHeader *BuddyAllocator::getbuddy(BlockHeader *addr) {//doublecheck this  -------  (address-start) ^ bs = buddy addr + start finds buddy address
    BlockHeader* address = (BlockHeader*)(((long(addr) - long(memory_block))^(addr->block_size)) + long(memory_block));
    return address;
}

bool BuddyAllocator::arebuddies(BlockHeader *block1, BlockHeader *block2) {
    return (block1->block_size == block2->block_size) &&
           ((block1 + (block2->block_size) == block2) || (block2 + (block1->block_size) == block1));
}

void BuddyAllocator::printlist() {
    cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
    for (int i = 0; i < FreeList.size(); i++) {
        cout << "[" << i << "] (" << ((1 << i) * basic_block_size)
             << ") : ";  // block size at index should always be 2^i * bbs
        int count = 0;
        BlockHeader *b = FreeList[i].head;
        // go through the list from head to tail and count
        while (b) {
            count++;
            // block size at index should always be 2^i * bbs
            // checking to make sure that the block is not out of place
            if (b->block_size != (1 << i) * basic_block_size) {
                cerr << "ERROR:: Block is in a wrong list" << endl;
                exit(-1);
            }
            b = b->next;
        }
        cout << count << endl;
    }
}



