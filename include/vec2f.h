#ifndef VEC2F_H
#define VEC2F_H

/* Data types */
struct vec2f {
  float x;
  float y;
};

/* Function prototypes */
struct vec2f vec2f_sub(struct vec2f left, struct vec2f right);
struct vec2f vec2f_scale(struct vec2f v, float scale);
float vec2f_dot(struct vec2f a, struct vec2f b);
float vec2f_length(struct vec2f v);
struct vec2f vec2f_normalize(struct vec2f v);
struct vec2f vec2f_make(float x, float y);

#endif