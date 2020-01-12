#include "TCPReceiver.h"
#include "stdafx.h"
#include "Global.h"

TCPReceiver::TCPReceiver():expectSeqNum(0) {
	ackPacket.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	ackPacket.checksum = 0;
	ackPacket.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		ackPacket.payload[i] = '.';
	}
	ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
}
TCPReceiver::~TCPReceiver() {}

void TCPReceiver::receive(const Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectSeqNum == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);
		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		ackPacket.acknum = packet.seqnum; //确认序号等于收到的报文序号
		ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
		pUtils->printPacket("接收方发送确认报文", ackPacket);
		pns->sendToNetworkLayer(SENDER, ackPacket);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		this->expectSeqNum++;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			cout << "此时接收方期待的序号是" << this->expectSeqNum << endl;
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", ackPacket);
		pns->sendToNetworkLayer(SENDER, ackPacket);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}