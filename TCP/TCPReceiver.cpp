#include "TCPReceiver.h"
#include "stdafx.h"
#include "Global.h"

TCPReceiver::TCPReceiver():expectSeqNum(0) {
	ackPacket.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	ackPacket.checksum = 0;
	ackPacket.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		ackPacket.payload[i] = '.';
	}
	ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
}
TCPReceiver::~TCPReceiver() {}

void TCPReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum && this->expectSeqNum == packet.seqnum) {
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		//ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		ackPacket.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
		pUtils->printPacket("���շ�����ȷ�ϱ���", ackPacket);
		pns->sendToNetworkLayer(SENDER, ackPacket);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		this->expectSeqNum++;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			cout << "��ʱ���շ��ڴ��������" << this->expectSeqNum << endl;
		}
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", ackPacket);
		pns->sendToNetworkLayer(SENDER, ackPacket);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���

	}
}