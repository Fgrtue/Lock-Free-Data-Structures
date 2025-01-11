#include <atomic>
#include <memory>

/*
    Hazard pointers plan

    We have two methods:

    1. HPointers::Aquire

        1. Walk through the list of hazard pointers =>
        try to return the one that is free
        if no such => create a new one and push it in front of the
        list

    2. HPointers::Release
        - Just sets active and ptr to null

    3. Reclaimation functions

        1) Add to reclaim later
            -> simply add another node to the front of the list
        2) Delete nodes with no hazards
            -> exchange the node from the node list with null
            -> walk through the list
            -> delete nodes that we can delete
            -> save nodes that we cannot delete
*/

template<class T, class N>
class hazard_pointers {

    struct HP {

        HP(): ptr_(nullptr), active_(false), next_(nullptr) {}

        HP*                 next_;
        std::atomic<T*>     ptr_;
        std::atomic<bool>   active_;
    };

    HP* acquire_hazard();

    void release_hazard(HP*);

    struct node_recl {

        node_recl(N* data): data(data), next_(nullptr) {} 

        void delete_node(N* p) {
            delete std::static_cast<N*>(p);
        }

        N*   data_;
        node_recl*  next_;
    };

    void insert_reclaim(node_recl*);

    void reclaim_later(N* node);

    void delete_nodes_with_no_hazards();

    std::atomic<HP*> hazards_list_;
    std::atomic<node_recl*> reclamation_list_;
};

template<class T, class N>
typename hazard_pointers<T, N>::HP*
hazard_pointers<T, N>::acquire_hazard() {

    HP* ptr = hazards_list_.load();
    for(; ptr ; ptr = ptr->next_) {

        if (ptr->active_.compare_exchange_strong(false, true)) {
            return ptr;
        }
    }
    HP* hazard_new = new HP();
    hazard_new->active_.store(true)
    do {
        hazard_new->next_ = hazards_list_.load();
    } while (!hazards_list_.compare_exchange_strong(hazard_new->next_, hazard_new));
    return hazard_new;
}

template<class T, class N>
void hazard_pointers<T, N>::release_hazard(HP* hp) {
    hp->ptr_.store(nullptr);
    hp->active_.store(false);
}

template<class T, class N>
void hazard_pointers<T, N>::insert_reclaim(node_recl* reclaim_new) {

    reclaim_new->next_ = reclamation_list_.load();
    while (!reclamation_list_.compare_exchange_strong(reclaim_new->next_, reclaim_new));
}

template<class T, class N>
void hazard_pointers<T, N>::reclaim_later(N* node) {

    node_recl* reclaim_new = new (node_recl(node));
    insert_reclaim(reclaim_new);
}

template<class T, class N>
void hazard_pointers<T, N>::delete_nodes_with_no_hazards() {
    node_recl* list_ptr = reclamation_list_.exchange(nullptr);
    node_recl* next_list;
    while (list_ptr) {
        next_list = list_ptr->next_;
        if (in_hazard(list_ptr)) {
            insert_reclaim(list_ptr);
        } else {
            list_ptr->delete_node();
        }
        list_ptr = next_list;
    }
    
}

