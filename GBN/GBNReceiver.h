#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class GBNReceiver :public RdtReceiver
{
private:
	int expectSeqNum;//�ڴ����յ������
	Packet ackPacket;
public:
	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����

public:
	GBNReceiver();
	virtual ~GBNReceiver();
};

#endif
