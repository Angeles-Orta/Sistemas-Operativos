CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -D_GNU_SOURCE
LIBS = -lpthread
TARGET = process_thread_comparison
TARGET_PORTABLE = process_thread_comparison_portable
SOURCE = main.c
SOURCE_PORTABLE = main_portable.c
TEST_SCRIPT = test_suite.sh
ANALYSIS_SCRIPT = analyze_results.py

# Directorios
RESULTS_DIR = test_results
ANALYSIS_DIR = analysis_output

.PHONY: all clean test help install-deps setup portable
.PHONY: test-quick test-correctness test-performance test-scalability
.PHONY: test-processes test-threads test-all analyze
.PHONY: demo benchmark

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)
	@echo "✓ Compilación completada exitosamente"

# Versión portable para sistemas con problemas de compatibilidad
portable: $(TARGET_PORTABLE)

$(TARGET_PORTABLE): $(SOURCE_PORTABLE)
	$(CC) $(CFLAGS) -o $(TARGET_PORTABLE) $(SOURCE_PORTABLE) $(LIBS)
	@echo "✓ Versión portable compilada exitosamente"
	@echo "  Usa ./$(TARGET_PORTABLE) si tienes problemas con la versión principal"

# Intentar compilar ambas versiones
both: $(TARGET) $(TARGET_PORTABLE)

# Configuración inicial
setup: $(TARGET)
	@echo "=== Configuración del entorno de pruebas ==="
	chmod +x $(TEST_SCRIPT)
	chmod +x $(ANALYSIS_SCRIPT)
	mkdir -p $(RESULTS_DIR)
	mkdir -p $(ANALYSIS_DIR)
	@echo "✓ Entorno configurado correctamente"

# Limpieza
clean:
	rm -f $(TARGET) $(TARGET_PORTABLE)
	@echo "✓ Archivos de compilación eliminados"

clean-all: clean
	rm -rf $(RESULTS_DIR)
	rm -rf $(ANALYSIS_DIR)
	@echo "✓ Todos los archivos generados eliminados"

# === PRUEBAS INDIVIDUALES ===

# Prueba de demostración rápida (intenta versión principal, luego portable)
demo: 
	@if [ -f "$(TARGET)" ]; then \
		echo "=== DEMOSTRACIÓN RÁPIDA (versión principal) ==="; \
		./$(TARGET) -w 2 -i 50000 -r 1; \
	elif [ -f "$(TARGET_PORTABLE)" ]; then \
		echo "=== DEMOSTRACIÓN RÁPIDA (versión portable) ==="; \
		./$(TARGET_PORTABLE) -w 2 -i 50000 -r 1; \
	else \
		echo "❌ No hay ejecutable disponible. Ejecuta 'make' primero."; \
		exit 1; \
	fi

# Verificar qué versión usar
check-version:
	@if [ -f "$(TARGET)" ]; then \
		echo "Usando versión principal: $(TARGET)"; \
		PROGRAM="./$(TARGET)"; \
	elif [ -f "$(TARGET_PORTABLE)" ]; then \
		echo "Usando versión portable: $(TARGET_PORTABLE)"; \
		PROGRAM="./$(TARGET_PORTABLE)"; \
	else \
		echo "❌ No hay ejecutable disponible."; \
		echo "   Prueba: make        (versión principal)"; \
		echo "   O:     make portable (versión compatible)"; \
		exit 1; \
	fi

# Pruebas rápidas para verificación
test-quick: $(TARGET)
	@echo "=== PRUEBA RÁPIDA ==="
	./$(TARGET) -w 2 -i 100000 -r 1

# Pruebas de corrección (detectar race conditions)
test-correctness: $(TARGET)
	@echo "=== PRUEBA DE CORRECCIÓN ==="
	@echo "Verificando sincronización y detectando race conditions..."
	./$(TARGET) -w 4 -i 500000 -r 2

# Pruebas de rendimiento
test-performance: $(TARGET)
	@echo "=== PRUEBA DE RENDIMIENTO ==="
	@echo "Midiendo throughput y utilización de CPU..."
	./$(TARGET) -w 4 -i 2000000 -r 3

# Pruebas de escalabilidad
test-scalability: $(TARGET)
	@echo "=== PRUEBA DE ESCALABILIDAD ==="
	@echo "Analizando rendimiento con diferente número de trabajadores..."
	@for workers in 1 2 4 8; do \
		echo "--- $$workers trabajadores ---"; \
		./$(TARGET) -w $$workers -i 1000000 -r 1; \
	done

# Solo procesos
test-processes: $(TARGET)
	@echo "=== SOLO PROCESOS ==="
	./$(TARGET) -p -w 4 -i 1000000 -r 2

