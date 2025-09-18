import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

# Verificar que matplotlib está disponible
try:
    import matplotlib.pyplot as plt
    import pandas as pd
except ImportError as e:
    print(f"Error: {e}")
    print("Instala las dependencias: pip3 install pandas matplotlib")
    sys.exit(1)

# Leer datos
try:
    df = pd.read_csv('scalability_data.csv')
except FileNotFoundError:
    print("Error: No se encontró scalability_data.csv")
    print("Ejecuta ./scalability_test.sh primero")
    sys.exit(1)

# Configurar estilo
plt.style.use('seaborn-v0_8')
plt.rcParams['figure.figsize'] = (12, 8)

# 1. Gráfico principal: Tiempo de ejecución vs número de hilos/procesos
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))

# Filtrar datos con sincronización
sync_data = df[df['sync_type'] == 'CON_SYNC']

# Separar procesos e hilos
procesos = sync_data[sync_data['test_type'] == 'PROCESOS']
hilos = sync_data[sync_data['test_type'] == 'HILOS']

# Gráfico 1: Tiempo de ejecución vs número de trabajadores
ax1.plot(procesos['num_threads'], procesos['execution_time_ms'], 
         'o-', label='Procesos', linewidth=2, markersize=6)
ax1.plot(hilos['num_threads'], hilos['execution_time_ms'], 
         's-', label='Hilos', linewidth=2, markersize=6)
ax1.set_xlabel('Número de Trabajadores')
ax1.set_ylabel('Tiempo de Ejecución (ms)')
ax1.set_title('Tiempo de Ejecución vs Número de Trabajadores')
ax1.legend()
ax1.grid(True, alpha=0.3)

# Gráfico 2: Throughput vs número de trabajadores
ax2.plot(procesos['num_threads'], procesos['throughput_ops_ms'], 
         'o-', label='Procesos', linewidth=2, markersize=6, color='blue')
ax2.plot(hilos['num_threads'], hilos['throughput_ops_ms'], 
         's-', label='Hilos', linewidth=2, markersize=6, color='red')
ax2.set_xlabel('Número de Trabajadores')
ax2.set_ylabel('Throughput (ops/ms)')
ax2.set_title('Throughput vs Número de Trabajadores')
ax2.legend()
ax2.grid(True, alpha=0.3)

# Gráfico 3: Utilización CPU
ax3.plot(procesos['num_threads'], procesos['cpu_utilization_percent'], 
         'o-', label='Procesos', linewidth=2, markersize=6)
ax3.plot(hilos['num_threads'], hilos['cpu_utilization_percent'], 
         's-', label='Hilos', linewidth=2, markersize=6)
ax3.set_xlabel('Número de Trabajadores')
ax3.set_ylabel('Utilización CPU (%)')
ax3.set_title('Utilización CPU vs Número de Trabajadores')
ax3.legend()
ax3.grid(True, alpha=0.3)
ax3.axhline(y=100, color='gray', linestyle='--', alpha=0.7, label='100% (1 CPU)')

# Gráfico 4: Speedup (aceleración)
if len(procesos) > 0 and len(hilos) > 0:
    baseline_proc = procesos[procesos['num_threads'] == 1]['execution_time_ms'].iloc[0]
    baseline_hilo = hilos[hilos['num_threads'] == 1]['execution_time_ms'].iloc[0]
    
    speedup_proc = baseline_proc / procesos['execution_time_ms']
    speedup_hilo = baseline_hilo / hilos['execution_time_ms']
    
    ax4.plot(procesos['num_threads'], speedup_proc, 'o-', label='Procesos', linewidth=2, markersize=6)
    ax4.plot(hilos['num_threads'], speedup_hilo, 's-', label='Hilos', linewidth=2, markersize=6)
    
    # Línea de speedup ideal (lineal)
    max_workers = max(df['num_threads'])
    ax4.plot([1, max_workers], [1, max_workers], 'k--', alpha=0.5, label='Speedup Ideal')

ax4.set_xlabel('Número de Trabajadores')
ax4.set_ylabel('Speedup (factor de aceleración)')
ax4.set_title('Speedup vs Número de Trabajadores')
ax4.legend()
ax4.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('scalability_analysis.png', dpi=300, bbox_inches='tight')
print("✓ Gráfico guardado como: scalability_analysis.png")

# 5. Gráfico específico solicitado: Tiempo vs Hilos
plt.figure(figsize=(10, 6))
plt.plot(hilos['num_threads'], hilos['execution_time_ms'], 
         's-', label='Hilos con Sincronización', linewidth=3, markersize=8, color='red')
plt.xlabel('Número de Hilos', fontsize=12)
plt.ylabel('Tiempo de Ejecución (ms)', fontsize=12)
plt.title('Tiempo de Ejecución vs Número de Hilos', fontsize=14, fontweight='bold')
plt.grid(True, alpha=0.3)
plt.legend(fontsize=11)

# Anotar algunos puntos importantes
for i, (x, y) in enumerate(zip(hilos['num_threads'], hilos['execution_time_ms'])):
    if i % 2 == 0:  # Anotar cada segundo punto para no saturar
        plt.annotate(f'{y:.0f}ms', (x, y), textcoords="offset points", 
                    xytext=(0,10), ha='center', fontsize=9)

plt.tight_layout()
plt.savefig('tiempo_vs_hilos.png', dpi=300, bbox_inches='tight')
print("✓ Gráfico específico guardado como: tiempo_vs_hilos.png")

# Mostrar estadísticas
print("\n=== ESTADÍSTICAS DE ESCALABILIDAD ===")
print("HILOS:")
print(f"  Mejor tiempo: {hilos['execution_time_ms'].min():.2f} ms con {hilos.loc[hilos['execution_time_ms'].idxmin(), 'num_threads']} hilos")
print(f"  Mejor throughput: {hilos['throughput_ops_ms'].max():.2f} ops/ms con {hilos.loc[hilos['throughput_ops_ms'].idxmax(), 'num_threads']} hilos")

print("\nPROCESOS:")
print(f"  Mejor tiempo: {procesos['execution_time_ms'].min():.2f} ms con {procesos.loc[procesos['execution_time_ms'].idxmin(), 'num_threads']} hilos")
print(f"  Mejor throughput: {procesos['throughput_ops_ms'].max():.2f} ops/ms con {procesos.loc[procesos['throughput_ops_ms'].idxmax(), 'num_threads']} hilos")

# Análisis de race conditions si hay datos
no_sync_data = df[df['sync_type'] == 'SIN_SYNC']
if len(no_sync_data) > 0:
    print(f"\nRACE CONDITIONS DETECTADAS:")
    incorrect = no_sync_data[no_sync_data['is_correct'] == False]
    print(f"  Casos incorrectos: {len(incorrect)} de {len(no_sync_data)}")
    if len(incorrect) > 0:
        avg_loss = ((incorrect['expected_counter'] - incorrect['final_counter']) / incorrect['expected_counter'] * 100).mean()
        print(f"  Pérdida promedio de datos: {avg_loss:.1f}%")

