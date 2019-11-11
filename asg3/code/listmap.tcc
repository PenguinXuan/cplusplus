// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::~listmap() {
    while (not empty()) {
        erase(begin());
    }
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::insert (const value_type& pair) {
    listmap<key_t,mapped_t,less_t>::iterator curr;
    if (empty()) {
        anchor()->prev = anchor()->next = new node(anchor(), anchor(), pair);
        curr = begin();
    } else {
            if (find(pair.first) != end()) {
                curr = find(pair.first);
                curr->second = pair.second;
            } else {
                curr = begin();
                while (curr != end() && less(curr->first, pair.first)) {
                    ++curr;
                }
                node *tmp = new node(curr.where, curr.where->prev, pair);
                tmp->next->prev = tmp;
                tmp->prev->next = tmp;
                curr = tmp;
            }
    }
    return curr;
}

//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::find (const key_type& that) {
    listmap<key_t,mapped_t,less_t>::iterator curr = begin();
    while (curr != end() && curr->first != that) {
        ++curr;
    }
    if (curr->first == that) {
        return curr;
    }
    return end();
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::erase (iterator position) {
    listmap<key_t,mapped_t,less_t>::iterator res = position.where->next;
    position.where->prev->next = position.where->next;
    position.where->next->prev = position.where->prev;
    delete position.where;
    return res;
}
