#include "device_model.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static command_result_t cmd(device_model_t *d, const char *payload, command_ack_t *ack)
{
    return device_apply_command(d, payload, payload ? strlen(payload) : 0, ack);
}

int main(void)
{
    device_model_t d;
    command_ack_t ack;
    char out[DEVICE_PAYLOAD_MAX + 1];
    char long_id[DEVICE_ID_MAX + 2];
    memset(long_id, 'a', sizeof(long_id) - 1);
    long_id[sizeof(long_id) - 1] = '\0';
    assert(!device_init(NULL, "run", "dev"));
    assert(!device_init(&d, NULL, "dev"));
    assert(!device_init(&d, "bad/#", "dev"));
    assert(!device_init(&d, "run", ""));
    assert(!device_init(&d, long_id, "dev"));
    assert(device_init(&d, "r_1", "d-2"));
    assert(strcmp(d.command_topic, "lab/r_1/d-2/command") == 0);
    assert(d.state == DEVICE_INITIALIZED && d.switch_on == 0 && d.sequence == 0);
    assert(!device_transition(&d, DEVICE_RUNNING));
    assert(device_transition(&d, DEVICE_CONNECTING));
    assert(device_transition(&d, DEVICE_CONNECTED));
    assert(device_transition(&d, DEVICE_SUBSCRIBED));
    assert(device_transition(&d, DEVICE_RUNNING));
    assert(device_transition(&d, DEVICE_BACKOFF));
    assert(device_transition(&d, DEVICE_RECONNECTING));
    assert(device_transition(&d, DEVICE_CONNECTED));
    assert(device_transition(&d, DEVICE_SUBSCRIBED));
    assert(device_transition(&d, DEVICE_RUNNING));
    assert(!device_transition(NULL, DEVICE_RUNNING));
    assert(device_backoff_ms(0) == 0 && device_backoff_ms(1) == 100);
    assert(device_backoff_ms(2) == 200 && device_backoff_ms(3) == 400);
    assert(device_backoff_ms(4) == 800 && device_backoff_ms(UINT_MAX) == 800);
    assert(device_telemetry_json(&d, out, sizeof(out)));
    assert(strstr(out, "\"sequence\":1") != NULL);
    assert(!device_telemetry_json(&d, out, 4));
    assert(!device_telemetry_json(NULL, out, sizeof(out)));
    device_telemetry_queued(&d, 123);
    device_puback(&d, 130);
    assert(d.sequence == 1 && d.published == 1 && d.puback == 1 && d.last_success_ms == 130);
    assert(cmd(&d, "{\"v\":1,\"request_id\":7,\"command\":\"set_switch\",\"value\":1}", &ack) == COMMAND_ACCEPTED);
    assert(d.switch_on == 1 && d.last_request_id == 7 && ack.accepted);
    assert(device_ack_json(&ack, out, sizeof(out)) && strstr(out, "\"accepted\":true") != NULL);
    assert(!device_ack_json(&ack, out, 3));
    assert(cmd(&d, " { \"value\" : 1 , \"command\" : \"set_switch\" , \"request_id\" : 7 , \"v\" : 1 } ", &ack) == COMMAND_DUPLICATE);
    assert(ack.duplicate && d.duplicate_commands == 1 && d.switch_on == 1);
    assert(cmd(&d, "{\"v\":1,\"request_id\":7,\"command\":\"set_switch\",\"value\":0}", &ack) == COMMAND_REJECTED);
    assert(strcmp(ack.reason, "conflict") == 0 && d.switch_on == 1);
    assert(cmd(&d, "{\"v\":1,\"request_id\":6,\"command\":\"set_switch\",\"value\":0}", &ack) == COMMAND_REJECTED);
    assert(strcmp(ack.reason, "stale") == 0 && d.switch_on == 1);
    const char *bad[] = {NULL, "", "{}", "[]", "{bad}",
      "{\"v\":2,\"request_id\":8,\"command\":\"set_switch\",\"value\":0}",
      "{\"v\":1,\"request_id\":8,\"command\":\"toggle\",\"value\":0}",
      "{\"v\":1,\"request_id\":8,\"command\":\"set_switch\"}",
      "{\"v\":1,\"request_id\":8,\"command\":\"set_switch\",\"value\":2}",
      "{\"v\":1,\"request_id\":8,\"command\":\"set_switch\",\"value\":true}",
      "{\"v\":1,\"request_id\":-1,\"command\":\"set_switch\",\"value\":0}",
      "{\"v\":1,\"request_id\":4294967296,\"command\":\"set_switch\",\"value\":0}",
      "{\"v\":1,\"request_id\":08,\"command\":\"set_switch\",\"value\":0}",
      "{\"v\":1,\"request_id\":8,\"command\":\"set_switch\",\"value\":0,\"value\":1}",
      "{\"v\":1,\"request_id\":8,\"command\":\"set_switch\",\"value\":0}extra"};
    for(size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
        assert(cmd(&d, bad[i], &ack) == COMMAND_REJECTED);
        assert(!ack.accepted && d.switch_on == 1);
    }
    memset(out, 'x', sizeof(out));
    assert(device_apply_command(&d, out, DEVICE_PAYLOAD_MAX + 1, &ack) == COMMAND_REJECTED);
    assert(d.switch_on == 1);
    assert(cmd(&d, "{\"v\":1,\"request_id\":8,\"command\":\"set_switch\",\"value\":0}", &ack) == COMMAND_ACCEPTED);
    assert(d.switch_on == 0 && d.last_request_id == 8);
    assert(d.received_commands == 5 + sizeof(bad) / sizeof(bad[0]) + 1);
    d.received_commands = UINT32_MAX;
    d.rejected_commands = UINT32_MAX;
    assert(cmd(&d, "not json", &ack) == COMMAND_REJECTED);
    assert(d.received_commands == UINT32_MAX && d.rejected_commands == UINT32_MAX);
    d.sequence = UINT32_MAX;
    assert(!device_telemetry_json(&d, out, sizeof(out)));
    d.published = d.puback = UINT32_MAX;
    device_telemetry_queued(&d, 200);
    device_puback(&d, 201);
    assert(d.published == UINT32_MAX && d.puback == UINT32_MAX && d.sequence == UINT32_MAX);
    assert(device_transition(&d, DEVICE_STOPPED));
    assert(!device_transition(&d, DEVICE_CONNECTING));
    assert(device_init(&d, "r2", "d2"));
    assert(d.received_commands == 0 && d.state == DEVICE_INITIALIZED);
    puts("device_model: PASS");
    return 0;
}
