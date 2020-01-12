#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include "stdafx.h"
#define N 4//���ʹ��ڴ�С
class GBNSender :public RdtSender
{
private:
	int base;//���͵���δȷ�ϵĵ�һ�����
	int nextseqnum;//���õ���δ���͵ĵ�һ�����
	int init_flag = 1;
	bool waitingstate;
	Packet packet_send[SeqMod];
public:
	bool getWaitingState();//���������绷������
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	GBNSender();
	virtual ~GBNSender();
};

#endif

