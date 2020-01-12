#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include "stdafx.h"
#define N 4//接收窗口大小

class SRReceiver :public RdtReceiver
{
private:
	int base;//期待接收到的序号
	int nextseqnum;//窗口的末端
	Packet ackPacket;
	bool ReceiverBufferFlags[SeqMod];
	Packet ReceiverBuffer[SeqMod];
public:
	void receive(const Packet& packet);	//接收报文，将被NetworkService调用

public:
	SRReceiver();
	virtual ~SRReceiver();
};

#endif
