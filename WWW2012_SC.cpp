#include "author.h"
#include "csv.h"
#include "edges.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <ctime>
#include <sstream>
using namespace std;

class skill
{
public:
	skill(){};
	vector<string> holders;
	void push_back(string s){holders.push_back(s);}
	void setFreq(double d){frequency = d;}
	bool isCover;// false;
	double getFreq(){return frequency;}
	string getName(){return name;}
	void setName(string n){name = n;}
	string i_prem;	//d(i*,s(a)) = d(i*,i')
	//vector<string>::size_type index;
	skill(string name, double freq):name(name),frequency(freq){holders.clear();isCover=false;};
private:
	string name;
	
	double frequency;
};

class author_skill : public author
{
public:
	author_skill(string name = NULL):author(name){};
	vector<skill*> skill_set;
};

class shortest
{
public:
	vector<string> road;
	double weight;
	string owner;
	shortest(double weight=100,string name=NULL):weight(weight),owner(name){};
};
void getskills(const float high,const float low ,vector<skill*>& collection);
void getIndex(map<string,vector<skill*>::size_type>& index);

template <class T> 
string ConvertToString(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

int exchange(edges *a,edges *b)
{
	edges t;
	t = *a;
	*a = *b;
	*b = t;
	return 0;
}

int partition(edges*edge,int p,int r)
{
  int i = p-1,j = p;
  
  for(;j<r;j++)
  {
	  if(edge[j].getWeight() <= edge[r].getWeight())
   {
    i++;
    exchange(edge+i,edge+j);
   }
  }
  exchange(&edge[i+1],&edge[r]);
  return i+1;
}
int quick_sort(edges edge[],int p,int r)
{
 if(p < r)
 {
  int q = partition(edge,p,r);
  quick_sort(edge,p,q-1);
  quick_sort(edge,q+1,r);
 }
 return 0;
}

void MST_Kruskal(edges edge[],map<string,double>& nodeSet,set<edges>& eSet,int n);
void findGroupForGivenLambda(const map<string,skill*>& TASK,double lambda,vector<string>&Q_apx,vector<string>&T_apx);
double getWorkLoad(int& max_workload,int* workload);
double getWorkLoad(int& max_workload,int*data,bool isPrint);
int main()
{
	const float FREQ_THRESHOLD_HIGH = 0.05;
	const float FREQ_THRESHOLD_LOW = 0.001;
	const unsigned int NUM_OF_TASK = 100;
	//{2,4,6,8,10,12,14,16,18,20};
	int num[] = {2,4,6,8,10,12,14,16,18,20};
	const double UPPERBOUND = 2135.06;
	const double BS_UpperBound[] = {3.934449955,7.833595526,11.52751515,14.52217418,17.73947517,19.76494182,21.69230768,23.69832,25.92347945,28.3909197};
	
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	vector<skill*> SKILLSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	ofstream skillOut;
	int max_workload = 1;
	const double lambda[] = {0.0001,0.0004,0.0016,0.0064,0.0256,0.1024,0.4096,1.6384,6.4};
	
	for(int lambda_index = 0;lambda_index<9;lambda_index++)
	{
		int workload [7159]={0};
		output.open(PATH+"resultWWW12_1\\"+"result_2_"+ConvertToString(lambda_index)+SUFFIX,ios::out|ios::binary|ios::trunc);
		skillOut.open(PATH+"resultWWW12_1\\"+"skill_2_"+ConvertToString(lambda_index)+SUFFIX,ios::out|ios::binary|ios::trunc);
		srand(time(0));
	for(unsigned int times=0;times!=NUM_OF_TASK;++times)
	{
		clock_t begin_time = clock();
		clock_t total_time = 0L;
		clock_t file_time = 0L;
		cout<<times+1<<endl;
		map<string,skill*> TASK;
		
		short pos = rand()%10;
		string line;
		while(TASK.size()!=num[pos])
		{
			int required_skill_index = rand()%SKILLSET.size();
			skillOut<<required_skill_index<<",";
			//cout<<required_skill_index<<endl;
			SKILLSET[required_skill_index]->isCover = false;
			TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
		}
		skillOut<<endl;
		map<string,author_skill*> AUTHORSET;
		//vector<string> GREEDYCOVER;
		//vector<string> GREEDYDIA;
		for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
		{
			for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
			{
				if(*it_holder=="")
					continue;
				if(AUTHORSET.find(*it_holder)==AUTHORSET.end())
				{
					author_skill* au = new author_skill(*it_holder);
					AUTHORSET.insert(make_pair(*it_holder,au));
					AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
				}
				else
				{
					AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
				}
			}

		}
		cout<<"AUTHOR SIZE:"<<AUTHORSET.size()<<endl;
		/*if(AUTHORSET.size()>100)
		{
			times--;
			continue;
		}*/
		cout<<"max_workload: "<<max_workload<<endl;
		map<string,map<string,double> > UNIVERSE_DIST;
		map<string,map<string,vector<string> > > UNIVERSE_PATH;
		vector<string> Q_apx;
		set<string> T_apx;
		for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
		{
			vector<string> row;
			ifstream holderPathFile;
			string line;
			map<string,double> shortestDist;
			string p = PATH+it_au_s->first+SUFFIX;
			clock_t read_start1 = clock();
			holderPathFile.open(p.c_str(),ios::in|ios::binary);
			if(holderPathFile.fail())
			{
				p = PATH + ConvertToString(index[it_au_s->first])+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);
			}	
			if(holderPathFile.fail())
			{
				cout<<it_au_s->first<<"OPEN FAIL"<<endl;
				return 0;
			}	
			while(getline(holderPathFile,line),holderPathFile.good())
			{
				csvRead::csvline_populate(row,line,',');
				if(row[1]=="DISCONNECTED")
				{
					UNIVERSE_DIST[it_au_s->first].insert(make_pair(row[0],999));
					vector<string> temp;
					UNIVERSE_PATH[it_au_s->first].insert(make_pair(row[0],temp));
				}
				else
				{
					vector<string> solution;
					UNIVERSE_DIST[it_au_s->first].insert(make_pair(row[0],atof(row[1].c_str())));
					for(int i = 2;i<row.size();++i)
					{
						solution.push_back(row[i]);
					}
					UNIVERSE_PATH[it_au_s->first].insert(make_pair(row[0],solution));
				}
			}
			holderPathFile.close();
			read_start1 = clock()-read_start1;
			file_time += read_start1;
		}
		double lambda_A = 0, lambda_1 = lambda[lambda_index], lambda_2 = UPPERBOUND;
		double epsilon = 0.1;
		bool RET = false;
		
		
		//findGroupForGivenLambda(TASK,lambda,Q_apx,T_apx);
		int coveredCount =0;
		int coverGrow = 0;
		double cost = 0;
		double MST = 0;
		while(!RET&&AUTHORSET.size()>0)
		{
			if(Q_apx.size()==0)
			{
				int currentMaxCover = 0;
				string maxCoverX;
				for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
				{
					int cover = 0;
					for(vector<skill*>::iterator it_skill=it_au_s->second->skill_set.begin();it_skill!=it_au_s->second->skill_set.end();++it_skill)
					{
						if (!(*it_skill)->isCover)
						{
							cover++;
						}
					}
					int* data = new int[3];
					data[0]=workload[index[it_au_s->second->getName()]];
					data[1]=TASK.size();
					data[2]=index.size();
					double load = getWorkLoad(max_workload,data);
					if(cover/load>currentMaxCover)
					{
						currentMaxCover = cover/load;
						maxCoverX = it_au_s->second->getName();
					}
				}
				for(vector<skill*>::iterator it_skill=AUTHORSET[maxCoverX]->skill_set.begin();it_skill!=AUTHORSET[maxCoverX]->skill_set.end();++it_skill)
				{
					(*it_skill)->isCover = true;
				}
				Q_apx.push_back(maxCoverX);
				//workload[index[maxCoverX]]++;
				//cout<<"workload[index[maxCoverX]] "<<workload[index[maxCoverX]]<<endl;;
				coveredCount+=currentMaxCover;
				AUTHORSET.erase(maxCoverX);
				RET = true;
				for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				{
					if(!it_task->second->isCover)
					{
						RET = false;
						break;
					}
				}
			}
			else
			{
			double mostCostEffective = 0;
			double benefit=0;
			vector<string> SKILL;
			string auth;
			string i_star;
			for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
			{
				set<string> skillToBeCovered;
				int cover = 0;
				for(vector<skill*>::iterator it_skill=it_au_s->second->skill_set.begin();it_skill!=it_au_s->second->skill_set.end();++it_skill)
				{
					if (!(*it_skill)->isCover)
					{
						cover++;
						skillToBeCovered.insert((*it_skill)->getName());
					}
				}
				bool fail=false;
				if(cover==0)
				{
					it_au_s=AUTHORSET.erase(it_au_s);
					continue;
				}
				double cost_MST=0;
				double dist = 9999;
				set<edges> graph;
			
				string des;
				for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
				{
					if(UNIVERSE_DIST[it_au_s->first][*it_NE]==999)
					{
						fail = true;
						break;
					}						
					if(UNIVERSE_DIST[it_au_s->first][*it_NE]<dist)
					{
						dist = UNIVERSE_DIST[it_au_s->first][*it_NE];
						des = *it_NE;
					}
				}
				if(fail)
				{
					it_au_s = AUTHORSET.erase(it_au_s);
					continue;
				}
				for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
				{
					for(vector<string>::iterator it_NE_NE = it_NE+1;it_NE_NE!=Q_apx.end();it_NE_NE++)
					{
						if(UNIVERSE_PATH[*it_NE][*it_NE_NE].size()==1)
						{
							edges e(*it_NE,*it_NE_NE);
							e.setWeight(UNIVERSE_DIST[*it_NE][*it_NE_NE]);
							graph.insert(e);
						}
					}
				}
				if(UNIVERSE_PATH[it_au_s->first][des].size()!=0)
				{
					edges e(it_au_s->first,*UNIVERSE_PATH[it_au_s->first][des].begin());
					e.setWeight(UNIVERSE_DIST[it_au_s->first][*UNIVERSE_PATH[it_au_s->first][des].begin()]);
					graph.insert(e);
					for(vector<string>::iterator it_sol = UNIVERSE_PATH[it_au_s->first][des].begin();it_sol!=UNIVERSE_PATH[it_au_s->first][des].end();it_sol++)
					{
						if(AUTHORSET.find(*it_sol)!=AUTHORSET.end())
						{
							for(vector<skill*>::iterator it_skill=AUTHORSET[*it_sol]->skill_set.begin();it_skill!=AUTHORSET[*it_sol]->skill_set.end();++it_skill)
							{
								skillToBeCovered.insert((*it_skill)->getName());
							}
						}
						else if(UNIVERSE_DIST.find(*it_sol)==UNIVERSE_DIST.end())
						{
							vector<string> row;
							ifstream holderPathFile;
							string line;
							map<string,double> shortestDist;
							string p = PATH+*it_sol+SUFFIX;
							clock_t read_start = clock();
							holderPathFile.open(p.c_str(),ios::in|ios::binary);
							if(holderPathFile.fail())
							{
								p = PATH + ConvertToString(index[*it_sol])+SUFFIX;
								holderPathFile.open(p.c_str(),ios::in|ios::binary);
							}	
							if(holderPathFile.fail())
							{
								cout<<*it_sol<<"OPEN FAIL"<<endl;
								return 0;
							}	
							while(getline(holderPathFile,line),holderPathFile.good())
							{
								csvRead::csvline_populate(row,line,',');
								if(row[1]=="DISCONNECTED")
								{
									UNIVERSE_DIST[*it_sol].insert(make_pair(row[0],999));
									vector<string> temp;
									UNIVERSE_PATH[*it_sol].insert(make_pair(row[0],temp));
								}
								else
								{
									vector<string> solution;
									UNIVERSE_DIST[*it_sol].insert(make_pair(row[0],atof(row[1].c_str())));
									for(int i = 2;i<row.size();++i)
									{
										solution.push_back(row[i]);
									}
									UNIVERSE_PATH[*it_sol].insert(make_pair(row[0],solution));
								}
							}
							holderPathFile.close();
							read_start = clock()-read_start;
							file_time += read_start;
						}
						if(it_sol+1 != UNIVERSE_PATH[it_au_s->first][des].end()){
						edges e(*it_sol,*(it_sol+1));
						e.setWeight(UNIVERSE_DIST[*it_sol][*(it_sol+1)]);
						graph.insert(e);}
					}
				}/*
				*FOR MST
				*/
				/*
				* use for the MST calculating
				*/
				edges *edge = new edges[graph.size()];
				map<string,double> MAKE_SET;
				set<edges> MST_edge;
				if(!fail&&graph.size()>=1)
				{
					int _index=0;
					for(set<edges>::iterator it = graph.begin();it!=graph.end();it++)
					{
						edge[_index++] = *it;
					}
					_index = 0;
					if(UNIVERSE_PATH[it_au_s->first][des].size()!=0)
					{
						for(vector<string>::iterator it_sol = UNIVERSE_PATH[it_au_s->first][des].begin();it_sol+1!=UNIVERSE_PATH[it_au_s->first][des].end();it_sol++)
						{
							MAKE_SET[*it_sol]=_index++;
						}
					}
					MAKE_SET[it_au_s->first]=_index++;
					for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
					{
					if(MAKE_SET.find(*it_NE)==MAKE_SET.end())
						MAKE_SET[*it_NE]=_index++;
					}

					MST_Kruskal(edge,MAKE_SET,MST_edge,graph.size());
				
					for(set<edges>::iterator it = MST_edge.begin();it!=MST_edge.end();++it)
					{
						cost_MST+= it->getWeight();
					}
					double workload_cost=0;
					int* data = new int[3];
					data[0] = workload[index[it_au_s->first]];
					//cout<<"data[0]: "<<data[0]<<endl;
					data[1] = TASK.size();
					data[2] = index.size();
					workload_cost += getWorkLoad(max_workload,data);
					//cout<<"workload_cost(it_au_s->first): "<<workload_cost<<endl;
					if(UNIVERSE_PATH[it_au_s->first][des].size()!=0)
					{
						for(vector<string>::iterator it_sol = UNIVERSE_PATH[it_au_s->first][des].begin();it_sol+1!=UNIVERSE_PATH[it_au_s->first][des].end();it_sol++)
						{
							int* data = new int[3];
							data[0] = workload[index[*it_sol]];
							//cout<<"data[0](it_sol)"<<data[0]<<endl;
						//cout<<"workload[index[*it_NE]] "<<workload[index[*it_NE]]<<endl;
							data[1] = TASK.size();
							data[2] = index.size();
							workload_cost += getWorkLoad(max_workload,data);
							//cout<<"workload_cost(it_sol): "<<workload_cost<<endl;
						}
					}
					/*for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
					{
						int* data = new int[3];
						data[0] = workload[index[*it_NE]];
						//cout<<"workload[index[*it_NE]] "<<workload[index[*it_NE]]<<endl;
						data[1] = TASK.size();
						data[2] = index.size();
						workload_cost += getWorkLoad(max_workload,data);
					}*/
					
					if(skillToBeCovered.size()/(cost_MST+lambda_1*workload_cost)>benefit)
					{
						//cout<<"lambda_1: "<< lambda_1<<" workload_cost: "<<workload_cost<<endl;
						benefit = skillToBeCovered.size()/(cost_MST+lambda_1*workload_cost);
						//cout<<"benefit: "<<benefit<<endl;
						auth = it_au_s->first;
						i_star = des;
						//cost = cost_MST+lambda_1*workload_cost;
						MST = cost_MST;
						SKILL.clear();
						for(set<string>::iterator it = skillToBeCovered.begin();it!=skillToBeCovered.end();++it)
						{
							SKILL.push_back(*it);
						}
					}
				}
			}
			Q_apx.push_back(auth);
			//cout<<"workload[index[auth]] "<<workload[index[auth]]<<endl;
			AUTHORSET.erase(auth);
			for(vector<string>::iterator it_sol = UNIVERSE_PATH[auth][i_star].begin();it_sol!=UNIVERSE_PATH[auth][i_star].end();it_sol++)
			{
				vector<string>::iterator position = find(Q_apx.begin(),Q_apx.end(),*it_sol);
				if(position==Q_apx.end())
				{
					Q_apx.push_back(*it_sol);
					//cout<<"workload[index[*it_sol]] "<<workload[index[*it_sol]]<<endl;
				}
				map<string,author_skill*>::iterator it=AUTHORSET.find(*it_sol);
				if(it!=AUTHORSET.end())
					AUTHORSET.erase(it);
			}
				
			for(vector<string>::iterator it = SKILL.begin();it!=SKILL.end();++it)
			{
				TASK[*it]->isCover = true;
			}
			RET = true;
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
			{
				if(!it_task->second->isCover)
				{
					RET = false;
					break;
				}
			}
			//cout<<AUTHORSET.size()<<endl;
			
			}
		}
		if(RET)
		{
			total_time = clock()-begin_time-file_time;
			cout<<"Computational time: "<<total_time<<","<<file_time<<","<<clock()-begin_time<<endl;
			string p_a_t_h="";
			string s_k_i_l_l="";
			cost = 0;
			for(vector<string>::iterator i_X_P = Q_apx.begin();i_X_P!=Q_apx.end();++i_X_P)
			{
				T_apx.insert(*i_X_P);
				int* data = new int[3];
				data[0] = workload[index[*i_X_P]];
				cout<<"data[0](it_sol)"<<data[0]<<endl;
				//cout<<"workload[index[*it_NE]] "<<workload[index[*it_NE]]<<endl;
				data[1] = TASK.size();
				data[2] = index.size();
				cout<<"getWorkLoad(max_workload,data): "<<getWorkLoad(max_workload,data,false)<<endl;
				cost += getWorkLoad(max_workload,data);
				
			}
			
			for(set<string>::iterator i_Tapx = T_apx.begin();i_Tapx!=T_apx.end();i_Tapx++)
			{
				p_a_t_h+=*i_Tapx+",";
				++workload[index[*i_Tapx]];
			}

			double max_personal_workload = 0;
			/*for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
				++workload[index[*it_NE]];
			*/
			for(unsigned int dex =0;dex<index.size();dex++)
			{
				if(workload[dex]>max_personal_workload&&workload[dex]<=100)
					max_personal_workload=workload[dex];
				if(max_personal_workload>100)
					cout<<"FFFFFFFFFFFF"<<endl;
			}
			int* data = new int[3];
			data[0] = max_personal_workload;
			data[1] = TASK.size();
			data[2] = index.size();
			getWorkLoad(max_workload,data);
			cout<<"max_personal_workload: "<<max_personal_workload<<endl;
			for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				cout<<"SKILL:"<<it_skill->first<<endl;
				s_k_i_l_l+=it_skill->first+",";
			}
			if(total_time==0)
				output<<lambda_1<<","<<total_time<<"|"<<file_time<<"|"<<clock()-begin_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<cost<<","<<MST<<endl;
			else
				output<<lambda_1<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<cost<<","<<MST<<endl;
			cout <<lambda_1<<","<<total_time<<","<<cost<<","<<MST*10.0<<endl;
			//system("pause");
		}
		
		/*UNIVERSE_DIST.clear();
		UNIVERSE_PATH.clear();
		AUTHORSET.clear();
		TASK.clear();*/
	}
	double max_personal_workload = 0;
	int maxDex = 0;
	for(unsigned int dex =0;dex<index.size();dex++)
	{
		if(workload[dex]>max_personal_workload&&workload[dex]<=100)
		{
			max_personal_workload=workload[dex];
			maxDex = dex;
		}
		
	}
	output<<max_personal_workload<<","<<maxDex<<endl;
	output.close();
	skillOut.close();
	}
	return 0;
}

