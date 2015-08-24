//
//  main.cpp
//  EnhancedSteiner
//
//  Created by WangXinyu on 15/3/12.
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

//#define NUM_OF_NODES 1712433
#define NUM_OF_NODES 10000
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
    
    std::ofstream out(getenv("OUTPUT"));
    std::cout.rdbuf(out.rdbuf());
    
//    const unsigned numOfEdges = 3826930;
    const unsigned numOfEdges = 3753288;
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
         clear vectors: skills, columns
         */
        skills.clear();
        columns.clear();
        
        
        
        
        csvRead::csvline_populate(columns, inputLine, '\t');
        
        int taskIndex = atoi(columns[0].c_str());
        int numOfSkills = (taskIndex/100+1)*4;
        srand(time(0)+taskIndex);
        
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
            tmpSkill.indexInGraph = NUM_OF_NODES+skillIndex-1;
            for(auto expert:experts)
            {
                tmpSkill.push_back(expert.numberLong());
            }
            skills.push_back(tmpSkill);
        }
        
        /*
         Build graph with sources, targets and dists
         */
        graph_t AMinerGraph(NUM_OF_NODES);
        for (unsigned edgeIndex = 0; edgeIndex<numOfEdges; ++edgeIndex)
            boost::add_edge(sources[edgeIndex],targets[edgeIndex],1-dists[edgeIndex],AMinerGraph);
        for (auto tmpSkill:skills)
        {
            for (auto holder:tmpSkill.holders)
            {
                boost::add_edge(tmpSkill.indexInGraph, holder, 2000000, AMinerGraph);
                boost::add_edge(holder, tmpSkill.indexInGraph, 2000000, AMinerGraph);
            }
        }
        
        std::vector<vertex_descriptor> p(num_vertices(AMinerGraph));
        std::vector<double> d(num_vertices(AMinerGraph));
        
        /*
         Random choose a skill from the task
         */
        
        std::vector<unsigned long> X_PREM;
        int randIndex = rand()%skills.size();
        X_PREM.push_back(skills[randIndex].indexInGraph);
        //skills.erase(std::find(skills.begin(), skills.end(), skills[randIndex]));
        skills[randIndex].isCover = true;
        bool isFinish = false;
        std::time_t start = clock();
        std::time_t duration = 0;
        std::time_t realStart = start;
        while (!isFinish) {
            
            float R_ia = 99999999999;
            unsigned long v_star=0;
            unsigned long destination=0;
            for (auto &tmpSkill:skills)
            {
                if (tmpSkill.isCover) {
                    continue;
                }
                int skillIndex = tmpSkill.indexInGraph;
                
                duration += clock()-start;
                
                // 计算基于 skillIndex 的最短距离
                vertex_descriptor s = vertex(skillIndex,AMinerGraph);
                dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                                     boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                     distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, AMinerGraph))));
                start = clock();
                
                for (auto oneOfX_PREM:X_PREM)
                {
                    float dist = d[oneOfX_PREM];
                    if (dist < R_ia) {
                        v_star = skillIndex;
                        destination = oneOfX_PREM;
                        R_ia = dist;
                    }
                }
                
                
            }
            if (v_star==0||destination==0) {
                //error
            }
            else
            {
                skills[v_star-NUM_OF_NODES].isCover = true;
                
                duration += clock()-start;
                
                vertex_descriptor s = vertex(v_star,AMinerGraph);
                dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                                     boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                     distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, AMinerGraph))));
                start = clock();
                
                while (destination!=v_star)
                {
                    destination = p[destination];
                    X_PREM.push_back(destination);
                    
                }
                
                
            }
            
            isFinish = true;
            
            for (auto &tmpSkill:skills)
            {
                if (!tmpSkill.isCover) {
                    isFinish = false;
                }
            }
        }
        
        duration += clock() - start;
        std::time_t realDura = clock() - realStart;
        //EnSteiner is finished
        std::cout<<taskIndex<<'\t'<<0<<'\t'<<duration*1000.0/CLOCKS_PER_SEC<<'\t'<<realDura*1000.0/CLOCKS_PER_SEC<<'\t';;
        for (auto &tmpSkill:skills)
        {
            std::cout << tmpSkill.getName() <<'\t';
        }
        std::cout << skills[randIndex].getName() << '\t'<<duration;
        
        for (auto oneOfX_PREM:X_PREM)
        {
            if (oneOfX_PREM<NUM_OF_NODES) {
                std::cout << '\t' << oneOfX_PREM;
            }
        }
        std::cout<<std::endl;
        
    }
    
    std::cout << "Hello, World!\n";
    return 0;
}
