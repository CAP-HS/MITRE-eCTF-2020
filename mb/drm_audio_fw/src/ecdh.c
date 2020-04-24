/* 

  Crypto using elliptic curves defined over the finite binary field GF(2^m) where m is prime.

  The curves used are the anomalous binary curves (ABC-curves) or also called Koblitz curves.

  This class of curves was chosen because it yields efficient implementation of operations.



  Curves available - their different NIST/SECG names and eqivalent symmetric security level:

      NIST      SEC Group     strength
    ------------------------------------
      K-163     sect163k1      80 bit
      B-163     sect163r2      80 bit
      K-233     sect233k1     112 bit
      B-233     sect233r1     112 bit
      K-283     sect283k1     128 bit
      B-283     sect283r1     128 bit
      K-409     sect409k1     192 bit
      B-409     sect409r1     192 bit
      K-571     sect571k1     256 bit
      B-571     sect571r1     256 bit



  Curve parameters from:

    http://www.secg.org/sec2-v2.pdf
    http://csrc.nist.gov/publications/fips/fips186-3/fips_186-3.pdf


  Reference:

    https://www.ietf.org/rfc/rfc4492.txt 
*/

#include <stdint.h>
#include "ecdh.h"


/* margin for overhead needed in intermediate calculations */
#define BITVEC_MARGIN     3
#define BITVEC_NBITS      (CURVE_DEGREE + BITVEC_MARGIN)
#define BITVEC_NWORDS     ((BITVEC_NBITS + 31) / 32)
#define BITVEC_NBYTES     (sizeof(uint32_t) * BITVEC_NWORDS)


/* Disable assertions? */
#ifndef DISABLE_ASSERT
 #define DISABLE_ASSERT 0
#endif

#if defined(DISABLE_ASSERT) && (DISABLE_ASSERT == 1)
 #define assert(...)
#else
 #include <assert.h>
#endif

/* Default to a (somewhat) constant-time mode?
   NOTE: The library is _not_ capable of operating in constant-time and leaks information via timing.
         Even if all operations are written const-time-style, it requires the hardware is able to multiply in constant time. 
         Multiplication on ARM Cortex-M processors takes a variable number of cycles depending on the operands...
*/
#ifndef CONST_TIME
  #define CONST_TIME 0
#endif

/* Default to using ECC_CDH (cofactor multiplication-variation) ? */
#ifndef ECDH_COFACTOR_VARIANT
  #define ECDH_COFACTOR_VARIANT 0
#endif

/******************************************************************************/


/* the following type will represent bit vectors of length (CURVE_DEGREE+MARGIN) */
typedef uint32_t bitvec_t[BITVEC_NWORDS];
typedef bitvec_t gf2elem_t;           /* this type will represent field elements */
typedef bitvec_t scalar_t;
 

/******************************************************************************/

/* Here the curve parameters are defined. */

 #if (ECC_CURVE == NIST_B163)
  #define coeff_a  1
  #define cofactor 2
/* NIST B-163 */
const gf2elem_t polynomial = { 0x000000c9, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000008 }; 
const gf2elem_t coeff_b    = { 0x4a3205fd, 0x512f7874, 0x1481eb10, 0xb8c953ca, 0x0a601907, 0x00000002 }; 
const gf2elem_t base_x     = { 0xe8343e36, 0xd4994637, 0xa0991168, 0x86a2d57e, 0xf0eba162, 0x00000003 }; 
const gf2elem_t base_y     = { 0x797324f1, 0xb11c5c0c, 0xa2cdd545, 0x71a0094f, 0xd51fbc6c, 0x00000000 }; 
const scalar_t  base_order = { 0xa4234c33, 0x77e70c12, 0x000292fe, 0x00000000, 0x00000000, 0x00000004 }; 
 #endif



/*************************************************************************************************/

/* Private / static functions: */


/* some basic bit-manipulation routines that act on bit-vectors follow */
static int bitvec_get_bit(const bitvec_t x, const uint32_t idx)
{
  return ((x[idx / 32U] >> (idx & 31U) & 1U));
}

static void bitvec_clr_bit(bitvec_t x, const uint32_t idx)
{
  x[idx / 32U] &= ~(1U << (idx & 31U));
}

static void bitvec_copy(bitvec_t x, const bitvec_t y)
{
  int i;
  for (i = 0; i < BITVEC_NWORDS; ++i)
  {
    x[i] = y[i];
  }
}

static void bitvec_swap(bitvec_t x, bitvec_t y)
{
  bitvec_t tmp;
  bitvec_copy(tmp, x);
  bitvec_copy(x, y);
  bitvec_copy(y, tmp);
}

