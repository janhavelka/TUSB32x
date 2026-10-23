// Independent translation unit exercises the header's constexpr/inline contract.
#include <TUSB32x/Status.h>

static_assert(TUSB32x::Status::Error(TUSB32x::Err::I2C_BUS, "bus", -70000).detail == -70000);
static_assert(TUSB32x::errorName(TUSB32x::Err::I2C_TIMEOUT)[4] == 'T');

const char* errorNameFromOtherTranslationUnit(TUSB32x::Err code) {
  return TUSB32x::errorName(code);
}

TUSB32x::Status statusFromOtherTranslationUnit() {
  return TUSB32x::Status::Error(TUSB32x::Err::I2C_ERROR, "other translation unit", 100000);
}
