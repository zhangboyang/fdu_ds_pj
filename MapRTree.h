#ifndef ZBY_MAPRTREE_H
#define ZBY_MAPRTREE_H
#include "common.h"
#include "MapRect.h"

#include <cmath>
#include <cstring>
#include <vector>

#ifdef DEBUG
//#define WITH_BRUTE_FORCE
#endif

#ifdef DEBUG
#include <cstdio>
#endif
#ifdef WITH_BRUTE_FORCE
#include <algorithm>
#endif

template <class TP>
class MapRTree {
    private:
    static const int Mhigh = 15;
    static const int Mlow = Mhigh / 2;
    class node {
        public:
        MapRect rect; // data-node should also have vaild rect
        int ch_cnt; // ch_cnt == -1, means this node is a data-node
              
        double enlargement(node *data) // compute enlargement if we add 'data' to this node
        {
            MapRect nrect = MapRect(rect, data->rect);
            assert(fcmp(nrect.area() - rect.area()) >= 0); // x87 need fcmp
            return nrect.area() - rect.area();
        }
    };
    
    class data_node : public node {
        public:
        TP data;
        
        data_node(TP obj) // data-node constructor
        {
            node::rect = obj->get_rect();
            node::ch_cnt = -1;
            data = obj;
        }
        
        #ifdef DEBUG
        void print()
        {
            printf("data_node %p:\n", this);
            printf(" rect=(%f, %f, %f, %f)\n", node::rect.left, node::rect.right, node::rect.bottom, node::rect.top);
            printf(" ch_cnt=%d\n", node::ch_cnt);
            printf(" data=%p\n", node::data);
        }
        #endif
    };

    class tree_node : public node {
        public:
        // although the type is tree_node, it might be point to a data node
        // since it's easy to write code to process data node specially
        tree_node *ch[Mhigh];
        
        #ifdef DEBUG
        void print()
        {
            printf("tree_node %p:\n", this);
            printf(" rect=(%f, %f, %f, %f)\n", node::rect.left, node::rect.right, node::rect.bottom, node::rect.top);
            printf(" ch_cnt=%d\n", node::ch_cnt);
            printf(" ch_list:");
            for (int i = 0; i < node::ch_cnt; i++) printf(" ch[%d]=%p", i, ch[i]);
            printf("\n");
        }
        #endif
        
        tree_node() {}
        
        tree_node(data_node *dnode) // create a leaf node, this node has one data-node
        {
            node::rect = dnode->rect;
            node::ch_cnt = 1;
            ch[0] = (tree_node *) dnode;
        }
        
        void update(tree_node *nd) // update rect using one node's rectangle
        {
            node::rect.merge(nd->rect);
        }
        
        void update() // update rect using all of our child's rectangles
        {
            node::rect = ch[0]->rect;
            for (int i = 1; i < node::ch_cnt; i++)
                node::rect.merge(ch[i]->rect);
        }

        #ifdef DEBUG
        void check_rect()
        {
            MapRect old_rect = node::rect;
            update();
            MapRect new_rect = node::rect;
            assert(fequ(new_rect.left, old_rect.left));
            assert(fequ(new_rect.right, old_rect.right));
            assert(fequ(new_rect.bottom, old_rect.bottom));
            assert(fequ(new_rect.top, old_rect.top));
        }
        #endif
        
        bool is_leaf()
        {
            // if our first children is data-node, this is leaf node
            assert(node::ch_cnt > 0);
            bool ret = ch[0]->ch_cnt < 0;
            #ifdef DEBUG
            for (int i = 0; i < node::ch_cnt; i++)
                assert(ret == (ch[i]->ch_cnt < 0));
            #endif
            return ret;
        }
        
