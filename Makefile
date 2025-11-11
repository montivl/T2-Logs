CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Iinclude

# Ejecutable principal
TARGET = experimentos

# Directorios
SRC_DIR = src
INC_DIR = include

# Archivos fuente y cabeceras
SOURCES = $(SRC_DIR)/experimentos.cpp
HEADERS = $(INC_DIR)/trie.hpp

# Regla principal
all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Ejecutar experimentos
run: $(TARGET)
	./$(TARGET)

# Limpiar binarios y resultados
clean:
	rm -f $(TARGET) out/*.csv *.csv

# Limpiar solo resultados (sin borrar binario)
clean-results:
	rm -f out/*.csv *.csv

.PHONY: all run clean clean-results
