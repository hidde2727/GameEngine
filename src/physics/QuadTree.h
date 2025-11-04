#ifndef ENGINE_PHYSICS_QUADTREE_H
#define ENGINE_PHYSICS_QUADTREE_H

#include "core/PCH.h"
#include "util/Vec2D.h"
#include "physics/AABB.h"

namespace Engine {
namespace Physics {

    typedef uint32_t NodeID;
    typedef uint32_t ChildID;

    // T is the data associated with a node
    // EFFICIENT_LOOKUP will make retrieving a node from it's ID faster (so not the querying)
    //      but it uses more memory
    template<class T, bool EFFICIENT_LOOKUP>
    class QuadTree {
    public:

        // All the objects insert inside(fully contained in) the specified aabb will have optimized queries
        // All the objects outside the aabb will have O(n) query time (n==amount of objects in tree)
        QuadTree(const AABB aabb) {
            Node root{};
            root._aabb = aabb;
            _nodes.push_back(root);
        }

        // Input true if you want continious IDs
        // Input false if you want to let the quadtree choose any arbitrary ID
        void SetContinuousIDs(const bool on) {
            _continuousChildIDs = on;
        }
        ChildID GetNextChildID() const {
            return _nextChildID;
        }
        // Insert some data that has a bounding box
        ChildID Insert(const T obj, const AABB aabb) {
            ChildID id = GetNextChildID();
            ASSERT(TryInsert(0, obj, aabb, id), "[Physics::QuadTree] Failed to insert node");// Insert at root node
            _nextChildID++;
            return id;
        }
        void Set(const ChildID id, const T obj, const AABB aabb) {
            TryRemove(0, id);
            TryInsert(0, obj, aabb, id);
        }
        T& Get(const ChildID id) {
        #if EFFICIENT_LOOKUP
            T* obj = TryGet(_childrenLocations[id], id);
            ASSERT(obj != nullptr, "[Physics::QuadTree] Failed to retrieve object, the id doesn't exist")
            return *obj;
        #else
            T* obj = TryGet(0, id);
            ASSERT(obj != nullptr, "[Physics::QuadTree] Failed to retrieve object, the id doesn't exist")
            return *obj;
        #endif
        }
        // Returns all the data with its bounding box inside the queried area
        std::vector<T> Query(const AABB aabb) const {
            std::vector<T> result;
            Query(0, aabb, result);// Query the root node
            return result;
        }
        // Removes the element with the id
        void Remove(const ChildID id) {
            #if EFFICIENT_LOOKUP
            TryRemove(_childrenLocations[id], id);
            #else
            TryRemove(0, id);
            #endif
        }

    private:
        struct Data {
            T _obj;
            AABB _aabb;
            ChildID _uuid;

            Data() = default;
            Data(const T obj, const AABB aabb, const ChildID uuid)
                    : _obj(obj), _aabb(aabb), _uuid(uuid) {}
            Data(const Data&) = default;
            Data(Data&&) noexcept = default;
            Data& operator=(const Data&) = default;
            Data& operator=(Data&&) noexcept = default;
        };
        struct Node {
            AABB _aabb;
            NodeID _nodes[4];
            std::vector<Data> _children;
            bool _leaf = true;

            Node() = default;
            Node(const AABB aabb, const NodeID nodes[4], const std::vector<Data>& children, bool leaf = true)
                    : _aabb(aabb), _nodes(nodes), _children(children), _leaf(leaf) {}
            Node(const Node&) = default;
            Node(Node&&) noexcept = default;
            Node& operator=(const Node&) = default;
            Node& operator=(Node&&) noexcept = default;
        };
        std::vector<Node> _nodes;
        std::priority_queue<NodeID> _emptyNodes;
        ChildID _nextChildID = 0;
        bool _continuousChildIDs = false;

        #if EFFICIENT_LOOKUP
        std::map<ChildID, NodeID> _childrenLocations;
        #endif


