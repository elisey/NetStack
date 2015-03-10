#pragma once
#include <stdint.h>

#define UniqueItemHandlerNUM_OF_ITEMS		5

template <typename T>
class UniqueItemHandler
{
public:
	UniqueItemHandler()	: oldestItemIndex(0), isInitialized(false)
	{
	}

	bool isFrameUnique(T &item)
	{
		if (isInitialized == false)	{
			return true;
		}

		unsigned int i;
		for (i = 0; i < UniqueItemHandlerNUM_OF_ITEMS; ++i) {
			if (item == items[i])	{
				return false;
			}
		}
		return true;
	}

	void putNewFrame(T &item)
	{
		// При первой инициализации содержимое буфера неопределено. Поэтому при первой
		// любого элемента заполняем этим элементом весь буфер. Пока буфер пустой, любой запрос
		// на уникльность будет возвращать правду.
		if (isInitialized == false)	{
			isInitialized = true;

			unsigned int i;
			for (i = 0; i < UniqueItemHandlerNUM_OF_ITEMS; ++i) {
				items[i] = item;
			}
		}
		else	{
			items[oldestItemIndex] = item;

			oldestItemIndex++;
			if (oldestItemIndex >= UniqueItemHandlerNUM_OF_ITEMS){
				oldestItemIndex = 0;
			}
		}
	}

private:
	T items[UniqueItemHandlerNUM_OF_ITEMS];
	uint8_t oldestItemIndex;
	bool isInitialized;
};
