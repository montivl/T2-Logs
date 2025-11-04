# T2-Logs

## Build
make

## Run
./test_recent
./test_frequency

## Qué hace
- `insert(w)`: inserta `w` letra a letra y crea un nodo terminal con '$'.
- `descend(v,c)`: baja desde `v` por `c` (o `nullptr`).
- `autocomplete(v)`: retorna el `best_terminal` del subárbol de `v`.
- `update_priority(v)`: actualiza prioridad del terminal `v` según la variante
  (reciente o frecuencia) y propaga `best_*` hacia la raíz.

## Notas de enunciado
- Σ = 27 (26 letras + `$`). `next` es arreglo fijo de punteros.  
- Medir `node_count` a medida que insertas (para memoria).  
- Dos variantes: más reciente (timestamp) y más frecuente (contador).  
