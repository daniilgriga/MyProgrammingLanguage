#pragma once

#define YELLOW      "\033[1;33m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define LIGHT_BLUE  "\033[1;36m"
#define PURPLE      "\033[1;35m"
#define BLUE        "\033[1;34m"
#define CLEAR_COLOR "\033[0m"

#define PURPLE_TEXT(text)     PURPLE text CLEAR_COLOR
#define BLUE_TEXT(text)       BLUE text CLEAR_COLOR
#define LIGHT_BLUE_TEXT(text) LIGHT_BLUE text CLEAR_COLOR
#define GREEN_TEXT(text)      GREEN text CLEAR_COLOR
#define RED_TEXT(text)        RED text CLEAR_COLOR
#define YELLOW_TEXT(text)     YELLOW text CLEAR_COLOR
