#include "Frame.h"
int main(void)
{
	Frame myFrame;
    while(1)
    {
    	myFrame.alloc();
    	Buffer buffer = myFrame.getBuffer();
    }
}
