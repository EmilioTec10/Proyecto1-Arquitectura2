import pandas as pd
import re
import plotly.express as px

# Leer el archivo
with open("message_times.txt", "r") as f:
    lines = f.readlines()

# Regex para extraer los datos
pattern = r"Instr \(PE (\d+) - ([A-Z_]+)\).*Bytes: (\d+), Metrica BW: (\d+)"
data = []

for i, line in enumerate(lines):
    match = re.search(pattern, line)
    if match:
        pe_id = int(match.group(1))
        instr_type = match.group(2)
        bytes_ = int(match.group(3))
        bw = int(match.group(4))
        label = f"#{i:02d} PE{pe_id}-{instr_type}"  # etiqueta única y ordenada
        data.append((label, pe_id, instr_type, bytes_, bw))

# Crear DataFrame
df = pd.DataFrame(data, columns=["Label", "PE", "Tipo", "Bytes", "BW"])

# === Tráfico ===
fig_trafico = px.bar(df,
                     x="Label",
                     y="Bytes",
                     color="Tipo",
                     title="Tráfico por Instrucción (Bytes)",
                     labels={"Label": "Instrucción", "Bytes": "Bytes Transferidos"},
                     template="plotly_dark")

fig_trafico.update_layout(
    xaxis_tickangle=-45,
    title_font_size=24,
    title_x=0.5,  # centrar
    margin=dict(t=80)
)
fig_trafico.write_html("grafico_trafico.html")
fig_trafico.show()

# === Ancho de Banda ===
fig_bw = px.bar(df,
                x="Label",
                y="BW",
                color="Tipo",
                title="Ancho de Banda por Instrucción (BW)",
                labels={"Label": "Instrucción", "BW": "BW"},
                template="plotly_dark")

fig_bw.update_layout(
    xaxis_tickangle=-45,
    title_font_size=24,
    title_x=0.5,
    margin=dict(t=80)
)
fig_bw.write_html("grafico_bw.html")
fig_bw.show()


# === Ciclos vs Bytes (steps.txt) ===
steps_data = []
steps_pattern = r"Ciclos hasta el momento: (\d+)\s+Bytes: (\d+)"

with open("steps.txt", "r") as f:
    for line in f:
        match = re.search(steps_pattern, line)
        if match:
            ciclos = int(match.group(1))
            bytes_ = int(match.group(2))
            steps_data.append((ciclos, bytes_))

# Crear DataFrame
df_steps = pd.DataFrame(steps_data, columns=["Ciclos", "Bytes"])

# Graficar usando "Ciclos" en X para que aparezca como tal en el tooltip
fig_steps = px.line(df_steps,
                    x="Ciclos",
                    y="Bytes",
                    title="Relación Ciclos vs Bytes (steps.txt)",
                    labels={"Ciclos": "Ciclos", "Bytes": "Bytes"},
                    template="plotly_dark",
                    markers=True)

fig_steps.update_layout(
    title_font_size=24,
    title_x=0.5,
    margin=dict(t=80)
)

fig_steps.write_html("grafico_ciclos_vs_bytes.html")
fig_steps.show()