# Solo hilos
test-threads: $(TARGET)
	@echo "=== SOLO HILOS ==="
	./$(TARGET) -t -w 4 -i 1000000 -r 2

# Benchmark completo comparativo
benchmark: $(TARGET)
	@echo "=== BENCHMARK COMPARATIVO ==="
	@echo "Ejecutando comparación exhaustiva entre procesos y hilos..."
	@echo ""
	@echo "--- Configuración 1: 2 trabajadores, 1M incrementos ---"
	./$(TARGET) -w 2 -i 1000000 -r 3
	@echo ""
	@echo "--- Configuración 2: 4 trabajadores, 2M incrementos ---"
	./$(TARGET) -w 4 -i 2000000 -r 3
	@echo ""
	@echo "--- Configuración 3: 8 trabajadores, 1M incrementos ---"
	./$(TARGET) -w 8 -i 1000000 -r 3

# === SUITE DE PRUEBAS AUTOMATIZADA ===

# Suite completa de pruebas
test-all: $(TARGET) setup
	@echo "=== EJECUTANDO SUITE COMPLETA DE PRUEBAS ==="
	./$(TEST_SCRIPT)

# Suite básica (sin pruebas de estrés)
test-suite-basic: $(TARGET) setup
	@echo "=== SUITE BÁSICA DE PRUEBAS ==="
	./$(TEST_SCRIPT) --basic-only

# Suite rápida para CI/desarrollo
test-suite-quick: $(TARGET) setup
	@echo "=== SUITE RÁPIDA ==="
	./$(TEST_SCRIPT) --quick

# Suite con pruebas de estrés
test-suite-stress: $(TARGET) setup
	@echo "=== SUITE CON PRUEBAS DE ESTRÉS ==="
	@echo "⚠️  Advertencia: Estas pruebas pueden tomar mucho tiempo"
	./$(TEST_SCRIPT) --with-stress

# === ANÁLISIS DE RESULTADOS ===

# Buscar archivo de resultados más reciente
LATEST_RESULTS = $(shell find $(RESULTS_DIR) -name "results_*.txt" -type f -printf '%T@ %p\n' 2>/dev/null | sort -rn | head -1 | cut -d' ' -f2-)

# Análisis básico del último resultado
analyze: 
	@if [ -z "$(LATEST_RESULTS)" ]; then \
		echo "❌ No se encontraron archivos de resultados."; \
		echo "   Ejecuta 'make test-all' primero."; \
		exit 1; \
	fi
	@echo "=== ANÁLISIS DE RESULTADOS ==="
	@echo "Analizando: $(LATEST_RESULTS)"
	python3 $(ANALYSIS_SCRIPT) "$(LATEST_RESULTS)"

# Análisis completo con gráficos
analyze-full:
	@if [ -z "$(LATEST_RESULTS)" ]; then \
		echo "❌ No se encontraron archivos de resultados."; \
		echo "   Ejecuta 'make test-all' primero."; \
		exit 1; \
	fi
	@echo "=== ANÁLISIS COMPLETO CON VISUALIZACIONES ==="
	python3 $(ANALYSIS_SCRIPT) "$(LATEST_RESULTS)" --plots --csv --report --output-dir $(ANALYSIS_DIR)

# Análisis de archivo específico
analyze-file:
	@if [ -z "$(FILE)" ]; then \
		echo "Uso: make analyze-file FILE=path/to/results.txt"; \
		exit 1; \
	fi
	python3 $(ANALYSIS_SCRIPT) "$(FILE)" --plots --csv --report --output-dir $(ANALYSIS_DIR)

# === UTILIDADES ===