        void split(tree_node *&nnode, tree_node *append) // can't be *&append, see adjust tree below
        {
            // if we add 'append', we would have Mhigh + 1 nodes.
            // so we need to split current node with to node 'nnode'
            assert(node::ch_cnt == Mhigh);
            
            nnode = new tree_node;
            tree_node **ch1 = ch, **ch2 = nnode->ch;
            tree_node *ch3[Mhigh + 1]; assert(sizeof(ch3) > sizeof(ch));
            memcpy(ch3, ch, sizeof(ch));
            ch3[Mhigh] = append;
            // split ch3 [0, Mhigh] to ch1[] and ch2[]
            
            #ifdef DEBUG
            for (int i = 0; i < Mhigh; i++)
                for (int j = i + 1; j <= Mhigh; j++)
                    assert(ch3[i] != ch3[j]);
            #endif
            
            double me = 0;
            int s1 = 0, s2 = 1; // two seeds
            for (int i = 0; i < Mhigh; i++)
                for (int j = i + 1; j <= Mhigh; j++) {
                    double e = MapRect(ch3[i]->rect, ch3[j]->rect).area()
                               - ch3[i]->rect.area() - ch3[j]->rect.area();
                    if (e >= me) { me = e; s1 = i; s2 = j; }
                }
            //printf("ch3:"); for (int i = 0; i <= Mhigh; i++) printf(" %p", ch3[i]); printf("\n");
            //printf("s1=%d s2=%d\n", s1, s2);
            
            int p1 = 1, p2 = 1, p3 = 2;
            MapRect g1, g2, ng1, ng2;

            ch1[0] = ch3[s1];
            ch2[0] = ch3[s2];
            g1 = ch1[0]->rect;
            g2 = ch2[0]->rect;
            
            assert(s1 < s2);
            if (s1 > 1) {
                ch3[s1] = ch3[0];
                ch3[s2] = ch3[1];
            } else if (s1 > 0) {
                ch3[s2] = ch3[0];
            } else if (s2 > 1) {
                ch3[s2] = ch3[1];
            }
            
            //printf("ch3:"); for (int i = 0; i <= Mhigh; i++) printf(" %p", ch3[i]); printf("\n");

            while (p3 <= Mhigh &&
                   p1 + (Mhigh - p3 + 1) > Mlow &&
                   p2 + (Mhigh - p3 + 1) > Mlow) {
                double md = 0;
                int c;
                for (int i = p3; i <= Mhigh; i++) {
                    double d = fabs((MapRect(g1, ch3[i]->rect).area() - g1.area()) - 
                                    (MapRect(g2, ch3[i]->rect).area() - g2.area()));
                    if (d >= md) { md = d; c = i; }
                    //printf("i=%d d=%f md=%f\n", i, d, md);
                }
                //printf("p1=%d p2=%d p3=%d c=%d\n", p1, p2, p3, c);
                ng1 = MapRect(g1, ch3[c]->rect);
                ng2 = MapRect(g2, ch3[c]->rect);
                if (ng1.area() - g1.area() < ng2.area() - g2.area()) {
                    ch1[p1++] = ch3[c];
                    g1 = ng1;
                } else {
                    ch2[p2++] = ch3[c];
                    g2 = ng2;
                }
                std::swap(ch3[p3++], ch3[c]);
            }
            
            if (p3 <= Mhigh) {
                if (p1 + (Mhigh - p3 + 1) == Mlow) {
                    assert(p2 + (Mhigh - p3 + 1) > Mlow);
                    while (p3 <= Mhigh) {
                        ch1[p1++] = ch3[p3];
                        g1.merge(ch3[p3++]->rect);
                    }
                } else {
                    assert(p2 + (Mhigh - p3 + 1) == Mlow);
                    assert(p1 + (Mhigh - p3 + 1) > Mlow);
                    while (p3 <= Mhigh) {
                        ch2[p2++] = ch3[p3];
                        g2.merge(ch3[p3++]->rect);
                    }
                }
            }
            
            node::rect = g1; node::ch_cnt = p1;
            nnode->rect = g2; nnode->ch_cnt = p2;
            assert(Mlow <= node::ch_cnt && node::ch_cnt <= Mhigh);
            assert(Mlow <= nnode->ch_cnt && nnode->ch_cnt <= Mhigh);
            assert(node::ch_cnt + nnode->ch_cnt == Mhigh + 1);
            #ifdef DEBUG
            //printf("ch1:"); for (int i = 0; i < p1; i++) printf(" %p", ch1[i]); printf("\n");
            //printf("ch2:"); for (int i = 0; i < p2; i++) printf(" %p", ch2[i]); printf("\n");
            for (int i = 0; i < p1; i++)
                for (int j = i + 1; j < p1; j++)
                    assert(ch1[i] != ch1[j]);
            for (int i = 0; i < p2; i++)
                for (int j = i + 1; j < p2; j++)
                    assert(ch2[i] != ch2[j]);
            check_rect();
            nnode->check_rect();
            #endif
        }
    };
    
