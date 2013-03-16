

/* wikipedia version */
double cube_root(unsigned long a_)
{
    unsigned long   x_ = 1;
    double          x, xxx, a;
    if (a_ == 0)
        return 0.0;
    if (a_ == 1)
        return 1.0;
    a = (double) a_;
    do {
        a_ >>= 3;
        x_ <<= 1;
    } while (a_);
    x = (double) x_;
    xxx = x * x * x, x = x * (xxx + a + a) / (xxx + xxx + a);
    /* Accurate to 2 decimal digits: */
    xxx = x * x * x, x = x * (xxx + a + a) / (xxx + xxx + a);
    /* Accurate to 7 decimal digits: */
    xxx = x * x * x, x = x * (xxx + a + a) / (xxx + xxx + a);
    /* Accurate to 15 decimal digits: */
    xxx = x * x * x, x = x * (xxx + a + a) / (xxx + xxx + a);
    return x;
}

/* apples`'s version of Halley's method */
double cbrt(double x)
{
  double x_a = 1, x_b = 0;
  while (x_a != x_b)
  {
    x_b = x_a;
    x_a = x_a * (((x_a * x_a * x_a) + (2 * x)) / (2 * (x_a * x_a * x_a) + x));
  }
  return x_a;
}

main(void)
{
  return 0;
}