#if defined(CONST_TIME) && (CONST_TIME == 0)
/* fast version of equality test */
static int bitvec_equal(const bitvec_t x, const bitvec_t y)
{
  int i;
  for (i = 0; i < BITVEC_NWORDS; ++i)
  {
    if (x[i] != y[i])
    {
      return 0;
    }
  }
  return 1;
}
#else
/* constant time version of equality test */
static int bitvec_equal(const bitvec_t x, const bitvec_t y)
{
  int ret = 1;
  int i;
  for (i = 0; i < BITVEC_NWORDS; ++i)
  {
    ret &= (x[i] == y[i]);
  }
  return ret;
}
#endif

static void bitvec_set_zero(bitvec_t x)
{
  int i;
  for (i = 0; i < BITVEC_NWORDS; ++i)
  {
    x[i] = 0;
  }
}

#if defined(CONST_TIME) && (CONST_TIME == 0)
/* fast implementation */
static int bitvec_is_zero(const bitvec_t x)
{
  uint32_t i = 0;
  while (i < BITVEC_NWORDS)
  {
    if (x[i] != 0)
    {
      break;
    }
    i += 1;
  }
  return (i == BITVEC_NWORDS);
}
#else
/* constant-time implementation */
static int bitvec_is_zero(const bitvec_t x)
{
  int ret = 1;
  int i = 0;
  for (i = 0; i < BITVEC_NWORDS; ++i)
  {
    ret &= (x[i] == 0);
  }
  return ret;
}
#endif

/* return the number of the highest one-bit + 1 */
static int bitvec_degree(const bitvec_t x)
{
  int i = BITVEC_NWORDS * 32;

  /* Start at the back of the vector (MSB) */
  x += BITVEC_NWORDS;

  /* Skip empty / zero words */
  while (    (i > 0)
          && (*(--x)) == 0)
  {
    i -= 32;
  }
  /* Run through rest if count is not multiple of bitsize of DTYPE */
  if (i != 0)
  {
    uint32_t u32mask = ((uint32_t)1 << 31);
    while (((*x) & u32mask) == 0)
    {
      u32mask >>= 1;
      i -= 1;
    }
  }
  return i;
}

/* left-shift by 'count' digits */
static void bitvec_lshift(bitvec_t x, const bitvec_t y, int nbits)
{
  int nwords = (nbits / 32);

  /* Shift whole words first if nwords > 0 */
  int i,j;
  for (i = 0; i < nwords; ++i)
  {
    /* Zero-initialize from least-significant word until offset reached */
    x[i] = 0;
  }
  j = 0;
  /* Copy to x output */
  while (i < BITVEC_NWORDS)
  {
    x[i] = y[j];
    i += 1;
    j += 1;
  }

  /* Shift the rest if count was not multiple of bitsize of DTYPE */
  nbits &= 31;
  if (nbits != 0)
  {
    /* Left shift rest */
    int i;
    for (i = (BITVEC_NWORDS - 1); i > 0; --i)
    {
      x[i]  = (x[i] << nbits) | (x[i - 1] >> (32 - nbits));
    }
    x[0] <<= nbits;
  }
}


/*************************************************************************************************/
/*
  Code that does arithmetic on bit-vectors in the Galois Field GF(2^CURVE_DEGREE).
*/
/*************************************************************************************************/


static void gf2field_set_one(gf2elem_t x)
{
  /* Set first word to one */
  x[0] = 1;
  /* .. and the rest to zero */
  int i;
  for (i = 1; i < BITVEC_NWORDS; ++i)
  {
    x[i] = 0;
  }
}

#if defined(CONST_TIME) && (CONST_TIME == 0)
/* fastest check if x == 1 */
static int gf2field_is_one(const gf2elem_t x) 
{
  /* Check if first word == 1 */
  if (x[0] != 1)
  {
    return 0;
  }
  /* ...and if rest of words == 0 */
  int i;
  for (i = 1; i < BITVEC_NWORDS; ++i)
  {
    if (x[i] != 0)
    {
      break;
    }
  }
  return (i == BITVEC_NWORDS);
}
#else
/* constant-time check */
static int gf2field_is_one(const gf2elem_t x)
{
  int ret = 0;
  /* Check if first word == 1 */
  if (x[0] == 1)
  {
    ret = 1;
  }
  /* ...and if rest of words == 0 */
  int i;
  for (i = 1; i < BITVEC_NWORDS; ++i)
  {
    ret &= (x[i] == 0);
  }
  return ret; //(i == BITVEC_NWORDS);
}
#endif


/* galois field(2^m) addition is modulo 2, so XOR is used instead - 'z := a + b' */
static void gf2field_add(gf2elem_t z, const gf2elem_t x, const gf2elem_t y)
{
  int i;
  for (i = 0; i < BITVEC_NWORDS; ++i)
  {
    z[i] = (x[i] ^ y[i]);
  }
}

