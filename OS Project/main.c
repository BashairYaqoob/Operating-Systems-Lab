#include "parking.h"
#include <ncurses.h>

void init_dashboard(void);
void draw_dashboard(void);
void end_dashboard(void);

volatile int paused = 0;
volatile int quit_flag = 0;

void pauseable_sleep(int seconds) {//would pause screen instantly
    for (int i = 0; i < seconds * 10; i++) {
	if (quit_flag) return;
        int ch = getch();
        if (ch == 'p' || ch == 'P') paused = !paused;
        if (ch == 'q' || ch == 'Q') { quit_flag = 1; return; }

        while (paused && !quit_flag) {
                attron(COLOR_PAIR(6) | A_BOLD);
                mvprintw(2, 10, "*** SIMULATION PAUSED --- Press P to Resume ***");
                attroff(COLOR_PAIR(6) | A_BOLD);
                refresh();
                usleep(150000);
                int c = getch();
                if (c == 'p' || c == 'P') paused = 0;
                if (c == 'q' || c == 'Q') { quit_flag = 1; return; }
        }
        usleep(100000);
    }
}

int main(void) {
    init_parking();
    init_dashboard();

    clear();
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1,  4, "+====================================================================+");
    mvprintw(2,  4, "|          [P]   SMART PARKING ALLOCATION SYSTEM   [P]              |");
    mvprintw(3,  4, "|                    Operating Systems Project                      |");
    mvprintw(4,  4, "+====================================================================+");
    attroff(COLOR_PAIR(1) | A_BOLD);

    mvprintw(6,  6, "WHAT THIS SIMULATION DEMONSTRATES:");
    mvprintw(7,  6, "------------------------------------");
    mvprintw(8,  6, "  * Each VEHICLE is an independent THREAD (pthread_create)");
    mvprintw(9,  6, "  * SEMAPHORES count & control available parking slots");
    mvprintw(10, 6, "  * MUTEX LOCKS protect the slot table from race conditions");
    mvprintw(11, 6, "  * Vehicles WAIT IN QUEUE when the parking lot is full");
    mvprintw(12, 6, "  * VIP/Emergency vehicles get PRIORITY SLOTS (slots 0 & 1)");
    mvprintw(13, 6, "  * Vehicles that wait too long will TIME OUT and leave");

    mvprintw(15, 6, "COLOUR GUIDE:");
    mvprintw(16, 6, "-------------");
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(17, 6, "  GREEN   = Slot is FREE (normal slot, no vehicle)");
    attroff(COLOR_PAIR(2) | A_BOLD);
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(18, 6, "  YELLOW  = VIP reserved slot (free) or VIP vehicle parked");
    attroff(COLOR_PAIR(3) | A_BOLD);
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(19, 6, "  RED     = Slot OCCUPIED by a Normal vehicle");
    attroff(COLOR_PAIR(4) | A_BOLD);
    attron(COLOR_PAIR(8) | A_BOLD);
    mvprintw(20, 6, "  RED/YLW = Slot OCCUPIED by an Emergency vehicle");
    attroff(COLOR_PAIR(8) | A_BOLD);
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(21, 6, "  MAGENTA = Vehicle is WAITING in queue for a slot");
    attroff(COLOR_PAIR(5) | A_BOLD);
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(22, 6, "  WHITE/R = Vehicle TIMED OUT (waited too long, left)");
    attroff(COLOR_PAIR(6) | A_BOLD);

    mvprintw(24, 6, "CONTROLS DURING SIMULATION:");
    mvprintw(25, 6, "---------------------------");
    mvprintw(26, 6, "  Press  P  = Pause / Resume the simulation instantly");
    mvprintw(27, 6, "  Press  Q  = Quit simulation early");

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(29, 6, "  Total Slots    : %d   (Slots 0-%d are VIP reserved)",
             TOTAL_SLOTS, VIP_SLOTS - 1);
    mvprintw(30, 6, "  Total Vehicles : %d", TOTAL_VEHICLES);
    mvprintw(31, 6, "  Vehicle types  : 70%% Normal  |  20%% VIP  |  10%% Emergency");
    mvprintw(32, 6, "  Parking stay   : 12 to 26 seconds per vehicle");
    mvprintw(33, 6, "  Arrival gap    : 3 seconds between each vehicle");
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(35, 4, "  >>> READ THE ABOVE, THEN PRESS  ENTER  TO START THE SIMULATION <<<");
    attroff(COLOR_PAIR(2) | A_BOLD);
    refresh();
