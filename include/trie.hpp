#pragma once
#include <array>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <deque>

// --- Políticas de prioridad -----------------------------------------------
// Frecuencia: priority = cantidad de accesos al nodo terminal
struct FrequencyPolicy {
    using Counter = uint64_t;
    static inline const char* name() { return "frequency"; }
    static void touch(Counter& node_priority, Counter& /*global_access_counter*/) {
        ++node_priority;
    }
};

// Reciente: priority = timestamp creciente (contador global de accesos)
struct RecentPolicy {
    using Counter = uint64_t;
    static inline const char* name() { return "recent"; }
    static void touch(Counter& node_priority, Counter& global_access_counter) {
        node_priority = ++global_access_counter;
    }
};

// --- Trie parametrizado por la Política ------------------------------------
template <typename PriorityPolicy>
class Trie {
public:
    using Counter = typename PriorityPolicy::Counter;

    struct Node {
        Node* parent = nullptr;
        // Σ = 27: 'a'..'z' y '$' como fin de palabra
        std::array<Node*, 27> next{};
        bool is_terminal = false;

        // Metadatos para autocompletar
        const std::string* str = nullptr;   // puntero al string de la palabra (si terminal)
        Counter priority = 0;               // prioridad del nodo terminal
        Node* best_terminal = nullptr;      // mejor terminal del subárbol
        Counter best_priority = 0;          // prioridad de ese mejor terminal

        Node() { next.fill(nullptr); }
    };

    Trie(): root_(new Node()), node_count_(1), global_access_counter_(0) {}

    ~Trie() { clear(root_); }

    // Inserta w carácter a carácter
    void insert(const std::string& w) {
        Node* v = root_;
        for (char ch : w) {
            int idx = char_to_index(ch);
            if (idx < 0) continue;
            if (!v->next[idx]) {
                v->next[idx] = new Node();
                v->next[idx]->parent = v;
                ++node_count_;
            }
            v = v->next[idx];
        }
        // Marca fin de palabra con '$'
        int end = end_index();
        if (!v->next[end]) {
            v->next[end] = new Node();
            v->next[end]->parent = v;
            ++node_count_;
        }
        Node* term = v->next[end];
        term->is_terminal = true;

        // Guardamos el string 
        strings_.emplace_back(w);
        term->str = &strings_.back();

        // Inicializamos prioridad en 0
        if (term->priority == 0) term->priority = 0;

        bubble_up(term);

    }

    // Retorna puntero al nodo tras descender por c desde v (o nullptr si no existe)
    Node* descend(Node* v, char c) const {
        if (!v) return nullptr;
        int idx = (c == '$') ? end_index() : char_to_index(c);
        if (idx < 0) return nullptr;
        return v->next[idx];
    }

    Node* autocomplete(Node* v) const {
        if (!v) return nullptr;
        return v->best_terminal;
    }

    // Actualiza prioridad de un nodo terminal y propaga best_* hacia la raíz
    void update_priority(Node* terminal) {
        assert(terminal && terminal->is_terminal);
        PriorityPolicy::touch(terminal->priority, global_access_counter_);
        bubble_up(terminal);
    }

    // Raíz
    Node* root() const { return root_; }

    // Cantidad de nodos (para medición de memoria pedida)
    size_t node_count() const { return node_count_; }

    // Utilidad: desciende por un string prefijo (sin forzar '$')
    Node* descend_prefix(const std::string& pref) const {
        Node* v = root_;
        for (char ch : pref) {
            v = descend(v, ch);
            if (!v) return nullptr;
        }
        return v;
    }

private:
    Node* root_;
    size_t node_count_;
    Counter global_access_counter_;
    // Guardamos strings en un contenedor 
    std::deque<std::string> strings_;

    static int end_index() { return 26; }

    static int char_to_index(char c) {
        if (c == '$') return end_index();
        if (std::isalpha(static_cast<unsigned char>(c))) {
            char x = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return (x - 'a');
        }
        return -1;
    }

    // Recalcula best_* de un nodo no necesariamente terminal
    static void recompute_best(Node* v) {
        // Mejor candidato: él mismo si terminal
        Node* best = (v->is_terminal ? v : nullptr);
        Counter bestp = (v->is_terminal ? v->priority : 0);

        // O alguno de sus hijos
        for (Node* u : v->next) {
            if (!u) continue;
            if (u->best_terminal && u->best_priority > bestp) {
                best = u->best_terminal;
                bestp = u->best_priority;
            }
        }
        v->best_terminal = best;
        v->best_priority = bestp;
    }

    // Sube actualizando best_* hasta que no mejore
    static void bubble_up(Node* from) {
        Node* v = from;
        while (v) {
            recompute_best(v);
            v = v->parent;
        }
    }


    static void clear(Node* v) {
        if (!v) return;
        for (Node* u : v->next) clear(u);
        delete v;
    }
};
