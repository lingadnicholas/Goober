#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <set>
#include <stack>
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
    double calculateHValue(const GeoCoord& cur, const GeoCoord& dest) const
    {
        return distanceEarthMiles(cur, dest); 
    }

    struct Node 
    {
        StreetSegment thisSeg;
        StreetSegment parent; 
        bool checked = false; 
        // f = g + h 
        double f = FLT_MAX, g = FLT_MAX, h=FLT_MAX; 
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
    void tracePath(ExpandableHashMap<GeoCoord, list<Node>>& map, StreetSegment dest, list<StreetSegment>& route, double& totalDistanceTravelled) const
    {
        //We use a stack because we are going from the destination to the source. 
        totalDistanceTravelled = 0; 
        stack<StreetSegment> path;
        GeoCoord startDest = dest.start;
        GeoCoord endDest = dest.end; 
        list<Node>* nodes = map.find(startDest); //Returns the list where the node is
        Node* cur = findNode(nodes, dest); 
        while (cur->parent.start != startDest) //While the current segment != the destination
        {
            path.push(cur->thisSeg);
            //I want to get tempNode's parent. 
            StreetSegment temp = cur->parent; 
            startDest = temp.start;
            totalDistanceTravelled += distanceEarthMiles(cur->thisSeg.start, cur->parent.start); 
            nodes = map.find(startDest);
            cur = findNode(nodes, dest);
        }

  
        path.push(dest);


        while (!path.empty())
        {
            route.push_back(path.top());
            path.pop();
        }
        return;
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
    //aStarSearch 
    if (isValid(end))
        return BAD_COORD; 
    if (isDestination(start, end))
    {
        route.clear(); 
        totalDistanceTravelled = 0; 
        return DELIVERY_SUCCESS; 
    }
    vector<StreetSegment> segs; 
    ExpandableHashMap<GeoCoord, list<Node>> closedList; 

    m_streetMap->getSegmentsThatStartWith(start, segs); 
    Node push;
    list<Node> li; 
    push.f = 0; push.g = 0; push.h = 0; 
    push.thisSeg.start = start;
    push.parent.start = start;
    push.thisSeg.end = segs[0].end; 
    push.parent.end = segs[0].end;
    li.push_back(push); 
    closedList.associate(start, li); 

    set<pPair> openList; 
    openList.insert(make_pair(0, make_pair(start, push.thisSeg.end)));

    while (!openList.empty())
    {
        pPair cur = *openList.begin(); 

        //Remove this f/Geocoord pairing from the open list
        openList.erase(openList.begin()); 

        //Add this to the closed list. 
        Node newNo;
        StreetSegment seg;
        seg.start = cur.second.first; 
        seg.end = cur.second.second;
        newNo.thisSeg = seg;
        newNo.parent = seg;
        list<Node>* curNodes;
        curNodes = closedList.find(cur.second.first);
        //Create new association if you need to.
        //Otherwise, push it back to where it belongs. 
        if (curNodes == nullptr)
        {
            list<Node> newLi;
            
            newLi.push_back(newNo);
            closedList.associate(cur.second.first, newLi); 
            curNodes = closedList.find(cur.second.first); 
        }
        else
        {
            curNodes->push_back(newNo); 
        }

        Node* curNode = findNode(curNodes, newNo.thisSeg); //CURRENT SEGMENT

        //Find the successors
        //Possible successors are those whose starts points == current endpoint. 
        //OR those whose start points == current startpoint (and don't == this one) 
        vector<StreetSegment> endSuccessors;
        vector<StreetSegment> startSuccessors; 
        m_streetMap->getSegmentsThatStartWith(curNode->thisSeg.end, endSuccessors);
        m_streetMap->getSegmentsThatStartWith(curNode->thisSeg.start, startSuccessors); 
        auto it = startSuccessors.begin(); 
        while (it != startSuccessors.end())
        {
            //Remove the one that duplicates this one. 
            if (*it == curNode->thisSeg)
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

        //Store the 'g', 'h', and 'f' of successors 
        double gNew, hNew, fNew; 
        StreetSegment* lowest = nullptr; 
        //Keeping a pointer to the segment with the lowest f value, check this vector
        it = combined.begin(); 
        while (it != endSuccessors.end())
        {
            //This iterator's segment
            StreetSegment s; s.start = it->start; s.end = end;

            if (isDestination(it->start, end))
            {
                //Set the parent of the destination in the closed list
                list<Node>* ptr = closedList.find(it->start); 
                Node* destNode = findNode(ptr, s);
                destNode->parent = curNode->thisSeg; 
                route.clear();
                tracePath(closedList, s, route, totalDistanceTravelled); 
                return DELIVERY_SUCCESS; 
            }
            else 
            {
                list<Node>* ptr = closedList.find(it->start); 
                bool onClosed = false; 
                //Check if not on closed list
                if (ptr != nullptr)
                {
                    Node* checkPtr = findNode(ptr, s); 
                    if (checkPtr != nullptr)
                        onClosed = true; 
                }
                //If it's NOT on the closed list, then
                if (!onClosed)
                {
                    //Calculate the g, h, and f
                    //g: distance from Start of current segment to this segment
                    //h: distance from start of this segment to destination
                    gNew = distanceEarthMiles(curNode->thisSeg.start, it->start);
                    hNew = distanceEarthMiles(it->start, end); 
                    fNew = gNew + hNew; 
                  
                    //Create new Node and pPair
                    Node newToClosed;
                    list<Node> newList; 
                    newToClosed.thisSeg.start = it->start; 
                    newToClosed.thisSeg.end = it->end; 
                    newToClosed.f = fNew;
                    newToClosed.g = gNew; 
                    newToClosed.h = hNew; 
                    newToClosed.parent = curNode->thisSeg; 
                    newList.push_back(newToClosed); 
                    pPair findThis;

                    findThis.first = FLT_MAX;
                    findThis.second.first = newToClosed.thisSeg.start; 
                    findThis.second.second = newToClosed.thisSeg.end;
                    //If it isn't on the open list, add to open list 
                    //Or if it's on the list already, check to see if this path 
                    //Is better
                    auto it = openList.find(findThis); 
                    if (it != openList.end() || it->first > fNew)
                    {
                        openList.insert(make_pair(fNew, findThis.second));

                        //Update the details of this 
                        //Create an association if necessary
                        if (ptr == nullptr)
                            closedList.associate(it->second.first, newList);
                        else //Otherwise push the node onto where it belongs. Or update that node
                        {
                            Node* update;
                            if (update = findNode(ptr, newToClosed.thisSeg))
                            {
                                update->f = fNew; update->g = gNew; update->h = hNew; 
                                update->parent = curNode->thisSeg; 
                            }
                            else
                            ptr->push_back(newToClosed);
                        }
                    }
                }
            }
            ++it;

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