//simulation runs when ENTER key is pressed
    nocbreak();
    echo();
    getch();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    pthread_t tids[TOTAL_VEHICLES];

    for (int i = 0; i < TOTAL_VEHICLES; i++) {
        if (quit_flag) break;

        vehicles[i].id            = i;
        vehicles[i].assigned_slot = -1;
        vehicles[i].status        = WAITING;
        vehicles[i].duration      = 12 + rand() % 15;

        int r = rand() % 10;
        if      (r < 7) vehicles[i].type = NORMAL;
        else if (r < 9) vehicles[i].type = VIP;
        else            vehicles[i].type = EMERGENCY;

        pthread_create(&tids[i], NULL, vehicle_thread, &vehicles[i]);

        draw_dashboard();
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(1, 46, "[P]=Pause [Q]=Quit | Vehicle #%02d arriving!", i);
        attroff(COLOR_PAIR(3) | A_BOLD);
        refresh();

        pauseable_sleep(3);
        if (quit_flag) break;
    }

    while (!quit_flag) {//simulation .. refreshes every 0.5s
        int ch = getch();
        if (ch == 'p' || ch == 'P') paused = !paused;
        if (ch == 'q' || ch == 'Q') break;

        while (paused && !quit_flag) {
                draw_dashboard();
                attron(COLOR_PAIR(6) | A_BOLD);
                mvprintw(2, 10, "*** SIMULATION PAUSED --- Press P to Resume ***");
                attroff(COLOR_PAIR(6) | A_BOLD);
                refresh();
                usleep(150000);
                int c = getch();
                if (c == 'p' || c == 'P') paused = 0;
                if (c == 'q' || c == 'Q') { quit_flag = 1; break; }
        }

        draw_dashboard();
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(1, 46, "[P]=Pause  [Q]=Quit");
        attroff(COLOR_PAIR(3) | A_BOLD);
        refresh();

        usleep(500000);

        int active = 0;
        for (int i = 0; i < TOTAL_VEHICLES; i++)
            if (vehicles[i].status == WAITING ||
                vehicles[i].status == PARKED)
                active++;
        if (active == 0) break;
    }
    for (int i = 0; i < TOTAL_VEHICLES; i++)
        pthread_join(tids[i], NULL);

    nodelay(stdscr, FALSE);
    halfdelay(10);

    for (int countdown = 15; countdown >= 0; countdown--) {
        draw_dashboard();
        int maxr, maxc;
        getmaxyx(stdscr, maxr, maxc);
        (void)maxc;
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(maxr - 2, 2,
            " === SIMULATION COMPLETE! Closing in %02d second(s)..."
            " Press ENTER to close now === ", countdown);
        attroff(COLOR_PAIR(2) | A_BOLD);
        refresh();
        int ch = getch();
        if (ch != ERR && (ch == '\n' || ch == 13 || ch == KEY_ENTER)) break;
    }

    sem_destroy(&sem_slots);
    end_dashboard();

    printf("\n===== SMART PARKING SIMULATION COMPLETE =====\n");
    printf("  Total Vehicles : %d\n", TOTAL_VEHICLES);
    printf("  Total Parked   : %d\n", total_parked);
    printf("  Total Departed : %d\n", total_departed);
    printf("  Total Timeout  : %d\n", total_timeout);
    printf("  Final Util     : %.1f%%\n", utilization);
    printf("=============================================\n");
    return 0;
}
