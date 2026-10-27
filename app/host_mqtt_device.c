#define _POSIX_C_SOURCE 200809L
#include "device_model.h"

#include <mosquitto.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    device_model_t model;
    struct mosquitto *client;
    int port, qos, subscribe_mid;
    unsigned retries, max_retries;
    uint64_t next_retry_ms, connect_deadline_ms, subscribe_deadline_ms, last_telemetry_ms, end_ms;
    bool stopping, fatal;
} service_t;

static volatile sig_atomic_t stop_requested;
static void on_signal(int unused) { (void)unused; stop_requested = 1; }

static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void state(service_t *s, device_state_t next)
{
    if(!device_transition(&s->model, next)) {
        fprintf(stderr, "STATE_ERROR from=%s to=%s\n", device_state_name(s->model.state), device_state_name(next));
        s->fatal = true;
        return;
    }
    printf("STATE %s\n", device_state_name(next));
    fflush(stdout);
}

static void backoff(service_t *s, const char *reason)
{
    if(s->stopping || s->model.state == DEVICE_BACKOFF || s->model.state == DEVICE_FAILED) return;
    s->model.last_error = reason;
    state(s, DEVICE_BACKOFF);
    s->next_retry_ms = now_ms() + device_backoff_ms(s->retries + 1);
    printf("EVENT DISCONNECTED reason=%s next_retry_ms=%u\n", reason, device_backoff_ms(s->retries + 1));
    fflush(stdout);
}

static void on_connect(struct mosquitto *client, void *user, int rc)
{
    service_t *s = user;
    if(rc != 0) { backoff(s, "connack_rejected"); return; }
    state(s, DEVICE_CONNECTED);
    if(s->fatal) return;
    s->retries = 0;
    int result = mosquitto_subscribe(client, &s->subscribe_mid, s->model.command_topic, 1);
    if(result != MOSQ_ERR_SUCCESS) {
        mosquitto_disconnect(client);
        backoff(s, "subscribe_enqueue_failed");
        return;
    }
    s->subscribe_deadline_ms = now_ms() + 2000;
    printf("EVENT CONNECTED subscribe_mid=%d\n", s->subscribe_mid);
    fflush(stdout);
}

static void on_subscribe(struct mosquitto *unused, void *user, int mid, int qos_count, const int *granted_qos)
{
    (void)unused;
    service_t *s = user;
    if(mid != s->subscribe_mid || qos_count != 1 || granted_qos == NULL || granted_qos[0] == 0x80) {
        mosquitto_disconnect(s->client);
        backoff(s, "suback_failed");
        return;
    }
    state(s, DEVICE_SUBSCRIBED);
    state(s, DEVICE_RUNNING);
    s->last_telemetry_ms = now_ms();
    printf("EVENT RUNNING subscribed=%s qos=%d\n", s->model.command_topic, granted_qos[0]);
    fflush(stdout);
}

static void on_disconnect(struct mosquitto *unused, void *user, int rc)
{
    (void)unused;
    service_t *s = user;
    if(!s->stopping) backoff(s, rc ? "connection_lost" : "unexpected_disconnect");
}

static void on_publish(struct mosquitto *unused, void *user, int mid)
{
    (void)unused;
    service_t *s = user;
    if(s->qos == 1) device_puback(&s->model, now_ms());
    printf("EVENT PUBLISH_CALLBACK mid=%d qos=%d confirmed_count=%lu\n", mid, s->qos,
           (unsigned long)s->model.puback);
    fflush(stdout);
}

static void on_message(struct mosquitto *client, void *user, const struct mosquitto_message *message)
{
    service_t *s = user;
    command_ack_t ack;
    char payload[DEVICE_PAYLOAD_MAX + 1];
    if(message == NULL || message->topic == NULL ||
       strcmp(message->topic, s->model.command_topic) != 0 || message->payloadlen < 0) return;
    command_result_t result = device_apply_command(&s->model, message->payload,
                                                    (size_t)message->payloadlen, &ack);
    if(!device_ack_json(&ack, payload, sizeof(payload))) { s->fatal = true; return; }
    int rc = mosquitto_publish(client, NULL, s->model.ack_topic, (int)strlen(payload), payload, 1, false);
    if(rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "ACK_PUBLISH_FAILED rc=%d\n", rc);
        backoff(s, "ack_publish_failed");
        return;
    }
    printf("EVENT COMMAND request_id=%lu result=%s switch=%d qos=%d\n",
           (unsigned long)ack.request_id,
           result == COMMAND_ACCEPTED ? "accepted" : (result == COMMAND_DUPLICATE ? "duplicate" : "rejected"),
           s->model.switch_on, message->qos);
    printf("EVENT ACK_QUEUED %s\n", payload);
    fflush(stdout);
}

static bool number(const char *text, unsigned min, unsigned max, unsigned *out)
{
    char *end = NULL;
    unsigned long value;
    if(text == NULL || *text == '\0' || *text == '-') return false;
    value = strtoul(text, &end, 10);
    if(*end != '\0' || value < min || value > max) return false;
    *out = (unsigned)value;
    return true;
}

static bool args(int argc, char **argv, service_t *s)
{
    const char *run = NULL, *id = NULL;
    unsigned port = 0, qos = 1, duration = 15000, retries = 20;
    for(int i = 1; i < argc; i++) {
        if(i + 1 >= argc) return false;
        if(strcmp(argv[i], "--port") == 0) { if(!number(argv[++i], 1, 65535, &port)) return false; }
        else if(strcmp(argv[i], "--run-id") == 0) run = argv[++i];
        else if(strcmp(argv[i], "--device-id") == 0) id = argv[++i];
        else if(strcmp(argv[i], "--qos") == 0) { if(!number(argv[++i], 0, 1, &qos)) return false; }
        else if(strcmp(argv[i], "--duration-ms") == 0) { if(!number(argv[++i], 1000, 30000, &duration)) return false; }
        else if(strcmp(argv[i], "--max-retries") == 0) { if(!number(argv[++i], 1, 100, &retries)) return false; }
        else return false;
    }
    if(port == 0 || !device_init(&s->model, run, id)) return false;
    s->port = (int)port;
    s->qos = (int)qos;
    s->max_retries = retries;
    s->end_ms = now_ms() + duration;
    return true;
}

