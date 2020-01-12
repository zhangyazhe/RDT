#include "TCPSender.h"
#include "stdafx.h"
#include "Global.h"

TCPSender::TCPSender() :base(0), nextseqnum(0), waitingstate(false) {}
TCPSender::~TCPSender(){}

bool TCPSender::getWaitingState() {
	return waitingstate;
}

bool TCPSender::send(const Message& message) {
	//��ʼ��packet_send
	if (init_flag == 1) {
		for (int i = 0; i < SeqMod; i++) {
			this->packet_send[i].seqnum = -1;
		}
		init_flag = 0;
	}
	if (nextseqnum < base + N){
		this->packet_send[nextseqnum % SeqMod].acknum = -1;//���Ը��ֶ�
		this->packet_send[nextseqnum % SeqMod].seqnum = nextseqnum;
		this->packet_send[nextseqnum % SeqMod].checksum = 0;
		memcpy(this->packet_send[nextseqnum % SeqMod].payload, message.data, sizeof(message.data));
		this->packet_send[nextseqnum % SeqMod].checksum = pUtils->calculateCheckSum(this->packet_send[nextseqnum % SeqMod]);
		pUtils->printPacket("���ͷ����ͱ���", this->packet_send[nextseqnum % SeqMod]);
		if (base == nextseqnum) {
			cout << "���ͷ�������ʱ��" << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		}
		pns->sendToNetworkLayer(RECEIVER, this->packet_send[nextseqnum % SeqMod]);
		nextseqnum = nextseqnum + 1;
		cout << "���ķ��ͺ�nextseqnum��Ϊ" << nextseqnum << endl;
		if (nextseqnum == base + N) {
			this->waitingstate = true;
		}
	}
		return true;
}

void TCPSender::receive(const Packet& ackPkt) {
	//�������ACK
	if (ackPkt.acknum == lastack) {
		ACK_count++;
		if (ACK_count == 4) {
			cout << "�յ�����������ACK�������ش�" << endl;
			pns->stopTimer(SENDER, ackPkt.acknum+1);										//���ȹرն�ʱ��
			pns->startTimer(SENDER, Configuration::TIME_OUT, ackPkt.acknum + 1);			//�����������ͷ���ʱ��
			pns->sendToNetworkLayer(RECEIVER, this->packet_send[base % SeqMod]);
		}
	}
	else {
		lastack = ackPkt.acknum;
		ACK_count = 1;
	}
	//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
		if (checkSum == ackPkt.checksum && ackPkt.acknum >= base) {
			int base_old = base;
			base = ackPkt.acknum + 1;
			//��ӡ������Ϣ
			cout << "���ͷ�������������Ϊ " << '[' << ' ';
			for (int i = base; i < base + N; i++) {
				if (packet_send[i % SeqMod].seqnum == -1) {
					cout << '*' << ' ';
				}
				else {
					cout << packet_send[i % SeqMod].seqnum << ' ';
				}
			}
			cout << ']' << endl;
			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
			if (base == nextseqnum) {
				cout << "base����nextseqnum��ֹͣ��ʱ�������¿�ʼ����" << endl;
				pns->stopTimer(SENDER, base_old);		//�رն�ʱ��
				this->waitingstate = false;
			}
			else {
				pns->stopTimer(SENDER, base_old);		//�رն�ʱ��
				pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			}
		}
		else {
			if (checkSum != ackPkt.checksum) {
				std::cout << "���ͷ��յ���ACK��" << endl;
			}else
				std::cout << "���ͷ�û����ȷ�յ�ȷ��,��Ų�����Ҫ�󣬼����ȴ�" << endl;
		}
}

void TCPSender::timeoutHandler(int seqnum) {
	cout << "���ͷ���ʱ��ʱ�䵽����ʱ�ش�" << endl;
	pns->stopTimer(SENDER, seqnum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);			//�����������ͷ���ʱ��
	int i = base;
	cout << "�ط�" << i << "�ű���" << endl;
	pUtils->printPacket("�ط�����", this->packet_send[i % SeqMod]);
	pns->sendToNetworkLayer(RECEIVER, this->packet_send[i % SeqMod]);
}