#include "TCPSender.h"
#include "stdafx.h"
#include "Global.h"

TCPSender::TCPSender() :base(0), nextseqnum(0), waitingstate(false) {}
TCPSender::~TCPSender(){}

bool TCPSender::getWaitingState() {
	return waitingstate;
}

bool TCPSender::send(const Message& message) {
	//初始化packet_send
	if (init_flag == 1) {
		for (int i = 0; i < SeqMod; i++) {
			this->packet_send[i].seqnum = -1;
		}
		init_flag = 0;
	}
	if (nextseqnum < base + N){
		this->packet_send[nextseqnum % SeqMod].acknum = -1;//忽略该字段
		this->packet_send[nextseqnum % SeqMod].seqnum = nextseqnum;
		this->packet_send[nextseqnum % SeqMod].checksum = 0;
		memcpy(this->packet_send[nextseqnum % SeqMod].payload, message.data, sizeof(message.data));
		this->packet_send[nextseqnum % SeqMod].checksum = pUtils->calculateCheckSum(this->packet_send[nextseqnum % SeqMod]);
		pUtils->printPacket("发送方发送报文", this->packet_send[nextseqnum % SeqMod]);
		if (base == nextseqnum) {
			cout << "发送方启动计时器" << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		}
		pns->sendToNetworkLayer(RECEIVER, this->packet_send[nextseqnum % SeqMod]);
		nextseqnum = nextseqnum + 1;
		cout << "报文发送后，nextseqnum变为" << nextseqnum << endl;
		if (nextseqnum == base + N) {
			this->waitingstate = true;
		}
	}
		return true;
}

void TCPSender::receive(const Packet& ackPkt) {
	//检测冗余ACK
	if (ackPkt.acknum == lastack) {
		ACK_count++;
		if (ACK_count == 4) {
			cout << "收到了三个冗余ACK，快速重传" << endl;
			pns->stopTimer(SENDER, ackPkt.acknum+1);										//首先关闭定时器
			pns->startTimer(SENDER, Configuration::TIME_OUT, ackPkt.acknum + 1);			//重新启动发送方定时器
			pns->sendToNetworkLayer(RECEIVER, this->packet_send[base % SeqMod]);
		}
	}
	else {
		lastack = ackPkt.acknum;
		ACK_count = 1;
	}
	//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
		if (checkSum == ackPkt.checksum && ackPkt.acknum >= base) {
			int base_old = base;
			base = ackPkt.acknum + 1;
			//打印窗口信息
			cout << "发送方滑动窗口内容为 " << '[' << ' ';
			for (int i = base; i < base + N; i++) {
				if (packet_send[i % SeqMod].seqnum == -1) {
					cout << '*' << ' ';
				}
				else {
					cout << packet_send[i % SeqMod].seqnum << ' ';
				}
			}
			cout << ']' << endl;
			pUtils->printPacket("发送方正确收到确认", ackPkt);
			if (base == nextseqnum) {
				cout << "base等于nextseqnum，停止计时器，重新开始发送" << endl;
				pns->stopTimer(SENDER, base_old);		//关闭定时器
				this->waitingstate = false;
			}
			else {
				pns->stopTimer(SENDER, base_old);		//关闭定时器
				pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			}
		}
		else {
			if (checkSum != ackPkt.checksum) {
				std::cout << "发送方收到的ACK损坏" << endl;
			}else
				std::cout << "发送方没有正确收到确认,序号不满足要求，继续等待" << endl;
		}
}

void TCPSender::timeoutHandler(int seqnum) {
	cout << "发送方定时器时间到，超时重传" << endl;
	pns->stopTimer(SENDER, seqnum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);			//重新启动发送方定时器
	int i = base;
	cout << "重发" << i << "号报文" << endl;
	pUtils->printPacket("重发报文", this->packet_send[i % SeqMod]);
	pns->sendToNetworkLayer(RECEIVER, this->packet_send[i % SeqMod]);
}