#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include "stdafx.h"
#define N 4

class SRSender :public RdtSender
{
private:
	int base;
	int nextseqnum;
	bool waitingstate;
	Packet packet_send[SeqMod];
	bool SenderBufferFlags[SeqMod];
public:
	bool getWaitingState();
	bool send(const Message& message);	
	void receive(const Packet& ackPkt);						
	void timeoutHandler(int seqNum);					

public:
	SRSender();
	virtual ~SRSender();
};

#endif

