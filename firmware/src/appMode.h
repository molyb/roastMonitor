#pragma once

enum class SetupAppResult {
    SUCCESS,
    FAILURE_LOAD_CONFIG,
    FAILURE_ALLOCATE_MEMORY,
    FAILURE_START_HTTP_SERVER,
    FAILURE_START_WS_SERVER
};

SetupAppResult setupApp();
void loopApp();