    tree_node *root;
    #ifdef DEBUG
    int data_node_cnt;
    int tree_node_cnt;
    #endif
    
    // insert data-node 'data' to our tree 'root'
    // if split-node occured, new node is saved to 'nnode', otherwise 'nnode' is set to NULL
    void tree_insert(tree_node *root, tree_node *dnode, tree_node *&nnode)
    {
        //printf("insert root=%p dnode=%p data=%p\n", root, dnode, dnode->data);
        if (root->is_leaf()) { // if this is leaf node, do insert staff
            if (root->ch_cnt < Mhigh) {
                // we have room for our data
                root->ch[root->ch_cnt++] = dnode;
                root->update(dnode);
                nnode = NULL;
            } else {
                // we need to split this leaf node
                root->split(nnode, dnode);
                // split() should have nodes updated
            }
            
            #ifdef DEBUG
            root->check_rect();
            #endif
            return;
        }
        
        // find leaf
        std::vector<int> cidl; // choosen child's id
        cidl.push_back(0);
        double min_enlargement = root->ch[0]->enlargement(dnode);
        for (int i = 1; i < root->ch_cnt; i++) {
            double cur_enlargement = root->ch[i]->enlargement(dnode);
            if (cur_enlargement < min_enlargement) { // choose the smaller one
                cidl.clear();
                cidl.push_back(i);
                min_enlargement = cur_enlargement;
            } else if (cur_enlargement == min_enlargement) {
                cidl.push_back(i); // if same add to it's list;
            }
        }
        
        // choose smallest leaf from 'cid'
        assert(cidl.size() > 0);
        int cid = cidl.front();
        double min_area = root->ch[cid]->rect.area();
        for (std::vector<int>::iterator it = ++cidl.begin(); it != cidl.end(); it++) {
            double cur_area = root->ch[*it]->rect.area();
            if (cur_area < min_area) {
                cid = *it;
                min_area = cur_area;
            }
        }
        
        tree_insert(root->ch[cid], dnode, nnode);
        
        // adjust tree
        root->update(root->ch[cid]); // always update by child
        if (nnode) {      
            // split occured
            if (root->ch_cnt < Mhigh) {
                // we have room for newly created node
                root->ch[root->ch_cnt++] = nnode;
                root->update(nnode);
                nnode = NULL;

            } else {
                // we need to split this node inorder to add 'nnode'
                root->split(nnode, nnode); // it's ok to pass two same arguments to split()
                // split() should have nodes updated
            }
        }
        
        #ifdef DEBUG
        root->check_rect();
        #endif
    }
    
    void tree_insert(TP data)
    {
        // create data-node
        data_node *dnode = new data_node(data);
        if (root == NULL) {
            // if tree is empty, create a node whose child is dnode
            // can't let root = dnode directly, since dnode is a data-node, not normal-node
            root = new tree_node(dnode);
        } else {
            // tree is not empty, call tree_insert(?, ?, ?) to insert
            tree_node *nnode;
            tree_insert(root, (tree_node *) dnode, nnode); // do insert
            if (nnode) {
                // old root splited, we should create new root
                tree_node *old_root = root;
                root = new tree_node;
                root->rect = MapRect(old_root->rect, nnode->rect);
                root->ch_cnt = 2;
                root->ch[0] = old_root;
                root->ch[1] = nnode;
            }
        }
    }
    
