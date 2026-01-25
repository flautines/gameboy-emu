# Nombre del compilador
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -pedantic
# -Wall -Wextra: Activa todas las advertencias
# -Iinclude: Busca archivos .h en nuestra carpeta include
# -g3: Incluye información cmpleta pra el depurador GDB

ifdef DEBUG
	CFLAGS += -g3 -Og -DDEBUG -Werror -Wfatal-errors
else
	CFLAGS += -O3 -DNDEBUG
endif

# Librerías para Raylib (Linux)
#LDFLAGS = -lraylib -lm -lpthread

# 1. Separamos el main del resto de la lógica
SRC_CORE = $(filter-out src/main.c, $(wildcard src/*.c))
OBJ_CORE = $(SRC_CORE:src/%.c=build/%.o)
OBJ_MAIN = build/main.o

ifdef TEST
# Añadimos un flag para que el código sepa qué test correr
    # Convierte 'cpu' en -DTEST_CPU
    TEST_FLAG = -DTEST_$(shell echo $(TEST) | tr '[:lower:]' '[:upper:]')
    CFLAGS += $(TEST_FLAG)
endif

TARGET = gameboy-emu

all: $(TARGET)

# Añadimos $(TEST_OBJ) a las d

# Cómo crear el ejecutable final
$(TARGET): $(OBJ_CORE) $(OBJ_MAIN)
	$(CC) $(OBJ_CORE) $(OBJ_MAIN) -o $(TARGET) $(LDFLAGS)

# Test
test: CFLAGS += -DMODO_TEST
# Añade librería cJson para el test de cpu
ifeq ($(TEST),cpu)
	LDFLAGS += -lcjson
endif
test: build/test_$(TEST).o $(OBJ_CORE) $(OBJ_MAIN)
	$(CC) $(OBJ_CORE) $(OBJ_MAIN) build/test_$(TEST).o -o $(TARGET)_test $(LDFLAGS)
	@echo "Binario de test listo: ./$(TARGET)_test"

# Cómo compilar cada archivo .c a .o
build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/test_%.o: tests/test_%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Limpia el proyecto
clean:
	rm -fr build $(TARGET) $(TARGET)_test