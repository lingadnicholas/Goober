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

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(std::string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, std::vector<StreetSegment>& segs) const;

    private:
    ExpandableHashMap<GeoCoord, std::vector<StreetSegment>> m_mapData; //This will hold the street segments
    std::vector<StreetSegment> streetSegs;
    std::vector<StreetSegment> revStreetSegs;
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
    StreetSegment street;

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


       
        //Get street name with getline if don't need to iterate through lines of Coords
        

        //m_numTimes is how many lines you need to iterate through to get all the coords of this street
        
        infile >> numTimes;        
        GeoCoord B;
        GeoCoord E;
        for (int i = 0; i < numTimes; i++)
        {

            //Read in the street segment data
            street.name = name;

            infile >> B.latitudeText >> B.longitudeText >> E.latitudeText >> E.longitudeText;
            street.start = B;
            street.end = E;
            streetSegs.push_back(street);

            auto it = streetSegs.begin();
            while (streetSegs.begin() != streetSegs.end())
            {
                std::vector<StreetSegment>* vals = m_mapData.find(B);
                //If a key with this starting point in the map is not found, create a new association
                if (vals == nullptr)
                    m_mapData.associate(B, streetSegs);
                else //If it is found, add on to its vector
                    vals->push_back(*it);
                it = streetSegs.erase(it);
            }
            //Then do the same but for the reverse. 
            street.start = E;
            street.end = B;
            revStreetSegs.push_back(street);
            auto endIt = revStreetSegs.begin();
            while (revStreetSegs.begin() != revStreetSegs.end())
            {
                std::vector<StreetSegment>* vals = m_mapData.find(B);
                //If a key with this starting point in the map is not found, create a new association
                if (vals == nullptr)
                    m_mapData.associate(B, revStreetSegs);
                else //If it is found, add on to its vector
                    vals->push_back(*endIt);
                endIt = revStreetSegs.erase(endIt);
            }
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
