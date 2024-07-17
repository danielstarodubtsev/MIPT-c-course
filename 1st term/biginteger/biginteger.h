#include <algorithm>
#include <compare>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const int kRationalToDoublePrecision = 20;
const int kBase = 1e9;
const int kBaseLength = 9;

enum Sign { positive, negative };

Sign operator! (const Sign& sign) {
  return sign == positive ? negative : positive;
}

Sign operator* (const Sign& first, const Sign& second) {
  return (first == negative || second == negative) &&
         (first == positive || second == positive) ? negative : positive;
}

class BigInteger {
 private:
  std::vector<long long> digits_;
  Sign sign_ = positive;

  void NormalizeDigits() {
    while (!digits_.empty() && digits_.back() == 0) {
      digits_.pop_back();
    }

    if (digits_.empty()) {
      digits_.push_back(0);
      sign_ = positive;
    }
  }

  void BuildFromUll(unsigned long long num) {
    while (num) {
      digits_.push_back(std::abs((int)(num % kBase)));
      num /= kBase;
    }
  }

  BigInteger(std::vector<long long> digits, Sign sign) : digits_(digits), sign_(sign) {}

 public:
  BigInteger operator- () const {
    if (digits_.size() > 1 || digits_[0] != 0) {
      return BigInteger{digits_, !sign_};
    }

    return BigInteger{digits_, positive};
  }

  BigInteger& operator+= (const BigInteger& second_num) {
    BigInteger num{second_num};
    std::vector<long long> res_digits;
    int extra = 0;
    int cur_res;

    if (num.digits_.size() < digits_.size()) {
      num.digits_.resize(digits_.size(), 0);
    } else if (num.digits_.size() > digits_.size()) {
      digits_.resize(num.digits_.size(), 0);
    }

    if (sign_ == positive && num.sign_ == positive) {
      for (int i = 0; i <= (int)digits_.size() - 1; i++) {
        cur_res = digits_[i] + num.digits_[i] + extra;
        res_digits.push_back(cur_res % kBase);
        extra = cur_res / kBase;
      }

      if (extra) {
        res_digits.push_back(1);
      }

      digits_ = res_digits;
      sign_ = positive;
      NormalizeDigits();
      return *this;
    }

    if (sign_ == positive && num.sign_ == negative) {
      bool swapped = false;

      if (*this < -num) {
        swapped = true;
        std::swap(digits_, num.digits_);
      }

      for (int i = 0; i <= (int)num.digits_.size() - 1; i++) {
        cur_res = digits_[i] - num.digits_[i] + extra;
        extra = 0;

        if (cur_res < 0) {
          extra = -1;
          cur_res += kBase;
        }

        res_digits.push_back(cur_res);
      }

      if (extra == -1) {
        extra = 0;
        res_digits[0] = kBase - res_digits[0];

        if (res_digits[0] == kBase) {
          res_digits[0] = 0;
          extra = 1;
        }

        for (size_t i = 1; i < res_digits.size(); i++) {
          res_digits[i] = kBase - 1 + extra - res_digits[i];
        }

        if (res_digits.back() == kBase - 1) {
          res_digits.pop_back();
        }
      }

      if (swapped) {
        std::swap(digits_, num.digits_);
      }

      digits_ = res_digits;
      sign_ = swapped ? negative : positive;
      NormalizeDigits();
      return *this;
    }

    *this = -*this;
    *this += -num;
    *this = -*this;
    return *this;
  }

  BigInteger& operator-= (const BigInteger& second_num) {
    return *this += -second_num;
  }

  BigInteger& operator*= (const BigInteger& second_num) {
    if (second_num.digits_.size() == 1 && second_num.digits_[0] == 0) {
      digits_.clear();
      digits_.push_back(0);
      sign_ = positive;
    }

    if (digits_.size() == 1 && digits_[0] == 0) {
      return *this;
    }

    sign_ = sign_ * second_num.sign_;

    BigInteger copy = BigInteger(*this);
    digits_.resize(second_num.digits_.size() + digits_.size());
    std::fill(digits_.begin(), digits_.end(), 0);

    for (size_t i = 0; i < second_num.digits_.size(); ++i) {
      for (size_t j = 0; j < copy.digits_.size(); ++j) {
        digits_[i + j] += second_num.digits_[i] * copy.digits_[j];
        if (digits_[i + j] >= kBase) {
          long long carry = digits_[i + j] / kBase;
          digits_[i + j] %= kBase;
          if (i + j + 1 != digits_.size()) {
            digits_[i + j + 1] += carry;
          }
        }
      }
    }

    NormalizeDigits();

    return *this;
  }

