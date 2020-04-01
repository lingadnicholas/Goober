#include "provided.h"
#include "ExpandableHashMap.h"
#include <set>
#include <stack>
#include <cfloat>
#include <map>
using namespace std;

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
        Node()
        {

        }
        Node(StreetSegment seg, Node* par, double f, double g)
            :thisSeg(seg), parent(par), f(f), g(g)
        {

        }
        StreetSegment thisSeg;
        Node* parent;
        // In A*, f = g + h, but here, we only use g and f.
        double f = FLT_MAX, g = FLT_MAX;
    };

    //Traces the path from the source to the destination
    void tracePath(map<double, Node>& closedList, GeoCoord start, list<StreetSegment>& route, double& totalDistanceTravelled) const;

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

    //This is not A*, but it still uses some type of heuristic
    //Ugliness: Initialize the first Node of the open list. 
    StreetSegment s(segs[0].start, segs[0].end, segs[0].name); 
    double firstf = distanceEarthMiles(start, end);
    Node push(s, nullptr, firstf, 0);
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
            double newg = 0, newf = 0;

            //This part is a mess...... I'm just initializing and pushing neighbors
            //Set coordinates
            StreetSegment newS(combined[i].start, combined[i].end, combined[i].name);
            //Set g, f
            newg = distanceEarthMiles(current.thisSeg.start, newS.start);
            newf = newg + distanceEarthMiles(combined[i].end, end); //Distance to end
            Node neighbor(newS, parent, newf, newg);
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
            //If not found add to open list so we can look at that neighbor later
            if (openFound == openList.end() && closedFound == closedList.end())
            {
                openList[newf] = neighbor;
            }
        }
    }

    return NO_ROUTE; //Somehow got here. 
}

void PointToPointRouterImpl::tracePath(map<double, Node>& closedList, GeoCoord start, list<StreetSegment>& route, double& totalDistanceTravelled) const
{
    //We use a stack because we are going from the destination to the source. 
    auto beginning = closedList.find(0);
    stack<StreetSegment> path;

    Node curNode = beginning->second;
    StreetSegment curSeg = curNode.thisSeg;
    //Until you reach the beginning: 
    for (;;) 
    {
        curSeg = curNode.thisSeg; 
        path.push(curSeg);
        totalDistanceTravelled += distanceEarthMiles(curSeg.end, curSeg.start);
        if (curNode.parent == nullptr) //We've reached the end
            break;
        curNode = *curNode.parent;
    }

    //We put everything on a stack (LIFO) so now we can pop beginning to end
    while (!path.empty())
    {
        route.push_back(path.top());
        path.pop();
    }
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