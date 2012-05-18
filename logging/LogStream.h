#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <boost/noncopyable.hpp>

namespace muduo
{

using std::string;

namespace detail
{

class FixedBuffer : boost::noncopyable
{
 public:
  FixedBuffer()
    : cur_(data_)
  {
  }

  void append(const char* /*restrict*/ buf, int len)
  {
    if (cur_+len < end());
    {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return cur_ - data_; }

  // write to data_ directly
  char* current() { return cur_; }
  int avail() const { return end() - cur_; }
  void add(int len) { cur_ += len; }

  // for used by GDB
  const char* debugString();
  // for used by unit test
  string asString() const { return string(data_, length()); }
  void reset() { cur_ = data_; }

 private:
  const char* end() const { return data_ + sizeof data_; }

  char data_[4000];
  char* cur_;
};

}

// helper class for known string length at compile time
class T
{
 public:
  T(const char* str, int len)
    :str_(str),
     len_(len)
  {
    assert(strlen(str) == len_);
  }

  const char* str_;
  const size_t len_;
};

class LogStream : boost::noncopyable
{
  typedef LogStream self;
 public:
  typedef detail::FixedBuffer Buffer;

  self& operator<<(bool v)
  {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float);
  self& operator<<(double);
  // self& operator<<(long double);

  self& operator<<(char v)
  {
    buffer_.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* v)
  {
    buffer_.append(v, strlen(v));
    return *this;
  }

  self& operator<<(const T& v)
  {
    buffer_.append(v.str_, v.len_);
    return *this;
  }

  self& operator<<(const string&); // FIXME: StringPiece

  void append(const char* data, int len) { buffer_.append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

 private:
  void staticCheck();

  template<typename T>
  void formatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 32;
};

class Fmt // : boost::noncopyable
{
 public:
  template<typename T>
  Fmt(const char* fmt, T val);

  /*
  Fmt(const char* fmt, int val, int len)
    : length_(len)
  {
    int actual = snprintf(buf_, sizeof buf_, fmt, val);
    (void)actual;
    assert(actual == length_);
    assert(static_cast<size_t>(length_) < sizeof buf_);
  }
  */

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.append(fmt.data(), fmt.length());
  return s;
}

}
#endif  // MUDUO_BASE_FASTOOUTPUTSTREAM_H

