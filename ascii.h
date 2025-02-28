#pragma once

#include "common.h"

bool
ascii_is_alpha(char ch);

bool
ascii_is_digit(char ch);

bool
ascii_is_upper(char ch);

bool
ascii_is_lower(char ch);

// https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Whitespace.html
bool
ascii_is_whitespace(char ch);
