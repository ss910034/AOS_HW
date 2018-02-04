#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>
using namespace std;
vector <int> v;
int n=80000,nf,period=10000;
int in1[80000];
int p[70];
int i,j=0,k;
int pgfaultcnt=0,cost=0;
int rs,pick,rsmax;
void SetData()
{
    int q=0,cho=0;
    int a[7]= {0,10,20,30,40,50,60};
    srand(time(NULL));
    cout<<"Which input do you want? 1.Semi-random 2.Locality 3.Enter by yourself:";
    cin>>cho;
    if(cho==1)
    {
        while(j!=80000)
        {
            rs=(rand()%400)+1;
            pick=(rand()%5)+1;
            for(i=0; i<pick; i++)
            {
                in1[j]=rs;
                rs++,j++;
                if(j == 80000)
                    break;
            }
        }
    }
    else if(cho==2)
    {
        while(q!=80000)
        {
            rs=(rand()%400)+1;
            pick=(rand()%31)+20;
            rsmax=rs+pick;
            for(i=0; i<period; i++)
            {
                in1[q]=(rand()%(rsmax-rs+1)+rs);
                q++;
                if(q==80000)
                    break;
            }
        }
    }
    else if(cho==3)
    {
        cout<<"Enter no of reference string:";
        cin>>n;
        cout<<"Enter reference string : ";
        for(i=0;i<n;i++)
            cin>>in1[i];
    }
    printf("Length of page reference sequence : %d.\n",n);
    printf("Enter no of frames,(1)10(2)20(3)30(4)40(5)50(6)60 : ");
    cin>>nf;
    nf=a[nf];
}

void initialize()                   //初始page frame buffer
{
    pgfaultcnt=0;
    for(i=0; i<nf; i++)
        p[i]=9999;
}

int isHit(int data)                 //搜尋是否hit
{
    int hit=0;
    for(j=0; j<nf; j++)
    {
        if(p[j]==data)              //找尋frame裡是否有data
        {
            hit=1;                  //找到則將hit設為1
            break;
        }
    }
    return hit;
}

int getHitIndex(int data)           //找尋hit的index值
{
    int hitind;
    for(k=0; k<nf; k++)
    {
        if(p[k]==data)              //找到則回傳index
        {
            hitind=k;
            break;
        }
    }
    return hitind;
}

void dispPgFaultCnt()               //印出 page fault 狀況
{
    printf("\nTotal no of page faults:%d \n\n",pgfaultcnt);
}

void fifo(int in[])
{
    initialize();
    for(i=0; i<n; i++)
    {
        if(isHit(in[i])==0)
        {
            for(k=0; k<nf-1; k++)
                p[k]=p[k+1];
            p[k]=in[i];
            pgfaultcnt++;
        }
    }
    printf("cost : %d ",cost);
    dispPgFaultCnt();
}

void optimal(int in[])
{
    pgfaultcnt=0,cost=0;
    int x=0,fr[nf]= {0},t=0,nf2;
    for(x=0; x<n; x++)
    {
        int check=0;
        for(int o=0; o<v.size(); o++)
        {
            if(v.at(o)==in[x])
            {
                cost++;
                check=1;
                break;
            }
        }
        if(!check)
        {
            if(v.size() >= nf)
            {

                for(int o=0; o<v.size(); o++)
                    fr[o]=v.at(o);
                nf2=nf;
                for(int y=x+1; y<n; y++)
                {
                    for(int o=0; o<nf; o++)
                    {
                        if(fr[o]==in[y])
                        {
                            fr[o]=-99;
                            nf2--;
                            break;
                            cost++;
                        }
                    }
                    if(nf2==1)
                        break;
                }
                for(t=0; t<nf-1; t++)
                    if(fr[t]!=-99)
                        break;
                for(std::vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
                    if (*iter == fr[t])
                    {
                        v.erase(iter);
                        break;
                    }
            }
            v.push_back(in[x]);
            pgfaultcnt++;
        }
    }
    printf("cost : %d ",cost);
    dispPgFaultCnt();
}

void secondchance(int in[])
{
    int j;
    int usedbit[70];
    int victimptr=0;
    initialize();
    cost=0;
    for(i=0; i<nf;usedbit[i]=0,i++);
    for(i=0; i<n; i++)
    {
        if(isHit(in[i]))
        {
            int hitindex=getHitIndex(in[i]);
            if(usedbit[hitindex]==0)
                usedbit[hitindex]=1;
                cost++;
        }
        else
        {
            pgfaultcnt++;
            if(usedbit[victimptr]==1)
            {
                do
                {
                    usedbit[victimptr]=0;
                    victimptr++;
                    if(victimptr==nf)
                        victimptr=0;
                        cost++;
                }
                while(usedbit[victimptr]!=0);
            }
            if(usedbit[victimptr]==0)
            {
                p[victimptr]=in[i];
                usedbit[victimptr]=1;
                victimptr++;
                cost++;
            }
        }
        if(victimptr==nf)
            victimptr=0;
    }
    printf("cost : %d ",cost);
    dispPgFaultCnt();
}

void lrusec(int in[])
{
    cost=0;
    int j;
    int usedbit[60],tbit[60];
    int victimptr=0,check=0,id=0;
    initialize();
    for(i=0; i<nf; i++)
        usedbit[i]=0,tbit[i]=0;
    for(i=0; i<n; i++)
    {
        check=0;
        if(isHit(in[i]))                                    //檢查是否hit
        {
            int hitindex=getHitIndex(in[i]);
            if(usedbit[hitindex]==0)                        //設定是否reference
                usedbit[hitindex]=1;
            if(tbit[hitindex]!=0)
                tbit[hitindex]=0;
            for(j=0; j<nf; j++)
                if(p[j]!=9999)
                    if(j!=hitindex)
                        tbit[j]++;
            cost++;
        }
        else
        {
            pgfaultcnt++;
            if(usedbit[victimptr]==1)
            {
                for(j=0; j<nf; j++)
                    if(usedbit[j]!=1)
                        check++;
                do
                {
                    usedbit[victimptr]=0;
                    victimptr++;
                    if(victimptr==nf)
                        victimptr=0;
                        cost++;
                }
                while(usedbit[victimptr]!=0);
                if(!check)
                {
                    for(j=0; j<nf; j++)
                        if(tbit[j]>check)
                        {
                            check=tbit[j];
                            victimptr=j;
                        }
                }
            }
            if(usedbit[victimptr]==0)
            {
                for(j=0; j<nf; j++)
                    if(p[j]!=9999)
                        if(j!=victimptr)
                            tbit[j]++;
                            cost++;
                tbit[victimptr]=0;
                p[victimptr]=in[i];
                usedbit[victimptr]=1;
                cost++;
                victimptr++;
            }
        }
        if(victimptr==nf)
            victimptr=0;
    }
    printf("cost : %d ",cost);
    dispPgFaultCnt();                           //顯示pagefault數量
}

int main()
{

    SetData();
    int choice;
    while(1)
    {
        printf("Page Replacement Algorithms\n1.Reset data\n2.FIFO\n3.Optimal\n4.Second Chance\n5.Mine\n6.Exit\nEnter your choice:");
        cin>>choice;
        switch(choice)
        {
        case 1:
            SetData();
            break;
        case 2:
            fifo(in1);
            break;
        case 3:
            optimal(in1);
            break;
        case 4:
            secondchance(in1);
            break;
        case 5:
            lrusec(in1);
            break;
        default:
            return 0;
            break;
        }
    }
}
