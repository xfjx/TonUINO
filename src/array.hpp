#ifndef SRC_ARRAY_HPP_
#define SRC_ARRAY_HPP_

template <class T, size_t N>
struct array {
  // Storage
  T data[N];

  static constexpr size_t length() { return N; }
  using type = T;

  // Item access
  T &operator[](size_t index) { return data[index]; }
  const T &operator[](size_t index) const { return data[index]; }

  // Iterators
  T *begin() { return &data[0]; }
  const T *begin() const { return &data[0]; }
  T *end() { return &data[N]; }
  const T *end() const { return &data[N]; }

  // Comparisons
  bool operator==(const array<T, N> &rhs) const {
    if (this == &rhs)
      return true;
    for (size_t i = 0; i < N; i++)
      if ((*this)[i] != rhs[i])
        return false;
    return true;
  }
  bool operator!=(const array<T, N> &rhs) const {
    return !(*this == rhs);
  }
  array& operator=(const array &rhs) {
    if (this != &rhs)
      for (size_t i = 0; i < N; i++)
        (*this)[i] = rhs[i];
    return *this;
  }
};





#endif /* SRC_ARRAY_HPP_ */
