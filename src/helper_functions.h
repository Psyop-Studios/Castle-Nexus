#ifndef PSY_HELPERS_H
#define PSY_HELPERS_H


#define PSY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PSY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define PSY_CLAMP(a, min, max) ((a) < (min) ? (min) : ((a) > (max) ? (max) : (a)))


#endif