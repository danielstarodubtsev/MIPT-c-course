#include <algorithm>
#include <cstring>
#include <iostream>

class String {
 private:
  char* string_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 1;

  void AllocateMemory() {
    char* new_string = new char[capacity_ + 1];
    memcpy(new_string, string_, size_ + 1);
    delete [] string_;
    string_ = new_string;
  }

  String(size_t length) : string_(new char[length + 1]), size_(length), capacity_(length) {}

 public:
  size_t length() const {
    return size_;
  }

  size_t size() const {
    return size_;
  }

  size_t capacity() const {
    return capacity_;
  }

  bool empty() const {
    return size_ == 0;
  }
  
  const char& operator[] (size_t index) const {
    return string_[index];
  }

  char& operator[] (size_t index) {
    return string_[index];
  }

  char* data() {
    return string_;
  }

  const char* data() const {
    return string_;
  }

  const char& front() const {
    return string_[0];
  }

  char& front() {
    return string_[0];
  }

  const char& back() const {
    return string_[size_ - 1];
  }

  char& back() {
    return string_[size_ - 1];
  }

  void clear() {
    size_ = 0;
    string_[0] = '\0';
  }

  void shrink_to_fit() {
    capacity_ = size_;
    AllocateMemory();
  }

  void push_back(const char& symbol) {
    ++size_;

    if (size_ - 1 == capacity_) {
      capacity_ *= 2;
      AllocateMemory();
    }

    string_[size_ - 1] = symbol;
    string_[size_] = '\0';
  }

  void pop_back() {
    --size_;
    string_[size_] = '\0';
  }

  String substr(size_t start, size_t count) const {
    String result(count);
    memcpy(result.string_, string_ + start, count);
    return result;
  }

  size_t find(const String& to_find) const {
    size_t result = std::search(string_, string_ + size_, to_find.string_, 
                                                to_find.string_ + to_find.size_) - string_;

    if (result + 1 == size_) {
      ++result;
    }

    return result;
  }

  size_t rfind(const String& to_find) const {
    size_t result = std::find_end(string_, string_ + size_, to_find.string_, 
                                                to_find.string_ + to_find.size_) - string_;

    if (result + 1 == size_) {
      ++result;
    }

    return result;
  }

  String& operator += (const String& second) {
    if (capacity_ < size_ + second.size_) {
      capacity_ = 2 * (size_ + second.size_) - 1;
      AllocateMemory();
    }

    memcpy(string_ + size_, second.string_, second.size_);
    size_ += second.size_;
    string_[size_] = '\0';
    
    return *this;
  }

  String& operator = (const String& second) {
    if (string_ != second.string_) {
      String result(second);
      std::swap(string_, result.string_);
      std::swap(size_, result.size_);
      std::swap(capacity_, result.capacity_);
    }

    return *this;
  }

  String() : string_(new char[2]) {}
  String(size_t length, char symbol) : String(length) {
    memset(string_, symbol, length);
    string_[size_] = '\0';
  }
  String(char symbol) : String(1, symbol) {}
  String(const char* begin) : String(strlen(begin), 'a') {
    memcpy(string_, begin, size_);
    string_[size_] = '\0';
  }
  String(const String& str) : string_(new char[str.capacity_ + 1]), 
                              size_(str.size_), capacity_(str.capacity_) {
    memcpy(string_, str.string_, size_);
    string_[size_] = '\0';
  }

  ~String() {
    delete[] string_;
  }
};

String operator + (const String& str1, const String& str2) {
  String result = str1;
  result += str2;
  return result;
}

bool operator == (const String& str1, const String& str2) {
  if (str1.size() != str2.size()) {
    return false;
  }

  for (size_t i = 0; i < str1.size(); i++) {
    if (str1[i] != str2[i]) {
      return false;
    }
  }

  return true;
}

bool operator != (const String& str1, const String& str2) {
  return !(str1 == str2);
}

bool operator < (const String& str1, const String& str2) {
  for (size_t i = 0; i < std::min(str1.size(), str2.size()); i++) {
    if (str2[i] > str1[i]) {
      return true;
    }

    if (str2[i] < str1[i]) {
      return false;
    }
  }

  return str1.size() < str2.size();
}

bool operator > (const String& str1, const String& str2) {
  return str2 < str1;
}

bool operator <= (const String& str1, const String& str2) {
  return !(str2 < str1);
}

bool operator >= (const String& str1, const String& str2) {
  return !(str1 < str2);
}

std::ostream& operator << (std::ostream& output_stream, const String& str) {
  output_stream << str.data();
  return output_stream;
}

std::istream& operator >> (std::istream& input_stream, String& str) {
  char current_char;

  while (input_stream.get(current_char)) {
    if (isspace(current_char)) {
      break;
    }

    str.push_back(current_char);
  }

  return input_stream;
}
