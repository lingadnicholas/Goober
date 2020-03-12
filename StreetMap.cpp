#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<std::string>()(g.latitudeText + g.longitudeText);
}
unsigned int hasher(const double& d)
{
    return std::hash<double>()(d); 
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(std::string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, std::vector<StreetSegment>& segs) const;

    private:
    ExpandableHashMap<GeoCoord, std::vector<StreetSegment>> m_mapData; //This will hold the street segments
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(std::string mapFile)
{

    std::ifstream infile(mapFile); 
    if (!infile)
        return false;
    //This will hold each street, which will then get pushed into mapData. 

    std::string name;
    double coords;
    std::string line;
    int numTimes = 0; 
    while (getline(infile, name))
    {
        //What you can do: 
        //Get street name with getline
        //Get number of coords (i) to expect using >> for integers and cin.ignore
        //Loop i number of times to get following input 

        //numTimes is how many lines you need to iterate through to get all the coords of this street
        
        infile >> numTimes;        
 
        for (int i = 0; i < numTimes; i++)
        {
            string beginLat, beginLon, endLat, endLon;
            //Read in the street segment data
            infile >> beginLat >> beginLon >> endLat >> endLon;

            GeoCoord B(beginLat, beginLon);
            GeoCoord E(endLat, endLon);
            StreetSegment street(B, E, name);
            StreetSegment reverseStreet(E, B, name);
            std::vector<StreetSegment> streetSegs;
            std::vector<StreetSegment> reversed;
            streetSegs.push_back(street);
            reversed.push_back(reverseStreet);
  
            vector<StreetSegment> temp;
            // Create a new street segment vector association IF NECESSARY (if cannot be found already)
            if (m_mapData.find(B) == nullptr)
                 m_mapData.associate(B, temp);
            //And then add 
            m_mapData.find(B)->push_back(street);

            //Do the same thing but for the reverse
            if (m_mapData.find(E) == nullptr)
                m_mapData.associate(E, temp);

            m_mapData.find(E)->push_back(reverseStreet);
           
        } 
        infile.ignore(10000, '\n');
    }
    return true; 
}



bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, std::vector<StreetSegment>& segs) const
{
    if (m_mapData.find(gc))
    {
        //Key found! So, empty out segs vector
        if (!segs.empty())
        {
            segs.clear();
        }
        //Then give segs the data in the pointer
        segs = *(m_mapData.find(gc)); 

        return true;
    }
    return false; 
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(std::string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, std::vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