        inline NodeID CreateNode(const AABB aabb) {
            NodeID node = 0;
            if(_emptyNodes.size()) {
                node = _emptyNodes.top();
                _emptyNodes.pop();
            } else {
                _nodes.emplace_back();
                node = _nodes.size() - 1;
            }
            _nodes[node]._aabb = aabb;
            return node;
        }
        bool TryInsert(const NodeID atNode, const T& obj, const AABB& aabb, const ChildID uuid) {
            // Not using a reference to _nodes[atNode] because we add Nodes very often in this function
            //      so using a reference will create unexpected bugs (reference won't be valid after adding a Node)
            
            if(!aabb.ContainedIn(_nodes[atNode]._aabb)) {
                // If this is the root node, insert it here
                if(atNode == 0) {
                    Data newChild(obj, aabb, uuid);
                    _nodes[atNode]._children.push_back(newChild);
                    #if EFFICIENT_LOOKUP
                    _childrenLocations[uuid] = atNode;
                    #endif
                    return true;
                }
                return false;
            }
            if(_nodes[atNode]._leaf && _nodes[atNode]._children.size() == 4) {
                // Split the node
                _nodes[atNode]._leaf = false;
                Util::Vec2F dimensions = _nodes[atNode]._aabb.GetHalfDimensions();
                Util::Vec2F newDimensions = _nodes[atNode]._aabb.GetHalfDimensions()/2;
                Util::Vec2F middle = _nodes[atNode]._aabb.GetMiddle();
                _nodes[atNode]._nodes[0] = CreateNode(AABB::FromMiddleAndDimensions(
                    Util::Vec2F(middle.x - newDimensions.x, middle.y - newDimensions.y), newDimensions
                ));
                _nodes[atNode]._nodes[1] = CreateNode(AABB::FromMiddleAndDimensions(
                    Util::Vec2F(middle.x + newDimensions.x, middle.y - newDimensions.y), newDimensions
                ));
                _nodes[atNode]._nodes[2] = CreateNode(AABB::FromMiddleAndDimensions(
                    Util::Vec2F(middle.x + newDimensions.x, middle.y + newDimensions.y), newDimensions
                ));
                _nodes[atNode]._nodes[3] = CreateNode(AABB::FromMiddleAndDimensions(
                    Util::Vec2F(middle.x - newDimensions.x, middle.y + newDimensions.y), newDimensions
                ));
                // Set the reference because _nodes may be reallocated
                _nodes[atNode] = _nodes[atNode];
                // Distribute the children
                std::vector<Data> newChildren;
                for(const Data& child : _nodes[atNode]._children) {
                    if(TryInsert(_nodes[atNode]._nodes[0], child._obj, child._aabb, child._uuid)) continue;
                    if(TryInsert(_nodes[atNode]._nodes[1], child._obj, child._aabb, child._uuid)) continue;
                    if(TryInsert(_nodes[atNode]._nodes[2], child._obj, child._aabb, child._uuid)) continue;
                    if(TryInsert(_nodes[atNode]._nodes[3], child._obj, child._aabb, child._uuid)) continue;
                    newChildren.push_back(child);
                }
                _nodes[atNode]._children = newChildren;
                // Continue like this isn't a leaf
            } else if(_nodes[atNode]._leaf) {
                // There is enough space in this node, so insert in here
                Data newChild(obj, aabb, uuid);
                _nodes[atNode]._children.push_back(newChild);
                #if EFFICIENT_LOOKUP
                _childrenLocations[uuid] = atNode;
                #endif
                return true;
            } 
            // TryInsert to child nodes
            if(TryInsert(_nodes[atNode]._nodes[0], obj, aabb, uuid)) return true;
            if(TryInsert(_nodes[atNode]._nodes[1], obj, aabb, uuid)) return true;
            if(TryInsert(_nodes[atNode]._nodes[2], obj, aabb, uuid)) return true;
            if(TryInsert(_nodes[atNode]._nodes[3], obj, aabb, uuid)) return true;
            Data newChild(obj, aabb, uuid);
            _nodes[atNode]._children.push_back(newChild);
            #if EFFICIENT_LOOKUP
            _childrenLocations[uuid] = atNode;
            #endif
            return true;
        }
        void Query(const NodeID atNode, const AABB& aabb, std::vector<T>& result) const {
            if(!aabb.HasOverlap(_nodes[atNode]._aabb)) {
                // If this is the root node, return all the root nodes children
                if(atNode == 0) {
                    for(const Data& data : _nodes[atNode]._children) {
                        if(data._aabb.HasOverlap(aabb)) result.push_back(data._obj);
                    }
                }
                return;
            }
            // Check our children
            for(const Data& data : _nodes[atNode]._children) {
                if(data._aabb.HasOverlap(aabb)) result.push_back(data._obj);
            }
            if(_nodes[atNode]._leaf) return;
            // Check our branches
            Query(_nodes[atNode]._nodes[0], aabb, result);
            Query(_nodes[atNode]._nodes[1], aabb, result);
            Query(_nodes[atNode]._nodes[2], aabb, result);
            Query(_nodes[atNode]._nodes[3], aabb, result);
        }
        // TODO, add logic to remove leafs that aren't in use
        bool TryRemove(const NodeID atNode, const ChildID id) {
            Node& self = _nodes[atNode];
            // Check our children
            int i = 0;
            int deleteIndex = -1;
            for(const Data& data : self._children) {
                if(data._uuid == id) {
                    // Found it
                    deleteIndex = i;
                    break;
                }
                i++;
            }
            if(deleteIndex != -1) {
                self._children.erase(self._children.begin() + deleteIndex);
                return true;
            }
            // Propagate through child nodes (if this isn't a leaf)
            if(self._leaf) return false;
            if(TryRemove(self._nodes[0], id)) return true;
            if(TryRemove(self._nodes[1], id)) return true;
            if(TryRemove(self._nodes[2], id)) return true;
            if(TryRemove(self._nodes[3], id)) return true;
            return false;
        }

        T* TryGet(const NodeID atNode, const ChildID id) {
            Node& self = _nodes[atNode];
            for(Data& data : self._children) {
                if(data._uuid == id) {
                    // Found it
                    return &data._obj;
                }
            }

            // Propagate through child nodes (if this isn't a leaf)
            if(self._leaf) return nullptr;
            T* obj = TryGet(self._nodes[0], id);
            if(obj) return obj;
            obj = TryGet(self._nodes[1], id);
            if(obj) return obj;
            obj = TryGet(self._nodes[2], id);
            if(obj) return obj;
            obj = TryGet(self._nodes[3], id);
            if(obj) return obj;
            return nullptr;
        }
    };

}
}

#endif