void getIndex(map<string,vector<skill*>::size_type>& index)
{
	vector<string> row;
	ifstream in("E:\\ProgramData\\Java\\DBLP\\20130725\\Test\\dblp_coauthor_l_r.csv000",ios::binary);
    if (in.fail())  { cout << "File not found" <<endl; return ; }
	unsigned int count=0;
	string line;
    while(getline(in, line)  && in.good() )
    {
        csvRead::csvline_populate(row, line, ',');

			index.insert(make_pair(row[0],count++));
    }
    in.close();
}


void getskills(const float high,const float low ,vector<skill*>& collection)
{
	ifstream readSkill ("E:\\ProgramData\\Java\\DBLP\\20130727\\Test\\skill_l.csv",ios::binary);
	vector<string> row;
	string line;
	vector<string>::size_type line_no=0;
	while(getline(readSkill,line)&&readSkill.good())
	{
		csvRead::csvline_populate(row,line,',');
		if(atof(row[1].c_str())<=high&&atof(row[1].c_str())>=low&&row[0].size()>1)
		{
			string name = row[0];
			if(name.find('/')<name.size())
				continue;
			double frequency = atof(row[1].c_str());
			skill *s = new skill(name,frequency);
			//s->index=line_no++;
			for(vector<string>::size_type i = 2;i!=row.size()&&row[i]!="";++i)
			{
				s->push_back(row[i]);
			}
			collection.push_back(s);
		}

	}
}

