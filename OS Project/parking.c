#include "parking.h"

ParkingSlot slots[TOTAL_SLOTS];
Vehicle     vehicles[TOTAL_VEHICLES];
int         waiting_queue[TOTAL_VEHICLES];
int         queue_front = 0, queue_rear = 0, queue_size = 0;
int         total_parked = 0, total_timeout = 0, total_departed = 0;
double      utilization = 0.0;

sem_t           sem_slots;
pthread_mutex_t slot_mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_parking(void) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        slots[i].slot_id    = i;
        slots[i].occupied   = 0;
        slots[i].vehicle_id = -1;
        slots[i].slot_type  = (i < VIP_SLOTS) ? VIP : NORMAL;
    }
//initializing everything to 0
    for (int i = 0; i < TOTAL_VEHICLES; i++) {
        vehicles[i].id            = i;
        vehicles[i].assigned_slot = -1;
        vehicles[i].duration      = 0;
        vehicles[i].status        = WAITING;
        vehicles[i].type          = NORMAL;
        vehicles[i].park_time     = 0;
        vehicles[i].depart_time   = 0;
    }

    sem_init(&sem_slots, 0, TOTAL_SLOTS);
}

void enqueue(int vehicle_id) {
    pthread_mutex_lock(&queue_mutex);
    if (queue_size < TOTAL_VEHICLES) {
        waiting_queue[queue_rear] = vehicle_id;
        queue_rear  = (queue_rear + 1) % TOTAL_VEHICLES;
        queue_size++;
    }
    pthread_mutex_unlock(&queue_mutex);
}

void dequeue_vehicle(int vehicle_id) {
    pthread_mutex_lock(&queue_mutex);
    for (int i = 0; i < queue_size; i++) {
        int idx = (queue_front + i) % TOTAL_VEHICLES;
        if (waiting_queue[idx] == vehicle_id) {
            for (int j = i; j < queue_size - 1; j++) {
                int cur  = (queue_front + j)     % TOTAL_VEHICLES;
                int next = (queue_front + j + 1) % TOTAL_VEHICLES;
                waiting_queue[cur] = waiting_queue[next];
            }
            queue_rear = (queue_rear - 1 + TOTAL_VEHICLES) % TOTAL_VEHICLES;
            queue_size--;
            break;
        }
    }
    pthread_mutex_unlock(&queue_mutex);
}

int find_free_slot(VehicleType type) { //vip prefer vip slots. if unavailable, they take normal slots
    if (type == VIP || type == EMERGENCY) {
        for (int i = 0; i < VIP_SLOTS; i++)
            if (!slots[i].occupied) return i;
        for (int i = VIP_SLOTS; i < TOTAL_SLOTS; i++)
            if (!slots[i].occupied) return i;
    } else {//normal fill all normal slots
        for (int i = VIP_SLOTS; i < TOTAL_SLOTS; i++)
            if (!slots[i].occupied) return i;
        for (int i = 0; i < VIP_SLOTS; i++)
            if (!slots[i].occupied) return i;
    }
    return -1;
}

void release_slot(int slot_id) {
    pthread_mutex_lock(&slot_mutex);
    slots[slot_id].occupied   = 0;
    slots[slot_id].vehicle_id = -1;
    pthread_mutex_unlock(&slot_mutex);

    sem_post(&sem_slots);
}

void update_stats() {
    pthread_mutex_lock(&stats_mutex);
    int occupied = 0;
    for (int i = 0; i < TOTAL_SLOTS; i++)
        if (slots[i].occupied) occupied++;
    utilization = (double)occupied / TOTAL_SLOTS * 100.0;
    pthread_mutex_unlock(&stats_mutex);
}

void* vehicle_thread(void* arg) {
    Vehicle* v = (Vehicle*)arg;

    v->status = WAITING;
    enqueue(v->id);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += MAX_WAIT_TIME;

    int acquired = sem_timedwait(&sem_slots, &ts);
    dequeue_vehicle(v->id);

    if (acquired != 0) {
        v->status = TIMEOUT;
        pthread_mutex_lock(&stats_mutex);
        total_timeout++;
        pthread_mutex_unlock(&stats_mutex);
        update_stats();
        return NULL;
    }

    pthread_mutex_lock(&slot_mutex);
    int slot = find_free_slot(v->type);
    if (slot == -1) {
        pthread_mutex_unlock(&slot_mutex);
        sem_post(&sem_slots);
        v->status = TIMEOUT;
        pthread_mutex_lock(&stats_mutex);
        total_timeout++;
        pthread_mutex_unlock(&stats_mutex);
        return NULL;
    }
    slots[slot].occupied   = 1;
    slots[slot].vehicle_id = v->id;
    v->assigned_slot       = slot;
    v->park_time           = time(NULL);
    v->status              = PARKED;
    pthread_mutex_unlock(&slot_mutex);

    pthread_mutex_lock(&stats_mutex);
    total_parked++;
    pthread_mutex_unlock(&stats_mutex);
    update_stats();

    //vehicle stays parked for v->duration seconds
    sleep((unsigned)v->duration);

    v->depart_time = time(NULL);
    v->status = DEPARTED;
    release_slot(slot);

    pthread_mutex_lock(&stats_mutex);
    total_departed++;
    pthread_mutex_unlock(&stats_mutex);
    update_stats();

    return NULL;
}

const char* type_str(VehicleType t) {
    switch(t) {
        case VIP:       return "VIP  ";
        case EMERGENCY: return "EMERG";
        default:        return "NRML ";
    }
}

const char* status_str(VehicleStatus s) {
    switch(s) {
        case WAITING:  return "WAITING  ";
        case PARKED:   return "PARKED   ";
        case DEPARTED: return "DEPARTED ";
        case TIMEOUT:  return "TIMED OUT";
        default:       return "UNKNOWN  ";
    }
}
