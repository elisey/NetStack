#include "MacFrame.h"
#include "Crc16.h"
MacFrame::MacFrame()	:	PoolNode(3)
{
}

uint8_t MacFrame::getPid()
{
	return buffer[0];
}
void MacFrame::setPid(uint8_t newPid)
{
	buffer[0] = newPid;
}

packetAckType_t MacFrame::getPacketAckType()
{
	return static_cast< packetAckType_t >(buffer[1]);
}
void MacFrame::setPacketAckType(packetAckType_t packetAckType)
{
	buffer[1] = static_cast< uint8_t >(packetAckType);
}

uint16_t MacFrame::getCrc()
{
	size_t length = buffer.getLenght();
	uint16_t crc =  (buffer[length - 2] << 8) | ( buffer[length - 1] );
	return crc;
}
void MacFrame::setCrc(uint16_t newCrc)
{
	size_t length = buffer.getLenght();
	buffer[length - 2] = (uint8_t)newCrc >> 8;
	buffer[length - 1] = (uint8_t)newCrc & 0xFF;
}
uint16_t MacFrame::calculateCrc()
{
	uint8_t *ptrBuffer = buffer.getDataPtr();
	size_t size = buffer.getLenght() - 2;

	uint8_t calculatedCrc;
	calculatedCrc = Crc16_Calculate(ptrBuffer, size);
	return calculatedCrc;
}
bool MacFrame::checkCrc()
{
	uint16_t crcFromPacket = getCrc();
	uint16_t calculatedCrc = calculateCrc();
	return (crcFromPacket == calculatedCrc);
}
