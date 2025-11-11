# T2-Logs

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

## Ejecución
1. Clonar el repositorio.
2. Ubicarse en la carpeta raíz `T2-Logs/`.
3. Ejecutar:
  ```bash
  make clean
  make run
  ```
4. Los resultados se guardarán en out/.


