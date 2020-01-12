#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class TCPReceiver :public RdtReceiver
{
private:
	int expectSeqNum;//期待接收到的序号
	Packet ackPacket;
public:
	void receive(const Packet& packet);	//接收报文，将被NetworkService调用

public:
	TCPReceiver();
	virtual ~TCPReceiver();
};

#endif
