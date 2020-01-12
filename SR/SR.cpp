#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "SRSender.h"
#include "SRReceiver.h"

int main()
{
	RdtSender* ps = new SRSender();
	RdtReceiver* pr = new SRReceiver();
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile(" ");//添加输入文件路径
	pns->setOutputFile(" ");//添加输出文件路径

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete

	return 0;
}