/* increment element */
static void gf2field_inc(gf2elem_t x)
{
  x[0] ^= 1;
}


/* field multiplication 'z := (x * y)' */
static void gf2field_mul(gf2elem_t z, const gf2elem_t x, const gf2elem_t y)
{
  int i;
  gf2elem_t tmp;
#if defined(CONST_TIME) && (CONST_TIME == 1)
  gf2elem_t blind;
  bitvec_set_zero(blind);
#endif
  assert(z != y);

  bitvec_copy(tmp, x);

  /* LSB set? Then start with x */
  if (bitvec_get_bit(y, 0) != 0)
  {
    bitvec_copy(z, x);
  }
  else /* .. or else start with zero */
  {
    bitvec_set_zero(z);
  }

  /* Then add 2^i * x for the rest */
  for (i = 1; i < CURVE_DEGREE; ++i)
  {
    /* lshift 1 - doubling the value of tmp */
    bitvec_lshift(tmp, tmp, 1);

    /* Modulo reduction polynomial if degree(tmp) > CURVE_DEGREE */
    if (bitvec_get_bit(tmp, CURVE_DEGREE))
    {
      gf2field_add(tmp, tmp, polynomial);
    }
#if defined(CONST_TIME) && (CONST_TIME == 1)
    else /* blinding operation */
    {
      gf2field_add(tmp, tmp, blind);
    }
#endif

    /* Add 2^i * tmp if this factor in y is non-zero */
    if (bitvec_get_bit(y, i))
    {
      gf2field_add(z, z, tmp);
    }
#if defined(CONST_TIME) && (CONST_TIME == 1)
    else /* blinding operation */
    {
      gf2field_add(z, z, blind);
    }
#endif
  }
}

/* field inversion 'z := 1/x' */
static void gf2field_inv(gf2elem_t z, const gf2elem_t x)
{
  gf2elem_t u, v, g, h;
  int i;

  bitvec_copy(u, x);
  bitvec_copy(v, polynomial);
  bitvec_set_zero(g);
  gf2field_set_one(z);
  
  while (!gf2field_is_one(u))
  {
    i = (bitvec_degree(u) - bitvec_degree(v));

    if (i < 0)
    {
      bitvec_swap(u, v);
      bitvec_swap(g, z);
      i = -i;
    }
#if defined(CONST_TIME) && (CONST_TIME == 1)
    else
    {
      bitvec_swap(u, v);
      bitvec_swap(v, u);
    }
#endif
    bitvec_lshift(h, v, i);
    gf2field_add(u, u, h);
    bitvec_lshift(h, g, i);
    gf2field_add(z, z, h);
  }
}

/*************************************************************************************************/
/*
   The following code takes care of Galois-Field arithmetic. 
   Elliptic curve points are represented  by pairs (x,y) of bitvec_t. 
   It is assumed that curve coefficient 'a' is {0,1}
   This is the case for all NIST binary curves.
   Coefficient 'b' is given in 'coeff_b'.
   '(base_x, base_y)' is a point that generates a large prime order group.
*/
/*************************************************************************************************/


static void gf2point_copy(gf2elem_t x1, gf2elem_t y1, const gf2elem_t x2, const gf2elem_t y2)
{
  bitvec_copy(x1, x2);
  bitvec_copy(y1, y2);
}

static void gf2point_set_zero(gf2elem_t x, gf2elem_t y)
{
  bitvec_set_zero(x);
  bitvec_set_zero(y);
}

static int gf2point_is_zero(const gf2elem_t x, const gf2elem_t y)
{
  return (    bitvec_is_zero(x)
           && bitvec_is_zero(y));
}

/* double the point (x,y) */
static void gf2point_double(gf2elem_t x, gf2elem_t y)
{
  /* iff P = O (zero or infinity): 2 * P = P */
  if (bitvec_is_zero(x))
  {
    bitvec_set_zero(y);
  }
  else
  {
    gf2elem_t l;

    gf2field_inv(l, x);
    gf2field_mul(l, l, y);
    gf2field_add(l, l, x);
    gf2field_mul(y, x, x);
    gf2field_mul(x, l, l);
#if (coeff_a == 1)
    gf2field_inc(l);
#endif
    gf2field_add(x, x, l);
    gf2field_mul(l, l, x);
    gf2field_add(y, y, l);
  }
}


