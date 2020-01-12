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
		pUtils->printPacket("���ͷ��������ݰ�", this->packet_send[nextseqnum % SeqMod]);
		nextseqnum = nextseqnum + 1;
		if (nextseqnum == base + N) {
			this->waitingstate = true;
		}
		return true;
	}
	else {
		cout << "���ܷ���" << endl;
		return false;
	}
	
}

void SRSender::receive(const Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= base && ackPkt.acknum < nextseqnum) {
		this->SenderBufferFlags[ackPkt.acknum % SeqMod] = true;//��Ǹ÷������յ�ȷ��
		while (this->SenderBufferFlags[this->base % SeqMod]) {
			this->SenderBufferFlags[this->base % SeqMod] = false;
			this->base++;
		}
		if (base == nextseqnum) {
			this->waitingstate = false;
		}
		pns->stopTimer(SENDER, ackPkt.acknum);
		cout << "���ͷ��յ���ACK[" << ackPkt.acknum << "]" << endl;
	}
	else{
		if (checkSum != ackPkt.checksum) {
			cout << "���ͷ��յ���ACK��" << endl;
		}
		else {
			std::cout << "���ͷ�û����ȷ�յ�ȷ��,��Ų�����Ҫ�󣬼����ȴ�" << endl;
		}
	}
}

void SRSender::timeoutHandler(int seqnum) {
	cout << "��ʱ���ط����Ϊ" << seqnum << "�����ݰ�" << endl;
	pns->stopTimer(SENDER, seqnum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packet_send[seqnum % SeqMod]);
}