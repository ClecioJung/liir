// MIT License

// Copyright (c) 2022 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//------------------------------------------------------------------------------
// HEADER
//------------------------------------------------------------------------------

#ifndef __PRINT_ERRORS
#define __PRINT_ERRORS


// Font definitions
#define RESET_FONT "\033[0m"
#define BOLD_FONT "\033[1m"
#define FAINT_FONT "\033[2m"
#define ITALIC_FONT_FONT "\033[3m"
#define UNDERLINE_FONT "\033[4m"

// Foreground color definitions
#define BLACK_FOREGROUND "\033[30m"
#define RED_FOREGROUND "\033[31m"
#define GREEN_FOREGROUND "\033[32m"
#define YELLOW_FOREGROUND "\033[33m"
#define BLUE_FOREGROUND "\033[34m"
#define MAGENTA_FOREGROUND "\033[35m"
#define CYAN_FOREGROUND "\033[36m"
#define WHITE_FOREGROUND "\033[37m"

#define BRIGHT_BLACK_FOREGROUND "\033[90m"
#define BRIGHT_RED_FOREGROUND "\033[91m"
#define BRIGHT_GREEN_FOREGROUND "\033[92m"
#define BRIGHT_YELLOW_FOREGROUND "\033[93m"
#define BRIGHT_BLUE_FOREGROUND "\033[94m"
#define BRIGHT_MAGENTA_FOREGROUND "\033[95m"
#define BRIGHT_CYAN_FOREGROUND "\033[96m"
#define BRIGHT_WHITE_FOREGROUND "\033[97m"

// Background color definitions
#define BLACK_BACKGROUND "\033[40m"
#define RED_BACKGROUND "\033[41m"
#define GREEN_BACKGROUND "\033[42m"
#define YELLOW_BACKGROUND "\033[43m"
#define BLUE_BACKGROUND "\033[44m"
#define MAGENTA_BACKGROUND "\033[45m"
#define CYAN_BACKGROUND "\033[46m"
#define WHITE_BACKGROUND "\033[47m"

#define BRIGHT_BLACK_BACKGROUND "\033[100m"
#define BRIGHT_RED_BACKGROUND "\033[101m"
#define BRIGHT_GREEN_BACKGROUND "\033[102m"
#define BRIGHT_YELLOW_BACKGROUND "\033[103m"
#define BRIGHT_BLUE_BACKGROUND "\033[104m"
#define BRIGHT_MAGENTA_BACKGROUND "\033[105m"
#define BRIGHT_CYAN_BACKGROUND "\033[106m"
#define BRIGHT_WHITE_BACKGROUND "\033[107m"

#ifndef __GNUC__
#define __attribute__(a)
#endif

void print_crash_and_exit(const char *const msg, ...) __attribute__((__noreturn__, format(printf, 1, 2)));
void print_error(const char *const msg, ...) __attribute__((format(printf, 1, 2)));
void print_warning(const char *const msg, ...) __attribute__((format(printf, 1, 2)));
void print_column(const int column);

#endif  // __PRINT_ERRORS

//------------------------------------------------------------------------------
// END
//------------------------------------------------------------------------------

// MIT License

// Copyright (c) 2022 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.