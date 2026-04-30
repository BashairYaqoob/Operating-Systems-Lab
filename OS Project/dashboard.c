#include "parking.h"
#include <ncurses.h>

#define COL_HEADER   1
#define COL_FREE     2
#define COL_VIP      3
#define COL_OCCUPIED 4
#define COL_WAITING  5
#define COL_TIMEOUT  6
#define COL_NORMAL   7
#define COL_EMG      8

void draw_dashboard() {
    int max_rows, max_cols;
    getmaxyx(stdscr, max_rows, max_cols);
    (void)max_cols;

    clear();
    int row = 0;

#define SAFE_PRINT(r, ...) do { if ((r) < max_rows - 1) { mvprintw((r), __VA_ARGS__); } } while(0)

    attron(COLOR_PAIR(COL_HEADER) | A_BOLD);
    mvprintw(row++, 2, "+================================================================+");
    mvprintw(row++, 2, "|         [P]  SMART PARKING ALLOCATION SYSTEM  [P]              |");
    mvprintw(row++, 2, "+================================================================+");
    attroff(COLOR_PAIR(COL_HEADER) | A_BOLD);
    row++;
    if (row < max_rows - 1) {
        SAFE_PRINT(row, 2, " LEGEND: ");
        attron(COLOR_PAIR(COL_FREE)     | A_BOLD); SAFE_PRINT(row, 11, "[FREE]");     attroff(COLOR_PAIR(COL_FREE)     | A_BOLD);
        SAFE_PRINT(row, 18, "=empty  ");
        attron(COLOR_PAIR(COL_OCCUPIED) | A_BOLD); SAFE_PRINT(row, 26, "[OCCUPIED]"); attroff(COLOR_PAIR(COL_OCCUPIED) | A_BOLD);
        SAFE_PRINT(row, 36, "=taken  ");
        attron(COLOR_PAIR(COL_VIP)      | A_BOLD); SAFE_PRINT(row, 44, "[VIP]");      attroff(COLOR_PAIR(COL_VIP)      | A_BOLD);
        SAFE_PRINT(row, 49, "=VIP slot  ");
        attron(COLOR_PAIR(COL_EMG)      | A_BOLD); SAFE_PRINT(row, 60, "[EMERG]");    attroff(COLOR_PAIR(COL_EMG)      | A_BOLD);
        SAFE_PRINT(row, 67, "=Emergency");
    }

    attron(A_BOLD);
    SAFE_PRINT(row++, 2, " PARKING LOT LAYOUT:");
    attroff(A_BOLD);

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        int col_offset = 2 + (i % 5) * 16;
        int r = row + (i / 5) * 5;
        if (r + 4 >= max_rows) continue;

        if (!slots[i].occupied) {
            if (slots[i].slot_type == VIP) {
                attron(COLOR_PAIR(COL_VIP) | A_BOLD);
                mvprintw(r,   col_offset, "+-------------+");
                mvprintw(r+1, col_offset, "| SLOT %02d     |", i);
                mvprintw(r+2, col_offset, "| VIP  SLOT   |");
                mvprintw(r+3, col_offset, "|    FREE     |");
                mvprintw(r+4, col_offset, "+-------------+");
                attroff(COLOR_PAIR(COL_VIP) | A_BOLD);
            } else {
                attron(COLOR_PAIR(COL_FREE));
                mvprintw(r,   col_offset, "+-------------+");
                mvprintw(r+1, col_offset, "| SLOT %02d     |", i);
                mvprintw(r+2, col_offset, "| NORMAL SLOT |");
                mvprintw(r+3, col_offset, "|    FREE     |");
                mvprintw(r+4, col_offset, "+-------------+");
                attroff(COLOR_PAIR(COL_FREE));
            }
        } else {
            int vid = slots[i].vehicle_id;
            VehicleType vt = vehicles[vid].type;
            const char* slot_label;
            if (slots[i].slot_type == VIP && vt == VIP)
                slot_label = "VIP  SLOT   ";
            else if (slots[i].slot_type == VIP && vt == EMERGENCY)
                slot_label = "VIP(EMERG)  ";
            else if (slots[i].slot_type == VIP)
                slot_label = "VIP SLOT    ";
            else
                slot_label = "NORMAL SLOT ";

            if      (vt == VIP)       attron(COLOR_PAIR(COL_VIP)     | A_BOLD);
            else if (vt == EMERGENCY) attron(COLOR_PAIR(COL_EMG)     | A_BOLD);
            else                      attron(COLOR_PAIR(COL_OCCUPIED) | A_BOLD);

            mvprintw(r,   col_offset, "+-------------+");
            mvprintw(r+1, col_offset, "| SLOT %02d     |", i);
            mvprintw(r+2, col_offset, "| %s|", slot_label);
            mvprintw(r+3, col_offset, "| VEH #%-2d OCC |", vid);
            mvprintw(r+4, col_offset, "+-------------+");

            if      (vt == VIP)       attroff(COLOR_PAIR(COL_VIP)     | A_BOLD);
            else if (vt == EMERGENCY) attroff(COLOR_PAIR(COL_EMG)     | A_BOLD);
            else                      attroff(COLOR_PAIR(COL_OCCUPIED) | A_BOLD);
        }
    }
    row += 11;

    if (row < max_rows - 1) {
        attron(COLOR_PAIR(COL_HEADER) | A_BOLD);
        SAFE_PRINT(row++, 2, "----------------------------------------------------------------");
        attroff(COLOR_PAIR(COL_HEADER) | A_BOLD);
    }

    if (row < max_rows - 1) {
        int bar_len = 50;
        int filled  = (int)(utilization / 100.0 * bar_len);
        mvprintw(row, 2, " Lot Capacity Used [");
        attron(COLOR_PAIR(COL_OCCUPIED) | A_BOLD);
        for (int i = 0; i < filled; i++)       mvprintw(row, 22 + i, "#");
        attroff(COLOR_PAIR(COL_OCCUPIED) | A_BOLD);
        attron(COLOR_PAIR(COL_FREE));
        for (int i = filled; i < bar_len; i++) mvprintw(row, 22 + i, "-");
        attroff(COLOR_PAIR(COL_FREE));
        mvprintw(row, 22 + bar_len, "] %5.1f%%", utilization);
    }
    if (row < max_rows - 1) {
        SAFE_PRINT(row++, 2,
             " Vehicles Parked: %-4d  Departed: %-4d  Timed Out: %-4d  In Queue: %-4d",
             total_parked, total_departed, total_timeout, queue_size);
    }

    row++;
    if (row < max_rows - 1) {
        attron(A_BOLD);
        SAFE_PRINT(row++, 2, " VEHICLES WAITING IN QUEUE (no slot available yet):");
        attroff(A_BOLD);
    }

    int snap_ids[TOTAL_VEHICLES];
    int snap_size = 0;
    pthread_mutex_lock(&queue_mutex);
    for (int i = 0; i < queue_size && i < TOTAL_VEHICLES; i++) {
        snap_ids[i] = waiting_queue[(queue_front + i) % TOTAL_VEHICLES];
        snap_size++;
    }
    pthread_mutex_unlock(&queue_mutex);

    if (row < max_rows - 1) {
        if (snap_size == 0) {
            attron(COLOR_PAIR(COL_FREE));
            SAFE_PRINT(row++, 4, "(Queue is empty - all vehicles have been served)");
            attroff(COLOR_PAIR(COL_FREE));
        } else {
            int shown = 0;
            for (int i = 0; i < snap_size && shown < 7; i++) {
                int vid = snap_ids[i];
                attron(COLOR_PAIR(COL_WAITING) | A_BOLD);
                if (row < max_rows - 1)
                    mvprintw(row, 4 + shown * 14,
                             "[VEH#%-2d %-5s]", vid, type_str(vehicles[vid].type));
                attroff(COLOR_PAIR(COL_WAITING) | A_BOLD);
                shown++;
            }
            row++;
        }
    }

    if (row < max_rows - 1) {
        attron(A_BOLD);
        SAFE_PRINT(row++, 2, " VEHICLE ACTIVITY LOG (most recent arrivals):");
        SAFE_PRINT(row++, 2, " %-8s %-10s %-10s %-12s %-14s %s",
                 "VEHICLE", "TYPE", "SLOT", "STATUS", "ACTUAL TIME", "ASSIGNED STAY");
        attroff(A_BOLD);
        attron(COLOR_PAIR(COL_HEADER));
        SAFE_PRINT(row++, 2, " %-8s %-10s %-10s %-12s %-14s %s",
                 "-------", "--------", "--------", "----------", "-----------", "-------------");
        attroff(COLOR_PAIR(COL_HEADER));
    }

    int shown_log = 0;
    for (int i = TOTAL_VEHICLES - 1; i >= 0 && shown_log < 10; i--) {
        Vehicle* v = &vehicles[i];
        if (v->duration == 0) continue;
	if (row >= max_rows - 1) break;

        if      (v->status == WAITING)  attron(COLOR_PAIR(COL_WAITING));
        else if (v->status == PARKED)   attron(COLOR_PAIR(COL_VIP) | A_BOLD);
        else if (v->status == DEPARTED) attron(COLOR_PAIR(COL_FREE));
        else                            attron(COLOR_PAIR(COL_TIMEOUT) | A_BOLD);

        char slot_str[12];
        if (v->assigned_slot >= 0)
            snprintf(slot_str, sizeof(slot_str), "Slot #%02d", v->assigned_slot);
        else
            snprintf(slot_str, sizeof(slot_str), "--------");
        char actual_str[20];
        if (v->status == PARKED && v->park_time > 0) {
            int secs = (int)(time(NULL) - v->park_time);
            snprintf(actual_str, sizeof(actual_str), "%ds (parked)", secs);
        } else if (v->status == DEPARTED && v->park_time > 0 && v->depart_time > 0) {
            int secs = (int)(v->depart_time - v->park_time);
            snprintf(actual_str, sizeof(actual_str), "%ds (stayed)", secs);
        } else if (v->status == WAITING) {
            snprintf(actual_str, sizeof(actual_str), "waiting...");
        } else {
            snprintf(actual_str, sizeof(actual_str), "timed out");
        }

        mvprintw(row++, 2, " Veh #%-3d  %-10s %-10s %-12s %-14s assigned: %ds",
                 v->id,
                 type_str(v->type),
                 slot_str,
                 status_str(v->status),
                 actual_str,
                 v->duration);

        if      (v->status == WAITING)  attroff(COLOR_PAIR(COL_WAITING));
        else if (v->status == PARKED)   attroff(COLOR_PAIR(COL_VIP) | A_BOLD);
        else if (v->status == DEPARTED) attroff(COLOR_PAIR(COL_FREE));
        else                            attroff(COLOR_PAIR(COL_TIMEOUT) | A_BOLD);

        shown_log++;
    }

    if (row < max_rows - 1) {
        attron(COLOR_PAIR(COL_HEADER));
        SAFE_PRINT(row, 2, " [Dashboard refreshes every second]   Press P=Pause  Q=Quit");
        attroff(COLOR_PAIR(COL_HEADER));
    }

    refresh();
}

void init_dashboard() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();

    init_pair(COL_HEADER,   COLOR_CYAN,    COLOR_BLACK);
    init_pair(COL_FREE,     COLOR_GREEN,   COLOR_BLACK);
    init_pair(COL_VIP,      COLOR_YELLOW,  COLOR_BLACK);
    init_pair(COL_OCCUPIED, COLOR_RED,     COLOR_BLACK);
    init_pair(COL_WAITING,  COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COL_TIMEOUT,  COLOR_WHITE,   COLOR_RED);
    init_pair(COL_NORMAL,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(COL_EMG,      COLOR_RED,     COLOR_YELLOW);
}

void end_dashboard(void) {
    endwin();
}
