#include "GBNSender.h"
#include "stdafx.h"
#include "Global.h"

GBNSender::GBNSender() :base(0), nextseqnum(0), waitingstate(false) {}
GBNSender::~GBNSender(){}

bool GBNSender::getWaitingState() {
	return waitingstate;
}

bool GBNSender::send(const Message& message) {
	//std::cout << "waitingState = " << this->waitingstate << endl;
	//if (this -> waitingstate) {
		//return false;//������ͷ��ڵȴ�ACK����ֹͣ��������
	//}                                                     
	//if (((base + N <= SeqMod) && (nextseqnum < base + N)) || (base + N > SeqMod) && (((nextseqnum < SeqMod) && (nextseqnum > base)) || ((nextseqnum > 0) && (nextseqnum < (base + N) % SeqMod)))) {
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
		//nextseqnum = (nextseqnum + 1) % SeqMod;
		nextseqnum = nextseqnum + 1;
		cout << "���ķ��ͺ�nextseqnum��Ϊ" << nextseqnum << endl;
		//if (nextseqnum == (base + N) % SeqMod) {
		if (nextseqnum == base + N) {
			//cout << "waitingState��Ϊtrue��ֹͣ����" << endl;
			this->waitingstate = true;
		}
	}
		return true;
}

void GBNSender::receive(const Packet& ackPkt) {
	//if (this->waitingstate == true) {//������ͷ����ڵȴ�ack��״̬�������´�������ʲô������
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
		if (checkSum == ackPkt.checksum && ackPkt.acknum >= base) {
			int base_old = base;
			//base = (ackPkt.acknum + 1) % SeqMod;
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
			//cout << "base��Ϊ" << base << endl;
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
	//}
}

void GBNSender::timeoutHandler(int seqnum) {
	cout << "���ͷ���ʱ��ʱ�䵽������N��" << endl;
	pns->stopTimer(SENDER, seqnum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);			//�����������ͷ���ʱ��
	int i = base;
	do {
		cout << "�ط�" << i << "�ű���" << endl;
		pUtils->printPacket("�ط�����", this->packet_send[i % SeqMod]);
		pns->sendToNetworkLayer(RECEIVER, this->packet_send[i % SeqMod]);
		//i = (i + 1) % SeqMod;
		i++;
	} while (i != nextseqnum);
}