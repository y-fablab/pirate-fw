//
//  util.h
//
//  Created by Gabriele Mondada on 2023.
//  Copyright (c) 2023 Gabriele Mondada.
//  Distributed under the terms of the MIT License.
//

#pragma once

#define ARRAY_LEN(a) (sizeof(a) / sizeof(*(a)))

template<typename T> T asym_div(T n, T d) {
    return (n < 0) ? ((n + 1) / d - 1) : (n / d);
}

template<typename T> T asym_mod(T n, T d) {
    return (n < 0) ? (d - 1 + (n + 1) % d) : (n % d);
}

template<typename T> T sign(T n) {
    if (n > 0) return 1;
    if (n < 0) return -1;
    return 0;
}
