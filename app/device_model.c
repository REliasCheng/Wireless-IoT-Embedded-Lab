#include "device_model.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct { const unsigned char *data; size_t length, pos; } cursor_t;

static void sat_inc(uint32_t *value) { if(*value != UINT32_MAX) ++*value; }

static bool valid_segment(const char *text)
{
    size_t n;
    if(text == NULL || (n = strlen(text)) == 0 || n > DEVICE_ID_MAX) return false;
    for(size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)text[i];
        if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
    }
    return true;
}

static bool topic(char *out, size_t capacity, const char *run, const char *id, const char *kind)
{
    int n = snprintf(out, capacity, "lab/%s/%s/%s", run, id, kind);
    return n > 0 && (size_t)n < capacity;
}

bool device_init(device_model_t *device, const char *run_id, const char *device_id)
{
    if(device == NULL || !valid_segment(run_id) || !valid_segment(device_id)) return false;
    memset(device, 0, sizeof(*device));
    memcpy(device->run_id, run_id, strlen(run_id) + 1);
    memcpy(device->device_id, device_id, strlen(device_id) + 1);
    if(!topic(device->telemetry_topic, sizeof(device->telemetry_topic), run_id, device_id, "telemetry") ||
       !topic(device->command_topic, sizeof(device->command_topic), run_id, device_id, "command") ||
       !topic(device->ack_topic, sizeof(device->ack_topic), run_id, device_id, "ack")) return false;
    device->state = DEVICE_INITIALIZED;
    device->last_error = "none";
    return true;
}

bool device_transition(device_model_t *device, device_state_t next)
{
    if(device == NULL) return false;
    bool allowed = false;
    switch(device->state) {
        case DEVICE_INITIALIZED: allowed = next == DEVICE_CONNECTING || next == DEVICE_STOPPED; break;
        case DEVICE_CONNECTING: allowed = next == DEVICE_CONNECTED || next == DEVICE_BACKOFF || next == DEVICE_FAILED || next == DEVICE_STOPPED; break;
        case DEVICE_CONNECTED: allowed = next == DEVICE_SUBSCRIBED || next == DEVICE_BACKOFF || next == DEVICE_STOPPED; break;
        case DEVICE_SUBSCRIBED: allowed = next == DEVICE_RUNNING || next == DEVICE_BACKOFF || next == DEVICE_STOPPED; break;
        case DEVICE_RUNNING: allowed = next == DEVICE_BACKOFF || next == DEVICE_STOPPED; break;
        case DEVICE_BACKOFF: allowed = next == DEVICE_RECONNECTING || next == DEVICE_FAILED || next == DEVICE_STOPPED; break;
        case DEVICE_RECONNECTING: allowed = next == DEVICE_CONNECTED || next == DEVICE_BACKOFF || next == DEVICE_FAILED || next == DEVICE_STOPPED; break;
        case DEVICE_FAILED: allowed = next == DEVICE_STOPPED; break;
        case DEVICE_STOPPED: allowed = false; break;
    }
    if(allowed) device->state = next;
    return allowed;
}

const char *device_state_name(device_state_t state)
{
    static const char *const names[] = {"INITIALIZED", "CONNECTING", "CONNECTED", "SUBSCRIBED",
                                       "RUNNING", "BACKOFF", "RECONNECTING", "STOPPED", "FAILED"};
    return (unsigned)state < sizeof(names) / sizeof(names[0]) ? names[state] : "INVALID";
}

unsigned device_backoff_ms(unsigned attempt)
{
    if(attempt == 0) return 0;
    if(attempt >= 4) return 800;
    return 100U << (attempt - 1);
}

bool device_telemetry_json(const device_model_t *device, char *out, size_t capacity)
{
    int n;
    uint32_t next;
    if(device == NULL || out == NULL || capacity == 0 || device->sequence == UINT32_MAX) return false;
    next = device->sequence + 1;
    n = snprintf(out, capacity,
                 "{\"v\":1,\"sequence\":%lu,\"temperature_c\":%u.%u,\"humidity_percent\":%u,\"switch\":%d}",
                 (unsigned long)next, 22U + (next % 7), next % 10, 45U + (next % 12), device->switch_on);
    return n > 0 && (size_t)n < capacity && (size_t)n <= DEVICE_PAYLOAD_MAX;
}

bool device_ack_json(const command_ack_t *ack, char *out, size_t capacity)
{
    int n;
    if(ack == NULL || out == NULL || ack->reason == NULL || capacity == 0) return false;
    n = snprintf(out, capacity,
                 "{\"v\":1,\"request_id\":%lu,\"accepted\":%s,\"duplicate\":%s,\"switch\":%d,\"reason\":\"%s\"}",
                 (unsigned long)ack->request_id, ack->accepted ? "true" : "false",
                 ack->duplicate ? "true" : "false", ack->switch_on, ack->reason);
    return n > 0 && (size_t)n < capacity && (size_t)n <= DEVICE_PAYLOAD_MAX;
}

