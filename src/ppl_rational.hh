#include <ppl.hh>

/*!
 * @brief Rational coefficient representation for PPL.
 */
class PPLRational {
private:
  Parma_Polyhedra_Library::Coefficient numerator;
  Parma_Polyhedra_Library::Coefficient denominator;

  /*!
   * @brief Compute the greatest common divisor (GCD) of two coefficients with
   * the Euclidean algorithm.
   */
  static Parma_Polyhedra_Library::Coefficient
  gcd(Parma_Polyhedra_Library::Coefficient a,
      Parma_Polyhedra_Library::Coefficient b) {
    while (b != 0) {
      Parma_Polyhedra_Library::Coefficient temp = b;
      b = a % b;
      a = temp;
    }

    return a < 0 ? -a : a;
  }

protected:
  /*
   * @brief Reduce the rational number to its simplest form.
   */
  void reduce() {
    if (denominator < 0) {
      numerator = -numerator;
      denominator = -denominator;
    }

    const Parma_Polyhedra_Library::Coefficient gcd = this->gcd(numerator, denominator);

    if (gcd != 0 && gcd != 1) {
      numerator /= gcd;
      denominator /= gcd;
    }
  }

public:
  PPLRational() : numerator(0), denominator(1) {}

  PPLRational(Parma_Polyhedra_Library::Coefficient numerator,
              Parma_Polyhedra_Library::Coefficient denominator)
      : numerator(numerator), denominator(denominator) {
    if (denominator == 0) {
      throw std::invalid_argument("Denominator cannot be zero.");
    }
    reduce();
  }

  Parma_Polyhedra_Library::Coefficient getNumerator() const {
    return numerator;
  }

  Parma_Polyhedra_Library::Coefficient getDenominator() const {
    return denominator;
  }

  /*
   * @brief Unary negation.
   */
  PPLRational operator-() const {
    return PPLRational(-numerator, denominator);
  }

  /*
   * @brief Subtraction between two rationals.
   */
  PPLRational operator-(const PPLRational &other) const {
    const auto num = numerator * other.denominator - other.numerator * denominator;
    const auto den = denominator * other.denominator;
    return PPLRational(num, den);
  }
};

static inline std::ostream &operator<<(std::ostream &os, const PPLRational &r) {
  os << r.getNumerator() << "/" << r.getDenominator();
  return os;
}

/*!
 * @brief Read a rational number from its decimal representation (e.g., "-1.05" or ".2").
 */
static inline std::istream &operator>>(std::istream &is, PPLRational &r) {
  Parma_Polyhedra_Library::Coefficient numerator, denominator;
  bool isNegative = false;
  bool lessThanOne = false;
  numerator = 0;
  denominator = 1;
  char ch;
  // Skip leading whitespace
  is >> std::ws;
  if (is.eof()) {
    is.setstate(std::ios::failbit);
    return is;
  }
  if (is.peek() == '-') {
      isNegative = true;
      is >> ch; // consume the '-'
  } else if (is.peek() == '+') {
      is >> ch; // consume the '+'
  }
  while (is.peek() == '.' || isdigit(is.peek())) {
    is >> ch;
    if (ch == '.') {
      if (lessThanOne) {
        // Second decimal point encountered, invalid input
        is.setstate(std::ios::failbit);
        return is;
      } else {
        lessThanOne = true;
      }
    } else if (isdigit(ch)) {
      numerator = numerator * 10 + (ch - '0');
      if (lessThanOne) {
        denominator *= 10;
      }
    } else {
      throw std::runtime_error(
          "Unexpected character encountered while parsing rational number.");
    }
  }

  r = PPLRational(isNegative ? -numerator : numerator, denominator);
  return is;
}
