//
//  main.cpp
//  MinSD
//
//  Created by WangXinyu on 15/3/13.
//  Copyright (c) 2015年 WangXinyu. All rights reserved.
//
#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

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

//#define NUM_OF_NODES 1712433
#define NUM_OF_NODES 9948
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
    unsigned indexInGraph;
    bool isCover;
    skill(std::string name, unsigned freq):name(name),frequency(freq){holders.clear();isCover=false;};
private:
    std::string name;
    
    unsigned frequency;
};


int main(int argc, const char * argv[]) {
    // insert code here...
    //comment after testing:
    std::ifstream in(getenv("INPUT"));
    std::cin.rdbuf(in.rdbuf());
    
//    const unsigned numOfEdges = 3826930;
    const unsigned numOfEdges = 3753288;
    /*
     Build graph with sources, targets and dists
     */
    graph_t AMinerGraph(1712431);
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
            std::auto_ptr<mongo::DBClientCursor> cursor = conn->query(/*"AMinerDB*/"GuruDB.skillCollection", skillSearchObj,0,0,NULL,mongo::QueryOption_NoCursorTimeout);
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
         Begin MinSD Algorithm
         */
        
        double sumOfDist = 0;
        std::time_t start = clock();
        for (auto &reqSKill : skills)
        {
            double distance = 999999;
            for (auto expert : reqSKill.holders)
            {
                if(d[expert]<distance)
                {
                    distance = d[expert];
                    reqSKill.i_prem = expert;
                }
            }
            sumOfDist += distance;
        }
        // Algorithm finished
        std::time_t end = clock() - start;
        std::time_t realEnd = end + start - realStart;
        
        std::cout << taskIndex << '\t' << sumOfDist << '\t' << end*1000.0/CLOCKS_PER_SEC<<'\t'<<realEnd*1000.0/CLOCKS_PER_SEC<<'\t'<< candidateIndex;
        if (sumOfDist > 999999) {
            std::cout << std::endl;
            continue;
        }
        std::set<unsigned long> experts;
        experts.insert(candidateIndex);
        for (auto reqSkill : skills)
        {
            unsigned long des = p[reqSkill.i_prem];
            do
            {
                experts.insert(des);
                des = p[des];
            }while (des != candidateIndex);
        }
        
        //revision:
        for (auto reqSkill : skills)
        {
            experts.insert(reqSkill.i_prem);
        }
        
        for (auto expert :experts)
        {
            std::cout<< '\t' << expert;
        }
        std::cout << std::endl;
        
        
    }
    
    std::cout << "Hello, World!\n";
    return 0;
}
