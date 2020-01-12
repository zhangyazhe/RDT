#include "SRSender.h"
#include "stdafx.h"
#include "Global.h"

SRSender::SRSender() :base(0), nextseqnum(0), waitingstate(false) {
	for (int i = 0; i < SeqMod; i++) {
		SenderBufferFlags[i] = false;
	}
}
SRSender::~SRSender() {}

bool SRSender::getWaitingState() {
	return waitingstate;
}

bool SRSender::send(const Message& message) {
	if (nextseqnum < base + N) {
		this->packet_send[nextseqnum % SeqMod].acknum = -1;
		this->packet_send[nextseqnum % SeqMod].seqnum = nextseqnum;
		this->packet_send[nextseqnum % SeqMod].checksum = 0;
		memcpy(this->packet_send[nextseqnum % SeqMod].payload, message.data, sizeof(message.data));
		this->packet_send[nextseqnum % SeqMod].checksum = pUtils->calculateCheckSum(this->packet_send[nextseqnum % SeqMod]);
		this->SenderBufferFlags[nextseqnum % SeqMod] = false;
		pns->startTimer(SENDER, Configuration::TIME_OUT, nextseqnum);
		pns->sendToNetworkLayer(RECEIVER, this->packet_send[nextseqnum % SeqMod]);
		pUtils->printPacket("发送方发送数据包", this->packet_send[nextseqnum % SeqMod]);
		nextseqnum = nextseqnum + 1;
		if (nextseqnum == base + N) {
			this->waitingstate = true;
		}
		return true;
	}
	else {
		cout << "不能发送" << endl;
		return false;
	}
	
}

void SRSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= base && ackPkt.acknum < nextseqnum) {
		this->SenderBufferFlags[ackPkt.acknum % SeqMod] = true;//标记该分组已收到确认
		while (this->SenderBufferFlags[this->base % SeqMod]) {
			this->SenderBufferFlags[this->base % SeqMod] = false;
			this->base++;
		}
		if (base == nextseqnum) {
			this->waitingstate = false;
		}
		pns->stopTimer(SENDER, ackPkt.acknum);
		cout << "发送方收到了ACK[" << ackPkt.acknum << "]" << endl;
	}
	else{
		if (checkSum != ackPkt.checksum) {
			cout << "发送方收到的ACK损坏" << endl;
		}
		else {
			std::cout << "发送方没有正确收到确认,序号不满足要求，继续等待" << endl;
		}
	}
}

void SRSender::timeoutHandler(int seqnum) {
	cout << "超时，重发序号为" << seqnum << "的数据包" << endl;
	pns->stopTimer(SENDER, seqnum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->packet_send[seqnum % SeqMod]);
}