void MST_Kruskal(edges edge[],map<string,double>& nodeSet,set<edges>& eSet,int n)
{
	quick_sort(edge,0,n-1);
	for(int i =0;i<n;i++)
	{
		double a1 = nodeSet[edge[i].getAuthor(false)];
		double a2 = nodeSet[edge[i].getAuthor(true)];
		if(a1!=a2)
		{
			eSet.insert(edge[i]);
			double s1 = nodeSet[edge[i].getAuthor(true)];
			double s2 = nodeSet[edge[i].getAuthor(false)];
			for(map<string,double>::iterator it = nodeSet.begin();it!=nodeSet.end();it++)
			{
				if(it->second==s1)
					it->second =s2;
			}

		}
	} 
}

void findGroupForGivenLambda(const map<string,skill*>& TASK,double lambda,vector<string>&Q_apx,vector<string>&T_apx)
{

}
double getWorkLoad(int& max_workload,int*data)
{
	//data[0] workload
	//data[1] numofskills
	//data[2] numberofperson
	double A = 2*log(data[1]*1.0)/log(2)*max_workload*log(2*data[2]*1.0)/log(2);
	//double A = 2*2*max_workload*log(2*data[2]*1.0)/log(2);
	while (A<data[0])
	{
		//system("pause");
		cout<<"data[0]: "<<data[0]<<" data[1]: "<<data[1]<<" data[2]: "<<data[2]<<" max_workload: "<<max_workload<<" A: "<<A<<endl;
		max_workload *=2;
		A = 2*log(data[1]*1.0)*max_workload*log(2*data[2]*1.0)/log(2);
	}
	double EXPLoad;
	EXPLoad = pow(2*data[2]*1.0,(data[0]/A));
	return EXPLoad;
}
double getWorkLoad(int& max_workload,int*data,bool isPrint)
{
	//data[0] workload
	//data[1] numofskills
	//data[2] numberofperson
	double A = 2*log(data[1]*1.0)/log(2)*max_workload*log(2*data[2]*1.0)/log(2);
	//double A = 2*2*max_workload*log(2*data[2]*1.0)/log(2);
	if(isPrint)
		cout<<"A: "<<A<<endl;
	while (A<data[0])
	{
		max_workload *=2;
		A = 2*log(data[1]*1.0)*max_workload*log(2*data[2]*1.0)/log(2);
		if(isPrint)
			cout<<"max_workload: "<<max_workload<<" A: "<<A<<endl;
	}
	double EXPLoad;
	EXPLoad = pow(2*data[2]*1.0,(data[0]/A));
	return EXPLoad;
}