    void tree_search(std::vector<TP> &result, tree_node *root, const MapRect &rect)
    {
        //root->print();
        assert(root->ch_cnt > 0);
        if (root->is_leaf()) {
            for (int i = 0; i < root->ch_cnt; i++) {
                assert(root->ch[i]->ch_cnt < 0);
                if (rect.intersect(root->ch[i]->rect))
                    result.push_back(((data_node *) root->ch[i])->data);
            }
            return;
        }
        
        for (int i = 0; i < root->ch_cnt; i++) {
            assert(root->ch[i]->ch_cnt > 0);
            if (rect.intersect(root->ch[i]->rect))
                tree_search(result, root->ch[i], rect);
        }
    }
    
    void tree_destruct(tree_node *root)
    {
        if (root->ch_cnt >= 0) {
            #ifdef DEBUG
            tree_node_cnt++;
            #endif
            for (int i = 0; i < root->ch_cnt; i++)
                tree_destruct(root->ch[i]);
            delete root;
        } else {
            #ifdef DEBUG
            data_node_cnt++;
            #endif
            delete (data_node *) root;
        }
    }
    
    // copy all r-tree rectanges to a vector
    // useful for learning r-tree internal details
    void tree_rect(std::vector<MapRect> &result, tree_node *root)
    {
        result.push_back(root->rect);
        for (int i = 0; i < root->ch_cnt; i++)
            tree_rect(result, root->ch[i]);
    }
    
    // make MapRTree non-copyable
    // copying a tree is very dangerous
    // especially when the original tree is destructed
    MapRTree(const MapRTree&);
    MapRTree& operator = (const MapRTree&);

    #ifdef WITH_BRUTE_FORCE
    std::vector<TP> bf_data;
    #endif
    public:
    
    MapRTree()
    {
        root = NULL;
        #ifdef DEBUG
        data_node_cnt = tree_node_cnt = 0;
        #endif
        assert(Mhigh > 1);
        assert(Mlow >= 1);
        assert(Mlow < Mhigh);
        assert((Mhigh + 1) / 2 >= Mlow);
    }
    
    ~MapRTree()
    {
        if (root) tree_destruct(root);
        // data_node_cnt and tree_node_cnt is caculated when destructing tree
        printd("destructing r-tree object %p, data_node=%d, tree_node=%d\n", this, data_node_cnt, tree_node_cnt);
    }
    
    
    
    // tree_ functions' wrapper, to verify with brute-force
    void insert(TP obj) {
        tree_insert(obj);
        #ifdef WITH_BRUTE_FORCE
        bf_data.push_back(obj);
        #endif
    }
    void find(std::vector<TP> &result, const MapRect &rect)
    {
        if (!root) return;
        #ifdef WITH_BRUTE_FORCE
        std::vector<TP> rt_result;
        tree_search(rt_result, root, rect);
        std::vector<TP> cp_result(rt_result);
        std::vector<TP> bf_result;
        for (typename std::vector<TP>::iterator it = bf_data.begin(); it != bf_data.end(); it++)
            if (rect.intersect((*it)->get_rect()))
                bf_result.push_back(*it);
        std::sort(rt_result.begin(), rt_result.end());
        std::sort(bf_result.begin(), bf_result.end());
        bool check_failed = false;
        if (rt_result.size() != bf_result.size()) check_failed = true;
        else {
            for (int i = 0; i < (int) rt_result.size(); i++)
                if (rt_result[i] != bf_result[i])
                    check_failed = true;
        }
        if (check_failed) {
            printf("rt_result=%lld, bf_result=%lld\n", (LL) rt_result.size(), (LL) bf_result.size());
            printf("r-tree result:\n");
            for (int i = 0; i < (int) rt_result.size(); i++)
                printf("%p ", rt_result[i]);
            printf("\n");
            printf("brute-force result:\n");
            for (int i = 0; i < (int) bf_result.size(); i++)
                printf("%p ", bf_result[i]);
            printf("\n");
            fail("r-tree bf check failed");
        }
        result.insert(result.end(), cp_result.begin(), cp_result.end());
        #else
        tree_search(result, root, rect);
        #endif
    }
    
    void get_all_tree_rect(std::vector<MapRect> &result)
    {
        if (root) tree_rect(result, root);
    }
};

#endif
