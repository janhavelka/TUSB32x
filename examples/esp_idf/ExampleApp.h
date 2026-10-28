/// @file ExampleApp.h
/// @brief Serialized native application lifecycle; owns its SDK resources and borrows the core.
#pragma once

namespace tusb32x_idf {

/// @brief Create the owned UART console, then qualified bus/device resources and read-only startup.
/// @return True when the diagnostic console can run, including bus/setup failures; false if absent.
/// @note Call once on the pinned core-0 owner task. No default bus setup occurs without explicit
/// board/timing/SDK-cleanup qualification. Resource creation is outside terminal callback bounds.
bool setupExample();

/// @brief Run one bounded owner turn: at most 64 input bytes, one poll and one 256-byte output offer.
/// @return True to continue, false after shutdown or unavailable console; never sleeps or retries.
/// @note Startup owns and takes its exact results before console admissions. UART loss disables
/// input rather than assembling commands from damaged bytes. Shutdown retains/reports effects,
/// ends binding, releases only owned resources, then drains output for at most 64 further turns.
/// A failed console delete gets at most 64 additional output-only turns, without another delete.
bool loopExample();

/// @brief Request orderly local shutdown from application task context, without SDK or core calls.
/// @note The serialized owner handles cancellation/end on its next turn. Not a CLI command or ISR API.
void requestStop();

} // namespace tusb32x_idf
