#include "trie.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <filesystem>


using namespace std;
using namespace std::chrono;

// Estructura para almacenar resultados de memoria
struct MemoryResult {
    size_t words_inserted;
    size_t chars_inserted;
    size_t node_count;
    double nodes_per_char;
};

// Estructura para almacenar resultados de tiempo
struct TimeResult {
    size_t words_inserted;
    size_t chars_in_batch;
    double time_ms;
    double time_per_char_ms;
};

// Estructura para resultados de autocompletado
struct AutocompleteResult {
    size_t words_processed;
    size_t total_chars_in_text;
    size_t chars_typed;
    double percentage;
};

/**
 * @brief Carga todas las palabras de un archivo de texto (una por línea).
 * @param path Ruta del archivo.
 * @return Vector con todas las palabras cargadas.
 */
vector<string> read_words(const string& filename) {
    vector<string> words;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: no se pudo abrir " << filename << endl;
        return words;
    }
    
    string word;
    while (file >> word) {
        words.push_back(word);
    }
    file.close();
    return words;
}

// Cuenta caracteres totales en un vector de palabras
size_t count_chars(const vector<string>& words, size_t start, size_t end) {
    size_t total = 0;
    for (size_t i = start; i < end && i < words.size(); ++i) {
        total += words[i].length();
    }
    return total;
}

/**
 * Realiza el experimento de memoria midiendo node_count
 * en potencias de 2.
 * @param words: vector de palabras del dataset.
 * @return vector de resultados (palabras, caracteres, nodos, ratio).
 */
template<typename Policy>
vector<MemoryResult> experiment_memory(const vector<string>& words) {
    cout << "Iniciando experimento de memoria con política " << Policy::name() << "..." << endl;
    
    Trie<Policy> trie;
    vector<MemoryResult> results;
    size_t N = words.size();
    size_t total_chars = 0;
    
    // Puntos de medición: 2^0, 2^1, ..., 2^17, N
    vector<size_t> checkpoints;
    for (int i = 0; i <= 17; ++i) {
        checkpoints.push_back(1 << i);
    }
    checkpoints.push_back(N);
    
    size_t next_checkpoint_idx = 0;
    
    for (size_t i = 0; i < N; ++i) {
        trie.insert(words[i]);
        total_chars += words[i].length();
        
        size_t current_count = i + 1;
        if (next_checkpoint_idx < checkpoints.size() && 
            current_count == checkpoints[next_checkpoint_idx]) {
            
            MemoryResult res;
            res.words_inserted = current_count;
            res.chars_inserted = total_chars;
            res.node_count = trie.node_count();
            res.nodes_per_char = static_cast<double>(res.node_count) / res.chars_inserted;
            
            results.push_back(res);
            cout << "  Checkpoint " << current_count << ": " 
                 << res.node_count << " nodos, "
                 << res.nodes_per_char << " nodos/char" << endl;
            
            ++next_checkpoint_idx;
        }
    }
    
    return results;
}

// Experimento 2: Tiempo de inserción
/**
 * @brief Mide el tiempo de inserción normalizado por carácter.
 * @tparam Policy Política de prioridad (FrequencyPolicy o RecentPolicy).
 * @param words Vector con todas las palabras del dataset base.
 * @param output Ruta del CSV donde guardar los resultados.
 * @details Divide las inserciones en M=16 bloques y mide tiempo por bloque.
 */
template<typename Policy>
vector<TimeResult> experiment_time(const vector<string>& words, int M = 16) {
    cout << "Iniciando experimento de tiempo con política " << Policy::name() << "..." << endl;
    
    Trie<Policy> trie;
    vector<TimeResult> results;
    size_t N = words.size();
    size_t batch_size = N / M;
    
    for (int batch = 0; batch < M; ++batch) {
        size_t start = batch * batch_size;
        size_t end = (batch == M - 1) ? N : start + batch_size;
        
        size_t chars_in_batch = count_chars(words, start, end);
        
        auto t_start = high_resolution_clock::now();
        
        for (size_t i = start; i < end; ++i) {
            trie.insert(words[i]);
        }
        
        auto t_end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t_end - t_start);
        double time_ms = duration.count() / 1000.0;
        
        TimeResult res;
        res.words_inserted = end;
        res.chars_in_batch = chars_in_batch;
        res.time_ms = time_ms;
        res.time_per_char_ms = time_ms / chars_in_batch;
        
        results.push_back(res);
        cout << "  Batch " << (batch + 1) << "/" << M << ": " 
             << time_ms << " ms, "
             << res.time_per_char_ms << " ms/char" << endl;
    }
    
    return results;
}

