/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/




//0x77 is the Head
//0xFF is the free
//0xAA is alocated
ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    assert(_n_frames <= FRAME_SIZE * 8);

    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;


    if(pool_head == NULL) {
        pool_head = this;
    }
    else {
        ContFramePool* temp = pool_head;
        while(temp->pool_next != NULL) {
            temp = temp->pool_next;
        }

        temp->pool_next = this;
    }

    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }



    //marking all bits as free in frame pool
    for(unsigned int i=0; i < _n_frames; i++) {
        bitmap[i] = 0xFF;
    }


    unsigned long cont_info;
    if (_info_frame_no == 0) {
        bitmap[0] = 0x0;
        nFreeFrames--;

        cont_info = 1;
    }
    else {
        cont_info = _info_frame_no;
    }

    for (int i = cont_info; i <= cont_info + n_info_frames - 1 ; i++) {
        bitmap[i] = 0xFF;
        nFreeFrames--;
    }

    Console::puts("Frame Pool initialized\n");
}




unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    if (nFreeFrames == 0 || nFreeFrames < _n_frames) {
        return 0;
    }
    // TODO: IMPLEMENTATION NEEEDED!

    unsigned int frame_number = base_frame_no;
    unsigned int frame_head = 0;
    unsigned int counter = 0;

    for (unsigned int i = 0; i < nframes - 1; i++) {
        if (bitmap[i] == 0xFF) {
            if (counter == 0) {
                frame_head = i;
            }
            counter++;
        }
        else {
            counter = 0;
        }

        if (counter == _n_frames) {
            frame_number += frame_head;
            bitmap[frame_head] = 0x77;
            nFreeFrames--;
            while(counter > 1) {
                frame_head++;
                bitmap[frame_head] = 0xAA;
                nFreeFrames--;
                counter--;
            }
            return(frame_number);
        }
    }

    //no space found for number of frames or no more free frames
    return 0;


    //assert(false);
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    int i ;
    for(i = _base_frame_no; i < _base_frame_no + _n_frames; i++){
        mark_inaccessible(i);
    }
}

void ContFramePool::mark_inaccessible(unsigned long _frame_no)
{
    // Let's first do a range check.
    assert ((_frame_no >= base_frame_no) && (_frame_no < base_frame_no + nframes));
    
    unsigned int bitmap_index = _frame_no - base_frame_no;
    bitmap[bitmap_index] = 0x80;
    nFreeFrames--;
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // TODO: IMPLEMENTATION NEEEDED!
    unsigned long baseFrame = 0;
    unsigned long numFrame = 0;
    unsigned long position;
    ContFramePool* temp = pool_head;

    while (temp != NULL) {
        baseFrame = temp->base_frame_no;
        numFrame = temp->nframes;

        if (_first_frame_no >= baseFrame && _first_frame_no <= (baseFrame + numFrame)) {
            unsigned char* tBitmap = temp->bitmap;
            unsigned long position = _first_frame_no - baseFrame;

            tBitmap[position] = 0xFF;
            position++;
            temp->nFreeFrames++;
            while (tBitmap[position] != 0xFF || tBitmap[position] != 0x77) {
                tBitmap[position] = 0xFF;
                position++;
                temp->nFreeFrames++;
            }
            break;
        }
        else {
            temp = temp->pool_next;
        }
    }

}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{

    unsigned long numberOfFrames = (FRAME_SIZE);
    return (_n_frames / numberOfFrames + (_n_frames % numberOfFrames > 0 ? 1 : 0));
    // TODO: IMPLEMENTATION NEEEDED!
}
