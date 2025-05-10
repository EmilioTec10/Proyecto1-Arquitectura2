#include <stdio.h>

// Funcionb que escribe un evento en un archivo de texto
void write_event(int pe_id, int num_bytes, int time) {
    FILE *file = fopen("events.txt", "a");  // modo append

    if (file == NULL) {
        perror("Error al abrir el archivo de eventos");
        return;
    }

    fprintf(file, "%d,%d,%d\n", pe_id, num_bytes, time);

    fclose(file);
}


int main(int argc, char const *argv[])
{
    write_event(1, 4, 10);
    return 0;
}
