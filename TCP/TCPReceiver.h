#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class TCPReceiver :public RdtReceiver
{
private:
	int expectSeqNum;//�ڴ����յ������
	Packet ackPacket;
public:
	void receive(const Packet& packet);	//���ձ��ģ�����NetworkService����

public:
	TCPReceiver();
	virtual ~TCPReceiver();
};

#endif
