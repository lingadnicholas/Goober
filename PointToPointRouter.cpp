#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <set>
#include <stack>
#include <cfloat>
#include <map>
#include <queue> 
using namespace std;
using pPair = pair<double, pair<GeoCoord, GeoCoord>>;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private: 
    const StreetMap* m_streetMap; 
    //Checks if given coordinate is valid or not
    bool isValid(const GeoCoord& check) const
    {
        vector<StreetSegment> a;
        if (m_streetMap->getSegmentsThatStartWith(check, a))
            return true; 
        return false; 
    }
    bool isDestination(const GeoCoord& check, const GeoCoord& dest) const
    {
        if (check == dest)
            return true;
        return false; 
    }

    struct Node 
    {
        StreetSegment thisSeg;
        Node* parent; 
        // f = g + h 
        double f = FLT_MAX, g = FLT_MAX, h=FLT_MAX; 

        bool operator<(const Node& rh)
        {
            if (thisSeg.start.latitude < rh.thisSeg.start.latitude && thisSeg.start.longitude < rh.thisSeg.start.longitude)
                return true;
            return false;
        }
        bool operator==(const Node& rh)
        {
            if (thisSeg.start == rh.thisSeg.start && thisSeg.end == rh.thisSeg.end)
                return true;
            return false;
        }
    };


    Node* findNode(list<Node>* curNodes, StreetSegment& thisSeg) const
    {

        auto listIt = curNodes->begin();

        while (listIt != curNodes->end())
        {
            //Find the correct node in the list whose segments are the same as newNo
            //And give a pointer to it
            if (listIt->thisSeg == thisSeg)
            {
                return &(*listIt);
                break;
            }
            ++listIt;
        }
        return nullptr; 
    }

    //Traces the path from the source to the destination
    void tracePath(map<double, Node>& closedList, GeoCoord start, list<StreetSegment>& route, double& totalDistanceTravelled) const
    {
        //We use a stack because we are going from the destination to the source. 
        auto beginning = closedList.find(0);
        totalDistanceTravelled = 0; 
        stack<StreetSegment> path;
        
        Node curNode = beginning->second; 
       for(;;) //While the current segment != the destination
        {
           StreetSegment curSeg = curNode.thisSeg; 
           path.push(curSeg); 
           totalDistanceTravelled += distanceEarthMiles(curSeg.end, curSeg.start); 
           if (curNode.parent == nullptr || curSeg.start == start)
               break;
           curNode = *curNode.parent; 
        }

        while (!path.empty())
        {
            route.push_back(path.top());
            path.pop();
        }
    }

    //Traces the path from the source to the destination
    void tracePath(Node* end, GeoCoord start, list<StreetSegment>& route, double& totalDistanceTravelled) const
    {
        //We use a stack because we are going from the destination to the source. 
        totalDistanceTravelled = 0;
        stack<StreetSegment> path;
        
        for(;;)
        {
            StreetSegment curSeg = end->thisSeg; 
            path.push(curSeg);
            totalDistanceTravelled += distanceEarthMiles(curSeg.end, curSeg.start);
            if (end->parent == nullptr || curSeg.start == start)
                break; 
            end = end->parent; 
        }

        while (!path.empty())
        {
            route.push_back(path.top());
            path.pop();
        }
    }
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    : m_streetMap(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{

    if (!isValid(end))
        return BAD_COORD; 
    if (isDestination(start, end))
    {
        route.clear(); 
        totalDistanceTravelled = 0; 
        return DELIVERY_SUCCESS; 
    }
    
    //Initialize open list and closed list 
    map<double, Node> openList; 
    map<double, Node> closedList; 

    vector<StreetSegment> segs; 
    m_streetMap->getSegmentsThatStartWith(start, segs); 
    if (segs.empty())
        return NO_ROUTE;

    //Ugliness: Initialize the first Node of the open list. 
    Node push; 
    push.h = distanceEarthMiles(start, end); 
    push.g = 0;
    push.f = push.h; 
    push.thisSeg.start = start; 
    push.thisSeg.end = segs[0].end; 
    push.thisSeg.name = segs[0].name; 
    push.parent = nullptr; 
    openList[push.f] = push;
    //End of the mess 

    //Search for a route 
    while (!openList.empty())
    {
        Node current = openList.begin()->second;
        openList.erase(openList.begin()); 

        //Push onto closed list 
        closedList[current.f] = current;

        auto parentItr = closedList.find(current.f); 
        Node* parent = &(parentItr->second);

        //Find the successors
        //Possible successors are those whose starts points == current endpoint. 
        //OR those whose start points == current startpoint (and don't == this one) 
        vector<StreetSegment> endSuccessors;
        vector<StreetSegment> startSuccessors;
        GeoCoord endSuc(current.thisSeg.end); 

        GeoCoord startSuc(current.thisSeg.end); 
        m_streetMap->getSegmentsThatStartWith(endSuc, endSuccessors);
        m_streetMap->getSegmentsThatStartWith(startSuc, startSuccessors);
        auto it = startSuccessors.begin();
        while (it != startSuccessors.end())
        {
            //Remove the street segment that duplicates this one. 
            if (*it == current.thisSeg)
                it = startSuccessors.erase(it);
            else
                ++it;
        }

        //Put both vectors into one combined vector. 
        vector<StreetSegment> combined = endSuccessors;
        if (!startSuccessors.empty())
        {
            it = startSuccessors.begin();
            while (it != startSuccessors.end())
            {
                combined.push_back(*it);
                ++it;
            }
        }
        
        //For each neighbor
        auto combinedIt = combined.begin();
        for (int i = 0; combinedIt != combined.end(); i++, ++combinedIt)
        {
            //Get all the information for this new neighbor
            double newg = 0, newh = 0, newf = 0;
  
            //This part is a mess...... I'm just initializing and pushing neighbors
            Node neighbor;
            //Set coordinates
            GeoCoord neighborStart(combined[i].start);
            GeoCoord neighborEnd(combined[i].end); 
            neighbor.thisSeg.start = neighborStart;
            neighbor.thisSeg.end = neighborEnd;
            neighbor.thisSeg.name = combined[i].name; 
            //Set g, h, f
            newg = distanceEarthMiles(current.thisSeg.start, neighbor.thisSeg.start);
            newh = distanceEarthMiles(combined[i].end, end); //Distance to end
            newf = newg + newh;
            neighbor.f = newf; neighbor.h = newf; neighbor.g = newg;
            neighbor.parent = parent;  //Set parent
            //End of the mess

            //If neighbor's ending coordinate is the goal, stop the search
          
            if (neighbor.thisSeg.end == end) 
            {
                //Push neighbor and current onto closedList 
                neighbor.f = 0; 
                closedList[neighbor.f] = neighbor; 
                closedList[current.f] = current;
                tracePath(closedList, start, route, totalDistanceTravelled); 
                return DELIVERY_SUCCESS;
            } 



            auto openFound = openList.find(newf); 
            auto closedFound = closedList.find(newf);
            //If not found, add neighbor to open list, so we can examine its neighbors. 
            if (openFound == openList.end() && closedFound == closedList.end())
            {
                openList[newf] = neighbor; 
            }
        }
    }
    
    return NO_ROUTE; //Somehow got here. 
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
