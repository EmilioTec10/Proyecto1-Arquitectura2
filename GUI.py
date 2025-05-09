import tkinter as tk
from tkinter import ttk, messagebox
import os
import subprocess

CARPETA_INSTRUCCIONES = "Instrucciones"
NUM_PES = 8
BACKEND_EXECUTABLE = "./main"  # Ajustar si cambia

class EditorWindow(tk.Toplevel):
    def __init__(self, master):
        super().__init__(master)
        self.title("Editor de instrucciones por PE")
        self.geometry("800x600")
        self.text_widgets = []

        self.notebook = ttk.Notebook(self)
        for i in range(NUM_PES):
            frame = ttk.Frame(self.notebook)
            self.notebook.add(frame, text=f"PE{i}")
            text = tk.Text(frame, wrap="none", width=80, height=25)
            text.pack(fill="both", expand=True)
            self.text_widgets.append(text)
            self.cargar_archivo(i)

        self.notebook.pack(expand=True, fill="both")

        frame_botones = tk.Frame(self)
        frame_botones.pack(pady=10)

        tk.Button(frame_botones, text="💾 Guardar", command=self.guardar_todos).pack(side=tk.LEFT, padx=5)
        tk.Button(frame_botones, text="↩️ Volver", command=self.destroy).pack(side=tk.LEFT, padx=5)

    def ruta_archivo(self, pe_id):
        return os.path.join(CARPETA_INSTRUCCIONES, f"pe{pe_id}.txt")

    def cargar_archivo(self, pe_id):
        try:
            with open(self.ruta_archivo(pe_id), "r") as f:
                contenido = f.read()
                self.text_widgets[pe_id].insert("1.0", contenido)
        except FileNotFoundError:
            messagebox.showwarning("Archivo no encontrado", f"No se encontró pe{pe_id}.txt")

    def guardar_todos(self):
        for i in range(NUM_PES):
            contenido = self.text_widgets[i].get("1.0", tk.END)
            try:
                with open(self.ruta_archivo(i), "w") as f:
                    f.write(contenido.strip() + "\n")
            except Exception as e:
                messagebox.showerror("Error", f"No se pudo guardar pe{i}.txt:\n{e}")
        messagebox.showinfo("Éxito", "Todos los archivos fueron guardados.")

class MainWindow:
    def __init__(self, root):
        self.root = root
        self.root.title("Simulador Interconnect MP")
        self.root.geometry("500x250")

        tk.Label(root, text="Proyecto Interconnect MP", font=("Arial", 16)).pack(pady=10)

        tk.Button(root, text="🚀 Ejecutar simulación", width=25, command=self.ejecutar_simulacion).pack(pady=10)
        tk.Button(root, text="🛠️ Editar instrucciones de los PEs", width=25, command=self.abrir_editor).pack(pady=10)

    def ejecutar_simulacion(self):
        if not os.path.exists(BACKEND_EXECUTABLE):
            messagebox.showerror("Error", f"No se encuentra el ejecutable:\n{BACKEND_EXECUTABLE}")
            return
        #esto de abajo captura el subproceso de la interfaz 
        result = subprocess.run([BACKEND_EXECUTABLE], capture_output=True, text=True)
        if result.returncode == 0:
            messagebox.showinfo("Simulación completada", "El backend se ejecutó correctamente.")
            print("Salida estándar del backend:")
            print(result.stdout)
        else:
            messagebox.showerror("Error de simulación", result.stderr)
            print("Errores (si los hubo):")
            print(result.stderr)


    def abrir_editor(self):
        EditorWindow(self.root)

# Lanzar la aplicación
if __name__ == "__main__":
    root = tk.Tk()
    app = MainWindow(root)
    root.mainloop()