static void attempt_reconnect(service_t *s)
{
    if(s->retries >= s->max_retries) {
        state(s, DEVICE_FAILED);
        s->model.last_error = "retry_exhausted";
        printf("EVENT FAILED retry_exhausted attempts=%u\n", s->retries);
        fflush(stdout);
        return;
    }
    ++s->retries;
    if(s->model.reconnects < UINT32_MAX) ++s->model.reconnects;
    state(s, DEVICE_RECONNECTING);
    printf("EVENT RECONNECT_ATTEMPT count=%u\n", s->retries);
    fflush(stdout);
    int rc = mosquitto_reconnect(s->client);
    if(rc != MOSQ_ERR_SUCCESS) backoff(s, "reconnect_failed");
    else s->connect_deadline_ms = now_ms() + 2000;
}

int main(int argc, char **argv)
{
    service_t s = {0};
    char client_id[80], payload[DEVICE_PAYLOAD_MAX + 1];
    if(!args(argc, argv, &s)) {
        fputs("Usage: iot_device --port N --run-id ID --device-id ID [--qos 0|1] [--duration-ms N] [--max-retries N]\n", stderr);
        return 2;
    }
    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);
    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS) return 2;
    snprintf(client_id, sizeof(client_id), "lab_%s_%s", s.model.run_id, s.model.device_id);
    s.client = mosquitto_new(client_id, true, &s);
    if(s.client == NULL) { mosquitto_lib_cleanup(); return 2; }
    if(mosquitto_int_option(s.client, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V311) != MOSQ_ERR_SUCCESS) {
        mosquitto_destroy(s.client);
        mosquitto_lib_cleanup();
        return 2;
    }
    mosquitto_connect_callback_set(s.client, on_connect);
    mosquitto_subscribe_callback_set(s.client, on_subscribe);
    mosquitto_disconnect_callback_set(s.client, on_disconnect);
    mosquitto_message_callback_set(s.client, on_message);
    mosquitto_publish_callback_set(s.client, on_publish);
    state(&s, DEVICE_CONNECTING);
    int rc = mosquitto_connect(s.client, "127.0.0.1", s.port, 5);
    if(rc != MOSQ_ERR_SUCCESS) backoff(&s, "initial_connect_failed");
    else s.connect_deadline_ms = now_ms() + 2000;

    while(!stop_requested && !s.fatal && s.model.state != DEVICE_FAILED && now_ms() < s.end_ms) {
        uint64_t now = now_ms();
        if(s.model.state == DEVICE_BACKOFF && now >= s.next_retry_ms) attempt_reconnect(&s);
        if((s.model.state == DEVICE_CONNECTING || s.model.state == DEVICE_RECONNECTING) &&
           now > s.connect_deadline_ms) {
            mosquitto_disconnect(s.client);
            backoff(&s, "connect_timeout");
        }
        if(s.model.state == DEVICE_CONNECTED && now > s.subscribe_deadline_ms) {
            mosquitto_disconnect(s.client);
            backoff(&s, "suback_timeout");
        }
        if(s.model.state == DEVICE_RUNNING && now - s.last_telemetry_ms >= 500) {
            if(!device_telemetry_json(&s.model, payload, sizeof(payload))) { s.fatal = true; break; }
            rc = mosquitto_publish(s.client, NULL, s.model.telemetry_topic,
                                   (int)strlen(payload), payload, s.qos, false);
            if(rc == MOSQ_ERR_SUCCESS) {
                device_telemetry_queued(&s.model, now);
                s.last_telemetry_ms = now;
                printf("EVENT TELEMETRY_QUEUED qos=%d %s\n", s.qos, payload);
                fflush(stdout);
            }
            else backoff(&s, "telemetry_publish_failed");
        }
        if(s.model.state != DEVICE_BACKOFF && s.model.state != DEVICE_FAILED) {
            rc = mosquitto_loop(s.client, 50, 1);
            if(rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_NO_CONN) backoff(&s, "network_loop_failed");
            else if(rc == MOSQ_ERR_NO_CONN && s.model.state != DEVICE_BACKOFF) backoff(&s, "no_connection");
        }
        else {
            struct timespec delay = {.tv_sec = 0, .tv_nsec = 20000000};
            nanosleep(&delay, NULL);
        }
    }
    bool failed = s.fatal || s.model.state == DEVICE_FAILED;
    s.stopping = true;
    if(s.model.state != DEVICE_STOPPED) state(&s, DEVICE_STOPPED);
    mosquitto_disconnect(s.client);
    mosquitto_destroy(s.client);
    mosquitto_lib_cleanup();
    printf("SUMMARY state=%s sequence=%lu queued=%lu puback=%lu commands=%lu rejected=%lu duplicates=%lu reconnects=%lu last_error=%s\n",
           device_state_name(s.model.state), (unsigned long)s.model.sequence,
           (unsigned long)s.model.published, (unsigned long)s.model.puback,
           (unsigned long)s.model.received_commands, (unsigned long)s.model.rejected_commands,
           (unsigned long)s.model.duplicate_commands, (unsigned long)s.model.reconnects, s.model.last_error);
    fflush(stdout);
    return failed ? 1 : 0;
}