// Experimento 3: Autocompletado
/**
 * @brief Simula la escritura palabra a palabra para evaluar autocompletado.
 * @tparam Policy Política de prioridad (FrequencyPolicy o RecentPolicy).
 * @param dict Palabras del diccionario base.
 * @param text Palabras del texto simulado (Wikipedia o aleatorio).
 * @param output Ruta del CSV donde guardar los resultados.
 * @details Reproduce el proceso descrito en el enunciado sección 4.3:
 * descender, autocompletar y actualizar prioridad en cada palabra.
 */
template<typename Policy>
vector<AutocompleteResult> experiment_autocomplete(
    Trie<Policy>& trie, 
    const vector<string>& text_words) {
    
    cout << "Iniciando experimento de autocompletado con política " 
         << Policy::name() << "..." << endl;
    
    vector<AutocompleteResult> results;
    size_t L = text_words.size();
    
    // Puntos de medición: 2^0, 2^1, ..., 2^21, L
    vector<size_t> checkpoints;
    for (int i = 0; i <= 21; ++i) {
        size_t cp = 1ULL << i;
        if (cp <= L) checkpoints.push_back(cp);
    }
    if (checkpoints.empty() || checkpoints.back() != L) {
        checkpoints.push_back(L);
    }
    
    size_t next_checkpoint_idx = 0;
    size_t chars_typed = 0;
    size_t total_chars = 0;
    
    for (size_t i = 0; i < L; ++i) {
        const string& w = text_words[i];
        total_chars += w.length();
        
        // Simulamos la escritura
        auto* v = trie.root();
        size_t descends = 0;
        bool found = false;
        
        for (size_t j = 0; j < w.length(); ++j) {
            v = trie.descend(v, w[j]);
            ++descends;
            
            if (!v) {
                // Palabra no está en el trie
                chars_typed += w.length();
                break;
            }
            
            // Intentamos autocompletar
            auto* completion = trie.autocomplete(v);
            if (completion && completion->str) {
                if (*completion->str == w) {
                    // ¡Autocompletado exitoso!
                    chars_typed += descends;
                    found = true;
                    break;
                }
            }
        }
        
        if (!found && v) {
            // Llegamos al final sin autocompletar
            chars_typed += w.length();
        }
        
        // Actualizamos prioridad si la palabra está en el trie
        if (v) {
            auto* terminal = trie.descend(v, '$');
            if (terminal && terminal->is_terminal) {
                trie.update_priority(terminal);
            }
        }
        
        // Guardamos checkpoint
        size_t current_count = i + 1;
        if (next_checkpoint_idx < checkpoints.size() && 
            current_count == checkpoints[next_checkpoint_idx]) {
            
            AutocompleteResult res;
            res.words_processed = current_count;
            res.total_chars_in_text = total_chars;
            res.chars_typed = chars_typed;
            res.percentage = (total_chars > 0) 
                ? (100.0 * chars_typed / total_chars) 
                : 0.0;
            
            results.push_back(res);
            cout << "  Checkpoint " << current_count << ": " 
                 << res.percentage << "% caracteres escritos" << endl;
            
            ++next_checkpoint_idx;
        }
    }
    
    return results;
}

// Guarda resultados de memoria a CSV
void save_memory_results(const string& filename, 
                        const vector<MemoryResult>& results) {
    ofstream file("out/" + filename);
    file << "words_inserted,chars_inserted,node_count,nodes_per_char\n";
    for (const auto& r : results) {
        file << r.words_inserted << "," 
             << r.chars_inserted << "," 
             << r.node_count << ","
             << r.nodes_per_char << "\n";
    }
    file.close();
    cout << "Resultados de memoria guardados en " << filename << endl;
}

// Guarda resultados de tiempo a CSV
void save_time_results(const string& filename, 
                      const vector<TimeResult>& results) {
    ofstream file("out/" + filename);
    file << "words_inserted,chars_in_batch,time_ms,time_per_char_ms\n";
    for (const auto& r : results) {
        file << r.words_inserted << "," 
             << r.chars_in_batch << "," 
             << r.time_ms << ","
             << r.time_per_char_ms << "\n";
    }
    file.close();
    cout << "Resultados de tiempo guardados en " << filename << endl;
}

