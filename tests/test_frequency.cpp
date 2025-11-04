#include "trie.hpp"
#include <cassert>
#include <iostream>

int main() {
    Trie<FrequencyPolicy> T;

    T.insert("car");
    T.insert("cat");
    T.insert("dog");

    auto v_cat_end  = T.descend_prefix("cat");
    auto term_cat   = T.descend(v_cat_end, '$');
    auto v_car_end  = T.descend_prefix("car");
    auto term_car   = T.descend(v_car_end, '$');
    assert(term_cat && term_cat->is_terminal);
    assert(term_car && term_car->is_terminal);

    T.update_priority(term_cat);
    T.update_priority(term_cat);
    T.update_priority(term_car);

    auto v_c = T.descend_prefix("c");
    auto a   = T.autocomplete(v_c);
    assert(a && a->is_terminal && *(a->str) == "cat");
    std::cout << "Frequency OK\n";
}
