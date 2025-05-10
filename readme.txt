tome en cuenta que necesitoa una computadora con sistema operativo linux para la ejecucion de este proyecto. 
descargue el archivo proyecto1.zip que contiene el proyecto completo y descomprimalo en su computadora. 

verifique que tiene instalado los lenguajes de programacion c++ y python en su computadora. 

pasos para la ejecucion del proyecto. 

Paso 1 Intalar bibliotecas (verficar cuales ocupan instalacion)
#include <iostream>
#include <vector>
#include <thread>
#include <memory> // para std::unique_ptr
#include <sstream> // para construir nombres de archivo
#include <iostream>
#include <chrono>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <cstdint>
#include <map>
#include <atomic>
#include <iostream>
#include <fstream>
#include <chrono>
#include <fstream>
#include <set>
#include <unordered_set>   

de python (tambien verficar cuales necesitan instalacion)
import pandas as pd
import re
import plotly.express as px
import tkinter as tk
from tkinter import ttk, messagebox
import os
import subprocess

para esto utilice el comando pip install. 


posteriormente, 
abra una terminal donde esta el proyecto, luego ejecute los siguientes comandos: 

g++ -Wall -Wextra -g3 "main.cpp" "memory.cpp" "interconnect.cpp" "PEs.cpp" "Node.cpp" "evento.cpp" "evento_q.cpp" "cache.cpp" -o "output/main"
./output/main

luego, abra el archivo gui.py y ejecutelo, puede ser con el IDE o con el comando: 
/bin/python3 "ruta del archivo GUI.py"

al ejecutar este programa va a encontrarse con una pantalla con un menu con 3 opciones: 
Ejecutar simulacion: le permite ejecutar la simulacion del sistema MP con interconnect de golpe. 
Editar instrucciones de los PEs: le permite acceder a los diferentes PEs que tiene el sistema, y las instrucciones contenidas en estos. 
es importante destacar que es posible modificarlas y guardas para su posterior ejecucion dentro del simulador MP con interconnect. 
Stepping: permite observar la ejecucion por pasos del los PEs en el simulador MP con interconnect. Provee la informacion de Ciclos y bytes en el sistema hasta el momento y un boton 
para anzar hacia el siguiente step del sistema.Tambien una ventana energente que indica que el programa finaliza su ejecucion.  


Posterior a la ejecucion ya sea por "Ejecutar simulacion" o "Stepping" es posible recurrir a los archivos para almacenamiento de informacion de ejecucion del simulador. 
en el archivo message_times.txt puede observar las diferentes estadisticas entre las instrucciones
como por ejemplo la cantidad de ciclos que requirio procesar una instruccion , los bytes que pasaron
por el bus para esa instruccion , el # de pe que genero dicha instruccion y la fraccion de
bandwith que requiere dicha instruccion. Por otro lado , en steps.txt podra observar
la ejecucion por pasos del programa y los bytes asociados dado un punto de tiempo x, que 
para este caso seran ciclos.

Finalmente, ejecute graficos.py con el ide de su preferencia o mediante comandos
observara que se generaran 3 graficos, un grafico relacionado al trafico de datos
en el bus, otro grafico relacionado al estres del bandwidth y un ultimo grafico
representativo de la fraccion de bandwidth que representa la instruccion.