// Guarda resultados de autocompletado a CSV
void save_autocomplete_results(const string& filename, 
                              const vector<AutocompleteResult>& results) {
    ofstream file("out/" + filename);
    file << "words_processed,total_chars,chars_typed,percentage\n";
    for (const auto& r : results) {
        file << r.words_processed << "," 
             << r.total_chars_in_text << "," 
             << r.chars_typed << ","
             << r.percentage << "\n";
    }
    file.close();
    cout << "Resultados de autocompletado guardados en " << filename << endl;
}

/**
 * @brief Función principal: ejecuta los tres experimentos y genera resultados en /out.
 *
 * - Inserta palabras del diccionario para medir memoria y tiempo.
 * - Construye los Tries (frecuencia y recencia).
 * - Ejecuta la simulación de autocompletado sobre tres datasets:
 *   Wikipedia, random y random_with_distribution.
 */
int main() {
    std::filesystem::create_directories("out");
    cout << "=== EXPERIMENTACIÓN TRIE ===" << endl;
    cout << fixed << setprecision(6);
    
    // Cargar dataset de palabras
    cout << "\nCargando words.txt..." << endl;
    vector<string> words = read_words("datos/words.txt");

    if (words.empty()) {
        cerr << "Error: no se pudieron cargar las palabras" << endl;
        return 1;
    }
    cout << "Cargadas " << words.size() << " palabras" << endl;
    
    // --- EXPERIMENTO 1: MEMORIA ---
    cout << "\n=== EXPERIMENTO 1: MEMORIA ===" << endl;
    auto mem_freq = experiment_memory<FrequencyPolicy>(words);
    save_memory_results("memory_frequency.csv", mem_freq);
    
    // --- EXPERIMENTO 2: TIEMPO ---
    cout << "\n=== EXPERIMENTO 2: TIEMPO ===" << endl;
    auto time_freq = experiment_time<FrequencyPolicy>(words);
    save_time_results("time_frequency.csv", time_freq);
    
    // --- EXPERIMENTO 3: AUTOCOMPLETADO ---
    cout << "\n=== EXPERIMENTO 3: AUTOCOMPLETADO ===" << endl;
    
    // Construir trie para frecuencia
    cout << "\nConstruyendo trie con política de frecuencia..." << endl;
    Trie<FrequencyPolicy> trie_freq;
    for (const auto& w : words) {
        trie_freq.insert(w);
    }
    
    // Construir trie para reciente
    cout << "Construyendo trie con política reciente..." << endl;
    Trie<RecentPolicy> trie_recent;
    for (const auto& w : words) {
        trie_recent.insert(w);
    }
    
    // Datasets de texto
    vector<string> datasets = {
        "datos/wikipedia.txt", 
        "datos/random.txt", 
        "datos/random_with_distribution.txt"
    };
    
    for (const auto& dataset : datasets) {
        cout << "\n--- Dataset: " << dataset << " ---" << endl;
        
        vector<string> text_words = read_words(dataset);
        if (text_words.empty()) {
            cerr << "Error: no se pudo cargar " << dataset << endl;
            continue;
        }
        cout << "Cargadas " << text_words.size() << " palabras de texto" << endl;
        
        // Frecuencia
        auto start_time = high_resolution_clock::now();
        auto results_freq = experiment_autocomplete(trie_freq, text_words);
        auto end_time = high_resolution_clock::now();
        auto duration_freq = duration_cast<milliseconds>(end_time - start_time);
        
        string base_name = std::filesystem::path(dataset).stem().string();

        save_autocomplete_results(
            "autocomplete_frequency_" + base_name + ".csv", 
            results_freq
        );
        cout << "Tiempo total (frecuencia): " << duration_freq.count() << " ms" << endl;
        
        // Reciente
        start_time = high_resolution_clock::now();
        auto results_recent = experiment_autocomplete(trie_recent, text_words);
        end_time = high_resolution_clock::now();
        auto duration_recent = duration_cast<milliseconds>(end_time - start_time);
        
        save_autocomplete_results(
            "autocomplete_recent_" + base_name + ".csv", 
            results_recent
        );
        cout << "Tiempo total (reciente): " << duration_recent.count() << " ms" << endl;
    }
    
    cout << "\n=== EXPERIMENTACIÓN COMPLETADA ===" << endl;
    return 0;
}