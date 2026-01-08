# Nombre del compilador
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -pedantic
# -Wall -Wextra: Activa todas las advertencias
# -Iinclude: Busca archivos .h en nuestra carpeta include
# -g3: Incluye información cmpleta pra el depurador GDB

ifdef DEBUG
	CFLAGS += -g3 -Og -DDEBUG -Werror
else
	CFLAGS += -O3 -DNDEBUG
endif

# Librerías para Raylib (Linux)
#LDFLAGS = -lraylib -lm -lpthread

# Archivos fuente y destino
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = gameboy-emu

# Regla principal
all: $(TARGET)

# Cómo crear el ejecutable final
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Cómo compilar cada archivo .c a .o
build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Limpia el proyecto
clean:
	rm -fr build $(TARGET)