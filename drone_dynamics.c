// Variabili per posizione e direzione
int x_cross = win_width / 2, y_cross = win_height / 2; // Posizione iniziale di cross
int dx_cross = 0, dy_cross = 0; // Direzione iniziale di cross
// Ciclo di movimento
while (1) {
    // Cancella il vecchio carattere
    mvwaddch(win, y_cross, x_cross, ' ');

    // Aggiorna la posizione
    x_cross += dx_cross;
    y_cross += dy_cross;

    // Rimbalza ai bordi
    if (x_cross <= 1  x_cross >= win_width - 2) dx_cross = -dx_cross; // Bordi laterali
    if (y_cross <= 1  y_cross >= win_height - 2) dy_cross = -dy_cross; // Bordi superiore/inferiore

    // Disegna il nuovo carattere
    mvwaddch(win, y_cross, x_cross, 'X');
    wrefresh(win);

    // Attendi un po' (per rallentare il movimento)
    usleep(50000);

    // Esci se premi un tasto
    if (wgetch(win) != ERR) break;
}