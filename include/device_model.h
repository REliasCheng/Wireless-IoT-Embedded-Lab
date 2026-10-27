#ifndef DEVICE_MODEL_H
#define DEVICE_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEVICE_ID_MAX 32
#define DEVICE_TOPIC_MAX 128
#define DEVICE_PAYLOAD_MAX 256

typedef enum {
    DEVICE_INITIALIZED, DEVICE_CONNECTING, DEVICE_CONNECTED,
    DEVICE_SUBSCRIBED, DEVICE_RUNNING, DEVICE_BACKOFF,
    DEVICE_RECONNECTING, DEVICE_STOPPED, DEVICE_FAILED
} device_state_t;

typedef enum { COMMAND_ACCEPTED, COMMAND_DUPLICATE, COMMAND_REJECTED } command_result_t;

typedef struct {
    char run_id[DEVICE_ID_MAX + 1];
    char device_id[DEVICE_ID_MAX + 1];
    char telemetry_topic[DEVICE_TOPIC_MAX];
    char command_topic[DEVICE_TOPIC_MAX];
    char ack_topic[DEVICE_TOPIC_MAX];
    device_state_t state;
    uint32_t sequence;
    uint32_t published;
    uint32_t puback;
    uint32_t received_commands;
    uint32_t rejected_commands;
    uint32_t duplicate_commands;
    uint32_t reconnects;
    uint32_t last_request_id;
    uint64_t last_success_ms;
    int switch_on;
    int last_request_value;
    const char *last_error;
} device_model_t;

typedef struct {
    uint32_t request_id;
    bool accepted;
    bool duplicate;
    const char *reason;
    int switch_on;
} command_ack_t;

bool device_init(device_model_t *device, const char *run_id, const char *device_id);
bool device_transition(device_model_t *device, device_state_t next);
const char *device_state_name(device_state_t state);
unsigned device_backoff_ms(unsigned attempt);
bool device_telemetry_json(const device_model_t *device, char *out, size_t capacity);
bool device_ack_json(const command_ack_t *ack, char *out, size_t capacity);
command_result_t device_apply_command(device_model_t *device, const void *payload,
                                      size_t length, command_ack_t *ack);
void device_telemetry_queued(device_model_t *device, uint64_t now_ms);
void device_puback(device_model_t *device, uint64_t now_ms);

#endif
