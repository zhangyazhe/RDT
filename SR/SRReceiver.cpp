#include "SRReceiver.h"
#include "stdafx.h"
#include "Global.h"

SRReceiver::SRReceiver() :base(0) {
	nextseqnum = base + N;
	ackPacket.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	ackPacket.checksum = 0;
	ackPacket.seqnum = -1;	//忽略该字段
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
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum == packet.checksum) {
		if (this->base == packet.seqnum) {
			cout << "接收方收到了序号为" << base << "的数据包" << endl;
			//取出Message，向上递交给应用层
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			//发送ACK
			ackPacket.acknum = packet.seqnum; //确认序号等于收到的报文序号
			ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
			cout << "接收方发送ACK[" << ackPacket.acknum << ']' << endl;
			pns->sendToNetworkLayer(SENDER, ackPacket);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
			ReceiverBufferFlags[base % SeqMod] = false;
			while (ReceiverBufferFlags[(++base) % SeqMod]) {
				//取出Message，向上递交给应用层
				Message msg;
				memcpy(msg.data, ReceiverBuffer[base % SeqMod].payload, sizeof(ReceiverBuffer[base % SeqMod].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				ReceiverBufferFlags[base % SeqMod] = false;
				ReceiverBufferFlags[++nextseqnum % SeqMod] = false;
			}
			nextseqnum++;
		}else if (packet.seqnum > base&& packet.seqnum < nextseqnum) {
			//需要缓存收到的报文
			ReceiverBufferFlags[packet.seqnum % SeqMod] = true;//标记已缓存
			ReceiverBuffer[packet.seqnum % SeqMod] = packet;//缓存
			cout << packet.seqnum << "号报文已被缓存" << endl;
			//发送ACK
			ackPacket.acknum = packet.seqnum; //确认序号等于收到的报文序号
			ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
			pUtils->printPacket("接收方发送确认报文", ackPacket);
			pns->sendToNetworkLayer(SENDER, ackPacket);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
		else if (packet.seqnum >= base - N && packet.seqnum <= base - 1) {
			ackPacket.acknum = packet.seqnum; //确认序号等于收到的报文序号
			ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
			pns->sendToNetworkLayer(SENDER, ackPacket);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
			cout << "接收方收到了不需要的过时报文" << packet.seqnum << ",但仍然发送ACK" << endl;
			pUtils->printPacket("接收方发送确认报文", ackPacket);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			cout << "此时接收方期待的序号是" << this->base << "~" << this->nextseqnum << "之间" << endl;
		}
	}else {
		pUtils->printPacket("接收方我收到了损坏的数据包", packet);
	}
}