static void space(cursor_t *c)
{
    while(c->pos < c->length && (c->data[c->pos] == ' ' || c->data[c->pos] == '\n' ||
          c->data[c->pos] == '\r' || c->data[c->pos] == '\t')) c->pos++;
}

static bool chr(cursor_t *c, unsigned char expected)
{
    space(c);
    if(c->pos >= c->length || c->data[c->pos] != expected) return false;
    c->pos++;
    return true;
}

/* Deliberately bounded protocol strings: printable ASCII without escapes. */
static bool string(cursor_t *c, char *out, size_t capacity)
{
    size_t used = 0;
    if(!chr(c, '"')) return false;
    while(c->pos < c->length && c->data[c->pos] != '"') {
        unsigned char b = c->data[c->pos++];
        if(b < 0x20 || b > 0x7e || b == '\\' || used + 1 >= capacity) return false;
        out[used++] = (char)b;
    }
    if(c->pos >= c->length || used == 0) return false;
    c->pos++;
    out[used] = '\0';
    return true;
}

static bool uint(cursor_t *c, uint32_t *out)
{
    uint32_t value = 0;
    size_t start;
    space(c);
    start = c->pos;
    if(start >= c->length || c->data[start] < '0' || c->data[start] > '9') return false;
    if(c->data[start] == '0' && start + 1 < c->length &&
       c->data[start + 1] >= '0' && c->data[start + 1] <= '9') return false;
    while(c->pos < c->length && c->data[c->pos] >= '0' && c->data[c->pos] <= '9') {
        unsigned digit = c->data[c->pos++] - '0';
        if(value > (UINT32_MAX - digit) / 10) return false;
        value = value * 10 + digit;
    }
    *out = value;
    return c->pos > start;
}

static bool parse(const void *payload, size_t length, uint32_t *request_id, int *value)
{
    cursor_t c = {(const unsigned char *)payload, length, 0};
    uint32_t version = 0, id = 0, val = 0;
    char key[24], command[24];
    unsigned seen = 0;
    if(payload == NULL || length == 0 || length > DEVICE_PAYLOAD_MAX || !chr(&c, '{')) return false;
    for(;;) {
        if(!string(&c, key, sizeof(key)) || !chr(&c, ':')) return false;
        if(strcmp(key, "v") == 0 && !(seen & 1)) { if(!uint(&c, &version)) return false; seen |= 1; }
        else if(strcmp(key, "request_id") == 0 && !(seen & 2)) { if(!uint(&c, &id)) return false; seen |= 2; }
        else if(strcmp(key, "command") == 0 && !(seen & 4)) {
            if(!string(&c, command, sizeof(command))) return false;
            seen |= 4;
        }
        else if(strcmp(key, "value") == 0 && !(seen & 8)) { if(!uint(&c, &val)) return false; seen |= 8; }
        else return false;
        space(&c);
        if(chr(&c, '}')) break;
        if(!chr(&c, ',')) return false;
    }
    space(&c);
    if(c.pos != c.length || seen != 15 || version != 1 || id == 0 || val > 1 ||
       strcmp(command, "set_switch") != 0) return false;
    *request_id = id;
    *value = (int)val;
    return true;
}

command_result_t device_apply_command(device_model_t *device, const void *payload,
                                      size_t length, command_ack_t *ack)
{
    uint32_t id = 0;
    int value = 0;
    if(device == NULL || ack == NULL) return COMMAND_REJECTED;
    sat_inc(&device->received_commands);
    ack->request_id = 0;
    ack->accepted = false;
    ack->duplicate = false;
    ack->switch_on = device->switch_on;
    ack->reason = "invalid";
    if(!parse(payload, length, &id, &value)) {
        sat_inc(&device->rejected_commands);
        device->last_error = "invalid_command";
        return COMMAND_REJECTED;
    }
    ack->request_id = id;
    if(id < device->last_request_id) ack->reason = "stale";
    else if(id == device->last_request_id) {
        if(value == device->last_request_value) {
            sat_inc(&device->duplicate_commands);
            ack->accepted = true;
            ack->duplicate = true;
            ack->reason = "duplicate";
            return COMMAND_DUPLICATE;
        }
        ack->reason = "conflict";
    }
    else {
        device->switch_on = value;
        device->last_request_id = id;
        device->last_request_value = value;
        device->last_error = "none";
        ack->accepted = true;
        ack->switch_on = value;
        ack->reason = "ok";
        return COMMAND_ACCEPTED;
    }
    sat_inc(&device->rejected_commands);
    device->last_error = ack->reason;
    return COMMAND_REJECTED;
}

void device_telemetry_queued(device_model_t *device, uint64_t now_ms)
{
    if(device == NULL) return;
    if(device->sequence < UINT32_MAX) ++device->sequence;
    sat_inc(&device->published);
    device->last_success_ms = now_ms;
}

void device_puback(device_model_t *device, uint64_t now_ms)
{
    if(device == NULL) return;
    sat_inc(&device->puback);
    device->last_success_ms = now_ms;
}
