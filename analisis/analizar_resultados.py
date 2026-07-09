"""Analisis de resultados experimentales - Tarea 3 Estructuras de Datos.

Lee results/results_tiempos.csv y results/results_memoria.csv, calcula el
promedio y la desviacion estandar del tiempo de ejecucion sobre las 20
repeticiones (results/results_tiempos_resumen.csv) y genera graficos
comparando las 5 estructuras de datos: tiempo y memoria vs. cantidad de
tweets procesados, uno por cada clave (user_id, user_screen_name).

Uso: python analisis/analizar_resultados.py
"""

import os

import matplotlib.pyplot as plt
import pandas as pd

RUTA_BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RUTA_RESULTADOS = os.path.join(RUTA_BASE, "results")
RUTA_PLOTS = os.path.join(RUTA_RESULTADOS, "plots")

# Orden fijo de estructuras y su color asociado. El orden ES el mecanismo de
# seguridad para daltonismo (validado con validate_palette.js): no reordenar
# ni reciclar colores entre estructuras.
ESTRUCTURAS = [
    "hash_abierto",
    "hash_cerrado_linear",
    "hash_cerrado_quadratic",
    "hash_cerrado_double",
    "unordered_map",
]
COLORES = {
    "hash_abierto": "#2a78d6",
    "hash_cerrado_linear": "#1baf7a",
    "hash_cerrado_quadratic": "#eda100",
    "hash_cerrado_double": "#008300",
    "unordered_map": "#4a3aa7",
}
ETIQUETAS = {
    "hash_abierto": "Hash abierto",
    "hash_cerrado_linear": "Hash cerrado (linear)",
    "hash_cerrado_quadratic": "Hash cerrado (quadratic)",
    "hash_cerrado_double": "Hash cerrado (double)",
    "unordered_map": "unordered_map (STL)",
}
# Estilo de linea por estructura, ademas del color. Necesario porque las 3
# variantes de hashing cerrado usan memoria IDENTICA en todos los checkpoints
# (el probing cambia como se buscan celdas, no cuantas se reservan): sus
# lineas quedan exactamente superpuestas y solo se veria la ultima dibujada.
# Con patrones de rayado distintos, los trazos de una llenan los espacios de
# la otra y las tres se distinguen. Tambien sirve como codificacion
# secundaria de identidad (no depender solo del color).
ESTILOS_LINEA = {
    "hash_abierto": "solid",
    "hash_cerrado_linear": (0, (6, 3)),        # rayas largas
    "hash_cerrado_quadratic": (0, (1, 1.5)),   # puntos
    "hash_cerrado_double": (0, (4, 2, 1, 2)),  # raya-punto
    "unordered_map": "solid",
}
CLAVES = {"user_id": "user_id", "screen_name": "user_screen_name"}

COLOR_SUPERFICIE = "#fcfcfb"
COLOR_TEXTO = "#0b0b0b"
COLOR_TEXTO_SECUNDARIO = "#52514e"
COLOR_GRILLA = "#e1e0d9"
COLOR_EJE = "#c3c2b7"


def calcular_resumen_tiempos(df_tiempos):
    """Promedio y desviacion estandar de tiempo_ejecucion sobre las 20
    repeticiones crudas, por (estructura, clave, checkpoint). Sin filtrado de
    outliers: el enunciado pide reportar promedio/desv. estandar tal cual, y
    advierte que una desviacion estandar alta hace el promedio poco
    confiable — eso se resuelve controlando el entorno de medicion (cerrar
    programas, correr en un momento sin uso del computador), no filtrando
    datos despues de medir."""
    return (
        df_tiempos.groupby(["estructura_de_datos", "clave", "cantidad_consultas"])[
            "tiempo_ejecucion"
        ]
        .agg(tiempo_promedio="mean", tiempo_desviacion_estandar="std",
             repeticiones_usadas="count")
        .reset_index()
    )


def graficar_metrica(df, columna_valor, columna_error, clave, titulo, ylabel,
                     nombre_archivo, escala=1.0):
    """Grafico de lineas: una linea por estructura, columna_valor vs.
    cantidad_consultas, filtrando por clave. Si columna_error no es None, se
    dibuja una banda sombreada de +/- 1 desviacion estandar."""
    fig, ax = plt.subplots(figsize=(8, 5), facecolor=COLOR_SUPERFICIE)
    ax.set_facecolor(COLOR_SUPERFICIE)

    subset_clave = df[df["clave"] == clave]
    for estructura in ESTRUCTURAS:
        datos = subset_clave[
            subset_clave["estructura_de_datos"] == estructura
        ].sort_values("cantidad_consultas")
        if datos.empty:
            continue
        x = datos["cantidad_consultas"]
        y = datos[columna_valor] * escala
        color = COLORES[estructura]
        ax.plot(x, y, color=color, linewidth=2,
                linestyle=ESTILOS_LINEA[estructura],
                label=ETIQUETAS[estructura])
        if columna_error is not None:
            err = datos[columna_error] * escala
            ax.fill_between(x, y - err, y + err, color=color, alpha=0.12,
                           linewidth=0)

    ax.set_title(titulo, color=COLOR_TEXTO, fontsize=13, loc="left")
    ax.set_xlabel("Cantidad de tweets procesados", color=COLOR_TEXTO_SECUNDARIO)
    ax.set_ylabel(ylabel, color=COLOR_TEXTO_SECUNDARIO)
    ax.grid(True, color=COLOR_GRILLA, linewidth=0.8)
    ax.set_axisbelow(True)
    for spine in ("top", "right"):
        ax.spines[spine].set_visible(False)
    for spine in ("left", "bottom"):
        ax.spines[spine].set_color(COLOR_EJE)
    ax.tick_params(colors=COLOR_TEXTO_SECUNDARIO)
    ax.legend(frameon=False, labelcolor=COLOR_TEXTO)

    fig.tight_layout()
    fig.savefig(os.path.join(RUTA_PLOTS, nombre_archivo), dpi=150)
    plt.close(fig)


def main():
    os.makedirs(RUTA_PLOTS, exist_ok=True)

    df_tiempos = pd.read_csv(os.path.join(RUTA_RESULTADOS, "results_tiempos.csv"), sep=";")
    df_memoria = pd.read_csv(os.path.join(RUTA_RESULTADOS, "results_memoria.csv"), sep=";")

    resumen = calcular_resumen_tiempos(df_tiempos)
    ruta_resumen = os.path.join(RUTA_RESULTADOS, "results_tiempos_resumen.csv")
    resumen.to_csv(ruta_resumen, index=False, sep=";")
    print(f"Resumen de tiempos (promedio/desv. estandar) escrito en {ruta_resumen}")

    for clave, nombre_clave in CLAVES.items():
        graficar_metrica(
            resumen, "tiempo_promedio", "tiempo_desviacion_estandar", clave,
            titulo=f"Tiempo de conteo vs. cantidad de tweets ({nombre_clave})",
            ylabel="Tiempo promedio (s)",
            nombre_archivo=f"tiempo_{clave}.png",
        )
        graficar_metrica(
            df_memoria, "memoria_bytes", None, clave,
            titulo=f"Memoria usada vs. cantidad de tweets ({nombre_clave})",
            ylabel="Memoria (KB)",
            nombre_archivo=f"memoria_{clave}.png",
            escala=1.0 / 1024.0,
        )

    print(f"Graficos guardados en {RUTA_PLOTS}")


if __name__ == "__main__":
    main()
