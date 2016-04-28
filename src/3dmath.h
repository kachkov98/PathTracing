#pragma once
#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <emmintrin.h>

struct Vec3
{
        double x, y, z;
        Vec3 (double X = 0.0, double Y = 0.0, double Z = 0.0) :
                x (X),
                y (Y),
                z (Z)
        {
        }
        Vec3 operator+ (const Vec3 &r)
        {
                return Vec3 (x + r.x, y + r.y, z + r.z);
        }
        Vec3 operator+= (const Vec3 &r)
        {
                x += r.x;
                y += r.y;
                z += r.z;
                return *this;
        }
        Vec3 operator- (const Vec3 &r)
        {
                return Vec3 (x - r.x, y - r.y, z - r.z);
        }
        Vec3 operator-= (const Vec3 &r)
        {
                x -= r.x;
                y -= r.y;
                z -= r.z;
                return *this;
        }
        Vec3 operator* (double r)
        {
                return Vec3 (x * r, y * r, z * r);
        }
        Vec3 operator*= (double r)
        {
                x *= r;
                y *= r;
                z *= r;
                return *this;
        }
        Vec3 operator* (Vec3 r)
        {
                return Vec3 (x * r.x, y * r.y, z * r.z);
        }
        Vec3 operator% (Vec3 r)
        {
                return Vec3 (y * r.z - z * r.y, z * r.x - x * r.z, x * r.y - y * r.x);
        }
        double operator^ (Vec3 r)
        {
                return x * r.x + y * r.y + z * r.z;
        }
        Vec3 norm ()
        {
                double inv_len = 1.0 / sqrt (x * x + y * y + z * z);
                x *= inv_len;
                y *= inv_len;
                z *= inv_len;
                return *this;
        }
        double len2 ()
        {
                return x * x + y * y + z * z;
        }
};

struct Ray
{
        Vec3 start, dir;
        Ray ()
        {
        }
        Ray (Vec3 Start, Vec3 Dir) :
                start (Start),
                dir (Dir)
        {
        }
};

inline double clamp (double x)
{
        return (x < 0.0) ? 0.0 : (x > 1.0) ? 1.0 : x;
}

inline unsigned char gamma (double x)
{
        return (unsigned char)(pow (x, 1.0 / 2.2) * 255.0 + 0.5);
}

__declspec(align(16)) static __m128i cur_seed;

inline void drand_init (unsigned int seed)
{
        cur_seed = _mm_set_epi32 (seed, seed + 1, seed, seed + 1);
}

inline double drand ()
{
        __declspec(align(16)) __m128i result;
        __declspec(align(16)) __m128i sseresult;
        __declspec(align(16)) __m128i cur_seed_split;
        __declspec(align(16)) __m128i multiplier;
        __declspec(align(16)) __m128i adder;
        __declspec(align(16)) __m128i mod_mask;
        __declspec(align(16)) __m128i sra_mask;
        __declspec(align(16)) static const unsigned int mult[4] = { 214013, 17405, 214013, 69069 };
        __declspec(align(16)) static const unsigned int gadd[4] = { 2531011, 10395331, 13737667, 1 };
        __declspec(align(16)) static const unsigned int mask[4] = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
        __declspec(align(16)) static const unsigned int masklo[4] = { 0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF };

        adder = _mm_load_si128 ((__m128i*) gadd);
        multiplier = _mm_load_si128 ((__m128i*) mult);
        mod_mask = _mm_load_si128 ((__m128i*) mask);
        sra_mask = _mm_load_si128 ((__m128i*) masklo);
        cur_seed_split = _mm_shuffle_epi32 (cur_seed, _MM_SHUFFLE (2, 3, 0, 1));
        cur_seed = _mm_mul_epu32 (cur_seed, multiplier);
        multiplier = _mm_shuffle_epi32 (multiplier, _MM_SHUFFLE (2, 3, 0, 1));
        cur_seed_split = _mm_mul_epu32 (cur_seed_split, multiplier);
        cur_seed = _mm_and_si128 (cur_seed, mod_mask);
        cur_seed_split = _mm_and_si128 (cur_seed_split, mod_mask);
        cur_seed_split = _mm_shuffle_epi32 (cur_seed_split, _MM_SHUFFLE (2, 3, 0, 1));
        cur_seed = _mm_or_si128 (cur_seed, cur_seed_split);
        cur_seed = _mm_add_epi32 (cur_seed, adder);
        sseresult = _mm_srai_epi32 (cur_seed, 16);
        sseresult = _mm_and_si128 (sseresult, sra_mask);
        _mm_storeu_si128 ((__m128i*) &result, sseresult);

        return result.m128i_u16[0] / 65536.0;
}

inline double fast_sin (double a)
{
        double sin;
        if (a < 0)
                sin = a * (1.27323954 + 0.405284735 * a);
        else
                sin = a * (1.27323954 - 0.405284735 * a);
        if (sin < 0.0)
                sin *= (0.745 - 0.255 * sin);
        else
                sin *= (0.745 + 0.255 * sin);
        return sin;
}

inline double fast_cos (double a)
{
        a += M_PI_2;
        if (a >= M_PI)
                a -= M_PI * 2.0;
        return fast_sin (a);
}
