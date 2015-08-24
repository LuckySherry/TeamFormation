//
//  main.cpp
//  RarestFirst
//
//  Created by WangXinyu on 15/3/11.
//  Copyright (c) 2015年 WangXinyu. All rights reserved.
//

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include "author.h"
#include "csv.h"
#include "edgeDB_guru.h"

#include "mongo/client/dbclient.h"
#include "mongo/client/init.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/property_map/property_map.hpp>

#include "stdlib.h"
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <ctime>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
boost::property<boost::vertex_distance_t, double>, boost::property<boost::edge_weight_t, double> > graph_t;
typedef boost::graph_traits < graph_t >::vertex_descriptor vertex_descriptor;


class skill
{
public:
    skill(){};
    std::vector<unsigned long> holders;
    void push_back(unsigned long s){holders.push_back(s);}
    void setFreq(unsigned d){frequency = d;}
    unsigned getFreq(){return frequency;}
    std::string getName(){return name;}
    void setName(std::string n){name = n;}
    unsigned long i_prem;	//d(i*,s(a)) = d(i*,i')
    //vector<string>::size_type index;
    skill(std::string name, unsigned freq):name(name),frequency(freq){holders.clear();};
private:
    std::string name;
    
    unsigned frequency;
};

class shortest
{
public:
    std::vector<std::string> road;
    double weight;
    std::string owner;
    shortest(double weight=100,std::string name=NULL):weight(weight),owner(name){};
};

class mongoClient
{
public:
    mongoClient();
    ~mongoClient();
private:
    bool clentExist=false;
    std::string uri;
    ;
};

mongoClient::mongoClient()
{
    if (!clentExist) {
        
    }
}


template <class T>
std::string ConvertToString(T value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}


int main(int argc, const char * argv[]) {
    //comment after testing:
    std::ifstream in(getenv("INPUT"));
    std::cin.rdbuf(in.rdbuf());
    
//    const unsigned numOfEdges = 3826930;
    const unsigned numOfEdges = 3753288;
    /*
     Build graph with sources, targets and dists
     */
//    graph_t AMinerGraph(1712431);
    graph_t AMinerGraph(10000);
    for (unsigned edgeIndex = 0; edgeIndex<numOfEdges; ++edgeIndex)
        boost::add_edge(sources[edgeIndex],targets[edgeIndex],1-dists[edgeIndex],AMinerGraph);
    std::vector<vertex_descriptor> p(num_vertices(AMinerGraph));
    std::vector<double> d(num_vertices(AMinerGraph));
    
    
    /*
     Create mongodb client
     */
    std::string uri = "mongodb://localhost:27017";
    std::string errmsg;
    mongo::ConnectionString cs = mongo::ConnectionString::parse(uri, errmsg);
    
    if (!cs.isValid()) {
        return EXIT_FAILURE;
    }
    
    boost::scoped_ptr<mongo::DBClientBase> conn(cs.connect(errmsg));
    if ( !conn ) {
        return EXIT_FAILURE;
    }
    
    /*
     Get skills from stdin
     */
    std::string inputLine;
    std::vector<std::string> columns;
    std::vector<skill> skills;
    
    while (getline(std::cin,inputLine)) {
        /*
         clear vectors: p, d, skills, columns
         */
        p.clear();
        d.clear();
        skills.clear();
        columns.clear();
        
        csvRead::csvline_populate(columns, inputLine, '\t');
        
        int taskIndex = atoi(columns[0].c_str());
        int numOfSkills = (taskIndex/100+1)*4;
        for (int skillIndex = 1; skillIndex<=numOfSkills; ++skillIndex) {
            std::string skillName = columns[skillIndex];
            mongo::BSONObj skillSearchObj = BSON("Skill"<<skillName);
            std::auto_ptr<mongo::DBClientCursor> cursor = conn->query("GuruDB.skillCollection"/*"AMinerDB.skillCollection"*/, skillSearchObj,0,0,NULL,mongo::QueryOption_NoCursorTimeout);
            if (!cursor.get()) {
                return EXIT_FAILURE;
            }
            
            mongo::BSONObj skillNode = cursor->next();
            unsigned expertSize = skillNode.getIntField("ExpertSize");
            std::vector<mongo::BSONElement> experts = skillNode["ExpertIndex"].Array();
            
            skill tmpSkill(skillName,expertSize);
            for(auto expert:experts)
            {
                tmpSkill.push_back(expert.numberLong());
            }
            skills.push_back(tmpSkill);
        }
        std::time_t realStart = clock();
        std::string rarestSkill = columns[numOfSkills+1];
        int candidateIndex = std::atoi(columns[columns.size()-1].c_str());
        
        // 计算基于 candidateIndex 的最短距离
        vertex_descriptor s = vertex(candidateIndex,AMinerGraph);
        dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                             boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, AMinerGraph))).
                                             distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, AMinerGraph))));
        /*
         Begin RarestFirst Algorithm
         */

        float R_i = 0;
        time_t start = std::clock();
        std::set<unsigned long> TASK_HOLDER_SET;
        for(auto &reqSkill:skills)
        {
            float R_ia = 9999999999;
            if (reqSkill.getName()==rarestSkill)
                continue;
            for (auto expert:reqSkill.holders)
            {
                TASK_HOLDER_SET.insert(expert);
                if (expert == candidateIndex) {
                    R_ia = 0;
                    break;
                }
                
                if (d[expert]<R_ia&&d[expert]<100000) {
                    R_ia = d[expert];
                    reqSkill.i_prem = expert;
                }
            }
            if (R_ia > R_i) {
                R_i = R_ia;
            }
        }
        
        std::time_t end = std::clock()-start;
        std::time_t realEnd = end+start-realStart;
        std::cout<<taskIndex<<'\t'<<R_i<<'\t'<<end*1000.0/CLOCKS_PER_SEC<<'\t'<<realEnd*1000.0/CLOCKS_PER_SEC<<'\t';
        if (R_i>=100000)
        {
            std::cout<<std::endl;
            continue;
        }
        std::string path = "", coordinator = "";
        
        for (auto &reqSkill : skills)
        {
            std::cout<<reqSkill.getName()<<'\t';
            if (reqSkill.getName()==rarestSkill)
                continue;
            unsigned long expert = reqSkill.i_prem;
            do
            {
                if (TASK_HOLDER_SET.find(expert)!=TASK_HOLDER_SET.end())
                    path += '\t'+ std::to_string(expert);
                else
                    coordinator += '\t'+ std::to_string(expert);
                expert = p[expert];
            }while (expert!=candidateIndex);
        }
        std::cout<<candidateIndex<<path<<coordinator<<std::endl;
        
        
        
    }
    std::cout << "Hello, World!\n";
    return 0;
}