  BigInteger& operator/= (const BigInteger& second_num) {
    std::vector<long long> res_digits;
    BigInteger current = 0;
    Sign res_sign = sign_ * second_num.sign_;
    BigInteger num{second_num.digits_, positive};

    for (int i = (int)digits_.size() - 1; i >= 0; i--) {
      current *= kBase;
      current += digits_[i];

      int lower = -1;
      int upper = kBase;
      int middle;
      BigInteger possible_res;
      while (lower < upper - 1) {
        middle = (lower + upper) / 2;
        possible_res = num;
        possible_res *= middle;
        if (possible_res <= current) {
          lower = middle;
        } else {
          upper = middle;
        }
      }
      res_digits.push_back(lower);
      possible_res = num;
      possible_res *= lower;
      current -= possible_res;
    }

    std::reverse(res_digits.begin(), res_digits.end());

    digits_ = res_digits;
    sign_ = res_sign;

    NormalizeDigits();
    
    return *this;
  }

  BigInteger& operator%= (const BigInteger& second_num) {
    BigInteger dividable_part = *this;
    dividable_part /= second_num;
    dividable_part *= second_num;
    return *this -= dividable_part;
  }

  BigInteger operator++ (int) {
    BigInteger temp = *this;
    *this += 1;
    return temp;
  }

  BigInteger operator-- (int) {
    BigInteger temp = *this;
    *this -= 1;
    return temp;
  }

  BigInteger& operator++ () {
    return *this += 1;
  }

  BigInteger& operator-- () {
    return *this -= 1;
  }

  BigInteger& operator= (const BigInteger& num)  = default;

  std::string toString() const {
    std::string result = std::to_string(digits_.back());

    for (int i = (int)digits_.size() - 2; i >= 0; --i) {
      std::string digit_str = std::to_string(digits_[i]);
      result += std::string(kBaseLength - digit_str.size(), '0') + digit_str;
    }

    size_t first_non_zero = 0;

    while (result[first_non_zero] == '0' && first_non_zero < result.size() - 1) {
      ++first_non_zero;
    }

    result = result.substr(first_non_zero, result.size());
    return sign_ == negative ? "-" + result : result;
  }

  BigInteger() {}

  BigInteger(int num) {
    sign_ = (num < 0) ? negative : positive;

    if (num == 0) {
      digits_.push_back(0);
    }

    BuildFromUll((unsigned long long)std::abs(num));
  }

  explicit BigInteger(unsigned long long num) {
    BuildFromUll(num);
  }

  BigInteger(const BigInteger& num) = default;

  BigInteger(const std::string& string) {
    int start = 0;

    if (string[0] == '-') {
      start = 1;
      sign_ = negative;
    }

    for (int i = (int)string.size() - 1; i >= start; i -= kBaseLength) {
      int start_index = std::max(i - kBaseLength + 1, start);
      digits_.push_back(std::stoi(string.substr(start_index, i - start_index + 1)));
    }
  }

  explicit operator bool() const {
    return digits_.size() != 1 || digits_[0] != 0;
  }

  friend bool operator== (const BigInteger& first_num, const BigInteger& second_num);
  friend bool operator< (const BigInteger& first_num, const BigInteger& second_num);
  friend bool operator<= (const BigInteger& first_num, const BigInteger& second_num);
};

BigInteger operator+ (const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result += num2;
  return result;
}

BigInteger operator- (const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result -= num2;
  return result;
}

BigInteger operator* (const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result *= num2;
  return result;
}

BigInteger operator/ (const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result /= num2;
  return result;
}

BigInteger operator% (const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result %= num2;
  return result;
}

bool operator== (const BigInteger& first_num, const BigInteger& second_num) {
  return first_num.digits_ == second_num.digits_ && first_num.sign_ == second_num.sign_;
}

bool operator!= (const BigInteger& first_num, const BigInteger& second_num) {
  return !(first_num == second_num);
}

bool operator< (const BigInteger& first_num, const BigInteger& second_num) {
  if ((first_num.sign_ == negative && second_num.sign_ == positive) ||
      (first_num.sign_ == positive && second_num.sign_ == positive && 
       first_num.digits_.size() < second_num.digits_.size()) ||
      (first_num.sign_ == negative && second_num.sign_ == negative &&
       first_num.digits_.size() > second_num.digits_.size())) {
      return true;
  }

  if ((first_num.sign_ == positive && second_num.sign_ == negative) ||
      (first_num.sign_ == positive && second_num.sign_ == positive && 
       first_num.digits_.size() > second_num.digits_.size()) ||
      (first_num.sign_ == negative && second_num.sign_ == negative &&
       first_num.digits_.size() < second_num.digits_.size())) {
      return false;
  }

  for (int i = (int)first_num.digits_.size() - 1; i >= 0; i--) {
    if ((first_num.digits_[i] < second_num.digits_[i] && first_num.sign_ == positive) ||
        (first_num.digits_[i] > second_num.digits_[i] && first_num.sign_ == negative)) {
      return true;
    }

    if ((first_num.digits_[i] < second_num.digits_[i] && first_num.sign_ == negative) ||
        (first_num.digits_[i] > second_num.digits_[i] && first_num.sign_ == positive)) {
      return false;
    }
  }

  return false;
}

bool operator> (const BigInteger& first_num, const BigInteger& second_num) {
  return second_num < first_num;
}

