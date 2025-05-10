
# Proyecto 1: Simulador MP con Interconnect

**Curso:** Arquitectura en Computadoras 2  
**Estudiantes:**  
- Mariana Rojas Rojas  
- Luis Alfredo González Sánchez  
- Emilio Alfaro Mayorga

---

## Descripción

Este proyecto implementa un **simulador de un sistema multiprocesador (MP) con interconexión (Interconnect)**, donde múltiples elementos de procesamiento (PEs) se comunican a través de un bus compartido. Además, integra una interfaz gráfica en Python para facilitar la interacción con el simulador, permitiendo la edición de instrucciones, la ejecución directa o por pasos, y la visualización de estadísticas del sistema.

---

## Requisitos del sistema

- **Sistema operativo:** Linux (requisito indispensable)
- **Lenguajes de programación:**  
  - C++ (compilador recomendado: `g++`)  
  - Python 3

---

## Descarga e instalación

1. **Descargue el proyecto:**  
   Obtenga el archivo `proyecto1.zip` y descomprímalo en su computadora.

2. **Verifique las dependencias:**  
   Asegúrese de tener los compiladores y bibliotecas necesarios instalados.

---

## Instalación de bibliotecas necesarias

### Para C++

El proyecto utiliza las siguientes bibliotecas estándar de C++ (no requieren instalación adicional si su compilador está correctamente instalado):

```cpp
#include <iostream>
#include <vector>
#include <thread>
#include <sstream>  
#include <chrono>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <atomic>
#include <set>
#include <unordered_set>
```

> **Nota:** Estas son todas parte de la biblioteca estándar de C++ (`libstdc++`).

### Para Python

El proyecto requiere las siguientes bibliotecas de Python:

- `pandas`
- `re` (incluida en la biblioteca estándar)
- `plotly`
- `tkinter` (normalmente viene preinstalada con Python)
- `os` (biblioteca estándar)
- `subprocess` (biblioteca estándar)

#### Instalación con pip

Ejecute este comando para instalar las bibliotecas adicionales necesarias:

```bash
pip install pandas plotly
```

> ⚠️ **Nota:** `tkinter` suele estar disponible por defecto. Si no está instalada, puede instalarla en Ubuntu con:

```bash
sudo apt-get install python3-tk
```

---

## Ejecución del proyecto

### 1️⃣ Compilar el simulador

Abra una terminal en la carpeta donde descomprimió el proyecto y ejecute el siguiente comando para compilar:

```bash
g++ -Wall -Wextra -g3 main.cpp memory.cpp interconnect.cpp PEs.cpp Node.cpp evento.cpp evento_q.cpp cache.cpp -o output/main
```

Luego, ejecute el programa:

```bash
./output/main
```

---

### 2️⃣ Ejecutar la interfaz gráfica

Puede abrir el archivo `GUI.py` desde su IDE preferido o ejecutarlo desde la terminal con:

```bash
python3 GUI.py
```

> ✅ **Recomendación:** Asegúrese de que su terminal se encuentre en la ruta donde está ubicado `GUI.py`.

---

## Opciones de la interfaz gráfica

Cuando abra la interfaz gráfica, verá un menú con 3 opciones principales:

1️⃣ **Ejecutar simulación:**  
   Ejecuta la simulación completa del sistema MP con Interconnect de forma continua.

2️⃣ **Editar instrucciones de los PEs:**  
   Le permite acceder a las instrucciones de los diferentes PEs (elementos de procesamiento), modificarlas y guardarlas para su posterior ejecución.

3️⃣ **Stepping:**  
   Permite ejecutar el simulador paso a paso. Esta opción muestra información detallada sobre los ciclos y los bytes procesados hasta el momento. Incluye un botón para avanzar al siguiente paso y una ventana emergente que indica la finalización de la ejecución.

---

## Archivos generados

Una vez ejecutada la simulación (ya sea completa o en modo Stepping), se generan archivos útiles para analizar la ejecución:

- `message_times.txt`:  
  Contiene estadísticas detalladas por instrucción, tales como:
  - Número de ciclos para procesar cada instrucción
  - Bytes transferidos en el bus para cada instrucción
  - Número del PE que generó la instrucción
  - Fracción de **bandwidth** utilizada por la instrucción

- `steps.txt`:  
  Guarda la ejecución por pasos, mostrando los bytes asociados a cada ciclo específico.

---

## Visualización de resultados

Para visualizar los gráficos generados a partir de la simulación:

1. Ejecute el archivo `graficos.py` desde su IDE o con el siguiente comando:

```bash
python3 graficos.py
```

2. Se generarán 3 gráficos principales:
   - **Tráfico de datos:** Representa el tráfico total en el bus.
   - **Estrés del bandwidth:** Muestra cómo se ha utilizado el ancho de banda del sistema.
   - **Fracción de bandwidth por instrucción:** Visualiza el impacto individual de cada instrucción sobre el bandwidth.

---

## Notas finales

- Este proyecto está diseñado para sistemas Linux y **no ha sido probado en otros sistemas operativos.**
- Asegúrese de tener las versiones actualizadas de `g++` y `Python 3` para evitar problemas de compatibilidad.
- Si encuentra algún problema al ejecutar el proyecto o durante la compilación, verifique las dependencias y la versión de su sistema operativo.

---

¡Gracias por utilizar nuestro simulador MP con Interconnect 🚀!
