#include "trie.hpp"
#include <cassert>
#include <iostream>

int main() {
    {
        Trie<RecentPolicy> T;
        T.insert("cat");
        auto v_z = T.descend_prefix("z");
        assert(v_z == nullptr);
        std::cout << "[OK] Prefijo inexistente retorna nullptr\n";
    }

    {
        Trie<FrequencyPolicy> T;
        T.insert("apple");
        T.insert("apricot");

        auto v1 = T.descend_prefix("apple");
        auto t1 = T.descend(v1, '$');
        auto v2 = T.descend_prefix("apricot");
        auto t2 = T.descend(v2, '$');

        T.update_priority(t1);
        T.update_priority(t2);

        auto v_pref = T.descend_prefix("ap");
        auto a = T.autocomplete(v_pref);
        assert(a && a->is_terminal);
        std::cout << "[OK] Empate manejado con prioridad estable ("
                  << *(a->str) << ")\n";
    }

    {
        Trie<RecentPolicy> T;
        T.insert("dog");
        T.insert("door");
        T.insert("doom");

        auto t_dog  = T.descend(T.descend_prefix("dog"), '$');
        auto t_door = T.descend(T.descend_prefix("door"), '$');
        auto t_doom = T.descend(T.descend_prefix("doom"), '$');

        T.update_priority(t_dog);
        auto v_d = T.descend_prefix("do");
        auto a1 = T.autocomplete(v_d);
        assert(a1 && *(a1->str) == "dog");

        T.update_priority(t_door);
        auto a2 = T.autocomplete(v_d);
        assert(a2 && *(a2->str) == "door");

        T.update_priority(t_doom);
        auto a3 = T.autocomplete(v_d);
        assert(a3 && *(a3->str) == "doom");

        std::cout << "[OK] Variante 'reciente' cambia correctamente de sugerencia\n";
    }

    std::cout << "Extra tests OK\n";
}