bool operator<= (const BigInteger& first_num, const BigInteger& second_num) {
  return !(second_num < first_num);
}

bool operator>= (const BigInteger& first_num, const BigInteger& second_num) {
  return !(first_num < second_num);
}

BigInteger operator"" _bi(unsigned long long num) {
  return BigInteger{num};
}

BigInteger operator"" _bi(const char* str) {
  return BigInteger{std::string{str}};
}

std::ostream& operator<< (std::ostream& output_stream, const BigInteger& num) {
  output_stream << num.toString();
  return output_stream;
}

std::istream& operator>> (std::istream& input_stream, BigInteger& num) {
  std::string string;
  input_stream >> string;
  num = string;
  return input_stream;
}

BigInteger Gcd(BigInteger num1, BigInteger num2) {
  if (num2 == 0) {
    return num1;
  }
  return Gcd(num2, num1 % num2);
}

class Rational {
 private:
  BigInteger num = 0;
  BigInteger denom = 1;

  void Normalize() {
    BigInteger gcd = Gcd(num, denom);
    num /= gcd;
    denom /= gcd;
    if (denom < 0) {
      num = -num;
      denom = -denom;
    }
  }

 public:
  Rational operator- () const {
    Rational result{-num, denom};
    return result;
  }

  Rational& operator+= (const Rational& second_frac) {
    BigInteger new_num = num * second_frac.denom + denom * second_frac.num;
    BigInteger new_denom = denom * second_frac.denom;

    num = new_num;
    denom = new_denom;
    Normalize();
    return *this;
  }

  Rational& operator-= (const Rational& second_frac) {
    *this += -second_frac;
    return *this;
  }

  Rational& operator*= (const Rational& second_frac) {
    BigInteger new_num = num * second_frac.num;
    BigInteger new_denom = denom * second_frac.denom;

    num = new_num;
    denom = new_denom;
    Normalize();
    return *this;
  }

  Rational& operator/= (const Rational& second_frac) {
    BigInteger new_num = num * second_frac.denom;
    BigInteger new_denom = denom * second_frac.num;

    num = new_num;
    denom = new_denom;
    Normalize();
    return *this;
  }

  Rational& operator= (const Rational& fraction) = default;

  std::string toString() const {
    if (denom == 1) {
      return num.toString();
    }
    return num.toString() + "/" + denom.toString();
  }

  std::string asDecimal(size_t precision = 0) const {
    BigInteger with_ten_power = num;

    for (int i = 0; i < (int)precision; i++) {
      with_ten_power *= 10;
    }

    std::string result = (with_ten_power / denom).toString();
    std::reverse(result.begin(), result.end());

    if (with_ten_power < 0) {
      result.pop_back();
    }
    for (int i = result.size(); i <= (int)precision; i++) {
      result.push_back('0');
    }
    if (with_ten_power < 0) {
      result.push_back('-');
    }
    std::reverse(result.begin(), result.end());
    result.insert(result.end() - precision, '.');
    return result;
  }

  explicit operator double() const {
    return std::stod(asDecimal(kRationalToDoublePrecision));
  }

  Rational() {}

  Rational(int num) : num(num) {}

  Rational(BigInteger num) : num(num) {}

  Rational(int num, int denom) : num(num), denom(denom) {
    Normalize();
  }

  Rational(BigInteger num, BigInteger denom) : num(num), denom(denom) {
    Normalize();
  }

  Rational(const Rational& fraction) = default;

  friend bool operator== (const Rational& first_frac, const Rational& second_frac);
  friend bool operator< (const Rational& first_frac, const Rational& second_frac);
};

Rational operator+ (const Rational& first_frac, const Rational& second_frac) {
  Rational result = first_frac;
  result += second_frac;
  return result;
}

Rational operator- (const Rational& first_frac, const Rational& second_frac) {
  Rational result = first_frac;
  result -= second_frac;
  return result;
}

Rational operator* (const Rational& first_frac, const Rational& second_frac) {
  Rational result = first_frac;
  result *= second_frac;
  return result;
}

Rational operator/ (const Rational& first_frac, const Rational& second_frac) {
  Rational result = first_frac;
  result /= second_frac;
  return result;
}

bool operator== (const Rational& first_frac, const Rational& second_frac) {
  return first_frac.num == second_frac.num && first_frac.denom == second_frac.denom;
}

bool operator!= (const Rational& first_frac, const Rational& second_frac) {
  return !(first_frac == second_frac);
}

bool operator< (const Rational& first_frac, const Rational& second_frac) {
  return first_frac.num * second_frac.denom < first_frac.denom * second_frac.num;
}

bool operator> (const Rational& first_frac, const Rational& second_frac) {
  return second_frac < first_frac;
}

bool operator<= (const Rational& first_frac, const Rational& second_frac) {
  return !(second_frac < first_frac);
}

bool operator>= (const Rational& first_frac, const Rational& second_frac) {
  return !(first_frac < second_frac);
}

std::ostream& operator<< (std::ostream& output_stream, const Rational& num) {
  output_stream << num.toString();
  return output_stream;
}
