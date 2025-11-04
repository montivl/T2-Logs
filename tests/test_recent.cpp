#include "trie.hpp"
#include <cassert>
#include <iostream>

int main() {
    Trie<RecentPolicy> T;

    T.insert("car");
    T.insert("cat");
    T.insert("dog");

    auto v_c = T.descend_prefix("c");
    auto v_car_end = T.descend_prefix("car");            // nodo en 'r'
    auto v_term_car = T.descend(v_car_end, '$');         // terminal de "car"
    assert(v_term_car && v_term_car->is_terminal);

    T.update_priority(v_term_car); // "car" usado más recientemente

    auto a1 = T.autocomplete(v_c);
    assert(a1 && a1->is_terminal && *(a1->str) == "car");
    std::cout << "Recent OK\n";
}