/* add two points together (x1, y1) := (x1, y1) + (x2, y2) */
static void gf2point_add(gf2elem_t x1, gf2elem_t y1, const gf2elem_t x2, const gf2elem_t y2)
{
  if (!gf2point_is_zero(x2, y2))
  {
    if (gf2point_is_zero(x1, y1))
    {
      gf2point_copy(x1, y1, x2, y2);
    }
    else
    {
      if (bitvec_equal(x1, x2))
      {
        if (bitvec_equal(y1, y2))
        {
          gf2point_double(x1, y1);
        }
        else
        {
          gf2point_set_zero(x1, y1);
        }
      }
      else
      {
        /* Arithmetic with temporary variables */
        gf2elem_t a, b, c, d;

        gf2field_add(a, y1, y2);
        gf2field_add(b, x1, x2);
        gf2field_inv(c, b);
        gf2field_mul(c, c, a);
        gf2field_mul(d, c, c);
        gf2field_add(d, d, c);
        gf2field_add(d, d, b);
#if (coeff_a == 1)
        gf2field_inc(d);
#endif
        gf2field_add(x1, x1, d);
        gf2field_mul(a, x1, c);
        gf2field_add(a, a, d);
        gf2field_add(y1, y1, a);
        bitvec_copy(x1, d);
      }
    }
  }
}



#if defined(CONST_TIME) && (CONST_TIME == 0)
/* point multiplication via double-and-add algorithm */
static void gf2point_mul(gf2elem_t x, gf2elem_t y, const scalar_t exp)
{
  gf2elem_t tmpx, tmpy;
  int i;
  int nbits = bitvec_degree(exp);

  gf2point_set_zero(tmpx, tmpy);

  for (i = (nbits - 1); i >= 0; --i)
  {
    gf2point_double(tmpx, tmpy);
    if (bitvec_get_bit(exp, i))
    {
      gf2point_add(tmpx, tmpy, x, y);
    }
  }
  gf2point_copy(x, y, tmpx, tmpy);
}
#else
/* point multiplication via double-and-add-always algorithm using scalar blinding */
static void gf2point_mul(gf2elem_t x, gf2elem_t y, const scalar_t exp)
{
  gf2elem_t tmpx, tmpy;
  gf2elem_t dummyx, dummyy;
  int i;
  int nbits = bitvec_degree(exp);

  gf2point_set_zero(tmpx, tmpy);
  gf2point_set_zero(dummyx, dummyy);

  for (i = (nbits - 1); i >= 0; --i)
  {
    gf2point_double(tmpx, tmpy);

    /* Add point if bit(i) is set in exp */
    if (bitvec_get_bit(exp, i))
    {
      gf2point_add(tmpx, tmpy, x, y);
    }
    /* .. or add the neutral element to keep operation constant-time */
    else
    {
      gf2point_add(tmpx, tmpy, dummyx, dummyy);
    }
  }
  gf2point_copy(x, y, tmpx, tmpy);
}
#endif



/* check if y^2 + x*y = x^3 + a*x^2 + coeff_b holds */
static int gf2point_on_curve(const gf2elem_t x, const gf2elem_t y)
{
  gf2elem_t a, b;

  if (gf2point_is_zero(x, y))
  {
    return 1;
  }
  else
  {
    gf2field_mul(a, x, x);
#if (coeff_a == 0)
    gf2field_mul(a, a, x);
#else
    gf2field_mul(b, a, x);
    gf2field_add(a, a, b);
#endif
    gf2field_add(a, a, coeff_b);
    gf2field_mul(b, y, y);
    gf2field_add(a, a, b);
    gf2field_mul(b, x, y);

    return bitvec_equal(a, b);
  }
}


/*************************************************************************************************/
/*
  Elliptic Curve Diffie-Hellman key exchange protocol.
*/
/*************************************************************************************************/


int ecdh_shared_secret(const uint8_t* private_key, const uint8_t* others_pub, uint8_t* output)
{
  /* Do some basic validation of other party's public key */
  if (    !gf2point_is_zero ((uint32_t*)others_pub, (uint32_t*)(others_pub + BITVEC_NBYTES))
       &&  gf2point_on_curve((uint32_t*)others_pub, (uint32_t*)(others_pub + BITVEC_NBYTES)) )
  {
    /* Copy other side's public key to output */
    unsigned int i;
    for (i = 0; i < (BITVEC_NBYTES * 2); ++i)
    {
      output[i] = others_pub[i];
    }

    /* Multiply other side's public key with own private key */
    gf2point_mul((uint32_t*)output,(uint32_t*)(output + BITVEC_NBYTES), (const uint32_t*)private_key);

    /* Multiply outcome by cofactor if using ECC CDH-variant: */
#if defined(ECDH_COFACTOR_VARIANT) && (ECDH_COFACTOR_VARIANT == 1)
 #if   (cofactor == 2)
    gf2point_double((uint32_t*)output, (uint32_t*)(output + BITVEC_NBYTES));
 #elif (cofactor == 4)
    gf2point_double((uint32_t*)output, (uint32_t*)(output + BITVEC_NBYTES));
    gf2point_double((uint32_t*)output, (uint32_t*)(output + BITVEC_NBYTES));
 #endif
#endif
    
    return 1;
  }
  else
  {
    return 0;
  }
}