# Mostrar resultados disponibles
list-results:
	@echo "=== ARCHIVOS DE RESULTADOS DISPONIBLES ==="
	@if [ -d "$(RESULTS_DIR)" ] && [ -n "$$(ls -A $(RESULTS_DIR) 2>/dev/null)" ]; then \
		ls -lht $(RESULTS_DIR)/*.txt 2>/dev/null | head -10; \
	else \
		echo "No hay archivos de resultados disponibles."; \
		echo "Ejecuta 'make test-all' para generar resultados."; \
	fi

# Resumen rápido del último resultado
quick-summary:
	@if [ -z "$(LATEST_RESULTS)" ]; then \
		echo "❌ No hay resultados disponibles."; \
		exit 1; \
	fi
	@echo "=== RESUMEN RÁPIDO ==="
	@echo "Archivo: $(LATEST_RESULTS)"
	@echo ""
	@echo "Throughput encontrados:"
	@grep "Throughput:" "$(LATEST_RESULTS)" | head -10
	@echo ""
	@echo "Resultados incorrectos (race conditions):"
	@grep -c "¿Resultado correcto?: NO" "$(LATEST_RESULTS)" 2>/dev/null || echo "0"

# Verificar estado del sistema
system-info:
	@echo "=== INFORMACIÓN DEL SISTEMA ==="
	@echo "Sistema operativo: $$(uname -a)"
	@echo "Compilador: $$($(CC) --version | head -1)"
	@echo "CPUs disponibles: $$(nproc)"
	@echo "Memoria total: $$(free -h | awk '/^Mem:/{print $$2}')"
	@echo "Memoria disponible: $$(free -h | awk '/^Mem:/{print $$7}')"
	@echo "Límites de procesos: $$(ulimit -u)"
	@echo "Límites de memoria: $$(ulimit -v) KB"

# === INSTALACIÓN DE DEPENDENCIAS ===

# Instalar dependencias del sistema (Ubuntu/Debian)
install-deps-apt:
	@echo "=== INSTALANDO DEPENDENCIAS (Ubuntu/Debian) ==="
	sudo apt-get update
	sudo apt-get install -y build-essential python3 python3-pip
	pip3 install matplotlib numpy || echo "⚠️  Matplotlib opcional no instalado"

# Instalar dependencias del sistema (CentOS/RHEL)
install-deps-yum:
	@echo "=== INSTALANDO DEPENDENCIAS (CentOS/RHEL) ==="
	sudo yum groupinstall -y "Development Tools"
	sudo yum install -y python3 python3-pip
	pip3 install matplotlib numpy || echo "⚠️  Matplotlib opcional no instalado"

# Detectar sistema e instalar dependencias automáticamente
install-deps:
	@if command -v apt-get >/dev/null 2>&1; then \
		$(MAKE) install-deps-apt; \
	elif command -v yum >/dev/null 2>&1; then \
		$(MAKE) install-deps-yum; \
	else \
		echo "❌ Sistema no soportado para instalación automática"; \
		echo "   Instala manualmente: build-essential, python3, python3-pip"; \
		echo "   Opcional: pip3 install matplotlib numpy"; \
	fi

# === AYUDA ===

help:
	@echo "=== SISTEMA DE PRUEBAS PROCESOS vs HILOS ==="
	@echo ""
	@echo "COMPILACIÓN:"
	@echo "  all              - Compilar el programa"
	@echo "  setup            - Configurar entorno de pruebas"
	@echo "  clean            - Limpiar archivos de compilación"
	@echo "  clean-all        - Limpiar todo (incluye resultados)"
	@echo ""
	@echo "PRUEBAS INDIVIDUALES:"
	@echo "  demo             - Demostración rápida"
	@echo "  test-quick       - Prueba rápida de verificación"
	@echo "  test-correctness - Verificar corrección y race conditions"
	@echo "  test-performance - Medir rendimiento"
	@echo "  test-scalability - Analizar escalabilidad"
	@echo "  test-processes   - Solo pruebas de procesos"
	@echo "  test-threads     - Solo pruebas de hilos"
	@echo "  benchmark        - Benchmark comparativo completo"
	@echo ""
	@echo "SUITES DE PRUEBAS:"
	@echo "  test-all         - Suite completa de pruebas"
	@echo "  test-suite-basic - Suite básica (sin estrés)"
	@echo "  test-suite-quick - Suite rápida para desarrollo"
	@echo "  test-suite-stress- Suite con pruebas de estrés"
	@echo ""
	@echo "ANÁLISIS:"
	@echo "  analyze          - Análisis básico del último resultado"
	@echo "  analyze-full     - Análisis completo con gráficos"
	@echo "  analyze-file FILE=path - Analizar archivo específico"
	@echo "  list-results     - Listar archivos de resultados"
	@echo "  quick-summary    - Resumen rápido del último resultado"
	@echo ""
	@echo "UTILIDADES:"
	@echo "  system-info      - Información del sistema"
	@echo "  install-deps     - Instalar dependencias del sistema"
	@echo "  help             - Mostrar esta ayuda"
	@echo ""
	@echo "EJEMPLOS DE USO:"
	@echo "  make demo                    # Demostración rápida"
	@echo "  make test-all                # Suite completa"
	@echo "  make analyze-full            # Análisis con gráficos"
	@echo "  make benchmark               # Benchmark detallado"
	@echo ""
	@echo "FLUJO TÍPICO:"
	@echo "  1. make setup                # Configurar entorno"
	@echo "  2. make test-all             # Ejecutar pruebas"
	@echo "  3. make analyze-full         # Analizar resultados"

# Target por defecto para mostrar ayuda si no se especifica nada
.DEFAULT_GOAL := help