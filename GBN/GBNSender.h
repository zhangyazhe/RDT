#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include "stdafx.h"
#define N 4//发送窗口大小
class GBNSender :public RdtSender
{
private:
	int base;//发送但还未确认的第一个序号
	int nextseqnum;//可用但还未发送的第一个序号
	int init_flag = 1;
	bool waitingstate;
	Packet packet_send[SeqMod];
public:
	bool getWaitingState();//由虚拟网络环境调用
	bool send(const Message& message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用

public:
	GBNSender();
	virtual ~GBNSender();
};

#endif

