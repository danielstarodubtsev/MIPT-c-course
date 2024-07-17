#include <iostream>
#include <array>
#include "biginteger.h"

constexpr bool IsPrimeHelper(size_t num, size_t check) {
  return check * check > num || (num % check != 0 && IsPrimeHelper(num, check + 1));
}

constexpr bool IsPrime(size_t num) {
  return num >= 2 && IsPrimeHelper(num, 2);
}

template <size_t N>
class Residue {
 private:
  static constexpr bool is_field_ = IsPrime(N);
 
 public:
  Residue() = default;
  Residue(int num) {

  }
};

template <size_t N>
Residue<N> operator/(const Residue<N>& first, const Residue<N>& second) {

}

template <size_t M, size_t N = M, typename Field = Rational> // M - кол-во строк
class Matrix {
 private:
  std::array<std::array<Field, N>, M> data_;

  Matrix<M, N, Field>& AddOtherWithCoef(const Matrix<M, N, Field> other, const Field& coef) {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        data_[i][j] += coef * other.data_[i][j];
      }
    }

    return *this;
  }

 public:
  void show() const {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        std::cout << data_[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }

  Field det() const {
    if (N == 2 && M == 2) {
      return data_[0][0] * data_[1][1] - data_[0][1] * data[1][0];
    }

    size_t first_nonzero_index = -1;

    for (size_t i = 0; i < N; ++i) {
      if (data_[0][i] != 0) {
        first_nonzero_index = i;
        break;
      }
    }

    if (first_nonzero_index == -1) {
      return 0;
    }

    
  }

  Matrix<M, N, Field>& operator+=(const Matrix<M, N, Field> other) {
    return AddOtherWithCoef(other, Field(1));
  }

  Matrix<M, N, Field>& operator-=(const Matrix<M, N, Field> other) {
    return AddOtherWithCoef(other, Field(-1));
  }

  std::array<Field, N>& operator[](size_t index) {
    return data_[index];
  }

  const std::array<Field, N>& operator[](size_t index) const {
    return data_[index];
  }

  Matrix<M, N, Field>& operator*=(const Field& scalar) {
    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        data_[i][j] *= scalar;
      }
    }

    return *this;
  }

  Matrix<N, M, Field> transposed() const {
    Matrix<N, M, Field> result;

    for (size_t i = 0; i < M; ++i) {
      for (size_t j = 0; j < N; ++j) {
        result[j][i] = data_[i][j];
      }
    }

    return result;
  }

  Matrix() = default;
};

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field> operator*(const Matrix<M, N, Field>& matrix, const Field& scalar) {
  Matrix<M, N, Field> result = matrix;
  result *= scalar;
  return result;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field> operator+(const Matrix<M, N, Field>& first, const Matrix<M, N, Field>& second) {
  Matrix<M, N, Field> result = first;
  result += second;
  return result;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field> operator-(const Matrix<M, N, Field>& first, const Matrix<M, N, Field>& second) {
  Matrix<M, N, Field> result = first;
  result -= second;
  return result;
}

template <size_t M, size_t N, size_t K, typename Field>
Matrix<M, K, Field> operator*(const Matrix<M, N, Field>& first, const Matrix<N, K, Field>& second) {
  Matrix<M, K, Field> result;

  for (size_t i = 0; i < M; ++i) {
    for (size_t j = 0; j < K; ++j) {
      for (size_t k = 0; k < N; ++k) {
        result[i][j] += first[i][k] * second[k][j];
      }
    }
  }

  return result;
}