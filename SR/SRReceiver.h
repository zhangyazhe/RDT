#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include "stdafx.h"
#define N 4//���մ��ڴ�С

class SRReceiver :public RdtReceiver
{
private:
	int base;//�ڴ����յ������
	int nextseqnum;//���ڵ�ĩ��
	Packet ackPacket;
	bool ReceiverBufferFlags[SeqMod];
	Packet ReceiverBuffer[SeqMod];
public:
	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����

public:
	SRReceiver();
	virtual ~SRReceiver();
};

#endif
