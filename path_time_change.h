#include<bits/stdc++.h>
using namespace std;
typedef long long ll;
const int maxn=2005;
const double car_length=8.5,car_width=3.5,T_safe=5;
double Dis_safe;
int n;
vector<pair<int,double> >E[maxn];
//l<=r+val
void Set(int l,int r,double val){//op0小于等于,op1大于等于
        E[r].push_back(make_pair(l,val));
}
double dis[maxn],dis_ans[maxn];
int vis[maxn];
queue<int>Q;

int k,l[maxn],Point_tot;
struct Point{
        double x,y,theta,t;//位置，方向，到达时间 
        int id;
        bool safe_dis(const Point&b)const{
                return (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)>Dis_safe*Dis_safe;
        }
}; 
struct Path{
        Path(){
                P.clear();
        }
        vector<Point>P;
        int last_cross;
};
vector<Path>loader;
vector<int>loader_key[maxn];
int value[maxn],value_vis[maxn];

int check(int T_check){//检查T_check时间内是否能完成所有任务 
        while(!Q.empty())Q.pop();
        int imp=0;//无解标记 
        int S=Point_tot+1,T=Point_tot+2;//额外起始时间点和终时间点，spfa从终时间点开始 
        n=Point_tot+2;
    for(int i=1;i<=n;i++)E[i].clear(),vis[i]=0,dis[i]=0x3f3f3f3f3f3f;
    for(int i=1;i<=k;i++){
            loader[i].last_cross=0;
    	Set(S,loader[i].P[0].id,-loader[i].P[0].t);
            Set(loader[i].P[l[i]-1].id,T,0);
                for(int j=0;j+1<l[i];j++){
                    Set(loader[i].P[j].id,loader[i].P[j+1].id,-(loader[i].P[j+1].t-loader[i].P[j].t));
//                    cout<<loader[i].P[j].id<<" "<<loader[i].P[j+1].id<<" "<<-(loader[i].P[j+1].t-loader[i].P[j].t)<<endl;
                }
        }
    Set(T,S,T_check);
    for(int i1=1;i1<=k;i1++)
            for(int i2=i1+1;i2<=k;i2++)
                    for(int j1=0;j1<l[i1];j1++)
                            for(int j2=0;j2<l[i2];j2++){
                                    if(!loader[i1].P[j1].safe_dis(loader[i2].P[j2])){
                                                Set(loader[i1].P[j1].id,loader[i2].P[j2].id,-T_safe);//设置交叉时限制 
						if(j2==0)return 0;
//						loader[i1].last_cross=max(loader[i1].last_cross,j1);
//						loader[i2].last_cross=max(loader[i2].last_cross,j2);//��¼���һ�ν���λ�� 
//						cout<<i1<<" "<<j1<<" "<<i2<<" "<<j2<<endl; 
                                        }
                                }
        dis[n]=T_check;
        Q.push(n);
        while(!Q.empty()&&!imp){
                int u=Q.front();Q.pop();
                if(++vis[u]>n){
                        imp=1;
                        break;
                }
                for(auto i :E[u]){
                        if(dis[u]+i.second<dis[i.first]){
                                dis[i.first]=dis[u]+i.second;
                                Q.push(i.first);
                        }
                }
        }
        if(imp)return 0;
        return 1;
//	for(int i=2;i<=n;i++){
//		printf(" %lld",dis[i]-dis[i-1]);
//	}
//	puts("");
}
void fSet(int l,int r,double val){
	E[l].push_back(make_pair(r,-val));
}
void fcheck(int T_check){//�Ż�ʱ�������check��������������ͣ�ɾimp��ɾST����set 
	while(!Q.empty())Q.pop();
	int S=Point_tot+1,T=Point_tot+2;//������ʼʱ������ʱ���
	n=Point_tot+2;
    for(int i=1;i<=n;i++)E[i].clear(),vis[i]=0,dis[i]=0;
    for(int i=1;i<=k;i++){
    	loader[i].last_cross=0;
    	fSet(S,loader[i].P[0].id,-loader[i].P[0].t);
    	fSet(loader[i].P[l[i]-1].id,T,0);
		for(int j=0;j+1<l[i];j++){
    		fSet(loader[i].P[j].id,loader[i].P[j+1].id,-(loader[i].P[j+1].t-loader[i].P[j].t));
//    		cout<<loader[i].P[j].id<<" "<<loader[i].P[j+1].id<<" "<<-(loader[i].P[j+1].t-loader[i].P[j].t)<<endl;
		}
	}
    //Set(T,S,T_check);
    for(int i1=1;i1<=k;i1++)
    	for(int i2=i1+1;i2<=k;i2++)
    		for(int j1=0;j1<l[i1];j1++)
    			for(int j2=0;j2<l[i2];j2++){
    				if(!loader[i1].P[j1].safe_dis(loader[i2].P[j2])){
						fSet(loader[i1].P[j1].id,loader[i2].P[j2].id,-T_safe);//���ý���ʱ���� 
						loader_key[i1].push_back(j1);
						loader_key[i2].push_back(j2);
//						loader[i1].last_cross=max(loader[i1].last_cross,j1);
//						loader[i2].last_cross=max(loader[i2].last_cross,j2);//��¼���һ�ν���λ�� 
						cout<<i1<<" "<<j1<<" "<<i2<<" "<<j2<<endl; 
					}
				}
	dis[S]=0;
	Q.push(S);
	while(!Q.empty()){
		int u=Q.front();Q.pop();
		for(auto i :E[u]){
			if(dis[u]+i.second>dis[i.first]){
				dis[i.first]=dis[u]+i.second;
				Q.push(i.first);
			}
		}
	}
	for(int i=1;i<=n;i++)dis_ans[i]=dis[i]; 
}
int t=0,base=1<<30,ans=base;
void dfs(int x){
        if(x==k+1){
                t=0,base=1<<30;
                while(base){
                //                cout<<t+base<<endl;
                        if(check(t+base)){
                                if(ans>t+base){
                                        ans=t+base;
                                        for(int i=1;i<=n;i++)dis_ans[i]=dis[i]; 
                                }
                        }
                        else t+=base;
                        base>>=1;
                }
                return;
        }
        for(value[x]=1;value[x]<=k;value[x]++)
                if(!value_vis[value[x]]){
                        value_vis[value[x]]=1;
                        dfs(x+1);
                        value_vis[value[x]]=0;
                }
}
int path_time_change_main(string id){ 
        freopen(( id +"\\data.txt").c_str(),"r",stdin);
        freopen(( id +"\\output.txt").c_str(),"w",stdout);
        scanf("%d",&k);//装载机数量，路径长度 
        Dis_safe=sqrt(car_length*car_length+car_width*car_width);
        loader.push_back((Path){});
        for(int i=1;i<=k;i++){
                scanf("%d",&l[i]);
                loader.push_back((Path){});
                for(int j=0;j<l[i];j++){
                        Point p;
                        scanf("%lf%lf%lf%lf",&p.x,&p.y,&p.theta,&p.t); 
                        p.id=++Point_tot;
                        loader[i].P.push_back(p);
//                        cout<<loader[i].P[j].t<<endl;
                }
        }
        dfs(1);
        if(ans==(1<<30))puts("IMPOSSIBLE");
        else{
                printf("%d\n",ans);
		fcheck(ans);
                int tot=0,tt=1;
//		cout<<loader[1].last_cross<<endl;
		//double fast_last_cross=0,last_fast_last_cross=0;
                for(int i=1;i<=n-2;i++){
//			if(loader[tt].last_cross<=tot){
//				if(tot==0)fast_last_cross+=loader[tt].P[0].t;
//				else fast_last_cross+=max((dis_ans[i]-dis_ans[i-1])-(loader[tt].P[tot].t-loader[tt].P[tot-1].t),0.0);
////				puts("okok");
//			}
			printf("%lf %lf %lf\n",loader[tt].P[tot].x,loader[tt].P[tot].y,dis_ans[i]);
//			printf("%lf %lf %lf %lf\n",loader[tt].P[tot].x,loader[tt].P[tot].y,dis_ans[i]-fast_last_cross,tot==0?0:((loader[tt].P[tot].t-loader[tt].P[tot-1].t)/(dis_ans[i]-fast_last_cross+last_fast_last_cross-dis_ans[i-1])));
//			last_fast_last_cross=fast_last_cross;
                        if(++tot==l[tt]){
                                puts("");
                                puts("");
                                tot=0;
//				fast_last_cross=last_fast_last_cross=0;
                                tt++;
                        }
                }        
        for(int i=1;i<=k;i++){
			sort(loader_key[i].begin(),loader_key[i].end());
			unique(loader_key[i].begin(),loader_key[i].end());
			for(auto j : loader_key[i])cout<<j<<" ";
			cout<<endl;	
		}	
	}
        freopen("CON", "r", stdin);
        freopen("CON", "w", stdout);
        std::cout << "\n结果已成功写入到 output.txt 文件中" << std::endl;
        return 0;
} 
