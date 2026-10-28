/// @file consumer.h
/// @brief Fixed checks shared by two public-only consumer translation units.
#pragma once

#include <TUSB32x/TUSB32x.h>

/// @brief Link pure functions and inspect facade copies in a separate translation unit.
/// @param[in] driver Borrowed driver; no callback or mutation occurs.
/// @return True when helper and copied-cache contracts hold.
bool inspectFromOtherTranslationUnit(const TUSB32x::TUSB32x& driver);
