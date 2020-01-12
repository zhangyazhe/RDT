#include "SRReceiver.h"
#include "stdafx.h"
#include "Global.h"

SRReceiver::SRReceiver() :base(0) {
	nextseqnum = base + N;
	ackPacket.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	ackPacket.checksum = 0;
	ackPacket.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		ackPacket.payload[i] = '.';
	}
	ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
	for (int i = 0; i < SeqMod; i++) {
		ReceiverBufferFlags[i] = false;
	}
}
SRReceiver::~SRReceiver() {}

void SRReceiver::receive(const Packet& packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum == packet.checksum) {
		if (this->base == packet.seqnum) {
			cout << "���շ��յ������Ϊ" << base << "�����ݰ�" << endl;
			//ȡ��Message�����ϵݽ���Ӧ�ò�
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			//����ACK
			ackPacket.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
			cout << "���շ�����ACK[" << ackPacket.acknum << ']' << endl;
			pns->sendToNetworkLayer(SENDER, ackPacket);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
			ReceiverBufferFlags[base % SeqMod] = false;
			while (ReceiverBufferFlags[(++base) % SeqMod]) {
				//ȡ��Message�����ϵݽ���Ӧ�ò�
				Message msg;
				memcpy(msg.data, ReceiverBuffer[base % SeqMod].payload, sizeof(ReceiverBuffer[base % SeqMod].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				ReceiverBufferFlags[base % SeqMod] = false;
				ReceiverBufferFlags[++nextseqnum % SeqMod] = false;
			}
			nextseqnum++;
		}else if (packet.seqnum > base&& packet.seqnum < nextseqnum) {
			//��Ҫ�����յ��ı���
			ReceiverBufferFlags[packet.seqnum % SeqMod] = true;//����ѻ���
			ReceiverBuffer[packet.seqnum % SeqMod] = packet;//����
			cout << packet.seqnum << "�ű����ѱ�����" << endl;
			//����ACK
			ackPacket.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
			pUtils->printPacket("���շ�����ȷ�ϱ���", ackPacket);
			pns->sendToNetworkLayer(SENDER, ackPacket);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else if (packet.seqnum >= base - N && packet.seqnum <= base - 1) {
			ackPacket.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
			pns->sendToNetworkLayer(SENDER, ackPacket);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
			cout << "���շ��յ��˲���Ҫ�Ĺ�ʱ����" << packet.seqnum << ",����Ȼ����ACK" << endl;
			pUtils->printPacket("���շ�����ȷ�ϱ���", ackPacket);
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			cout << "��ʱ���շ��ڴ��������" << this->base << "~" << this->nextseqnum << "֮��" << endl;
		}
	}else {
		pUtils->printPacket("���շ����յ����𻵵����ݰ�", packet);
	}
}