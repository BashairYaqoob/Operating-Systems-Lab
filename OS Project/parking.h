#ifndef PARKING_H
#define PARKING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define TOTAL_SLOTS     10
#define TOTAL_VEHICLES  15
#define MAX_WAIT_TIME   15
#define VIP_SLOTS       2

typedef enum { NORMAL, VIP, EMERGENCY } VehicleType;
typedef enum { WAITING, PARKED, DEPARTED, TIMEOUT } VehicleStatus;

typedef struct {
    int id;
    VehicleType type;
    int assigned_slot;// -1 for not parked
    time_t park_time;
    time_t depart_time;
    int duration;
    VehicleStatus status;
} Vehicle;

typedef struct {
    int slot_id;
    int occupied;
    int vehicle_id;
    VehicleType slot_type;
} ParkingSlot;

extern ParkingSlot slots[TOTAL_SLOTS];
extern Vehicle    vehicles[TOTAL_VEHICLES];
extern int        waiting_queue[TOTAL_VEHICLES];
extern int        queue_front, queue_rear, queue_size;
extern int        total_parked, total_timeout, total_departed;
extern double     utilization;
extern sem_t      sem_slots;
extern pthread_mutex_t slot_mutex;
extern pthread_mutex_t queue_mutex;
extern pthread_mutex_t stats_mutex;

void  init_parking();
void* vehicle_thread(void* arg);
int   find_free_slot(VehicleType type);
void  release_slot(int slot_id);
void  enqueue(int vehicle_id);
void   dequeue_vehicle(int vehicle_id);
void  update_stats();
const char* type_str(VehicleType t);
const char* status_str(VehicleStatus s);

#endif
