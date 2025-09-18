#!/bin/bash

# Script para generar datos de escalabilidad específicos para las preguntas
# Genera archivos CSV para crear gráficos

set -e

PROGRAM="./process_thread_comparison"
OUTPUT_DIR="scalability_results"
CSV_FILE="${OUTPUT_DIR}/scalability_data.csv"
DETAILED_LOG="${OUTPUT_DIR}/detailed_output.log"

# Verificar que el programa existe
if [ ! -f "$PROGRAM" ]; then
    echo "Error: $PROGRAM no encontrado. Compilando..."
    make clean && make
fi

# Crear directorio de resultados
mkdir -p "$OUTPUT_DIR"

echo "=== PRUEBA DE ESCALABILIDAD DETALLADA ==="
echo "Generando datos para gráficos y análisis..."
echo ""

# Inicializar archivo CSV
echo "test_type,sync_type,num_threads,execution_time_ms,throughput_ops_ms,cpu_utilization_percent,final_counter,expected_counter,is_correct" > "$CSV_FILE"

# Función para extraer métricas de la salida
extract_metrics() {
    local output="$1"
    local test_type="$2"
    local sync_type="$3"
    local num_workers="$4"
    local expected=$((num_workers * 500000))
    
    local execution_time=$(echo "$output" | grep "Tiempo de ejecución:" | sed 's/.*: \([0-9.]*\) ms/\1/')
    local throughput=$(echo "$output" | grep "Throughput:" | sed 's/.*: \([0-9.]*\) ops\/ms/\1/')
    local cpu_util=$(echo "$output" | grep "Utilización CPU:" | sed 's/.*: \([0-9.]*\)%/\1/')
    local final_counter=$(echo "$output" | grep "Contador final:" | sed 's/.*: \([0-9]*\)/\1/')
    local is_correct=$(echo "$output" | grep "¿Resultado correcto?: SÍ" > /dev/null && echo "true" || echo "false")
    
    # Escribir al CSV
    echo "${test_type},${sync_type},${num_workers},${execution_time},${throughput},${cpu_util},${final_counter},${expected},${is_correct}" >> "$CSV_FILE"
}

# Configuraciones de prueba
WORKERS_LIST="1 2 3 4 6 8 12 16 20"
INCREMENTS=500000

echo "Configuración:"
echo "- Incrementos por trabajador: $INCREMENTS"
echo "- Iteraciones por configuración: 1"
echo "- Trabajadores a probar: $WORKERS_LIST"
echo ""

for workers in $WORKERS_LIST; do
    echo ">>> Probando con $workers trabajadores <<<"
    
    # Procesos con sincronización
    echo "  - Procesos con sincronización..."
    output=$($PROGRAM -p -s -w $workers -i $INCREMENTS -r 1 2>&1)
    echo "$output" >> "$DETAILED_LOG"
    extract_metrics "$output" "PROCESOS" "CON_SYNC" "$workers"
    
    # Hilos con sincronización
    echo "  - Hilos con sincronización..."
    output=$($PROGRAM -t -s -w $workers -i $INCREMENTS -r 1 2>&1)
    echo "$output" >> "$DETAILED_LOG"
    extract_metrics "$output" "HILOS" "CON_SYNC" "$workers"
    
    # Solo para algunos casos, probar sin sincronización
    if [ $workers -le 8 ]; then
        echo "  - Procesos sin sincronización..."
        output=$($PROGRAM -p -n -w $workers -i $INCREMENTS -r 1 2>&1)
        echo "$output" >> "$DETAILED_LOG"
        extract_metrics "$output" "PROCESOS" "SIN_SYNC" "$workers"
        
        echo "  - Hilos sin sincronización..."
        output=$($PROGRAM -t -n -w $workers -i $INCREMENTS -r 1 2>&1)
        echo "$output" >> "$DETAILED_LOG"
        extract_metrics "$output" "HILOS" "SIN_SYNC" "$workers"
    fi
    
    echo ""
done

echo "=== PRUEBA COMPLETADA ==="
echo "Datos guardados en:"
echo "  - CSV: $CSV_FILE"
echo "  - Log detallado: $DETAILED_LOG"
echo ""


chmod +x "${OUTPUT_DIR}/plot_scalability.py"

echo "Para generar gráficos, ejecuta:"
echo "  cd $OUTPUT_DIR"
echo "  python3 plot_scalability.py"
echo ""

# Intentar generar gráficos automáticamente
echo "Intentando generar gráficos automáticamente..."
cd "$OUTPUT_DIR"
if python3 plot_scalability.py 2>/dev/null; then
    echo "✓ Gráficos generados exitosamente"
else
    echo "⚠ No se pudieron generar gráficos automáticamente"
    echo "  Instala dependencias: pip3 install pandas matplotlib"
    echo "  Luego ejecuta: cd $OUTPUT_DIR && python3 plot_scalability.py"
fi