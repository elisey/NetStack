#pragma once
#include <stdint.h>
#include <stddef.h>
#include "debug.h"

class Buffer {
public:
	Buffer() : ptrData(NULL), lenght(0)
	{
	}
	uint8_t* operator&()
	{
		return ptrData;
	}
	uint8_t& operator[](int i)
	{
		return ptrData[i];
	}
	unsigned int getLenght()
	{
		return lenght;
	}
	void setLenght(unsigned int newValue)
	{
		lenght = newValue;
	}

	void setDataPtr(uint8_t *_ptrData)
	{
		ptrData = _ptrData;
	}
	uint8_t* getDataPtr()
	{
		return ptrData;
	}

private:
	uint8_t *ptrData;
	unsigned int lenght;
};
