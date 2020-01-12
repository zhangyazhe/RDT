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
		//return false;//如果发送方在等待ACK，则停止发送数据
	//}                                                     
	//if (((base + N <= SeqMod) && (nextseqnum < base + N)) || (base + N > SeqMod) && (((nextseqnum < SeqMod) && (nextseqnum > base)) || ((nextseqnum > 0) && (nextseqnum < (base + N) % SeqMod)))) {
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
		//nextseqnum = (nextseqnum + 1) % SeqMod;
		nextseqnum = nextseqnum + 1;
		cout << "报文发送后，nextseqnum变为" << nextseqnum << endl;
		//if (nextseqnum == (base + N) % SeqMod) {
		if (nextseqnum == base + N) {
			//cout << "waitingState变为true，停止发送" << endl;
			this->waitingstate = true;
		}
	}
		return true;
}

void GBNSender::receive(const Packet& ackPkt) {
	//if (this->waitingstate == true) {//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
		if (checkSum == ackPkt.checksum && ackPkt.acknum >= base) {
			int base_old = base;
			//base = (ackPkt.acknum + 1) % SeqMod;
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
			//cout << "base变为" << base << endl;
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
	//}
}

void GBNSender::timeoutHandler(int seqnum) {
	cout << "发送方定时器时间到，回退N步" << endl;
	pns->stopTimer(SENDER, seqnum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);			//重新启动发送方定时器
	int i = base;
	do {
		cout << "重发" << i << "号报文" << endl;
		pUtils->printPacket("重发报文", this->packet_send[i % SeqMod]);
		pns->sendToNetworkLayer(RECEIVER, this->packet_send[i % SeqMod]);
		//i = (i + 1) % SeqMod;
		i++;
	} while (i != nextseqnum);
}