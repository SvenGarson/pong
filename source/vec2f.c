#include <vec2f.h>
#include <math.h>

struct vec2f vec2f_sub(struct vec2f left, struct vec2f right)
{
  return (struct vec2f){
    left.x - right.x,
    left.y - right.y
  };
}

struct vec2f vec2f_scale(struct vec2f v, float scale)
{
  return (struct vec2f){ v.x * scale, v.y * scale };
}

float vec2f_dot(struct vec2f a, struct vec2f b)
{
  return (a.x * b.x) + (a.y * b.y);
}

float vec2f_length(struct vec2f v)
{
  return sqrt((v.x * v.x) + (v.y * v.y));
}

struct vec2f vec2f_normalize(struct vec2f v)
{
  const float vlen = vec2f_length(v);
  if (vlen != 0.0f)
    return (struct vec2f){ v.x / vlen, v.y / vlen};
  else
    return v;
}

struct vec2f vec2f_make(float x, float y)
{
  return (struct vec2f) { x, y };
}
